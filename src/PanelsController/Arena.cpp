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
spi_settings_(SPISettings(constants::SPI_CLOCK, constants::SPI_BIT_ORDER, constants::SPI_DATA_MODE))
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
  pinMode(constants::RESET_PIN, OUTPUT);
  digitalWriteFast(constants::RESET_PIN, LOW);

  for (uint8_t col_index = 0; col_index<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++row_index)
    {
      const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[row_index][col_index];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
  }
}

void Arena::setupRegions()
{
  for (uint8_t region_index = 0; region_index<constants::REGION_COUNT_PER_ARENA; ++region_index)
  {
    regions_[region_index].setup(constants::REGION_SPI_PTRS[region_index]);
  }
}

void Arena::beginTransferPanels()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::REGION_COUNT_PER_ARENA; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Arena::endTransferPanels()
{
  for (uint8_t region_index = 0; region_index<constants::REGION_COUNT_PER_ARENA; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Arena::transferRegions()
{
  for (uint8_t col_index = 0; col_index<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++row_index)
    {
      beginTransferPanels();
      transferPanels(row_index, col_index);
      endTransferPanels();
    }
  }
}

void Arena::transferPanels(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::REGION_COUNT_PER_ARENA; ++region_index)
  {
    regions_[region_index].transferPanel();
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}
