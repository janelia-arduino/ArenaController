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

void Storage::printFileHeaders()
{
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

  // sprintf(file_name_, "f%d_c%d_r%d_g%d",
  //   constants::frame_count,
  //   constants::panel_count_max_per_region_col,
  //   constants::panel_count_max_per_region_row,
  //   constants::region_count_per_frame);

  sd_.chdir(constants::directory);
  // mkdirShow();
  // chdirShow();
}

void Storage::mkdirShow()
{
  sd_.mkdir(constants::directory);
}

void Storage::chdirShow()
{
  sd_.chdir(constants::directory);
}
