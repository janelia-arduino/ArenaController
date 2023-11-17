// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.hpp"


using namespace panels_controller;

void PanelsController::setup()
{
  Serial.begin(115200);

  display_.setup();
}

void PanelsController::writeFramesToStorage()
{
  display_.writeFramesToStorage();
}

void PanelsController::showFrameFromStorage()
{
  display_.showFrameFromStorage();
}

void PanelsController::showFrameFromRAM()
{
  display_.showFrameFromRAM();
}
