// ----------------------------------------------------------------------------
// PanelsController.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_H
#define PANELS_CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>

#include "Constants.h"


class PanelsController
{
public:
  PanelsController();
  void setup();
  void update();
private:
  uint8_t panel_row_index_;
  uint8_t panel_col_index_;
  const SPISettings spi_settings_;
};

#endif
