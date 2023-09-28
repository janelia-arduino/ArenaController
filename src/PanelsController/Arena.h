// ----------------------------------------------------------------------------
// Arena.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_ARENA_H
#define PANELS_CONTROLLER_ARENA_H

#include <Arduino.h>
#include <SPI.h>

#include "Constants.h"
#include "Region.h"
#include "TransferTracker.h"


class Arena
{
public:
  Arena();

  void setup();
  void update();

private:
  const SPISettings spi_settings_;
  Region regions_[panels_controller::constants::region_count_per_arena];

  void setupPins();
  void setupRegions();
  void transferRegions();
  void beginTransferPanels();
  void endTransferPanels();
  void transferPanels(uint8_t row_index, uint8_t col_index);
};

#endif
