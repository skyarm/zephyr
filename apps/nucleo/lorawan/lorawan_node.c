/**
 ******************************************************************************
 *
 *          Portions COPYRIGHT 2020 STMicroelectronics
 *
 * @file    LmHandler.c
 * @author  MCD Application Team
 * @brief   LoRaMAC Layer handling definition
 ******************************************************************************
 */
#include <logging/log.h>
#include "secure-element.h"

#include "lorawan_node.h"

#include <LoRaMac.h>
#include <LoRaMacClassB.h>

#include "Region.h"
#include "crypto_config.h"

#include "LoRaMacTest.h"

static const struct lorawan_node_config *lorawan_node_config = NULL;

struct lorawan_node_runtime {
	bool duty_cycle_enabled;        /* ths parameter defined in region files */
	uint32_t duty_cycle_time;       /* the next duty cycle time */
	uint8_t ping_periodicity;
	uint32_t device_address;
	uint8_t device_eui[8];
	uint8_t join_eui[8];
	uint32_t supported_regions;
	bool ctx_restore_done;
#if (LORAMAC_CLASSB_ENABLED == 1)
	/*Indicates if a switch to Class B operation is pending or not. */
	bool classb_pending;
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
};

static struct lorawan_node_runtime lorawan_node_runtime = {
	.duty_cycle_enabled = false,    /* will be set in lorawan_node_init as region settings */
	.duty_cycle_time = UINT32_MAX,
	.ping_periodicity = 7,          /* to max ping periodicity */
	.device_address = 0,
	.device_eui = { 0 },
	.join_eui = { 0 },
	.supported_regions = 0,
	.ctx_restore_done = false,
#if (LORAMAC_CLASSB_ENABLED == 1)
	.classb_pending = false,
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
};

struct lorawan_node_mac_config {
	/* Used to notify node of LoRaMac events */
	LoRaMacPrimitives_t primitives;
	/* LoRaMac callbacks */
	LoRaMacCallback_t callbacks;
};

static struct lorawan_node_mac_config lorawan_node_mac_config = { 0 };

#if (LORAMAC_CLASSB_ENABLED == 1)
static enum lorawan_node_status lorawan_node_beacon_req(void)
{
	MlmeReq_t mlme_req;

	mlme_req.Type = MLME_BEACON_ACQUISITION;
	return (enum lorawan_node_status)LoRaMacMlmeRequest(&mlme_req);
}

static enum lorawan_node_status lorawan_node_ping_slot_req(uint8_t periodicity)
{
	LoRaMacStatus_t status;
	MlmeReq_t mlme_req;

	mlme_req.Type = MLME_PING_SLOT_INFO;
	mlme_req.Req.PingSlotInfo.PingSlot.Fields.Periodicity = periodicity;
	mlme_req.Req.PingSlotInfo.PingSlot.Fields.RFU = 0;
	status = LoRaMacMlmeRequest(&mlme_req);
	if (status == LORAMAC_STATUS_OK) {
		lorawan_node_runtime.ping_periodicity = periodicity;
		return lorawan_node_send(0, NULL, 0, false, false);
	} else {
		return (enum lorawan_node_status)status;
	}
}
#endif /* LORAMAC_CLASSB_ENABLED == 1 */

static void lorawan_node_mcps_confirm(McpsConfirm_t *mcps_confirm)
{
	struct lorawan_node_cb_data_sent_params cb_params;

	cb_params.is_mcps_confirm = true;
	cb_params.status = (enum lorawan_node_event_status)mcps_confirm->Status;
	cb_params.data_rate = mcps_confirm->Datarate;
	cb_params.uplink_counter = mcps_confirm->UpLinkCounter;
	cb_params.tx_power = mcps_confirm->TxPower;
	cb_params.channel = mcps_confirm->Channel;
	cb_params.ack_received = mcps_confirm->AckReceived;

	if (lorawan_node_config->callbacks.data_sent) {
		lorawan_node_config->callbacks.data_sent(&cb_params);
	}
}

