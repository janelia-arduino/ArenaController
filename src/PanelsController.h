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
#include <EventResponder.h>

#include "Constants.h"


class TransferTracker
{
public:
  static void setup();
  static EventResponderRef getTransferPanelCompleteEvent();
  static void beginTransferPanels();
  static void endTransferPanels();
  static bool allTransferPanelsComplete();
  static void transferPanelCompleteCallback(EventResponderRef event_responder);
private:
  static EventResponder transfer_panel_complete_event_;
  static uint8_t transfer_panel_complete_count_;
};

class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransferPanel(SPISettings spi_settings);
  void endTransferPanel();
  void transferPanel();
private:
  uint8_t output_buffer_[panels_controller::constants::BYTE_COUNT_PER_PANEL_GRAYSCALE];
  SPIClass * spi_ptr_;
};

class Arena
{
public:
  Arena();

  void setup();
  void update();

private:
  const SPISettings spi_settings_;
  Region regions_[panels_controller::constants::REGION_COUNT_PER_ARENA];

  void setupPanelSelectPins();
  void setupRegions();
  void transferRegions();
  void beginTransferPanels();
  void endTransferPanels();
  void transferPanels(uint8_t row_index, uint8_t col_index);
};

class PanelsController
{
public:
  void setup();
  void update();
private:
  Arena arena_;
};

#endif
