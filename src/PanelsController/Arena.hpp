// ----------------------------------------------------------------------------
// Arena.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_ARENA_HPP
#define PANELS_CONTROLLER_ARENA_HPP

#include <Arduino.h>
#include <SPI.h>
#include <NativeEthernet.h>

#include "Constants.hpp"
#include "Region.hpp"
#include "TransferTracker.hpp"
#include "Patterns.hpp"
#include "Card.hpp"


class Arena
{
public:
  Arena();
  void setup();
  void writeFramesToCard();
  void displayFrameFromCard();
  void displayFrameFromRAM();
private:
  const SPISettings spi_settings_;
  Region regions_[panels_controller::constants::region_count_per_frame];
  uint8_t frame_index_;
  Card card_;
  uint8_t panel_buffer_[panels_controller::constants::byte_count_per_panel_grayscale];
  bool display_from_card_;

  void setupPins();
  void setupRegions();
  void setupCard();

  void beginTransferFrame();
  void endTransferFrame();
  void transferFrame();
  void beginTransferPanelsAcrossRegions();
  void endTransferPanelsAcrossRegions();
  void transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index);
};

#endif
