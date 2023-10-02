// ----------------------------------------------------------------------------
// TransferTracker.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "TransferTracker.h"


using namespace panels_controller;

void TransferTracker::setup()
{
  transfer_panel_complete_event_.attachImmediate(&TransferTracker::transferPanelCompleteCallback);
}

EventResponderRef TransferTracker::getTransferPanelCompleteEvent()
{
  return transfer_panel_complete_event_;
}

void TransferTracker::beginTransferPanels()
{
  transfer_panel_complete_count_ = 0;
}

void TransferTracker::endTransferPanels()
{
}

bool TransferTracker::allTransferPanelsComplete()
{
  return transfer_panel_complete_count_ == constants::region_count_per_frame;
}

void TransferTracker::transferPanelCompleteCallback(EventResponderRef event_responder)
{
  ++transfer_panel_complete_count_;
}

EventResponder TransferTracker::transfer_panel_complete_event_;
uint8_t TransferTracker::transfer_panel_complete_count_ = 0;
