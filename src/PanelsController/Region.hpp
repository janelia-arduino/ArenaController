// ----------------------------------------------------------------------------
// Region.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_REGION_HPP
#define PANELS_CONTROLLER_REGION_HPP

#include <SPI.h>

#include "Constants.hpp"
#include "TransferTracker.hpp"


class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransferPanel(SPISettings spi_settings);
  void endTransferPanel();
  void transferPanel(const uint8_t * panel_buffer_ptr, uint8_t panel_buffer_byte_count);
private:
  SPIClass * spi_ptr_;
};

#endif
