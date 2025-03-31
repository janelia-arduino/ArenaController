// SPDX-FileCopyrightText: 2024 Cesanta Software Limited
// SPDX-License-Identifier: GPL-2.0-only or commercial
// Generated by Mongoose Wizard, https://mongoose.ws/wizard/

// Default mock implementation of the API callbacks

#include "mongoose_glue.h"


static struct leds s_leds = {false};
void glue_get_leds(struct leds *data) {
  *data = s_leds;  // Sync with your device
}
void glue_set_leds(struct leds *data) {
  s_leds = *data; // Sync with your device
}
