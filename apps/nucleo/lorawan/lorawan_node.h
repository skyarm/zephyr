/**
 ******************************************************************************
 *
 *          Portions COPYRIGHT 2020 STMicroelectronics
 *
 * @file    LmHandler.h
 * @author  MCD Application Team
 * @brief   Header for LoRaMAC Layer handling module
 ******************************************************************************
 */
#ifndef LORAWAN_NODE_H
#define LORAWAN_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifndef LORAWAN_NODE_ABP_VERSION
#define LORAWAN_NODE_ABP_VERSION  0x01000300 /* 1.0.3.0 */
#endif

enum lorawan_node_region {
	/**
	 * AS band on 923MHz
	 */
	LORAWAN_NODE_REGION_AS923 = 0,
	/**
	 * Australian band on 915MHz
	 */
	LORAWAN_NODE_REGION_AU915,
	/**
	 * Chinese band on 470MHz
	 */
	LORAWAN_NODE_REGION_CN470,
	/**
	 * Chinese band on 779MHz
	 */
	LORAWAN_NODE_REGION_CN779,
	/**
	 * European band on 433MHz
	 */
	LORAWAN_NODE_REGION_EU433,
	/**
	 * European band on 868MHz
	 */
	LORAWAN_NODE_REGION_EU868,
	/**
	 * South korean band on 920MHz
	 */
	LORAWAN_NODE_REGION_KR920,
	/**
	 * India band on 865MHz
	 */
	LORAWAN_NODE_REGION_IN865,
	/**
	 * North american band on 915MHz
	 */
	LORAWAN_NODE_REGION_US915,
	/**
	 * Russia band on 864MHz
	 */
	LORAWAN_NODE_REGION_RU864
};

enum lorawan_node_data_rate {
	LORAWAN_NODE_DR_0       = 0,
	LORAWAN_NODE_DR_1       = 1,
	LORAWAN_NODE_DR_2       = 2,
	LORAWAN_NODE_DR_3       = 3,
	LORAWAN_NODE_DR_4       = 4,
	LORAWAN_NODE_DR_5       = 5,
	LORAWAN_NODE_DR_6       = 6,
	LORAWAN_NODE_DR_7       = 7,
	LORAWAN_NODE_DR_8       = 8,
	LORAWAN_NODE_DR_9       = 9,
	LORAWAN_NODE_DR_10      = 10,
	LORAWAN_NODE_DR_11      = 11,
	LORAWAN_NODE_DR_12      = 12,
	LORAWAN_NODE_DR_13      = 13,
	LORAWAN_NODE_DR_14      = 14,
	LORAWAN_NODE_DR_15      = 15
};

enum lorawan_node_tx_power {
	LORAWAN_NODE_TX_POWER_0         = 0,
	LORAWAN_NODE_TX_POWER_1         = 1,
	LORAWAN_NODE_TX_POWER_2         = 2,
	LORAWAN_NODE_TX_POWER_3         = 3,
	LORAWAN_NODE_TX_POWER_4         = 4,
	LORAWAN_NODE_TX_POWER_5         = 5,
	LORAWAN_NODE_TX_POWER_6         = 6,
	LORAWAN_NODE_TX_POWER_7         = 7,
	LORAWAN_NODE_TX_POWER_8         = 8,
	LORAWAN_NODE_TX_POWER_9         = 9,
	LORAWAN_NODE_TX_POWER_10        = 10,
	LORAWAN_NODE_TX_POWER_11        = 11,
	LORAWAN_NODE_TX_POWER_12        = 12,
	LORAWAN_NODE_TX_POWER_13        = 13,
	LORAWAN_NODE_TX_POWER_14        = 14,
	LORAWAN_NODE_TX_POWER_15        = 15
};

enum lorawan_node_class {
	/**
	 * LoRaWAN device class A
	 * LoRaWAN Specification V1.0.2, chapter 3
	 */
	LORAWAN_NODE_CLASS_A    = 0x00,
	/**
	 * LoRaWAN device class B
	 * LoRaWAN Specification V1.0.2, chapter 8
	 */
	LORAWAN_NODE_CLASS_B    = 0x01,
	/**
	 * LoRaWAN device class C
	 * LoRaWAN Specification V1.0.2, chapter 17
	 */
	LORAWAN_NODE_CLASS_C    = 0x02
};

