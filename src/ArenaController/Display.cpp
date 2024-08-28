// ----------------------------------------------------------------------------
// Display.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Display.hpp"


using namespace arena_controller;

Display::Display() :
spi_settings_(SPISettings(constants::spi_clock_speed, constants::spi_bit_order, constants::spi_data_mode))
{}

void Display::setSpiClockSpeed(uint32_t spi_clock_speed)
{
  spi_settings_ = SPISettings(spi_clock_speed, constants::spi_bit_order, constants::spi_data_mode);
}

void Display::showFrame()
{
  beginTransferFrame();
  transferFrame(storage_ptr_->tpa_header_.panel_count_per_frame_col/constants::region_count_per_frame, storage_ptr_->tpa_header_.panel_count_per_frame_row);
  endTransferFrame(storage_ptr_->tpa_header_.frame_count_x);
}

void Display::setup(Storage & storage)
{
  storage_ptr_ = &storage;
  setupPins();
  setupRegions();
  TransferTracker::setup();
  frame_index_ = 0;
}

void Display::setupPins()
{
  pinMode(constants::reset_pin, OUTPUT);
  digitalWriteFast(constants::reset_pin, LOW);

  for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
    {
      const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
  }
}

void Display::setupRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].setup(constants::region_spi_ptrs[region_index]);
  }
}

void Display::beginTransferFrame()
{
  // if (frame_index_ == 0)
  // {
    storage_ptr_->rewindTpaFileForReading();
  // }
}

void Display::endTransferFrame(uint16_t frame_count)
{
  if (++frame_index_ == frame_count)
  {
    frame_index_ = 0;
  };
}

void Display::transferFrame(uint8_t panel_count_per_region_col, uint8_t panel_count_per_region_row)
{
  for (uint8_t col_index = 0; col_index<panel_count_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<panel_count_per_region_row; ++row_index)
    {
      beginTransferPanelsAcrossRegions();
      transferPanelsAcrossRegions(row_index, col_index);
      endTransferPanelsAcrossRegions();
    }
  }
}

void Display::beginTransferPanelsAcrossRegions()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Display::endTransferPanelsAcrossRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Display::transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    storage_ptr_->readPanelFromFile(panel_buffer_[region_index], constants::byte_count_per_panel_grayscale);
    regions_[region_index].transferPanel(panel_buffer_[region_index], constants::byte_count_per_panel_grayscale);
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}
