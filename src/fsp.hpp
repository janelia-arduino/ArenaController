#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>

#include "constants.hpp"
#include "bsp.hpp"
#include "signals.hpp"

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
  static void Arena_displayAllOnFrames(QP::QActive * const ao, QP::QEvt const * e);

  static void Display_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_setDisplayFrequency(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_displayFrames(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_armDisplayFrameTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_disarmDisplayFrameTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Display_transferFrame(QP::QActive * const ao, QP::QEvt const * e);

  static void SerialCommandInterface_subscribe(QP::QActive * const ao, QP::QEvt const * e);
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

  static void EthernetCommandInterface_subscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_armEthernetTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_disarmEthernetTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_beginEthernet(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_checkForIPAddress(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_beginServer(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_checkForClient(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_pollEthernetCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_readEthernetBinaryCommand(QP::QActive * const ao, QP::QEvt const * e);
  static void EthernetCommandInterface_writeEthernetBinaryResponse(QP::QActive * const ao, QP::QEvt const * e);

  static void Frame_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_resetIndicies(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_beginTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_endTransferPanelSet(QP::QActive * const ao, QP::QEvt const * e);
  static bool Frame_ifFrameNotTransferred(QP::QActive * const ao, QP::QEvt const * e);
  static void Frame_publishFrameTransferred(QP::QActive * const ao, QP::QEvt const * e);

  static void Watchdog_initializeAndSubscribe(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_armWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_disarmWatchdogTimer(QP::QActive * const ao, QP::QEvt const * e);
  static void Watchdog_feedWatchdog(QP::QActive * const ao, QP::QEvt const * e);

  static String processStringCommand(String command);
};

#endif // FSP_HPP
