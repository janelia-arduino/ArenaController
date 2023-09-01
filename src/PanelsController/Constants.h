// ----------------------------------------------------------------------------
// Constants.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_CONSTANTS_H
#define PANELS_CONTROLLER_CONSTANTS_H


namespace panels_controller
{
namespace constants
{
// SPI Settings
constexpr uint32_t SPI_CLOCK = 4000000;
constexpr uint8_t SPI_BIT_ORDER = MSBFIRST;
constexpr uint8_t SPI_DATA_MODE = SPI_MODE0;

// message byte
constexpr uint8_t BIT_COUNT_PER_BYTE = 8;

// quarter panel pixels
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL_ROW = 8;
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL_COL = 8;
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL = \
  PIXEL_COUNT_PER_QUARTER_PANEL_ROW * PIXEL_COUNT_PER_QUARTER_PANEL_COL; // 64

// quarter panel grayscale
constexpr uint8_t BIT_COUNT_PER_PIXEL_GRAYSCALE = 4;
constexpr uint8_t PIXEL_COUNT_PER_BYTE_GRAYSCALE = \
  BIT_COUNT_PER_BYTE / BIT_COUNT_PER_PIXEL_GRAYSCALE; // 2

// quarter panel binary
constexpr uint8_t BIT_COUNT_PER_PIXEL_BINARY = 1;
constexpr uint8_t PIXEL_COUNT_PER_BYTE_BINARY = \
  BIT_COUNT_PER_BYTE / BIT_COUNT_PER_PIXEL_BINARY; // 8

// quarter panel message bytes
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_CONTROL = 1;
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_GRAYSCALE = \
  BYTE_COUNT_PER_QUARTER_PANEL_CONTROL + \
  PIXEL_COUNT_PER_QUARTER_PANEL/PIXEL_COUNT_PER_BYTE_GRAYSCALE; // 33
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_BINARY = \
  BYTE_COUNT_PER_QUARTER_PANEL_CONTROL + \
  PIXEL_COUNT_PER_QUARTER_PANEL/PIXEL_COUNT_PER_BYTE_BINARY; // 9

// panel
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL_ROW = 2;
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL_COL = 2;
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL = \
  QUARTER_PANEL_COUNT_PER_PANEL_ROW * QUARTER_PANEL_COUNT_PER_PANEL_COL; // 4

// arena
constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_ROW = 5;
constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_COL = 6;
constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA = \
  PANEL_COUNT_MAX_PER_ARENA_ROW * PANEL_COUNT_MAX_PER_ARENA_COL; // 30
constexpr uint16_t BYTE_COUNT_MAX_PER_ARENA_GRAYSCALE = \
  PANEL_COUNT_MAX_PER_ARENA * \
  BYTE_COUNT_PER_QUARTER_PANEL_GRAYSCALE * \
  QUARTER_PANEL_COUNT_PER_PANEL; // 3960
}
}
#include "TEENSY41.h"
#endif
