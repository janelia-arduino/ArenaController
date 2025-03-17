#ifndef BSP_HPP
#define BSP_HPP
#include "constants.hpp"


struct BSP {
  static void init();

  static void ledOff();
  static void ledOn();

  static void initializeWatchdog();
  static void feedWatchdog();

  static void initializeArena();
  static void initializeDisplay();
  static void initializeFrame();

  static String processStringCommand(String command);

  static void beginSerial();
  static void pollSerialCommand();
  static uint8_t readSerialByte();
  static String readSerialStringCommand(uint8_t first_byte);
  static void writeSerialStringResponse(String response);

  static void beginEthernet();
  static void checkForEthernetIPAddress();
  static void beginEthernetServer();
  static void pollEthernetCommand();

  static void armDisplayFrameTimer(uint32_t frequency_hz);
  static void disarmDisplayFrameTimer();
  static void displayFrame();
  static void setDisplayFrequency(uint32_t frequency_hz);

  static uint8_t getRegionRowPanelCountMax();
  static uint8_t getRegionColPanelCountMax();

  static void enablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void disablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void transferPanelSet(const uint8_t (*panel_buffer)[], uint8_t panel_buffer_byte_count);

};

#endif // BSP_HPP
