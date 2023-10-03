// ----------------------------------------------------------------------------
// Arena.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Arena.h"


using namespace panels_controller;

Arena::Arena() :
spi_settings_(SPISettings(constants::spi_clock, constants::spi_bit_order, constants::spi_data_mode))
{}

void Arena::setup()
{
  setupPins();
  setupRegions();
  setupCard();
  TransferTracker::setup();
  frame_index_ = 0;
  display_from_card_ = true;
}

void Arena::writeFramesToCard()
{
  card_.openFileForWriting();

  for (uint8_t frame_index = 0; frame_index<constants::frame_count; ++frame_index)
  {
    for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
    {
      for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
      {
        for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
        {
          if (frame_index < constants::half_frame_count)
          {
            card_.writePanelToFile(patterns::all_on, constants::byte_count_per_panel_grayscale);
          }
          else
          {
            card_.writePanelToFile(patterns::all_off, constants::byte_count_per_panel_grayscale);
          }
        }
      }
    }
  }
  card_.closeFile();
}

void Arena::displayFrameFromCard()
{
  beginTransferFrame();
  transferFrame();
  endTransferFrame();
}

void Arena::displayFrameFromRAM()
{
  display_from_card_ = false;
  beginTransferFrame();
  transferFrame();
  endTransferFrame();
}

void Arena::setupPins()
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

void Arena::setupRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].setup(constants::region_spi_ptrs[region_index]);
  }
}

void Arena::setupCard()
{
  card_.setup();
}

void Arena::beginTransferFrame()
{
  if (frame_index_ == 0)
  {
    card_.openFileForReading();
  }
}

void Arena::endTransferFrame()
{
  if (++frame_index_ == constants::frame_count)
  {
    frame_index_ = 0;
    card_.closeFile();
  };
}

void Arena::transferFrame()
{
  for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
    {
      beginTransferPanelsAcrossRegions();
      transferPanelsAcrossRegions(row_index, col_index);
      endTransferPanelsAcrossRegions();
    }
  }
}

void Arena::beginTransferPanelsAcrossRegions()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Arena::endTransferPanelsAcrossRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Arena::transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    if (display_from_card_)
    {
      card_.readPanelFromFile(panel_buffer_, constants::byte_count_per_panel_grayscale);
      regions_[region_index].transferPanel(panel_buffer_, constants::byte_count_per_panel_grayscale);
    }
    else
    {
      if (frame_index_ < constants::half_frame_count)
      {
        regions_[region_index].transferPanel(patterns::all_on, constants::byte_count_per_panel_grayscale);
      }
      else
      {
        regions_[region_index].transferPanel(patterns::all_off, constants::byte_count_per_panel_grayscale);
      }
    }
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}
