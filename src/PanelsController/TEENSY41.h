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
// SPI Settings
constexpr uint8_t SPI_COUNT_PER_ARENA = 2;
constexpr SPIClass * SPI_PTRS[SPI_COUNT_PER_ARENA] = {&SPI, &SPI1};

constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_SPI_ROW = PANEL_COUNT_MAX_PER_ARENA_ROW;
constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_SPI_COL = \
  PANEL_COUNT_MAX_PER_ARENA_COL/SPI_COUNT_PER_ARENA; // 3

constexpr uint8_t PANEL_CLOCK_SELECT_PINS[SPI_COUNT_PER_ARENA][PANEL_COUNT_MAX_PER_ARENA_SPI_ROW][PANEL_COUNT_MAX_PER_ARENA_SPI_COL] =
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
