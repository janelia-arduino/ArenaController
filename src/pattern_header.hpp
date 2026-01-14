#ifndef PATTERN_HEADER_HPP
#define PATTERN_HEADER_HPP

namespace AC
{
union PatternHeader
{
  struct
  {
    uint64_t frame_count_x : 16;
    uint64_t frame_count_y : 16;
    uint64_t grayscale_value : 8;
    uint64_t panel_count_per_frame_row : 8;
    uint64_t panel_count_per_frame_col : 8;
  };
  uint64_t bytes;
};
} // namespace AC
#endif
