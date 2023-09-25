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
constexpr uint8_t RESET_PIN = 37;

// region
constexpr uint8_t REGION_COUNT_PER_ARENA = 2;
constexpr SPIClass * REGION_SPI_PTRS[REGION_COUNT_PER_ARENA] = {&SPI, &SPI1};

constexpr uint8_t PANEL_COUNT_MAX_PER_REGION_ROW = PANEL_COUNT_MAX_PER_ARENA_ROW;
constexpr uint8_t PANEL_COUNT_MAX_PER_REGION_COL = \
  PANEL_COUNT_MAX_PER_ARENA_COL/REGION_COUNT_PER_ARENA; // 3

constexpr uint8_t PANEL_SELECT_PINS[PANEL_COUNT_MAX_PER_REGION_ROW][PANEL_COUNT_MAX_PER_REGION_COL] =
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
