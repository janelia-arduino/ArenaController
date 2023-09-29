// ----------------------------------------------------------------------------
// ConstantsTeensy41.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_CONSTANTS_TEENSY41_H
#define PANELS_CONTROLLER_CONSTANTS_TEENSY41_H
#include <SPI.h>
#include "Constants.h"


#if defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41)
namespace panels_controller
{
namespace constants
{
constexpr uint8_t reset_pin = 37;

// region
constexpr uint8_t region_count_per_frame = 2;
constexpr SPIClass * region_spi_ptrs[region_count_per_frame] = {&SPI, &SPI1};

constexpr uint8_t panel_count_max_per_region_row = panel_count_max_per_frame_row;
constexpr uint8_t panel_count_max_per_region_col = \
  panel_count_max_per_frame_col/region_count_per_frame; // 3

constexpr uint8_t panel_select_pins[panel_count_max_per_region_row][panel_count_max_per_region_col] =
{
  {3, 8, 32},
  {4, 9, 33},
  {5, 29, 34},
  {6, 30, 35},
  {7, 31, 36}
};
}
}
#endif
#endif