static void lorawan_node_mcps_indication(McpsIndication_t *mcps_indication)
{
	DeviceClass_t device_class;
	struct lorawan_node_cb_data_received_params cb_params;

	cb_params.is_mcps_indication = true;
	cb_params.status = (enum lorawan_node_event_status)mcps_indication->Status;

	if (cb_params.status != LORAWAN_NODE_EVENT_STATUS_OK) {
		return;
	}

	if (mcps_indication->BufferSize > 0) {
		cb_params.data_rate = mcps_indication->RxDatarate;
		cb_params.rssi = mcps_indication->Rssi;
		cb_params.snr = mcps_indication->Snr;
		cb_params.downlink_counter = mcps_indication->DownLinkCounter;
		cb_params.rx_slot = mcps_indication->RxSlot;

		if (lorawan_node_config->callbacks.data_received) {
			lorawan_node_config->callbacks.data_received(mcps_indication->Port,
								     mcps_indication->Buffer, mcps_indication->BufferSize, &cb_params);
		}
	}

	device_class = lorawan_node_get_current_class();
	if ((mcps_indication->FramePending == true) && (device_class == CLASS_A)) {
		/**
		 * *The server signals that it has pending data to be sent.
		 * We schedule an uplink as soon as possible to flush the server.
		 * Send an empty message
		 */
		lorawan_node_send(0, NULL, 0, false, true);
	}
}

static void lorawan_node_mlme_confirm(MlmeConfirm_t *mlme_confirm)
{
	switch (mlme_confirm->MlmeRequest) {
	case MLME_JOIN:
	{
		struct lorawan_node_cb_join_request_params cb_params;
		cb_params.mode = LORAWAN_NODE_ACTIVATION_OTAA;

		MibRequestConfirm_t mib_req;
		mib_req.Type = MIB_DEV_ADDR;
		LoRaMacMibGetRequestConfirm(&mib_req);
		lorawan_node_runtime.device_address = mib_req.Param.DevAddr;
		lorawan_node_get_tx_data_rate(&cb_params.data_rate);
		cb_params.status = (enum lorawan_node_event_status)mlme_confirm->Status;
		/* Notify upper layer */
		if (lorawan_node_config->callbacks.join_request) {
			lorawan_node_config->callbacks.join_request(&cb_params);
		}
	}
	break;
	case MLME_LINK_CHECK:
	{
		/* Check DemodMargin */
		/* Check NbGateways */
	}
	break;
	case MLME_DEVICE_TIME:
	{
#if (LORAMAC_CLASSB_ENABLED == 1)
		if (lorawan_node_runtime.classb_pending == true) {
			lorawan_node_beacon_req();
		} else {
			SysTime_t systime = SysTimeGet();
			if (lorawan_node_config->callbacks.device_time) {
				lorawan_node_config->callbacks.device_time(systime.Seconds, systime.SubSeconds);
			}
		}
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
	}
	break;
#if (LORAMAC_CLASSB_ENABLED == 1)
	case MLME_BEACON_ACQUISITION:
	{
		if (mlme_confirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
			/* Beacon has been acquired */
			/* Request server for ping slot */
			lorawan_node_ping_slot_req(lorawan_node_runtime.ping_periodicity);
		} else {
			/* Beacon not acquired, Request Device Time again. */
			lorawan_node_device_time_req();
		}
	}
	break;
	case MLME_PING_SLOT_INFO:
	{
		if (mlme_confirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
			MibRequestConfirm_t mibReq;

			/* Class B is now activated */
			mibReq.Type = MIB_DEVICE_CLASS;
			mibReq.Param.Class = CLASS_B;
			LoRaMacMibSetRequestConfirm(&mibReq);

			if (lorawan_node_config->callbacks.class_changed) {
				lorawan_node_config->callbacks.class_changed(CLASS_B);
			}

			lorawan_node_runtime.classb_pending = false;
		} else {
			lorawan_node_ping_slot_req(lorawan_node_runtime.ping_periodicity);
		}
	}
	break;
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
	default:
		break;
	}
}

