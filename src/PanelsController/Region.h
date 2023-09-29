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


class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransferPanel(SPISettings spi_settings);
  void endTransferPanel();
  void transferPanel(uint8_t * panel_buffer_ptr, uint8_t panel_buffer_byte_count);
private:
  SPIClass * spi_ptr_;
};

#endif
