// ----------------------------------------------------------------------------
// TransferTracker.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_TRANSFER_TRACKER_H
#define PANELS_CONTROLLER_TRANSFER_TRACKER_H

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

#endif