static void lorawan_node_mlme_indication(MlmeIndication_t *mlme_indication)
{
	struct lorawan_node_cb_beacon_status_params cb_params;

	switch (mlme_indication->MlmeIndication) {
#if (LORAMAC_CLASSB_ENABLED == 1)
	case MLME_BEACON_LOST:
	{
		MibRequestConfirm_t mib_req;
		/* Switch to class A again */
		mib_req.Type = MIB_DEVICE_CLASS;
		mib_req.Param.Class = CLASS_A;
		LoRaMacMibSetRequestConfirm(&mib_req);

		cb_params.status = LORAWAN_NODE_EVENT_STATUS_BEACON_LOST;
		cb_params.time = 0;
		cb_params.info_desc = 0;
		memset(cb_params.info_data, 0, 6);
		if (lorawan_node_config->callbacks.beacon_status) {
			lorawan_node_config->callbacks.beacon_status(&cb_params);
		}
		if (lorawan_node_config->callbacks.class_changed) {
			lorawan_node_config->callbacks.class_changed(CLASS_A);
		}

		lorawan_node_device_time_req();
	}
	break;
	case MLME_BEACON:
	{
		cb_params.status = (enum lorawan_node_event_status)mlme_indication->Status;
		cb_params.time = mlme_indication->BeaconInfo.Time.Seconds;
		cb_params.freq = mlme_indication->BeaconInfo.Frequency;
		cb_params.rssi = mlme_indication->BeaconInfo.Rssi;
		cb_params.snr = mlme_indication->BeaconInfo.Snr;
		cb_params.info_desc = mlme_indication->BeaconInfo.GwSpecific.InfoDesc;
		memcpy(cb_params.info_data, mlme_indication->BeaconInfo.GwSpecific.Info,
		       sizeof(cb_params.info_data));
		if (lorawan_node_config->callbacks.beacon_status) {
			lorawan_node_config->callbacks.beacon_status(&cb_params);
		}
		break;
	}
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
	default:
		break;
	}
}

static void lorawan_node_nvm_ctx_event(LoRaMacNvmCtxModule_t module)
{
}

static bool lorawan_node_nvm_ctx_store(void)
{
	return false;
}

static bool lorawan_node_nvm_ctx_restore(void)
{
	return false;
}

enum lorawan_node_status lorawan_node_init(const struct lorawan_node_config *config)
{
	if (lorawan_node_config) {
		/**
		 * The lorawan node is already initialed
		 * But when the device reset and the SRAM keep the data,
		 * Can calc a checksum of config data and need not initial
		 * the device again?
		 */
		assert(0);
		return LORAWAN_NODE_STATUS_ERROR;
	}

	/**
	 * initial lora_node_mac_config
	 */
	lorawan_node_mac_config.primitives.MacMcpsConfirm = lorawan_node_mcps_confirm;
	lorawan_node_mac_config.primitives.MacMcpsIndication = lorawan_node_mcps_indication;
	lorawan_node_mac_config.primitives.MacMlmeConfirm = lorawan_node_mlme_confirm;
	lorawan_node_mac_config.primitives.MacMlmeIndication = lorawan_node_mlme_indication;
	lorawan_node_mac_config.callbacks.NvmContextChange = lorawan_node_nvm_ctx_event;
	lorawan_node_mac_config.callbacks.GetBatteryLevel = config->callbacks.get_battery_level;
	lorawan_node_mac_config.callbacks.GetTemperatureLevel = config->callbacks.get_temperature;
	lorawan_node_mac_config.callbacks.MacProcessNotify = config->callbacks.mac_process;

	/**
	 * Initial runtime data
	 */
#if (STATIC_DEVICE_ADDRESS == 1)
	lorawan_node_runtime.device_address = LORAWAN_DEVICE_ADDRESS;
#else
	lorawan_node_runtime.device_address = GetDevAddr();
#endif /* STATIC_DEVICE_ADDRESS != 1 */

	lorawan_node_runtime.duty_cycle_time = k_uptime_get_32();

