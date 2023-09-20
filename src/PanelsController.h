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


class Region
{
public:
  void setup(SPIClass * spi_ptr);
  void beginTransaction(SPISettings spi_settings);
  void endTransaction();
  void transfer();
  bool transferComplete();
private:
  uint8_t output_buffer_[panels_controller::constants::BYTE_COUNT_PER_PANEL_GRAYSCALE];
  EventResponder transferred_event_;
  SPIClass * spi_ptr_;
  volatile bool transfer_complete_;
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
  bool allTransfersComplete();
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
