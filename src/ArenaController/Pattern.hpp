// ----------------------------------------------------------------------------
// Pattern.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_CONSTANTS_PATTERN_HPP
#define ARENA_CONTROLLER_CONSTANTS_PATTERN_HPP

#include "Constants.hpp"


namespace arena_controller
{
namespace pattern
{
constexpr uint8_t all_on_grayscale_panel[constants::byte_count_per_panel_grayscale] =
{
  1,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  1,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  1,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  1,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
};

constexpr uint8_t all_off_grayscale_panel[constants::byte_count_per_panel_grayscale] =
{
  1,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};

constexpr uint8_t identify_quarter_panels[constants::byte_count_per_panel_grayscale] =
{
  1,
  15, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  15, 0, 15, 0,
  15, 0, 15, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  255, 0, 15, 0,
  15, 0, 255, 0,
  255, 0, 15, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  1,
  255, 0, 255, 0,
  255, 0, 255, 0,
  255, 0, 255, 0,
  255, 0, 255, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
};


struct PatHeader
{
  unsigned int frame_count_x : 16;
  unsigned int frame_count_y : 16;
  unsigned int grayscale_value : 8;
  unsigned int row_count : 8;
  unsigned int col_count : 8;
};
constexpr uint8_t pat_header_size = 7;

struct QuarterPanel
{
  
};

struct RowQuadrant
{
  uint8_t row_signifier;
  uint8_t stretch_array[constants::panel_count_max_per_frame_col];
  unsigned int data : 3072;
};

struct Frame
{
  unsigned int panel_row : 8;
  unsigned int frame_count_y : 16;
  unsigned int grayscale_value : 8;
  unsigned int row_count : 8;
  unsigned int col_count : 8;
};

}
}
#endif