	assert(config->ping_periodicity <= 7); /* ping periodicity must be 0 to 7 */
	lorawan_node_runtime.ping_periodicity = config->ping_periodicity;

#if (LORAMAC_CLASSB_ENABLED == 1)
	lorawan_node_runtime.classb_pending = false;
#endif /* LORAMAC_CLASSB_ENABLED == 1 */
	lorawan_node_runtime.ctx_restore_done = false;

#ifdef  REGION_AS923
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_AS923);
#endif /* REGION_AS923 */
#ifdef  REGION_AU915
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_AU915);
#endif /* REGION_AU915 */
#ifdef  REGION_CN470
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_CN470);
#endif /* REGION_CN470 */
#ifdef  REGION_CN779
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_CN779);
#endif /* REGION_CN779 */
#ifdef  REGION_EU433
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_EU433);
#endif /* REGION_EU433 */
#ifdef  REGION_EU868
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_EU868);
#endif /* REGION_EU868 */
#ifdef  REGION_KR920
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_KR920);
#endif /* REGION_KR920 */
#ifdef  REGION_IN865
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_IN865);
#endif /* REGION_IN865 */
#ifdef  REGION_US915
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_US915);
#endif /* REGION_US915 */
#ifdef  REGION_RU864
	lorawan_node_runtime.supported_regions |= (1 << LORAMAC_REGION_RU864);
#endif /* REGION_RU864 */

	if (lorawan_node_runtime.supported_regions == 0) {
		assert(0); /* At least one region shall be defined */
		return LORAWAN_NODE_STATUS_ERROR;
	}

	/**
	 * Initial LoraMac
	 */
	LoRaMacStatus_t status;
	if (0U != ((1 << (config->active_region)) & lorawan_node_runtime.supported_regions)) {
		status = LoRaMacInitialization(&lorawan_node_mac_config.primitives,
					       &lorawan_node_mac_config.callbacks, config->active_region);
		if (status != LORAMAC_STATUS_OK) {
			return (enum lorawan_node_status)status;
		}
	} else {
		assert(0);
		/* Region is not defined in the MW: set lorawan_conf.h accordingly */
		return LORAWAN_NODE_STATUS_ERROR;
	}

	/* Try to restore from NVM and query the mac if possible. */
	MibRequestConfirm_t mib_req;
	if (lorawan_node_nvm_ctx_restore()) {
		lorawan_node_runtime.ctx_restore_done = true;
	} else {
		lorawan_node_runtime.ctx_restore_done = false;
		/* Read secure-element DEV_EUI and JOIN_EUI values. */
		mib_req.Type = MIB_DEV_EUI;
		status = LoRaMacMibGetRequestConfirm(&mib_req);
		assert(status == LORAMAC_STATUS_OK);
		memcpy(lorawan_node_runtime.device_eui, mib_req.Param.DevEui, 8);

		mib_req.Type = MIB_JOIN_EUI;
		status = LoRaMacMibGetRequestConfirm(&mib_req);
		assert(status == LORAMAC_STATUS_OK);
		memcpy(lorawan_node_runtime.join_eui, mib_req.Param.JoinEui, 8);
	}

	mib_req.Type = MIB_PUBLIC_NETWORK;
	mib_req.Param.EnablePublicNetwork = config->public_network;
	status = LoRaMacMibSetRequestConfirm(&mib_req);
	assert(status == LORAMAC_STATUS_OK);

	mib_req.Type = MIB_REPEATER_SUPPORT;
	mib_req.Param.EnableRepeaterSupport = config->repeater_supported;
	status = LoRaMacMibSetRequestConfirm(&mib_req);
	assert(status == LORAMAC_STATUS_OK);

	mib_req.Type = MIB_ADR;
	mib_req.Param.AdrEnable = config->adr_enabled;
	status = LoRaMacMibSetRequestConfirm(&mib_req);
	assert(status == LORAMAC_STATUS_OK);

	mib_req.Type = MIB_SYSTEM_MAX_RX_ERROR;
	mib_req.Param.SystemMaxRxError = 20;
	status = LoRaMacMibSetRequestConfirm(&mib_req);
	assert(status == LORAMAC_STATUS_OK);

	GetPhyParams_t get_phy;
	PhyParam_t phy_param;
	get_phy.Attribute = PHY_DUTY_CYCLE;
	phy_param = RegionGetPhyParam(config->active_region, &get_phy);
	lorawan_node_runtime.duty_cycle_enabled = (bool) phy_param.Value;

	/* override previous value if reconfigure new region */
	LoRaMacTestSetDutyCycleOn(lorawan_node_runtime.duty_cycle_enabled);

	/**
	 * Initial the lorawan_node_config at last of function, thus can
	 * get if the device is initialed when in multithread environment
	 */
	lorawan_node_config = config;

	return LORAWAN_NODE_STATUS_OK;
}

