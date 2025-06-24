#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>

#include "constants.hpp"
#include "bsp.hpp"
#include "signals.hpp"
#include "records.hpp"

#include "ArenaController.hpp"
#include "Arena.hpp"
#include "Display.hpp"
#include "SerialCommandInterface.hpp"
#include "EthernetCommandInterface.hpp"
#include "Frame.hpp"
#include "Watchdog.hpp"



struct FSP
{
  static void ArenaController_setup();

  static void Arena_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_activateCommandInterfaces(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_deactivateCommandInterfaces(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_deactivateDisplay(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_displayFrames(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_fillFrameBufferWithAllOn(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_fillFrameBufferWithStream(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_postNextFrameReady(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_beginDisplayingPattern(QP::QActive * const ao, QP::QEvt const * e);
  static void Arena_endDisplayingPattern(QP::QActive * const ao, QP::QEvt const * e);

  static void Display_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_setDisplayFrequency(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_armDisplayFrameTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_disarmDisplayFrameTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_transferFrame(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_defer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_recall(QP::QActive * const ao, QP::QEvt const * e);

  static void SerialCommandInterface_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_armSerialTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_disarmSerialTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_beginSerial(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_pollSerialCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_readFirstByte(QP::QActive * const ao, QP::QEvt const * e);
  static bool SerialCommandInterface_ifBinaryCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_readSerialStringCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_processStringCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_writeSerialStringResponse(QP::QActive * const ao, QP::QEvt const * e);
  static void SerialCommandInterface_writeSerialBinaryResponse(QP::QActive * const ao, QP::QEvt const * e);

  static void EthernetCommandInterface_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_armEthernetTimer(QP::QActive * const ao, QP::QEvt const * e);
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

  static void Frame_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_fillFrameBufferWithAllOn(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_fillFrameBufferWithStream(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_reset(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_beginTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_endTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static bool Frame_ifFrameNotTransferred(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_publishFrameTransferred(QP::QActive * const ao, QP::QEvt const * e);

  static void Watchdog_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_armWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_disarmWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_feedWatchdog(QP::QActive * const ao, QP::QEvt const * e);

  static uint8_t processBinaryCommand(uint8_t const * command_buffer,
    size_t command_byte_count,
    uint8_t response[AC::constants::byte_count_per_response_max]);
  static void processStreamCommand(uint8_t const * command_buffer, uint32_t command_byte_count);
  static void processStringCommand(const char * command, char * response);
};

#endif // FSP_HPP