enum lorawan_node_activation_mode {
	/**
	 * None
	 */
	LORAWAN_NODE_ACTIVATION_NONE    = 0,
	/**
	 * Activation By Personalization (ACTIVATION_TYPE_ABP)
	 */
	LORAWAN_NODE_ACTIVATION_ABP     = 1,
	/**
	 * Over-The-Air Activation (ACTIVATION_TYPE_OTAA)
	 */
	LORAWAN_NODE_ACTIVATION_OTAA    = 2,
};

enum lorawan_node_status {
	/*!
     * Service started successfully
     */
	LORAWAN_NODE_STATUS_OK = 0,
	/*!
     * Service not started - LoRaMAC is busy
     */
	LORAWAN_NODE_STATUS_BUSY,
	/*!
     * Service unknown
     */
	LORAWAN_NODE_STATUS_SERVICE_UNKNOWN,
	/*!
     * Service not started - invalid parameter
     */
	LORAWAN_NODE_STATUS_PARAMETER_INVALID,
	/*!
     * Service not started - invalid frequency
     */
	LORAWAN_NODE_STATUS_FREQUENCY_INVALID,
	/*!
     * Service not started - invalid datarate
     */
	LORAWAN_NODE_STATUS_DATARATE_INVALID,
	/*!
     * Service not started - invalid frequency and datarate
     */
	LORAWAN_NODE_STATUS_FREQ_AND_DR_INVALID,
	/*!
     * Service not started - the device is not in a LoRaWAN
     */
	LORAWAN_NODE_STATUS_NO_NETWORK_JOINED,
	/*!
     * Service not started - payload length error
     */
	LORAWAN_NODE_STATUS_LENGTH_ERROR,
	/*!
     * Service not started - the specified region is not supported
     * or not activated with preprocessor definitions.
     */
	LORAWAN_NODE_STATUS_REGION_NOT_SUPPORTED,
	/*!
     * The application data was not transmitted
     * because prioritized pending MAC commands had to be sent.
     */
	LORAWAN_NODE_STATUS_SKIPPED_APP_DATA,
	/*!
     * An MCPS or MLME request can return this status. In this case,
     * the MAC cannot send the frame, as the duty cycle limits all
     * available bands. When a request returns this value, the
     * variable "DutyCycleWaitTime" in "ReqReturn" of the input
     * parameters contains the remaining time to wait. If the
     * value is constant and does not change, the expected time
     * on air for this frame is exceeding the maximum permitted
     * time according to the duty cycle time period, defined
     * in Region.h, DUTY_CYCLE_TIME_PERIOD. By default this time
     * is 1 hour, and a band with 1% duty cycle is then allowed
     * to use an air time of 36 seconds.
     */
	LORAWAN_NODE_STATUS_DUTYCYCLE_RESTRICTED,
	LORAWAN_NODE_STATUS_NO_CHANNEL_FOUND,
	LORAWAN_NODE_STATUS_NO_FREE_CHANNEL_FOUND,
	LORAWAN_NODE_STATUS_BUSY_BEACON_RESERVED_TIME,
	LORAWAN_NODE_STATUS_BUSY_PING_SLOT_WINDOW_TIME,
	LORAWAN_NODE_STATUS_BUSY_UPLINK_COLLISION,
	LORAWAN_NODE_STATUS_CRYPTO_ERROR,
	LORAWAN_NODE_STATUS_FCNT_HANDLER_ERROR,
	LORAWAN_NODE_STATUS_MAC_COMMAD_ERROR,
	LORAWAN_NODE_STATUS_CLASS_B_ERROR,
	LORAWAN_NODE_STATUS_CONFIRM_QUEUE_ERROR,
	LORAWAN_NODE_STATUS_MC_GROUP_UNDEFINED,
	LORAWAN_NODE_STATUS_ERROR
};