enum lorawan_node_status lorawan_node_join(enum lorawan_node_activation_mode mode)
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mib_req;

	assert(lorawan_node_config);

	if (mode == LORAWAN_NODE_ACTIVATION_OTAA) {
		MlmeReq_t mlme_req;

		status = LoRaMacStart();
		if (status != LORAMAC_STATUS_OK) {
			return (enum lorawan_node_status)status;
		}
		/* Starts the OTAA join procedure */
		mlme_req.Type = MLME_JOIN;
		mlme_req.Req.Join.Datarate = lorawan_node_config->tx_data_rate;
		status = LoRaMacMlmeRequest(&mlme_req);
		assert(status == LORAMAC_STATUS_OK);

		return LORAWAN_NODE_STATUS_OK;

	} else {
		if (lorawan_node_runtime.ctx_restore_done == false) {
			/* Tell the MAC layer which network server version are we connecting too. */
			mib_req.Type = MIB_ABP_LORAWAN_VERSION;
			mib_req.Param.AbpLrWanVersion.Value = LORAWAN_NODE_ABP_VERSION;
			LoRaMacMibSetRequestConfirm(&mib_req);

			mib_req.Type = MIB_NET_ID;
			mib_req.Param.NetID = lorawan_node_config->network_id;
			status = LoRaMacMibSetRequestConfirm(&mib_req);
			assert(status == LORAMAC_STATUS_OK);

			mib_req.Type = MIB_DEV_ADDR;
			mib_req.Param.DevAddr = lorawan_node_runtime.device_address;
			status = LoRaMacMibSetRequestConfirm(&mib_req);
			assert(status == LORAMAC_STATUS_OK);
		}

		status = LoRaMacStart();
		if (status != LORAMAC_STATUS_OK) {
			return (enum lorawan_node_status)status;
		}

		mib_req.Type = MIB_NETWORK_ACTIVATION;
		mib_req.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
		status = LoRaMacMibSetRequestConfirm(&mib_req);
		assert(status == LORAMAC_STATUS_OK);

		struct lorawan_node_cb_join_request_params join_params;
		join_params.mode = ACTIVATION_TYPE_ABP;
		join_params.status = LORAMAC_EVENT_INFO_STATUS_OK;
		join_params.data_rate = LORAWAN_NODE_DR_0;

		if (lorawan_node_config->callbacks.join_request) {
			lorawan_node_config->callbacks.join_request(&join_params);
		}
		return LORAWAN_NODE_STATUS_OK;
	}
}

