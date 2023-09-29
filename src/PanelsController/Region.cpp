// Region.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Region.h"


using namespace panels_controller;

void Region::setup(SPIClass * spi_ptr)
{
  spi_ptr_ = spi_ptr;
  spi_ptr_->begin();
}

void Region::beginTransferPanel(SPISettings spi_settings)
{
  spi_ptr_->beginTransaction(spi_settings);
}

void Region::endTransferPanel()
{
  spi_ptr_->endTransaction();
}

void Region::transferPanel(uint8_t * panel_buffer_ptr, uint8_t panel_buffer_byte_count)
{
  spi_ptr_->transfer(panel_buffer_ptr, NULL, panel_buffer_byte_count, TransferTracker::getTransferPanelCompleteEvent());
}