enum lorawan_node_event_status {
	LORAWAN_NODE_EVENT_STATUS_OK = 0,
	LORAWAN_NODE_EVENT_STATUS_ERROR,
	LORAWAN_NODE_EVENT_STATUS_TX_TIMEOUT,
	LORAWAN_NODE_EVENT_STATUS_RX1_TIMEOUT,
	LORAWAN_NODE_EVENT_STATUS_RX2_TIMEOUT,
	LORAWAN_NODE_EVENT_STATUS_RX1_ERROR,
	LORAWAN_NODE_EVENT_STATUS_RX2_ERROR,
	LORAWAN_NODE_EVENT_STATUS_JOIN_FAIL,
	LORAWAN_NODE_EVENT_STATUS_DOWNLINK_REPEATED,
	LORAWAN_NODE_EVENT_STATUS_TX_DR_PAYLOAD_SIZE_ERROR,
	LORAWAN_NODE_EVENT_STATUS_ADDRESS_FAIL,
	LORAWAN_NODE_EVENT_STATUS_MIC_FAIL,
	LORAWAN_NODE_EVENT_STATUS_MULTICAST_FAIL,
	LORAWAN_NODE_EVENT_STATUS_BEACON_LOCKED,
	LORAWAN_NODE_EVENT_STATUS_BEACON_LOST,
	LORAWAN_NODE_EVENT_STATUS_BEACON_NOT_FOUND,
};

/**
 * @brief Join notification parameters
 */
struct lorawan_node_cb_join_request_params {
	enum lorawan_node_activation_mode mode;
	enum lorawan_node_event_status status;
	int8_t data_rate;
};

/**
 * @brief Callback data sent parameters
 */
struct lorawan_node_cb_data_sent_params {
	bool is_mcps_confirm;
	enum lorawan_node_event_status status;
	uint8_t ack_received;
	int8_t data_rate;
	uint32_t uplink_counter;
	int8_t tx_power;
	uint8_t channel;
};

/**
 * @brief Rx callback parameters
 */
struct lorawan_node_cb_data_received_params {
	bool is_mcps_indication;
	enum lorawan_node_event_status status;
	int8_t data_rate;
	int8_t rssi;
	int8_t snr;
	uint32_t downlink_counter;
	int8_t rx_slot;
};

/**
 * @brief Beacon status callback parameters
 */
struct lorawan_node_cb_beacon_status_params {
	enum lorawan_node_event_status status;
	/**
	 * Timestamp in seconds since 00:00:00, Sunday 6th of January 1980
	 * (start of the GPS epoch) modulo 2^32
	 */
	uint32_t time;
	uint32_t freq;
	uint8_t data_rate;
	int16_t rssi;
	int8_t snr;
	/* Info descriptor - can differ for each gateway */
	uint8_t info_desc;
	/* Info - can differ for each gateway */
	uint8_t info_data[6];
};

struct lorawan_node_callbacks {
	/**
	 * @brief Get the current battery level
	 * @retval value  Battery level ( 0: very low, 254: fully charged )
	 */
	uint8_t (*get_battery_level)(void);
	/**
	 * @brief Get the current temperature
	 * @retval value  Temperature in degree Celsius
	 */
	float (*get_temperature)(void);
	/**
	 * @brief    Will be called each time a Radio IRQ is handled by the MAC
	 *          layer.
	 * @warning  Runs in a IRQ context. Should only change variables state.
	 */
	void (*mac_process)(void);
	/**
	 * @brief Notifies the upper layer that a network has been joined
	 * @param [in] params notification parameters
	 */
	void (*join_request)(const struct lorawan_node_cb_join_request_params *params);
	/**
	 * @brief Notifies upper layer that a frame has been transmitted
	 * @param [in] params notification parameters
	 */
	void (*data_sent)(const struct lorawan_node_cb_data_sent_params *params);
	/**
	 * @brief Notifies the upper layer that an applicative frame has been received
	 * @param [in] appData Received applicative data
	 * @param [in] params notification parameters
	 */
	void (*data_received)(uint8_t port, const void *data, uint8_t size, const struct lorawan_node_cb_data_received_params  *params);
	/**
	 * @brief Notifies the upper layer that class mode has been received
	 * @param [in] deviceClass device class
	 */
	void (*class_changed)(enum lorawan_node_class device_class);
	/**
	 * @brief Notifies the upper layer that beacon status has been received
	 * @param [in] params beacon parameter
	 */
	void (*beacon_status)(const struct lorawan_node_cb_beacon_status_params *params);
	/**
	 * @brief Notifies the upper layer that device time response has been received
	 * @param [in] params beacon parameter
	 */
	void (*device_time)(uint32_t seconds, uint16_t subseconds);
};

/**
 * @brief LoRaMac handler parameters
 */