enum lorawan_node_status lorawan_node_send(uint8_t port, const void *data, uint8_t size,
					   bool tx_confirmed, bool allow_delayed_tx)
{
	LoRaMacStatus_t status;
	McpsReq_t mcps_req;
	LoRaMacTxInfo_t tx_info;

	assert(lorawan_node_config);

	if (lorawan_node_is_joined() == false) {
		return LORAWAN_NODE_STATUS_NO_NETWORK_JOINED;
	}

	if (LoRaMacIsBusy()) {
		return LORAWAN_NODE_STATUS_BUSY;
	}

	if (k_uptime_get_32() < lorawan_node_runtime.duty_cycle_time) {
		return LORAWAN_NODE_STATUS_DUTYCYCLE_RESTRICTED;
	}

	mcps_req.Req.Unconfirmed.Datarate = lorawan_node_config->tx_data_rate;
	if (LoRaMacQueryTxPossible(size, &tx_info) != LORAMAC_STATUS_OK) {
		/* Send empty frame in order to flush MAC commands */
		mcps_req.Type = MCPS_UNCONFIRMED;
		mcps_req.Req.Unconfirmed.fBuffer = NULL;
		mcps_req.Req.Unconfirmed.fBufferSize = 0;
		LoRaMacMcpsRequest(&mcps_req, allow_delayed_tx);
		lorawan_node_runtime.duty_cycle_time = k_uptime_get_32();
		lorawan_node_runtime.duty_cycle_time += mcps_req.ReqReturn.DutyCycleWaitTime;
		return LORAWAN_NODE_STATUS_SKIPPED_APP_DATA;
	} else {
		mcps_req.Req.Unconfirmed.fPort = port;
		mcps_req.Req.Unconfirmed.fBufferSize = size;
		mcps_req.Req.Unconfirmed.fBuffer = (void *)data;
		if (tx_confirmed == false) {
			mcps_req.Type = MCPS_UNCONFIRMED;
		} else {
			mcps_req.Type = MCPS_CONFIRMED;
			mcps_req.Req.Confirmed.NbTrials = 8;
		}
		status = LoRaMacMcpsRequest(&mcps_req, allow_delayed_tx);
		lorawan_node_runtime.duty_cycle_time = k_uptime_get_32();
		lorawan_node_runtime.duty_cycle_time += mcps_req.ReqReturn.DutyCycleWaitTime;
		return (enum lorawan_node_status)status;
	}
}

enum lorawan_node_status lorawan_node_request_class(enum lorawan_node_class new_class)
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mib_req;
	enum lorawan_node_class current_lass;

	assert(lorawan_node_config);

	if (lorawan_node_is_joined() == false) {
		return LORAWAN_NODE_STATUS_NO_NETWORK_JOINED;
	}

	mib_req.Type = MIB_DEVICE_CLASS;
	status = LoRaMacMibGetRequestConfirm(&mib_req);
	if (status != LORAMAC_STATUS_OK) {
		return (enum lorawan_node_status)status;
	}
	current_lass = (enum lorawan_node_class)mib_req.Param.Class;

	/* Attempt to switch only if class update */
	if (current_lass != new_class) {
		switch (new_class) {
		case LORAWAN_NODE_CLASS_A:
		{
			if (current_lass != LORAWAN_NODE_CLASS_A) {
				mib_req.Param.Class = CLASS_A;
				status = LoRaMacMibSetRequestConfirm(&mib_req);
				if (status == LORAMAC_STATUS_OK) {
					/* Switch is instantaneous */
					if (lorawan_node_config->callbacks.class_changed) {
						lorawan_node_config->callbacks.class_changed(LORAWAN_NODE_CLASS_A);
					}
					return LORAWAN_NODE_STATUS_OK;
				} else {
					return (enum lorawan_node_status)status;
				}
			}
		}
		break;
		case LORAWAN_NODE_CLASS_B:
		{
#if (LORAMAC_CLASSB_ENABLED == 1)
			if (current_lass != LORAWAN_NODE_CLASS_A) {
				return LORAWAN_NODE_STATUS_PARAMETER_INVALID;
			} else {
				/* Beacon must first be acquired */
				lorawan_node_runtime.classb_pending = true;
				return lorawan_node_device_time_req();
			}
#else /* LORAMAC_CLASSB_ENABLED == 0 */
			return LORAWAN_NODE_STATUS_PARAMETER_INVALID;
#endif /* LORAMAC_CLASSB_ENABLED */
		}
		break;
		case LORAWAN_NODE_CLASS_C:
		{
			if (current_lass != LORAWAN_NODE_CLASS_A) {
				return LORAWAN_NODE_STATUS_PARAMETER_INVALID;
			} else {
				/* Switch is instantaneous */
				mib_req.Param.Class = CLASS_C;
				status = LoRaMacMibSetRequestConfirm(&mib_req);
				if (status == LORAMAC_STATUS_OK) {
					if (lorawan_node_config->callbacks.class_changed) {
						lorawan_node_config->callbacks.class_changed(CLASS_C);
					}
					return LORAWAN_NODE_STATUS_OK;
				} else {
					return (enum lorawan_node_status)status;
				}
			}
		}
		break;
		default:
			return LORAWAN_NODE_STATUS_PARAMETER_INVALID;
		}
	}
	return LORAWAN_NODE_STATUS_PARAMETER_INVALID;
}

