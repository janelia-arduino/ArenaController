#ifndef BSP_HPP
#define BSP_HPP
#include "constants.hpp"
#include "pattern_header.hpp"


struct BSP {
  static void init();

  static void ledOff();
  static void ledOn();

  static void initializeWatchdog();
  static void feedWatchdog();

  static void initializeArena();

  static bool beginSerial();
  static bool pollSerialCommand();
  static uint8_t readSerialByte();
  static void readSerialStringCommand(char * const command_str,
    char first_char);
  static void writeSerialStringResponse(char * const response);

  static bool initializeEthernet();
  static void pollEthernet();
  static bool createEthernetServerConnection();
  static void writeEthernetBinaryResponse(void * const connection,
    uint8_t response[AC::constants::byte_count_per_response_max],
    uint8_t response_byte_count);

  static void armRefreshTimer(uint32_t frequency_hz,
    void (*isr)());
  static void disarmRefreshTimer();

  static void initializeFrame();
  static uint8_t * const getFrameBuffer();
  static uint8_t getPanelCountPerRegionRow();
  static uint8_t getPanelCountPerRegionCol();
  static uint8_t getRegionCountPerFrame();
  static uint8_t getPanelCountPerFrameRow();
  static uint8_t getPanelCountPerFrameCol();

  static void fillFrameBufferWithAllOn(uint8_t * const buffer,
    bool grayscale);
  static uint16_t decodePatternFrameBuffer(const uint8_t * const pattern_frame_buffer,
    bool grayscale);
  static void fillFrameBufferWithDecodedFrame(uint8_t * const buffer,
    bool grayscale);
  static void enablePanelSetSelectPin(uint8_t row_index,
    uint8_t col_index);
  static void disablePanelSetSelectPin(uint8_t row_index,
    uint8_t col_index);
  static void transferPanelSet(const uint8_t * const buffer,
    uint16_t & buffer_position,
    uint8_t panel_byte_count);

  static uint8_t * const getPatternFrameBuffer();
  static bool initializePatternCard();
  static uint64_t openPatternFileForReading(uint16_t pattern_id);
  static void closePatternFile();
  static AC::PatternHeader rewindPatternFileAndReadHeader();
  static void readNextPatternFrameFromFileIntoBuffer(uint8_t * buffer,
    uint64_t byte_count_per_pattern_frame,
    bool positive_direction);
  static uint64_t getByteCountPerPatternFrameGrayscale();
  static uint64_t getByteCountPerPatternFrameBinary();
};

#endif // BSP_HPP