struct lorawan_node_config {
	bool public_network;
	enum lorawan_node_region active_region;
	bool network_id;
	bool adr_enabled;
	int8_t tx_data_rate;
	uint32_t device_address;
	/**
	 * Periodicity of the ping slots
	 */
	uint8_t ping_periodicity;
	const struct lorawan_node_callbacks callbacks;
};

/**
 * @brief LoRaWAN join parameters for over-the-Air activation (OTAA)
 *
 * Note that all of the fields use LoRaWAN 1.1 terminology.
 *
 * All parameters are optional if a secure element is present in which
 * case the values stored in the secure element will be used instead.
 */
struct lorawan_node_join_otaa {
	/** Join EUI */
	uint8_t *join_eui;
	/** Network Key */
	uint8_t *nwk_key;
	/** Application Key */
	uint8_t *app_key;
	/**
	 * Device Nonce
	 *
	 * Starting with LoRaWAN 1.0.4 the DevNonce must be monotonically
	 * increasing for each OTAA join with the same EUI. The DevNonce
	 * should be stored in non-volatile memory by the application.
	 */
	uint16_t dev_nonce;
};

/**
 * @brief LoRaWAN join parameters for activation by personalization (ABP)
 */
struct lorawan_node_join_abp {
	/** Device address on the network */
	uint32_t dev_addr;
	/** Application session key */
	uint8_t *app_skey;
	/** Network session key */
	uint8_t *nwk_skey;
	/** Application EUI */
	uint8_t *app_eui;
};

/**
 * @brief LoRaWAN join parameters
 */
struct lorawan_node_join_config {
	/** Join parameters */
	union {
		struct lorawan_node_join_otaa otaa; /**< OTAA join parameters */
		struct lorawan_node_join_abp abp;   /**< ABP join parameters */
	};

	/** Device EUI. Optional if a secure element is present. */
	uint8_t *dev_eui;

	/** Activation mode */
	enum lorawan_node_activation_mode mode;
};

/**
 * @brief LoRaMac handler initialisation
 * @param [in] handlerCallbacks LoRaMac handler callbacks
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_init(const struct lorawan_node_config *config);

/**
 * @brief Join a LoRa Network in classA
 * @param [in] mode Activation mode (OTAA or ABP)
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_join(const struct lorawan_node_join_config *join_cfg);

/**
 * @brief Processes the LoRaMac and Radio events. When no pendig operation asks to go in low power mode.
 * @remark This function must be called in the main loop.
 */
void lorawan_node_process(void);

/**
 * @brief Instructs the MAC layer to send a ClassA uplink
 * @param [in] appData Data to be sent
 * @param [in] isTxConfirmed Indicates if the uplink requires an acknowledgement
 * @param [out] nextTxIn Time before next uplink window available
 * @param [in] allowDelayedTx when set to true, the frame will be delayed
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_send(uint8_t port, const void *data, uint8_t size, bool tx_confirmed);


/**
 * @brief request a gps time from network
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_device_time_req(void);

/**
 * @brief Stop a LoRa Network connection
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_stop(void);

/**
 * @brief Request the MAC layer to change LoRaWAN class
 * @note Callback \ref LmHandlerConfirmClass informs upper layer that the change has occurred
 * @note Only switch from class A to class B/C OR from class B/C to class A is allowed
 * @param [in] new_class New class to be requested
 * @retval LORAWAN_NODE_STATUS_OK: success, otherwise failed.
 */
enum lorawan_node_status lorawan_node_request_class(enum lorawan_node_class new_class);

/**
 * @brief Gets the current LoRaWAN class
 * @retval current class
 */
enum lorawan_node_class lorawan_node_get_current_class();

/**
 * @brief Gets the current LoRaWAN time
 * @param subseconds: milliseconds;
 * @retval seconds
 */
uint32_t lorawan_node_get_current_time(uint16_t *subseconds);
/**
 * @brief Is the device switching Class B and pending
 * @retval true: pending, otherwise false
 */
bool lorawan_node_classb_pending();

/**
 * @brief Is the device initialed
 * @retval true: initialed, otherwise false
 */
bool lorawan_node_is_initialed();

/**
 * @brief Gets the next duty cycle time
 * @retval next duty cycle time
 */
uint32_t lorawan_node_get_duty_cycle_time();

/**
 * @brief   Check whether the Device is joined to the network
 * @retval  true: joined false: not joined
 */
bool lorawan_node_is_joined(void);

