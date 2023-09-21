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


// volatile bool event_happened = false;
// void asyncEventResponder(EventResponderRef event_responder)
// {
//   digitalWriteFast(CS_PIN, HIGH);
//   event_happened = true;
// }
class TransferTracker
{
public:
  static void setup();
  static EventResponderRef getPanelTransferCompleteEvent();
  static void beginPanelTransfers();
  static bool allPanelTransfersComplete();
  static void transferCompleteCallback(EventResponderRef event_responder);
private:
  static EventResponder panel_transfer_complete_event_;
  static uint8_t panel_transfer_complete_count_;
};

class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransaction(SPISettings spi_settings);
  void endTransaction();
  void transfer();
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
  void beginTransactions();
  void endTransactions();
  void transferRegions();
  void transferPanels(uint8_t r, uint8_t c);
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
