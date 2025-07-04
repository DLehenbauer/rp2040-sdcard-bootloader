/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

// Standard
#include <stdio.h>
#include <string.h>

// Pico SDK
#include <pico/stdlib.h>

// SPI/FatFS
#include <ff.h>
#include <diskio.h>
#include <f_util.h>
#include <hw_config.h>
#include <rtc.h>

// Project
#include "diag.h"
#include "transport.h"

#define PC_NAME "0:"
#define FIRMWARE_FILENAME (PC_NAME BOOTLOADER_FIRMWARE_FILENAME)

static spi_t spis[] = {{
    .hw_inst    = __CONCAT(spi, BOOTLOADER_SD_SPI),
    .miso_gpio  = BOOTLOADER_SD_SPI_RX_PIN,
    .mosi_gpio  = BOOTLOADER_SD_SPI_TX_PIN,
    .sck_gpio   = BOOTLOADER_SD_SPI_SCK_PIN,
    .baud_rate  = BOOTLOADER_SD_BAUD_RATE
}};

static sd_card_t sd_cards[] = {{
    .pcName = PC_NAME,
    .spi = &spis[0],
    .ss_gpio = BOOTLOADER_SD_SPI_CSN_PIN,
    .card_detect_gpio = BOOTLOADER_SD_DETECT_PIN,
    .use_card_detect = BOOTLOADER_SD_USE_DETECT
}};

size_t sd_get_num() { return count_of(sd_cards); }

sd_card_t* sd_get_by_num(size_t num) {
    return num < sd_get_num()
        ?  &sd_cards[num]
        : NULL;
}

size_t spi_get_num() { return count_of(spis); }

spi_t* spi_get_by_num(size_t num) {
    return num < spi_get_num()
        ? &spis[num]
        : NULL;
}

static FIL file = { 0 };

void transport_init() {
    time_init();
}

bool uf2_exists() {
    sd_card_t* pSd = sd_get_by_num(0);

    if (pSd->mounted && !pSd->sd_test_com(pSd)) {
        f_unmount(pSd->pcName);
        pSd->mounted = false;
    }

    FRESULT fr = FR_OK;

    if (!pSd->mounted) {
        fr = f_mount(&pSd->fatfs, pSd->pcName, 1);
        
        if (fr != FR_OK) {
            pSd->m_Status |= STA_NOINIT;
            pSd->mounted = false;
            return false;
        }
        
        pSd->mounted = true;
    }

    FILINFO fileInfo;
    fr = f_stat(FIRMWARE_FILENAME, &fileInfo);
    return (FR_OK == fr && fileInfo.fsize > 0);
}

bool read_uf2(prog_t* prog, accept_block_cb_t callback) {
    if (!uf2_exists()) {
        return false;
    }

    if (f_open(&file, FIRMWARE_FILENAME, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
        return false;
    }

    bool ok = true;

    while (true) {
        struct uf2_block block;
        size_t bytes_read = 0;

        ok = f_read(&file, &block, sizeof(block), &bytes_read) == FR_OK;
        if (!ok) {
            break;
        }

        if (bytes_read < sizeof(block)) {
            ok = (bytes_read == 0);
            break;
        }

        ok = callback(prog, &block);
        if (!ok) {
            break;
        }
    }

    f_close(&file);
    return ok;
}

bool remove_uf2() {
    FRESULT fr = f_unlink(FIRMWARE_FILENAME);
    return fr == FR_OK;
}
