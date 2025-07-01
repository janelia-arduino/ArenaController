#ifndef P_HPP
#define P_HPP

// #include <SD.h>
// #include <SPI.h>
// #include "constants.hpp"


// namespace AC
// {
// union PatternHeader
// {
//   struct
//   {
//     uint64_t frame_count_x : 16;
//     uint64_t frame_count_y : 16;
//     uint64_t grayscale_value : 8;
//     uint64_t panel_count_per_frame_row : 8;
//     uint64_t panel_count_per_frame_col : 8;
//   };
//   uint64_t bytes;
// };

// class Psdfsdfsd
// {
// public:
//   Pattern();
//   bool initializeCard();
//   uint64_t openFileForReading(uint16_t pattern_id);
//   void setValid();
//   bool isValid();
//   PatternHeader & rewindFileReadHeader();
//   void setByteCountPerFrame(uint16_t byte_count_per_frame);
//   void closeFile();
//   void readNextFrameIntoBufferFromFile(uint8_t * buffer);
//   uint64_t fileSize();
//   uint64_t filePosition();
// private:
//   bool valid_;
//   FsFile file_;
//   uint64_t file_size_;
//   uint64_t file_position_;
//   PatternHeader header_;
//   uint16_t byte_count_per_frame_;
// };
// }
#endif
