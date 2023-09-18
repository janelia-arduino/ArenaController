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


// bool spi_finished[panel_controller::constants::SPI_COUNT_PER_ARENA];

class PanelsController
{
public:
  PanelsController();

  void setup();
  void update();

  void transferFrameSynchronously();
  void transferFrameAsynchronously();
private:
  uint8_t panel_row_index_;
  uint8_t panel_col_index_;
  const SPISettings spi_settings_;
  uint8_t output_buffers_[panels_controller::constants::REGION_COUNT_PER_ARENA][panels_controller::constants::BYTE_COUNT_PER_PANEL_GRAYSCALE];
  uint8_t input_buffers_[panels_controller::constants::REGION_COUNT_PER_ARENA][panels_controller::constants::BYTE_COUNT_PER_PANEL_GRAYSCALE];
  EventResponder event_responder_;

  void setupPanelClockSelectPins();
  void setupOutputBuffers();
  void transferPanelSynchronously();
};

#endif
