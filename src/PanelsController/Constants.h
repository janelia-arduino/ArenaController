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
// pixels in message byte
constexpr uint8_t BIT_COUNT_PER_BYTE = 8;

constexpr uint8_t BIT_COUNT_PER_PIXEL_GRAYSCALE = 4;
constexpr uint8_t BIT_COUNT_PER_PIXEL_BINARY = 1;

constexpr uint8_t PIXEL_COUNT_PER_BYTE_GRAYSCALE = BIT_COUNT_PER_BYTE / BIT_COUNT_PER_PIXEL_GRAYSCALE;
constexpr uint8_t PIXEL_COUNT_PER_BYTE_BINARY = BIT_COUNT_PER_BYTE / BIT_COUNT_PER_PIXEL_BINARY;

// pixels in quarter panel
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL_ROW = 8;
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL_COL = 8;
constexpr uint8_t PIXEL_COUNT_PER_QUARTER_PANEL = PIXEL_COUNT_PER_QUARTER_PANEL_ROW * PIXEL_COUNT_PER_QUARTER_PANEL_COL;

// message bytes per quarter panel
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_CONTROL = 1;
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_GRAYSCALE = BYTE_COUNT_PER_QUARTER_PANEL_CONTROL + PIXEL_COUNT_PER_QUARTER_PANEL/PIXEL_COUNT_PER_BYTE_GRAYSCALE;
constexpr uint8_t BYTE_COUNT_PER_QUARTER_PANEL_BINARY = BYTE_COUNT_PER_QUARTER_PANEL_CONTROL + PIXEL_COUNT_PER_QUARTER_PANEL/PIXEL_COUNT_PER_BYTE_BINARY;

// panel
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL_ROW = 2;
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL_COL = 2;
constexpr uint8_t QUARTER_PANEL_COUNT_PER_PANEL = QUARTER_PANEL_COUNT_PER_PANEL_ROW * QUARTER_PANEL_COUNT_PER_PANEL_COL;

constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_ROW = 5;
constexpr uint8_t PANEL_COUNT_MAX_PER_ARENA_COL = 6;
}
}
#include "TEENSY41.h"
#endif
