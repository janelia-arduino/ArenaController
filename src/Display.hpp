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
// #include <NativeEthernet.h>
// #include <qpcpp.hpp>

#include "Storage.hpp"

#include "ArenaController/Constants.hpp"
#include "ArenaController/Region.hpp"
#include "ArenaController/TransferTracker.hpp"
#include "ArenaController/Pattern.hpp"

// using namespace QP;

class ArenaController;

namespace arena_controller
{
class Display
{
public:
  Display();
  void setSpiClockSpeed(uint32_t spi_clock_speed);
  void showFrame();
private:
  SPISettings spi_settings_;
  Region regions_[arena_controller::constants::region_count_per_frame];
  uint8_t frame_index_;
  Storage * storage_ptr_;
  uint8_t panel_buffer_[arena_controller::constants::region_count_per_frame][arena_controller::constants::byte_count_per_panel_grayscale];

  void setup(Storage & storage);
  void setupSerial();
  void setupPins();
  void setupRegions();
  // void setupEthernet();

  // void getMacAddress(uint8_t * mac_address);

  void beginTransferFrame();
  void endTransferFrame(uint16_t frame_count);
  void transferFrame(uint8_t panel_count_per_region_col, uint8_t panel_count_per_region_row);
  void beginTransferPanelsAcrossRegions();
  void endTransferPanelsAcrossRegions();
  void transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index);
  friend class ::ArenaController;
};
}
#endif
