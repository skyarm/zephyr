/*
 * Copyright (c) 2019 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * Based on ST7789V sample:
 * Copyright (c) 2019 Marc Reilly
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#include <device.h>
#include <drivers/display.h>
#include <drivers/gpio.h>
#include <ff.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <stdio.h>
#include <storage/disk_access.h>
#include <storage/flash_map.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#define LED0_NODE DT_ALIAS(led0)
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0_NAME DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif

const struct device* sample_led() {
  const struct device* dev = device_get_binding(LED0_NAME);
  if (dev == NULL) {
    return NULL;
  }

  if (gpio_pin_configure(dev, LED0_PIN, GPIO_OUTPUT_ACTIVE | LED0_FLAGS) < 0) {
    return NULL;
  }
  return dev;
}

const struct device* led0_device;

#ifdef SAMPLE_DISPLAY
#if DT_NODE_HAS_STATUS(DT_INST(0, sitronix_st7789v2), okay)
#define DISPLAY_DEV_NAME DT_LABEL(DT_INST(0, sitronix_st7789v2))
#endif

uint16_t line_buf[240];

void draw(const struct device* dev, uint16_t color) {
  for (int index = 0; index < 240; index++) {
    line_buf[index] = sys_cpu_to_be16(color);
  }

  struct display_buffer_descriptor desc;
  desc.buf_size = sizeof(line_buf);
  desc.pitch = 240;
  desc.width = 240;
  desc.height = 1;
  gpio_pin_set(led0_device, LED0_PIN, 1);
  for (int line = 0; line < 240; line++) {
    display_write(dev, 0, line, &desc, line_buf);
  }
  gpio_pin_set(led0_device, LED0_PIN, 0);
}

void sample_display() {
  LOG_INF("Display sample for %s", DISPLAY_DEV_NAME);

  const struct device* dev = device_get_binding(DISPLAY_DEV_NAME);

  if (dev == NULL) {
    LOG_ERR("Device %s not found. Aborting sample.", DISPLAY_DEV_NAME);
    return;
  }

  for (;;) {
    draw(dev, 0xffff);
    k_msleep(1000);
    draw(dev, 0xf800);
    k_msleep(1000);
    draw(dev, 0x07e0);
    k_msleep(1000);
    draw(dev, 0x001f);
    k_msleep(1000);
  }
}
#endif /*SAMPLE_DISPLAY*/

#define BTN0_GPIO_NODE DT_ALIAS(sw0)
#define BTN0_GPIO_LABEL DT_GPIO_LABEL(BTN0_GPIO_NODE, gpios)
#define BTN0_GPIO_PIN DT_GPIO_PIN(BTN0_GPIO_NODE, gpios)
#define BTN0_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(BTN0_GPIO_NODE, gpios))

static struct gpio_callback button_cb_data;
void button_pressed(const struct device* dev, struct gpio_callback* cb,
                    uint32_t pins) {
  static int pressed = 1;
  if (pressed) {
    pressed = 0;
    gpio_pin_set(led0_device, LED0_PIN, 0);
  } else {
    pressed = 1;
    gpio_pin_set(led0_device, LED0_PIN, 1);
  }
}

void sample_button() {
  const struct device* btn0;
  int retcode;

  btn0 = device_get_binding(BTN0_GPIO_LABEL);
  if (btn0 == NULL) {
    gpio_pin_set(led0_device, LED0_PIN, 1);
    return;
  }

  retcode = gpio_pin_configure(btn0, BTN0_GPIO_PIN, BTN0_GPIO_FLAGS);
  if (retcode != 0) {
    gpio_pin_set(led0_device, LED0_PIN, 1);
    return;
  }

  retcode = gpio_pin_interrupt_configure(btn0, BTN0_GPIO_PIN,
                                         GPIO_INT_EDGE_TO_ACTIVE);
  if (retcode != 0) {
    gpio_pin_set(led0_device, LED0_PIN, 1);
    return;
  }

  gpio_init_callback(&button_cb_data, button_pressed, BIT(BTN0_GPIO_PIN));
  gpio_add_callback(btn0, &button_cb_data);
}

char fname[128];
#define PARTITION_NODE DT_NODELABEL(lfs0)
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
void sample_fs_qspi() {
  int rc;
  struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
  snprintf(fname, sizeof(fname), "%s/boot_count", mp->mnt_point);

  struct fs_statvfs sbuf;
  rc = fs_statvfs(mp->mnt_point, &sbuf);
  if (rc < 0) {
    LOG_ERR("FAIL: statvfs: %d\n", rc);
    return;
  }

  LOG_INF(
      "%s: bsize = %lu ; frsize = %lu ;"
      " blocks = %lu ; bfree = %lu\n",
      mp->mnt_point, sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks, sbuf.f_bfree);

  struct fs_file_t file;
  fs_file_t_init(&file);
  rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
  if (rc < 0) {
    LOG_ERR("FAIL: open %s: %d\n", fname, rc);
    return;
  }

  uint32_t boot_count = 0;

  rc = fs_read(&file, &boot_count, sizeof(boot_count));
  if (rc < 0) {
    return;
  }
  LOG_INF("%s read count %u: %d\n", fname, boot_count, rc);
  rc = fs_seek(&file, 0, FS_SEEK_SET);
  if (rc < 0) {
    return;
  }

  boot_count += 1;
  rc = fs_write(&file, &boot_count, sizeof(boot_count));
  rc = fs_close(&file);

  struct fs_dir_t dir;
  fs_dir_t_init(&dir);
  rc = fs_opendir(&dir, mp->mnt_point);
  while (rc >= 0) {
    struct fs_dirent ent = {0};

    rc = fs_readdir(&dir, &ent);
    if (rc < 0) {
      break;
    }
    if (ent.name[0] == 0) {
      break;
    }
    LOG_INF("  %c %u %s\n", (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
           ent.size, ent.name);
  }

  (void)fs_closedir(&dir);
}

static FATFS fat_fs;
static struct fs_mount_t fatfs_mp = {
    .type = FS_FATFS, .fs_data = &fat_fs, .mnt_point = "/sdmmc1"};

void sample_fs_sdmmc() {
  int rc = fs_mount(&fatfs_mp);

  if (rc != FR_OK) {
    LOG_ERR("Mount %s failed.", fatfs_mp.mnt_point);
    return;
  }
  struct fs_dir_t dirp;
  static struct fs_dirent entry;

  fs_dir_t_init(&dirp);
  rc = fs_opendir(&dirp, fatfs_mp.mnt_point);
  if (rc) {
    LOG_ERR("Open dir from %s failed.", fatfs_mp.mnt_point);
    fs_unmount(&fatfs_mp);
    return;
  }
  for (;;) {
    rc = fs_readdir(&dirp, &entry);

    /* entry.name[0] == 0 means end-of-dir */
    if (rc || entry.name[0] == 0) {
      break;
    }

    if (entry.type == FS_DIR_ENTRY_DIR) {
      LOG_ERR("[DIR ] %s\n", entry.name);
    } else {
      LOG_ERR("[FILE] %s (size = %zu)\n", entry.name, entry.size);
    }
  }
  fs_closedir(&dirp);
  fs_unmount(&fatfs_mp);
}

void main(void) {
  led0_device = sample_led();
  sample_button();
  sample_fs_qspi();
  sample_fs_sdmmc();
#ifdef SAMPLE_DISPLAY
  sample_display();
#endif
}
