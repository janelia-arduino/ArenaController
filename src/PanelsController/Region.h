// ----------------------------------------------------------------------------
// Region.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_REGION_H
#define PANELS_CONTROLLER_REGION_H

#include <SPI.h>

#include "Constants.h"
#include "TransferTracker.h"
#include "Patterns.h"


class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransferPanel(SPISettings spi_settings);
  void endTransferPanel();
  void transferPanel(uint8_t row_index, uint8_t col_index);
private:
  // uint8_t output_buffer_[panels_controller::constants::BYTE_COUNT_PER_PANEL_GRAYSCALE];
  SPIClass * spi_ptr_;
};

#endif
