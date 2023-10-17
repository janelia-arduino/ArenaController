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

  arena_.setup();
}

void PanelsController::writeFramesToCard()
{
  arena_.writeFramesToCard();
}

void PanelsController::displayFrameFromCard()
{
  arena_.displayFrameFromCard();
}

void PanelsController::displayFrameFromRAM()
{
  arena_.displayFrameFromRAM();
}
