// ----------------------------------------------------------------------------
// Constants.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_CONSTANTS_HPP
#define PANELS_CONTROLLER_CONSTANTS_HPP


namespace panels_controller
{
namespace constants
{
// Serial Settings
constexpr uint32_t baud_rate = 2000000;

// SPI Settings
constexpr uint32_t spi_clock = 5000000;
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

// frame
constexpr uint8_t panel_count_max_per_frame_row = 5;
constexpr uint8_t panel_count_max_per_frame_col = 6;
constexpr uint8_t panel_count_max_per_frame = \
  panel_count_max_per_frame_row * panel_count_max_per_frame_col; // 30
constexpr uint16_t byte_count_max_per_frame_grayscale = \
  panel_count_max_per_frame * \
  byte_count_per_panel_grayscale; // 3960

// frames
constexpr uint8_t frame_count = 100;
constexpr uint8_t half_frame_count = frame_count / 2;

// files
constexpr uint8_t file_name_size_max = 24;
constexpr uint64_t file_length = \
  frame_count *\
  byte_count_max_per_frame_grayscale; // 396000

const char directory[] = "show";

}
}
#include "ConstantsTeensy41.hpp"
#endif
