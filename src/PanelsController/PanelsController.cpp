// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

void Region::setup(SPIClass * spi_ptr)
{
  spi_ptr_ = spi_ptr;
  spi_ptr_->begin();

  for (uint8_t b = 0; b<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++b)
  {
    output_buffer_[b] = 1;
  }
}

void Region::beginTransaction(SPISettings spi_settings)
{
  spi_ptr_->beginTransaction(spi_settings);
}

void Region::endTransaction()
{
  spi_ptr_->endTransaction();
}

void Region::transfer()
{
    spi_ptr_->transfer(output_buffer_, NULL, constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, transferred_event_);
}

bool Region::transferComplete()
{
  return transfer_complete_;
}

Arena::Arena() :
spi_settings_(SPISettings(constants::SPI_CLOCK, constants::SPI_BIT_ORDER, constants::SPI_DATA_MODE))
{}

void Arena::setup()
{
  setupPanelSelectPins();
  setupRegions();
}

void Arena::update()
{
  transferRegions();
}

void Arena::setupPanelSelectPins()
{
  for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++c)
  {
    for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++r)
    {
      const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[r][c];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
  }
}

void Arena::setupRegions()
{
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    regions_[g].setup(constants::REGION_SPI_PTRS[g]);
  }
}

void Arena::beginTransactions()
{
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    regions_[g].beginTransaction(spi_settings_);
  }
}

void Arena::endTransactions()
{
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    regions_[g].endTransaction();
  }
}

void Arena::transferRegions()
{
  for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++c)
  {
    for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++r)
    {
      beginTransactions();
      transferPanels(r, c);
      endTransactions();
    }
  }
}

void Arena::transferPanels(uint8_t r, uint8_t c)
{
  const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[r][c];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    regions_[g].transfer();
  }

  delayMicroseconds(300);
  // while (not allTransfersComplete())
  // {
  //   yield();
  // }

  digitalWriteFast(cs_pin, HIGH);
}

bool Arena::allTransfersComplete()
{
  bool all_transfers_complete = true;
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    all_transfers_complete = all_transfers_complete and regions_[g].transferComplete();
  }
  return all_transfers_complete;
}

void PanelsController::setup()
{
  Serial.begin(115200);

  arena_.setup();
}

void PanelsController::update()
{
  arena_.update();
}
