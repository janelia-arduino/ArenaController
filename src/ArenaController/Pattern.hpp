// ----------------------------------------------------------------------------
// Pattern.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_CONSTANTS_PATTERN_HPP
#define ARENA_CONTROLLER_CONSTANTS_PATTERN_HPP

#include <SdFat.h>

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
  unsigned int panel_count_row : 8;
  unsigned int panel_count_col : 8;
};
constexpr uint8_t pat_header_size = 7;

struct QuarterPanel
{
  uint8_t stretch;
  uint8_t data[constants::pixel_count_per_quarter_panel_col][constants::byte_count_per_quarter_panel_row_grayscale];
};

struct Panel
{
  QuarterPanel quarter_panels[constants::quarter_panel_count_per_panel_row][constants::quarter_panel_count_per_panel_col];
};

struct Frame
{
  Panel panels[constants::panel_count_max_per_frame_row][constants::panel_count_max_per_frame_col];
};

class Pattern
{
public:
  bool importFromPat(ExFile & file);
private:
  char filename_stem_[constants::filename_length_max];
  uint16_t frame_count_x_;
  uint16_t frame_count_y_;
  uint8_t grayscale_value_;
  uint8_t panel_count_row_;
  uint8_t panel_count_col_;
  Frame frames_[constants::frame_count_max_y][constants::frame_count_max_x];

  const char * getFilenameSuffix(const char * filename);
  void getFilenameStem(char * stem, const char * filename);
};

}
}
#endif
