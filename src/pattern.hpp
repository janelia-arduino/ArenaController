#ifndef PATTERN_HPP
#define PATTERN_HPP

#include <SD.h>
#include <SPI.h>
#include "constants.hpp"


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

class Pattern
{
public:
  bool initializeCard();
  uint64_t openFileForReading(uint16_t pattern_id);
  PatternHeader & rewindFileReadHeader();
  void closeFile();
  void readFrameIntoBufferFromFile(uint8_t * buffer,
    uint16_t byte_count_per_frame);
private:
  FsFile file_;
  uint64_t file_size_;
  uint64_t position_;
  PatternHeader header_;
};
}
#endif
