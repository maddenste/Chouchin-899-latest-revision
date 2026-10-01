#include "sys_config.h"
#include "typesdef.h"
#include "dev.h"
#include "devid.h"
#include "hal/spi_nor.h"
#include "clock_storage.h"
#include <string.h>

#define CLOCK_FLASH_SIZE 0x200000u
#define CLOCK_SECTOR_SIZE 0x1000u
#define CLOCK_SLOT_A 0x1fc000u
#define CLOCK_SLOT_B 0x1fd000u
#define FACTORY_SLOT_A 0x1fe000u

extern struct spi_nor_flash flash0;
static uint32 current_slot;
static uint32 current_generation;
static uint8 flash_opened;

static int flash_ready(void)
{
    // Refuse to write if flash geometry is not the verified 2 MiB layout.
    if (flash0.size != CLOCK_FLASH_SIZE ||
        flash0.sector_size != CLOCK_SECTOR_SIZE ||
        CLOCK_SLOT_B + CLOCK_SECTOR_SIZE != FACTORY_SLOT_A) return 0;
    if (!flash_opened) {
        if (spi_nor_open(&flash0) != RET_OK) return 0;
        flash_opened = 1u;
    }
    return 1;
}

static void read_slot(uint32 address, struct txw_clock_settings *record)
{
    memset(record, 0, sizeof(*record));
    spi_nor_read(&flash0, address, (uint8 *)record, sizeof(*record));
}

int clock_storage_load(struct txw_clock_settings *settings)
{
    struct txw_clock_settings first, second;
    int valid_first, valid_second;
    if (settings == NULL) return 0;
    txw_clock_settings_defaults(settings);
    current_slot = 0u;
    current_generation = 0u;
    if (!flash_ready()) return 0;
    read_slot(CLOCK_SLOT_A, &first);
    read_slot(CLOCK_SLOT_B, &second);
    valid_first = txw_clock_settings_valid(&first);
    valid_second = txw_clock_settings_valid(&second);
    if (!valid_first && !valid_second) return 0;
    if (valid_first && (!valid_second ||
        (int32)(first.generation - second.generation) > 0)) {
        *settings = first;
        current_slot = CLOCK_SLOT_A;
    } else {
        *settings = second;
        current_slot = CLOCK_SLOT_B;
    }
    current_generation = settings->generation;
    return 1;
}

int clock_storage_save(struct txw_clock_settings *settings)
{
    struct txw_clock_settings candidate, readback;
    uint32 destination;
    if (settings == NULL || !flash_ready()) return 0;
    destination = current_slot == CLOCK_SLOT_A ? CLOCK_SLOT_B : CLOCK_SLOT_A;
    candidate = *settings;
    txw_clock_settings_seal(&candidate, current_generation + 1u);
    if (!txw_clock_settings_valid(&candidate)) return 0;
    spi_nor_sector_erase(&flash0, destination);
    spi_nor_write(&flash0, destination, (uint8 *)&candidate,
                  sizeof(candidate));
    read_slot(destination, &readback);
    if (memcmp(&candidate, &readback, sizeof(candidate)) != 0 ||
        !txw_clock_settings_valid(&readback)) return 0;
    *settings = readback;
    current_slot = destination;
    current_generation = readback.generation;
    return 1;
}

int clock_storage_reset(struct txw_clock_settings *settings)
{
    struct txw_clock_settings blank;
    uint32 stale_slot = current_slot;
    if (settings == NULL) return 0;
    txw_clock_settings_defaults(&blank);
    // Commit a newer tombstone before erasing the old credentials. If power
    // fails mid-reset, the newer blank record wins when loading settings.
    if (!clock_storage_save(&blank)) return 0;
    if (stale_slot != 0u && stale_slot != current_slot) {
        spi_nor_sector_erase(&flash0, stale_slot);
    }
    *settings = blank;
    return 1;
}
