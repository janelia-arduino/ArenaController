// ----------------------------------------------------------------------------
// Pat.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_PAT_HPP
#define ARENA_CONTROLLER_PAT_HPP

#include <Arduino.h>

#include "Constants.hpp"


namespace arena_controller
{
namespace pat
{
struct Header
{
  unsigned int frame_count_x : 16;
  unsigned int frame_count_y : 16;
  unsigned int grayscale_value : 8;
  unsigned int row_count : 8;
  unsigned int col_count : 8;
};
constexpr uint8_t header_size = 7;

struct RowQuadrant
{
  uint8_t row;
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

struct Pat
{
};
}
}
#endif
