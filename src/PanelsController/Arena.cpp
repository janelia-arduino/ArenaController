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
  TransferTracker::setup();
}

void Arena::update()
{
  transferRegions();
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
  for (uint8_t region_index = 0; region_index<constants::region_count_per_arena; ++region_index)
  {
    regions_[region_index].setup(constants::region_spi_ptrs[region_index]);
  }
}

void Arena::beginTransferPanels()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_arena; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Arena::endTransferPanels()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_arena; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Arena::transferRegions()
{
  for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
    {
      beginTransferPanels();
      transferPanels(row_index, col_index);
      endTransferPanels();
    }
  }
}

void Arena::transferPanels(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::region_count_per_arena; ++region_index)
  {
    regions_[region_index].transferPanel(row_index, col_index);
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}
