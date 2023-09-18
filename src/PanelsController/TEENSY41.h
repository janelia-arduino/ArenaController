// ----------------------------------------------------------------------------
// TEENSY41.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_TEENSY41_CONSTANTS_H
#define PANELS_CONTROLLER_TEENSY41_CONSTANTS_H
#include <SPI.h>
#include "Constants.h"


#if defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41)
namespace panels_controller
{
namespace constants
{
// region
constexpr uint8_t REGION_COUNT_PER_ARENA = 2;
constexpr SPIClass * REGION_SPI_PTRS[REGION_COUNT_PER_ARENA] = {&SPI, &SPI1};

constexpr uint8_t PANEL_COUNT_MAX_PER_REGION_ROW = PANEL_COUNT_MAX_PER_ARENA_ROW;
constexpr uint8_t PANEL_COUNT_MAX_PER_REGION_COL = \
  PANEL_COUNT_MAX_PER_ARENA_COL/REGION_COUNT_PER_ARENA; // 3

constexpr uint8_t PANEL_CLOCK_SELECT_PINS[REGION_COUNT_PER_ARENA][PANEL_COUNT_MAX_PER_REGION_ROW][PANEL_COUNT_MAX_PER_REGION_COL] =
{
  {
    {5, 10, 24},
    {6, 11, 25},
    {7, 21, 26},
    {8, 22, 27},
    {9, 23, 28}
  },
  {
    {5, 10, 24},
    {6, 11, 25},
    {7, 21, 26},
    {8, 22, 27},
    {9, 23, 28},
  }
};
}
}
#endif
#endif
