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

  // for (uint8_t byte_index = 0; byte_index<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++byte_index)
  // {
  //   if ((byte_index == 0) || (byte_index == 33) || (byte_index == 66) || (byte_index == 99))
  //   {
  //     output_buffer_[byte_index] = 255;
  //   }
  //   else
  //   {
  //     output_buffer_[byte_index] = 255;
  //   }
  // }
}

void Region::beginTransferPanel(SPISettings spi_settings)
{
  spi_ptr_->beginTransaction(spi_settings);
}

void Region::endTransferPanel()
{
  spi_ptr_->endTransaction();
}

void Region::transferPanel(uint8_t row_index, uint8_t col_index)
{
  // spi_ptr_->transfer(output_buffer_, NULL, constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, TransferTracker::getTransferPanelCompleteEvent());
  spi_ptr_->transfer(pattern, NULL, constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, TransferTracker::getTransferPanelCompleteEvent());
}

