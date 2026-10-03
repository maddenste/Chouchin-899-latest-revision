// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLOCK_STORAGE_H
#define CLOCK_STORAGE_H

#include "../../clock_settings.h"

// Dedicated two-sector journal immediately below the factory syscfg slots.
// Never calls syscfg_init/write and never touches 0x1FE000/0x1FF000.
int clock_storage_load(struct txw_clock_settings *settings);
int clock_storage_save(struct txw_clock_settings *settings);
int clock_storage_reset(struct txw_clock_settings *settings);

#endif
