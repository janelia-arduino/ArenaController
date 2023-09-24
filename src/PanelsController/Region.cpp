// Region.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Region.h"


using namespace panels_controller;

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

