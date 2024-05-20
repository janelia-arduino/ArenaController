// ----------------------------------------------------------------------------
// Display.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_DISPLAY_HPP
#define ARENA_CONTROLLER_DISPLAY_HPP

#include <Arduino.h>
#include <SPI.h>
#include <NativeEthernet.h>
#include <qpcpp.hpp>

#include "Constants.hpp"
#include "Region.hpp"
#include "TransferTracker.hpp"
#include "Patterns.hpp"
#include "Storage.hpp"

using namespace QP;

class Display
{
public:
  Display();
  void setup();
  void setupFileFromStorage();
  void writeFramesToStorage();
  void showFrameFromStorage();
  void showFrameFromRAM();
private:
  const SPISettings spi_settings_;
  Region regions_[arena_controller::constants::region_count_per_frame];
  uint8_t frame_index_;
  Storage storage_;
  uint8_t panel_buffer_[arena_controller::constants::byte_count_per_panel_grayscale];
  bool show_from_storage_;

  void setupSerial();
  void setupPins();
  void setupRegions();
  void setupStorage();
  void setupEthernet();

  void getMacAddress(uint8_t * mac_address);

  void beginTransferFrame();
  void endTransferFrame();
  void transferFrame();
  void beginTransferPanelsAcrossRegions();
  void endTransferPanelsAcrossRegions();
  void transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index);
};

#endif
