#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>

#include "constants.hpp"
#include "bsp.hpp"
#include "signals.hpp"
#include "records.hpp"
#include "pattern_header.hpp"

#include "Shared.hpp"
#include "Events.hpp"
#include "Arena.hpp"
#include "Analog.hpp"
#include "Display.hpp"
#include "SerialCommandInterface.hpp"
#include "EthernetCommandInterface.hpp"
#include "Frame.hpp"
#include "Watchdog.hpp"
#include "Pattern.hpp"
#include "Card.hpp"


struct FSP
{
  static void ArenaController_setup();

  static void Arena_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_activateCommandInterfaces(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_deactivateCommandInterfaces(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_deactivateDisplay(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_displayFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_fillFrameBufferWithAllOn(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_fillFrameBufferWithDecodedFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_endPlayingPattern(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_allOffTransition(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_allOnTransition(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_streamFrameTransition(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_playPatternTransition(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_initializeAnalog(QP::QActive * const ao, QP::QEvt const * e);

  static void Analog_initialize(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Analog_initializeOutput(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Analog_enterInitialized(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Analog_setOutput(QP::QHsm * const hsm, QP::QEvt const * e);

  static void Display_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_setRefreshRate(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_armRefreshTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_disarmRefreshTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_transferFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_defer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_recall(QP::QActive * const ao, QP::QEvt const * e);

  static void SerialCommandInterface_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_armSerialTimerLowSpeed(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_armSerialTimerHighSpeed(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_disarmSerialTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_initializeSerial(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_pollSerial(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_analyzeCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_processBinaryCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_writeBinaryResponse(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_writePatternFinishedResponse(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_updateStreamCommand(QP::QActive * const ao, QP::QEvt const * e);
  static bool SerialCommandInterface_ifStreamCommandComplete(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_processStreamCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_storeRuntimeDuration(QP::QActive * const ao, QP::QEvt const * e);

  static void EthernetCommandInterface_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_armEthernetTimerLowSpeed(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_armEthernetTimerHighSpeed(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_disarmEthernetTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_initializeEthernet(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_pollEthernet(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_createServerConnection(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_analyzeCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_updateStreamCommand(QP::QActive * const ao, QP::QEvt const * e);
  static bool EthernetCommandInterface_ifStreamCommandComplete(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_processBinaryCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_processStreamCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_writeBinaryResponse(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_writePatternFinishedResponse(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_storeRuntimeDuration(QP::QActive * const ao, QP::QEvt const * e);

  static void Frame_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_fillFrameBufferWithAllOn(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_fillFrameBufferWithDecodedFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_saveFrameReference(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_deleteFrameReference(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_reset(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_beginTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_endTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static bool Frame_ifFrameNotTransferred(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_publishFrameTransferred(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_switchGrayscale(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_defer(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_recall(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_setGrayscale(QP::QActive * const ao, QP::QEvt const * e);

  static void Watchdog_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_armWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_disarmWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_feedWatchdog(QP::QActive * const ao, QP::QEvt const * e);

  static void Pattern_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_checkAndStoreParameters(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_armFindCardTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_endRuntimeDuration(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_armTimers(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_disarmTimersAndCleanup(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_deactivateDisplay(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_readFrameFromFile(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_saveFrameReference(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_deleteFrameReference(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_decodeFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_fillFrameBufferWithDecodedFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_defer(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_recall(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_displayFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_initializeFrameIndex(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_setFrameCountPerPattern(QP::QActive * const ao, QP::QEvt const * e);
  static void Pattern_setByteCountPerFrame(QP::QActive * const ao, QP::QEvt const * e);

  static void Card_initialize(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_storeParameters(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_findCard(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_postAllOff(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_openFile(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_closeFile(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_checkFile(QP::QHsm * const hsm, QP::QEvt const * e);
  static void Card_checkPattern(QP::QHsm * const hsm, QP::QEvt const * e);

  static uint16_t frameIndexToAnalogValue(uint16_t frame_index, uint16_t frame_count_per_pattern);
  static void appendMessage(uint8_t* response, uint8_t& response_byte_count, const char* message);
  static uint8_t processBinaryCommand(uint8_t const * command_buffer,
    size_t command_byte_count,
    uint8_t response[AC::constants::byte_count_per_response_max]);
  static void processStreamCommand(uint8_t const * buffer, uint32_t frame_byte_count);
  static void processStringCommand(const char * command, char * response);
};

#endif // FSP_HPP
