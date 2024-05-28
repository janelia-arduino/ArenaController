// Storage.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Storage.hpp"


using namespace arena_controller;

void Storage::listFiles()
{
  sd_.ls(LS_R);
}

void Storage::printFileInformation()
{
  ExFile file;
  dir_.rewind();
  while (file.openNext(&dir_, O_RDONLY))
  {
    file.printFileSize(&Serial);
    Serial.write(' ');
    file.printModifyDateTime(&Serial);
    Serial.write(' ');
    file.printName(&Serial);
    if (file.isDir())
    {
      // Indicate a directory.
      Serial.write('/');
    }
    Serial.println();
    file.close();
  }
  if (dir_.getError())
  {
    Serial.println("openNext failed");
  }
  else
  {
    Serial.println("Done!");
  }
}

void Storage::printFileHeaders()
{
  ExFile file;
  dir_.rewind();
  while (file.openNext(&dir_, O_RDONLY))
  {
    file.printName(&Serial);
    Serial.println("");
    file.rewind();
    patterns::Header header;
    file.read(&header, patterns::header_size);
    Serial.print("frame_count_x: ");
    Serial.println(header.frame_count_x);
    Serial.print("frame_count_y: ");
    Serial.println(header.frame_count_y);
    Serial.print("grayscale_value: ");
    Serial.println(header.grayscale_value);
    Serial.print("row_count: ");
    Serial.println(header.row_count);
    Serial.print("col_count: ");
    Serial.println(header.col_count);
    file.close();
    Serial.println("--");
  }
}

void Storage::printFileSizes()
{
  ExFile file;
  dir_.rewind();
  while (file.openNext(&dir_, O_RDONLY))
  {
    file.printName(&Serial);
    Serial.print(' ');
    file.printFileSize(&Serial);
    Serial.println("");
    file.rewind();
    patterns::Header header;
    file.read(&header, patterns::header_size);

    long file_size = 0;
    file_size += patterns::header_size;
    file_size += (1 + header.col_count + 32*header.col_count)*4*header.row_count*header.frame_count_x*header.frame_count_y;

    Serial.print("calculated file size: ");
    Serial.println(file_size);
    file.close();
    Serial.println("--");
  }
}

void Storage::openFileForWriting()
{
  file_.remove(file_name_);
  file_.preAllocate(constants::file_length);
  file_.open(file_name_, O_WRITE | O_CREAT | O_APPEND);
}

void Storage::openFileForReading()
{
  file_.open(file_name_);
}

void Storage::rewindFileForReading()
{
  file_.rewind();
  file_position_ = 0;
}

void Storage::closeFile()
{
  file_.close();
}

void Storage::writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.write(panel_buffer, panel_byte_count);
  file_.sync();
}

void Storage::readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.read(panel_buffer, panel_byte_count);
  file_position_ = file_position_ + panel_byte_count;
  file_.seekSet(file_position_);
}

void Storage::setup()
{
  sd_.begin(SD_CONFIG);

  dir_.open(constants::directory);

  // sprintf(file_name_, "f%d_c%d_r%d_g%d",
  //   constants::frame_count,
  //   constants::panel_count_max_per_region_col,
  //   constants::panel_count_max_per_region_row,
  //   constants::region_count_per_frame);

  // sd_.chdir(constants::directory);
  // mkdirShow();
  // chdirShow();
}
