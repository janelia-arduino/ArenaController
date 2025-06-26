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
  // void writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count);
  // void readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count);
  // pattern::TpaHeader tpa_header_;
private:
  FsFile file_;
  uint64_t position_;
  PatternHeader header_;
};
}
#endif
