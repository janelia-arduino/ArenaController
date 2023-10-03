// Card.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Card.h"


using namespace panels_controller;

void Card::setup()
{
  sd_.begin(SD_CONFIG);

  sprintf(file_name_, "f%d_c%d_r%d_g%d",
    constants::frame_count,
    constants::panel_count_max_per_region_col,
    constants::panel_count_max_per_region_row,
    constants::region_count_per_frame);

  mkdirDisplay();
  chdirDisplay();
}

void Card::openFileForWriting()
{
  file_.remove(file_name_);
  file_.preAllocate(constants::file_length);
  file_.open(file_name_, O_WRITE | O_CREAT | O_APPEND);
}

void Card::openFileForReading()
{
  file_.open(file_name_);
  file_.rewind();
  file_position_ = 0;
}

void Card::closeFile()
{
  file_.close();
}

void Card::writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.write(panel_buffer, panel_byte_count);
  file_.sync();
}

void Card::readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.read(panel_buffer, panel_byte_count);
  file_position_ = file_position_ + panel_byte_count;
  file_.seekSet(file_position_);
}

void Card::mkdirDisplay()
{
  sd_.mkdir(constants::directory);
}

void Card::chdirDisplay()
{
  sd_.chdir(constants::directory);
}
