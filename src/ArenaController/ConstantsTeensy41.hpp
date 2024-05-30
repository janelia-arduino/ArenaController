// ----------------------------------------------------------------------------
// ConstantsTeensy41.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_CONSTANTS_TEENSY41_HPP
#define ARENA_CONTROLLER_CONSTANTS_TEENSY41_HPP
#include <SPI.h>

#include "Constants.hpp"


#if defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41)
namespace arena_controller
{
namespace constants
{
// constexpr uint8_t reset_pin = 37;
constexpr uint8_t reset_pin = 34;

// frame
constexpr uint8_t panel_count_max_per_frame_row = 5;
constexpr uint8_t panel_count_max_per_frame_col = 12;
constexpr uint8_t panel_count_max_per_frame = \
  panel_count_max_per_frame_row * panel_count_max_per_frame_col; // 60
constexpr uint16_t byte_count_max_per_frame_grayscale = \
  panel_count_max_per_frame * \
  byte_count_per_panel_grayscale; // 7920

// region
// constexpr uint8_t region_count_per_frame = 2;
// constexpr SPIClass * region_spi_ptrs[region_count_per_frame] = {&SPI, &SPI1};

// constexpr uint8_t panel_count_max_per_region_row = panel_count_max_per_frame_row;
// constexpr uint8_t panel_count_max_per_region_col = \
//   panel_count_max_per_frame_col/region_count_per_frame; // 3

// constexpr uint8_t panel_select_pins[panel_count_max_per_region_row][panel_count_max_per_region_col] =
// {
//   {3, 8, 32},
//   {4, 9, 33},
//   {5, 29, 34},
//   {6, 30, 35},
//   {7, 31, 36}
// };
constexpr uint8_t region_count_per_frame = 2;
constexpr SPIClass * region_spi_ptrs[region_count_per_frame] = {&SPI, &SPI1};

constexpr uint8_t panel_count_max_per_region_row = panel_count_max_per_frame_row;
constexpr uint8_t panel_count_max_per_region_col = \
  panel_count_max_per_frame_col/region_count_per_frame; // 6

constexpr uint8_t panel_select_pins[panel_count_max_per_region_row][panel_count_max_per_region_col] =
{
  {0, 6, 24, 31, 20, 39},
  {2, 7, 25, 32, 17, 38},
  {3, 8, 28, 23, 16, 37},
  {4, 9, 29, 22, 41, 36},
  {5, 10, 30, 21, 40, 35}
};

// files
const char base_dir_str[] = "patterns";

}
}
#endif
#endif
