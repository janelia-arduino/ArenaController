#include "pattern.hpp"


using namespace AC;

Pattern::Pattern()
{
  valid_ = false;
}

bool Pattern::initializeCard()
{
  // return SD.sdfs.begin(SdioConfig(DMA_SDIO));
  return SD.sdfs.begin(SdioConfig(FIFO_SDIO));
}

uint64_t Pattern::openFileForReading(uint16_t pattern_id)
{
  valid_ = false;
  char filename_str[constants::filename_str_len];
  sprintf(filename_str, "pat%0*d.pat", constants::pattern_id_str_len, pattern_id);
  file_ = SD.sdfs.open(filename_str, O_RDONLY);
  file_size_ = file_.fileSize();
  return file_size_;
}

void Pattern::setValid()
{
  valid_ = true;
}

bool Pattern::isValid()
{
  return valid_;
}

PatternHeader & Pattern::rewindFileReadHeader()
{
  file_.rewind();
  file_.read(&header_, constants::pattern_header_size);
  file_position_ = constants::pattern_header_size;
  file_.seekSet(file_position_);
  return header_;
}

void Pattern::setByteCountPerFrame(uint16_t byte_count_per_frame)
{
  byte_count_per_frame_ = byte_count_per_frame;
}

void Pattern::closeFile()
{
  file_.close();
}

void Pattern::readNextFrameIntoBufferFromFile(uint8_t * buffer)
{
  file_.read(buffer, byte_count_per_frame_);
  file_position_ = file_position_ + byte_count_per_frame_;
  if (file_position_ > (file_size_ - byte_count_per_frame_))
  {
    rewindFileReadHeader();
  }
  else
  {
    file_.seekSet(file_position_);
  }
}

uint64_t Pattern::fileSize()
{
  return file_size_;
}

uint64_t Pattern::filePosition()
{
  return file_position_;
}