/**
 * @brief Indicates if the LoRaMacHandler is busy
 * @retval status [true] Busy, [false] free
 */
bool lorawan_node_is_busy(void);

/**
 * @brief Gets the current ClassB Ping periodicity
 * @retval ping periodicity
 */
uint8_t lorawan_node_get_ping_periodicity();

/**
 * @brief Sets the ClassB Ping periodicity
 * @param [in] periodicity ping periodicity
 * Set the new periodicity must as this:
 * 1.Device must be changed to Class A if device isn't Class A mode, 
 * 		or the device didn't activated.
 * 2.Call lorawan_node_set_ping_periodicity set to new ping periodicity;
 * 3.Changed to Class B again
 */
bool lorawan_node_set_ping_periodicity(uint8_t periodicity);
/**
 * @brief Gets the LoRaWAN Device EUI (if OTAA)
 * @retval devEUI LoRaWAN DevEUI
 */
const uint8_t *lorawan_node_get_dev_eui();

/**
 * @brief Gets the LoRaWAN AppEUI
 * @retval LoRaWAN AppEUI
 */
const uint8_t *lorawan_node_get_app_eui();

/**
 * @brief Gets the LoRaWAN Network ID if ABP or after the Join if OTAA)
 * @retval current network ID
 */
uint32_t lorawan_node_get_network_id();

/**
 * @brief Gets the LoRaWAN Device Address if ABP or after the Join if OTAA
 * @retval current device address
 */
uint32_t lorawan_node_get_dev_addr();

/**
 * @brief Sets the LoRaWAN Device Address (if ABP)
 * @param [in] devAddr device address
 * @retval true success, else failed
 */
bool lorawan_node_set_dev_addr(uint32_t addr);

/**
 * @brief Gets the current active region
 * @retval Current active region
 */
enum lorawan_node_region lorawan_node_get_active_region();

/**
 * @brief Gets the Adaptive data rate (1 = the Network manages the DR, 0 = the device manages the DR)
 * @retval Adaptive data rate enabled
 */
bool lorawan_node_is_adr_enabled();

/**
 * @brief Sets the Adaptive data rate (1 = the Network manages the DR, 0 = the device manages the DR)
 * @param [in] adrEnable Adaptive data rate flag
 */
void lorawan_node_enable_adr(bool enabled);

/**
 * @brief Gets the current datarate
 * @retval Current TX datarate
 */
uint8_t lorawan_node_get_tx_data_rate();

/**
 * @brief Sets the current datarate
 * @param [in] txDatarate new TX datarate
 * @retval true success, else failed
 */
bool lorawan_node_set_tx_data_rate(int8_t dr);

/**
 * @brief Gets the duty cycle flag
 * @retval dutyCycleEnable duty cycle flag
 */
bool lorawan_node_is_duty_cycle_enabled();

/**
 * @brief Sets the current datarate
 * @param [in] enabled duty cycle enabled
 */
void lorawan_node_enable_duty_cycle(bool enabled);

/**
 * @brief Gets the current RX_2 datarate and frequency
 * @retval x2 parameters
 */
uint32_t lorawan_node_get_rx2_freq();
/**
 * @brief Gets the current RX_2 datarate and frequency
 * @retval rx2 parameters
 */
uint32_t lorawan_node_get_rx2_data_rate();

/**
 * @brief Gets the current TX power
 * @retval X power
 */
uint8_t lorawan_node_get_tx_power();

/**
 * @brief Gets the current RX1 delay (after the TX done)
 * @retval rxDelay RX1 delay
 */
uint32_t lorawan_node_get_rx1_delay();

/**
 * @brief Gets the current RX2 delay (after the TX done)
 * @retval RX2 delay
 */
uint32_t lorawan_node_get_rx2_delay();

/**
 * @brief Gets the current RX1 Join delay (after the TX done)
 * @retval RX1 Join delay
 */
uint32_t lorawan_node_get_accept_rx1_delay();

/**
 * @brief Gets the current RX2 Join delay (after the TX done)
 * @retval accept rx2 delay in milliseconds
 */
uint32_t lorawan_node_get_accept_rx2_delay();

/**
 * @brief Sets the TX power
 * @param [in] power TX power
 */
void lorawan_node_set_tx_power(int8_t power);

#ifdef __cplusplus
}
#endif

#endif /* LORAWAN_NODE_H */
