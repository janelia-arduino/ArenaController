#include "pattern.hpp"


using namespace AC;

bool Pattern::initializeCard()
{
  return SD.sdfs.begin(SdioConfig(DMA_SDIO));
}

uint64_t Pattern::openFileForReading(uint16_t pattern_id)
{
  char filename_str[constants::filename_str_len];
  sprintf(filename_str, "pat%0*d.pat", constants::pattern_id_str_len, pattern_id);
  file_ = SD.sdfs.open(filename_str, O_RDONLY);
  return file_.fileSize();
}

PatternHeader & Pattern::rewindFileReadHeader()
{
  file_.rewind();
  file_.read(&header_, constants::pattern_header_size);
  position_ = constants::pattern_header_size;
  file_.seekSet(position_);
  return header_;
}

void Pattern::closeFile()
{
  file_.close();
}

// void Pattern::writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count)
// {
//   file_.write(panel_buffer, panel_byte_count);
//   file_.sync();
// }

// void Pattern::readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count)
// {
//   file_.read(panel_buffer, panel_byte_count);
//   file_position_ = file_position_ + panel_byte_count;
//   file_.seekSet(file_position_);
// }

// const char * Pattern::getFilenameSuffix(const char * filename)
// {
//   const char * dot = strrchr(filename, '.');
//   if(!dot || dot == filename) return "";
//   return dot + 1;
// }

// void Pattern::getFilenameStem(char * stem, const char * filename)
// {
//   const char * dot = strrchr(filename, '.');
//   if(!dot || dot == filename) return "";
//   strncpy(stem, filename, (strlen(filename) - strlen(dot)));
// }
