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
constexpr SPIClass * PANEL_SPI_PTRS[PANEL_COUNT_MAX_PER_ARENA_ROW][PANEL_COUNT_MAX_PER_ARENA_COL] =
{
  {&SPI, &SPI, &SPI, &SPI1, &SPI1, &SPI1},
  {&SPI, &SPI, &SPI, &SPI1, &SPI1, &SPI1},
  {&SPI, &SPI, &SPI, &SPI1, &SPI1, &SPI1},
  {&SPI, &SPI, &SPI, &SPI1, &SPI1, &SPI1},
  {&SPI, &SPI, &SPI, &SPI1, &SPI1, &SPI1}
};

constexpr uint8_t PANEL_CLOCK_SELECT_PINS[PANEL_COUNT_MAX_PER_ARENA_ROW][PANEL_COUNT_MAX_PER_ARENA_COL] =
{
  {5, 10, 24, 5, 10, 24},
  {6, 11, 25, 6, 11, 25},
  {7, 21, 26, 7, 21, 26},
  {8, 22, 27, 8, 22, 27},
  {9, 23, 28, 9, 23, 28}
};
}
}

#endif
#endif
