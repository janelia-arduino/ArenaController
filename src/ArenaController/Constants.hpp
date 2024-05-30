// ----------------------------------------------------------------------------
// Constants.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_CONSTANTS_HPP
#define ARENA_CONTROLLER_CONSTANTS_HPP


namespace arena_controller
{
namespace constants
{
// Serial Settings
constexpr uint32_t baud_rate = 2000000;

// SPI Settings
constexpr uint32_t spi_clock_speed = 5000000;
constexpr uint8_t spi_bit_order = MSBFIRST;
constexpr uint8_t spi_data_mode = SPI_MODE0;

// Ethernet Settings
constexpr uint8_t mac_address_size = 6;

// message byte
constexpr uint8_t bit_count_per_byte = 8;

// quarter panel pixels
constexpr uint8_t pixel_count_per_quarter_panel_row = 8;
constexpr uint8_t pixel_count_per_quarter_panel_col = 8;
constexpr uint8_t pixel_count_per_quarter_panel = \
  pixel_count_per_quarter_panel_row * pixel_count_per_quarter_panel_col; // 64

// quarter panel grayscale
constexpr uint8_t bit_count_per_pixel_grayscale = 4;
constexpr uint8_t pixel_count_per_byte_grayscale = \
  bit_count_per_byte / bit_count_per_pixel_grayscale; // 2

// quarter panel binary
constexpr uint8_t bit_count_per_pixel_binary = 1;
constexpr uint8_t pixel_count_per_byte_binary = \
  bit_count_per_byte / bit_count_per_pixel_binary; // 8

// quarter panel message bytes
constexpr uint8_t byte_count_per_quarter_panel_control = 1;
constexpr uint8_t byte_count_per_quarter_panel_grayscale = \
  byte_count_per_quarter_panel_control + \
  pixel_count_per_quarter_panel/pixel_count_per_byte_grayscale; // 33
constexpr uint8_t byte_count_per_quarter_panel_binary = \
  byte_count_per_quarter_panel_control + \
  pixel_count_per_quarter_panel/pixel_count_per_byte_binary; // 9

// panel
constexpr uint8_t quarter_panel_count_per_panel_row = 2;
constexpr uint8_t quarter_panel_count_per_panel_col = 2;
constexpr uint8_t quarter_panel_count_per_panel = \
  quarter_panel_count_per_panel_row * quarter_panel_count_per_panel_col; // 4

// panel message bytes
constexpr uint8_t byte_count_per_panel_grayscale = \
  byte_count_per_quarter_panel_grayscale * \
  quarter_panel_count_per_panel; // 132
constexpr uint8_t byte_count_per_panel_binary = \
  byte_count_per_quarter_panel_binary * \
  quarter_panel_count_per_panel; // 36

}
}
#include "ConstantsTeensy41.hpp"
#endif
