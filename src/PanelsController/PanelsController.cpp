// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

void TransferTracker::setup()
{
  transfer_panel_complete_event_.attachImmediate(&TransferTracker::transferPanelCompleteCallback);
}

EventResponderRef TransferTracker::getTransferPanelCompleteEvent()
{
  return transfer_panel_complete_event_;
}

void TransferTracker::beginTransferPanels()
{
  transfer_panel_complete_count_ = 0;
}

void TransferTracker::endTransferPanels()
{
}

bool TransferTracker::allTransferPanelsComplete()
{
  return transfer_panel_complete_count_ == constants::REGION_COUNT_PER_ARENA;
}

void TransferTracker::transferPanelCompleteCallback(EventResponderRef event_responder)
{
  ++transfer_panel_complete_count_;
}

EventResponder TransferTracker::transfer_panel_complete_event_;
uint8_t TransferTracker::transfer_panel_complete_count_ = 0;

void Region::setup(SPIClass * spi_ptr)
{
  spi_ptr_ = spi_ptr;
  spi_ptr_->begin();

  for (uint8_t byte_index = 0; byte_index<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++byte_index)
  {
    if ((byte_index == 0) || (byte_index == 33) || (byte_index == 66) || (byte_index == 99))
    {
      output_buffer_[byte_index] = 1;
    }
    else
    {
      output_buffer_[byte_index] = 25;
    }
  }
}

void Region::beginTransferPanel(SPISettings spi_settings)
{
  spi_ptr_->beginTransaction(spi_settings);
}

void Region::endTransferPanel()
{
  spi_ptr_->endTransaction();
}

void Region::transferPanel()
{
  spi_ptr_->transfer(output_buffer_, NULL, constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, TransferTracker::getTransferPanelCompleteEvent());
}

Arena::Arena() :
spi_settings_(SPISettings(constants::SPI_CLOCK, constants::SPI_BIT_ORDER, constants::SPI_DATA_MODE))
{}

void Arena::setup()
{
  setupPanelSelectPins();
  setupRegions();
  TransferTracker::setup();
}

void Arena::update()
{
  transferRegions();
}

void Arena::setupPanelSelectPins()
{
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

void PanelsController::setup()
{
  Serial.begin(115200);

  arena_.setup();
}

void PanelsController::update()
{
  arena_.update();
}
