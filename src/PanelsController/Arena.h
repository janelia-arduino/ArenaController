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
#include "Patterns.h"
#include "Card.h"


class Arena
{
public:
  Arena();

  void setup();
  void update();

  void writeFramesToCard();
private:
  const SPISettings spi_settings_;
  Region regions_[panels_controller::constants::region_count_per_frame];
  uint8_t frame_index_;
  char file_name_[panels_controller::constants::file_name_size_max];
  Card card_;
  uint64_t file_position_;
  uint8_t panel_buffer_[panels_controller::constants::byte_count_per_panel_grayscale];

  void setupPins();
  void setupRegions();
  void setupCard();
  void beginTransferFrames();
  void endTransferFrames();
  void transferFrames();
  void beginTransferFrame();
  void endTransferFrame();
  void transferFrame();
  void beginTransferPanelsAcrossRegions();
  void endTransferPanelsAcrossRegions();
  void transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index);
};

#endif