void lorawan_node_process(void)
{
	assert(lorawan_node_config);
	LoRaMacProcess();
	lorawan_node_nvm_ctx_store();
}

enum lorawan_node_status lorawan_node_device_time_req(void)
{
	LoRaMacStatus_t status;
	MlmeReq_t mlme_req;

	mlme_req.Type = MLME_DEVICE_TIME;
	status = LoRaMacMlmeRequest(&mlme_req);
	if (status == LORAMAC_STATUS_OK) {
		return lorawan_node_send(0, NULL, 0, false, false);
	} else {
		return (enum lorawan_node_status)status;
	}
}

enum lorawan_node_status lorawan_node_stop(void)
{
	assert(lorawan_node_config);

	LoRaMacStatus_t status = LoRaMacDeInitialization();

	if (status == LORAMAC_STATUS_OK) {
		lorawan_node_config = NULL;
	}
	return (enum lorawan_node_status)status;
}

bool lorawan_node_is_busy(void)
{
	assert(lorawan_node_config);

	if (lorawan_node_is_joined() == false) {
		return true;
	}

	if (LoRaMacIsBusy()) {
		return true;
	}

	return false;
}

bool lorawan_node_is_joined()
{
	MibRequestConfirm_t mibReq;
	LoRaMacStatus_t status;

	assert(lorawan_node_config);

	mibReq.Type = MIB_NETWORK_ACTIVATION;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);
	if (mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE) {
		return false;
	} else {
		return true;
	}
}

enum lorawan_node_class lorawan_node_get_current_class()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_DEVICE_CLASS;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return (enum lorawan_node_class)mibReq.Param.Class;
}

uint32_t lorawan_node_get_current_time(uint16_t *subseconds)
{
	SysTime_t systime = SysTimeGet();

	if (subseconds) {
		*subseconds = systime.SubSeconds;
	}
	return systime.Seconds;
}

bool lorawan_node_classb_pending()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.classb_pending;
}

bool lorawan_node_is_initialed()
{
	return lorawan_node_config != NULL;
}

uint32_t lorawan_node_get_duty_cycle_time()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.duty_cycle_time;
}

uint32_t lorawan_node_get_supported_regions()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.supported_regions;
}

uint8_t lorawan_node_get_ping_periodicity()
{
	assert(lorawan_node_config);
#if (LORAMAC_CLASSB_ENABLED == 1)
	return lorawan_node_runtime.ping_periodicity;
#else /* LORAMAC_CLASSB_ENABLED == 0 */
	return (uint8_t)-1;
#endif /* LORAMAC_CLASSB_ENABLED */
}

bool lorawan_node_set_ping_periodicity(uint8_t periodicity)
{
	assert(lorawan_node_config);
#if (LORAMAC_CLASSB_ENABLED == 1)
	enum lorawan_node_class current_class = lorawan_node_get_current_class();
	if (lorawan_node_is_joined() == false) {
		lorawan_node_runtime.ping_periodicity = periodicity;
		return true;
	}
	if (lorawan_node_is_joined() && current_class == LORAWAN_NODE_CLASS_A) {
		lorawan_node_runtime.ping_periodicity = periodicity;
		return true;
	} else {
		return false;
	}
#else /* LORAMAC_CLASSB_ENABLED == 0 */
	return false;
#endif /* LORAMAC_CLASSB_ENABLED */
}

enum lorawan_node_beacon_state lorawan_node_get_beacon_state()
{
	assert(lorawan_node_config);
#if (LORAMAC_CLASSB_ENABLED == 1)
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;
	LoRaMacClassBNvmCtx_t *CtxClassB;

	mibReq.Type = MIB_NVM_CTXS;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	CtxClassB = (LoRaMacClassBNvmCtx_t *) mibReq.Param.Contexts->ClassBNvmCtx;
	assert(CtxClassB != NULL);

	return (enum lorawan_node_beacon_state)CtxClassB->BeaconCtx.BeaconState;
#else /* LORAMAC_CLASSB_ENABLED == 0 */
	return LORAWAN_NODE_BEACON_UNKNOWN;
#endif /* LORAMAC_CLASSB_ENABLED */
}

uint8_t lorawan_node_get_tx_data_rate()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibGet;

	assert(lorawan_node_config);

	mibGet.Type = MIB_CHANNELS_DATARATE;
	status = LoRaMacMibGetRequestConfirm(&mibGet);
	assert(status == LORAMAC_STATUS_OK);

	return mibGet.Param.ChannelsDatarate;
}

bool lorawan_node_set_tx_data_rate(int8_t txDatarate)
{
	assert(lorawan_node_config);

	if (lorawan_node_config->adr_enabled == true) {
		return false;
	}

	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	mibReq.Type = MIB_CHANNELS_DATARATE;
	mibReq.Param.ChannelsDatarate = txDatarate;
	status = LoRaMacMibSetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return true;
}

enum lorawan_node_region lorawan_node_get_active_region()
{
	assert(lorawan_node_config);
	return lorawan_node_config->active_region;
}

const uint8_t *lorawan_node_get_dev_eui()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.device_eui;
}

const uint8_t *lorawan_node_get_app_eui()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.join_eui;
}

uint32_t lorawan_node_get_network_id()
{
	assert(lorawan_node_config);
	return lorawan_node_config->network_id;
}

uint32_t lorawan_node_get_dev_addr()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.device_address;
}

bool lorawan_node_set_dev_addr(uint32_t addr)
{
	assert(lorawan_node_config);
#if (STATIC_DEVICE_ADDRESS != 1)
	lorawan_node_runtime.device_address = addr;
	return true;
#else /* STATIC_DEVICE_ADDRESS == 1 */
	return false;
#endif /* STATIC_DEVICE_ADDRESS */
}

bool lorawan_node_is_adr_enabled()
{
	assert(lorawan_node_config);
	return lorawan_node_config->adr_enabled;
}

void lorawan_node_enable_adr(bool enabled)
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_ADR;
	mibReq.Param.AdrEnable = enabled;

	status = LoRaMacMibSetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);
}

bool lorawan_node_is_duty_cycle_enabled()
{
	assert(lorawan_node_config);
	return lorawan_node_runtime.duty_cycle_enabled;
}

void lorawan_node_enable_duty_cycle(bool dutyCycleEnable)
{
	assert(lorawan_node_config);

	LoRaMacTestSetDutyCycleOn(dutyCycleEnable);
	lorawan_node_runtime.duty_cycle_enabled = dutyCycleEnable;
}

uint8_t lorawan_node_get_tx_power()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_CHANNELS_TX_POWER;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.ChannelsTxPower;
}

void lorawan_node_set_tx_power(int8_t txPower)
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_CHANNELS_TX_POWER;
	mibReq.Param.ChannelsTxPower = txPower;
	status = LoRaMacMibSetRequestConfirm(&mibReq);

	assert(status == LORAMAC_STATUS_OK);
}

uint32_t lorawan_node_get_rx1_delay()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_RECEIVE_DELAY_1;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.ReceiveDelay1;
}

uint32_t lorawan_node_get_rx2_delay()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_RECEIVE_DELAY_2;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.ReceiveDelay2;
}

uint32_t lorawan_node_get_rx2_freq()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_RX2_CHANNEL;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.Rx2Channel.Frequency;
}

uint32_t lorawan_node_get_rx2_data_rate()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_RX2_CHANNEL;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.Rx2Channel.Datarate;
}

uint32_t lorawan_node_get_accept_rx1_delay()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_JOIN_ACCEPT_DELAY_1;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.JoinAcceptDelay1;
}

uint32_t lorawan_node_get_accept_rx2_delay()
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mibReq;

	assert(lorawan_node_config);

	mibReq.Type = MIB_JOIN_ACCEPT_DELAY_2;
	status = LoRaMacMibGetRequestConfirm(&mibReq);
	assert(status == LORAMAC_STATUS_OK);

	return mibReq.Param.JoinAcceptDelay2;
}
