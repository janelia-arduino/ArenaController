#include <Arduino.h>
#include <Watchdog_t4.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <SPI.h>
#include <EventResponder.h>
#include <SdFat.h>
#include <Wire.h>
#include <Adafruit_MCP4728.h>

#include "ArenaController.hpp"

extern "C"
{
#include "mongoose_glue.h"
}

using namespace QP;
using namespace AC;

namespace AC
{
namespace constants
{
// QS Serial Pins
constexpr uint8_t qs_serial_stream_tx_pin = 53;
constexpr uint8_t qs_serial_stream_rx_pin = 52;

// SPI Settings
constexpr uint8_t spi_bit_order = MSBFIRST;
constexpr uint8_t spi_data_mode = SPI_MODE0;
constexpr uint32_t spi_clock_speed = 4000000;

constexpr uint8_t reset_pin = 34;

// frame
constexpr uint8_t panel_count_per_frame_row_max_bsp = 5;
constexpr uint8_t panel_count_per_frame_col_max_bsp = 12;
constexpr uint8_t panel_count_per_frame_row = 2;
constexpr uint8_t panel_count_per_frame_col = 12;

// region
constexpr uint8_t region_count_per_frame = 2;
constexpr SPIClass *region_spi_ptrs[region_count_per_frame] = {&SPI, &SPI1};
constexpr uint8_t region_cipo_pins[region_count_per_frame] = {12, 1};

constexpr uint8_t panel_count_per_region_row_max = panel_count_per_frame_row_max_bsp;
constexpr uint8_t panel_count_per_region_col_max = \
  panel_count_per_frame_col_max_bsp/region_count_per_frame; // 6
constexpr uint8_t panel_count_per_region_row = panel_count_per_frame_row;
constexpr uint8_t panel_count_per_region_col = \
  panel_count_per_frame_col/region_count_per_frame; // 6

constexpr uint8_t panel_set_select_pins[panel_count_per_region_row_max][panel_count_per_region_col_max] =
{
  {0, 6, 24, 31, 20, 39},
  {2, 7, 25, 32, 17, 38},
  {3, 8, 28, 23, 16, 37},
  {4, 9, 29, 22, 41, 36},
  {5, 10, 30, 21, 40, 35}
};

} // namespace constants
} // namespace AC

//----------------------------------------------------------------------------
// QS facilities

static QP::QSpyId const l_BSP_ID = {1U}; // QSpy source ID

//----------------------------------------------------------------------------
// Static global variables
static QEvt const panelSetTransferredEvt = {PANEL_SET_TRANSFERRED_SIG, 0U, 0U};

static WDT_T4<WDT1> wdt;
static EventResponder transfer_panel_complete_event;
static uint8_t transfer_panel_complete_count;

// Serial Communication Interface
// static HardwareSerial & serial_communication_interface_stream = Serial1;
// static usb_serial_class & qs_serial_stream = Serial;
static usb_serial_class & serial_communication_interface_stream = Serial;
static HardwareSerialIMXRT & qs_serial_stream = Serial1;

// Ethernet Communication Interface
static const char *s_lsn = "tcp://192.168.10.62:62222";

// Log
static char log_str[constants::string_log_length_max];
static uint16_t log_str_pos = 0;

struct QuarterPanel
{
  uint8_t stretch;
  uint8_t data[constants::pixel_count_per_quarter_panel_row][constants::byte_count_per_quarter_panel_row_grayscale];
};

struct Panel
{
  QuarterPanel quarter_panels[constants::quarter_panel_count_per_panel_row][constants::quarter_panel_count_per_panel_col];
};

struct Region
{
  Panel panels[constants::panel_count_per_region_row][constants::panel_count_per_region_col];
};

struct DecodedFrame
{
  Region regions[constants::region_count_per_frame];
};
static DecodedFrame decoded_frame;

// Pattern
static PatternHeader pattern_header;

// SD Card
static SdFs sd;
static FsFile pattern_file;

// Analog Output
static Adafruit_MCP4728 analog_output_chip;

//----------------------------------------------------------------------------
// Local functions
void watchdogCallback ()
{
}

//----------------------------------------------------------------------------
// BSP functions

void BSP::init()
{
  // initialize the hardware used in this sketch...
  // NOTE: interrupts are configured and started later in QF::onStartup()

  // setup pins
  pinMode(LED_BUILTIN, OUTPUT);
  ledOff();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    pinMode(constants::region_cipo_pins[region_index], INPUT);
    SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
    spi_ptr->begin();
  }
  QS_OBJ_DICTIONARY(&l_BSP_ID);
}

void BSP::ledOff()
{
  digitalWriteFast(LED_BUILTIN, LOW);
}

void BSP::ledOn()
{
  digitalWriteFast(LED_BUILTIN, HIGH);
}

void BSP::initializeWatchdog()
{
  WDT_timings_t config;
  config.trigger = constants::watchdog_trigger_seconds;
  config.timeout = constants::watchdog_timeout_seconds;
  config.callback = watchdogCallback;
  wdt.begin(config);
}

void BSP::feedWatchdog()
{
  wdt.feed();
}

void BSP::initializeArena()
{
  pinMode(constants::reset_pin, OUTPUT);
  digitalWriteFast(constants::reset_pin, LOW);
}

bool BSP::initializeSerial()
{
  // Serial.begin() is optional on Teensy. USB hardware initialization is performed before setup() runs.
  // serial_communication_interface_stream.begin(constants::serial_baud_rate);
  // serial_communication_interface_stream.setTimeout(constants::serial_timeout);
  return true;
}

bool BSP::pollSerial()
{
  return serial_communication_interface_stream.available();
}

uint8_t BSP::readSerialByte()
{
  return serial_communication_interface_stream.read();
}

void BSP::writeSerialBinaryResponse(uint8_t response[AC::constants::byte_count_per_response_max],
  uint8_t response_byte_count)
{
  serial_communication_interface_stream.write(response, response_byte_count);
}

void BSP::readSerialStringCommand(char * const command_str,
  char first_char)
{
  char command_tail[constants::string_command_length_max];
  size_t chars_read = serial_communication_interface_stream.readBytesUntil(constants::command_termination_character,
    command_tail, constants::string_command_length_max - 1);
  command_tail[chars_read] = '\0';
  command_str[0] = first_char;
  command_str[1] = '\0';
  strcat(command_str, command_tail);
}

void BSP::writeSerialStringResponse(char * const response)
{
  serial_communication_interface_stream.println(response);
}

void log_fn(char ch, void *param)
{
  if ((ch == '\n') || (log_str_pos == (constants::string_log_length_max - 1)))
  {
    log_str[log_str_pos] = 0;
    QS_BEGIN_ID(ETHERNET_LOG, AO_EthernetCommandInterface->m_prio)
      QS_STR(log_str);
    QS_END()
    log_str[0] = 0;
    log_str_pos = 0;
  }
  else if (ch != '\r')
  {
    log_str[log_str_pos++] = ch;
  }
}

bool BSP::initializeEthernet()
{
  mg_log_set_fn(log_fn, 0);
  ethernet_init();
  mongoose_init();
  return true;
}

void BSP::pollEthernet()
{
  mongoose_poll();
}

void sfn(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_OPEN && c->is_listening == 1)
  {
    MG_INFO(("SERVER is listening"));
  }
  else if (ev == MG_EV_ACCEPT)
  {
    MG_INFO(("SERVER accepted a connection"));
  }
  else if (ev == MG_EV_READ)
  {
    struct mg_iobuf *r = &c->recv;
    MG_INFO(("SERVER got data: %lu bytes", r->len));

    CommandEvt *cev = Q_NEW(CommandEvt, ETHERNET_COMMAND_AVAILABLE_SIG);
    cev->connection = c;
    cev->binary_command = r->buf;
    cev->binary_command_byte_count = r->len;
    QF::PUBLISH(cev, &l_BSP_ID);
  }
  else if (ev == MG_EV_WRITE)
  {
    MG_INFO(("MG_EV_WRITE"));
  }
  else if (ev == MG_EV_CLOSE)
  {
    MG_INFO(("SERVER disconnected"));
  }
  else if (ev == MG_EV_ERROR)
  {
    MG_INFO(("SERVER error: %s", (char *) ev_data));
  }
  else if (ev == MG_EV_POLL)
  {
  }
  else
  {
    MG_INFO(("event %lu", ev));
  }
}

bool BSP::createEthernetServerConnection()
{
  struct mg_connection *c = mg_listen(&g_mgr, s_lsn, sfn, NULL);
  if (c == NULL)
  {
    MG_INFO(("SERVER cannot open a connection"));
    return false;
  }
  return true;
}

void BSP::writeEthernetBinaryResponse(void * const connection,
  uint8_t response[constants::byte_count_per_response_max],
  uint8_t response_byte_count)
{
  struct mg_connection * c = (struct mg_connection *)connection;
  struct mg_iobuf *r = &c->recv;
  mg_send(c, response, response_byte_count);
  r->len = 0;
}

void transferPanelCompleteCallback(EventResponderRef event_responder)
{
  ++transfer_panel_complete_count;
  if (transfer_panel_complete_count == constants::region_count_per_frame)
  {
    AO_Frame->POST(&panelSetTransferredEvt, &l_BSP_ID);
  }
}

void BSP::armRefreshTimer(uint32_t frequency_hz,
  void (*isr)())
{
  uint32_t microseconds = constants::microseconds_per_second / frequency_hz;
  Timer3.initialize(microseconds);
  Timer3.attachInterrupt(isr);
}

void BSP::disarmRefreshTimer()
{
  Timer3.stop();
  Timer3.detachInterrupt();
}

void BSP::initializeFrame()
{
  transfer_panel_complete_event.attachImmediate(&transferPanelCompleteCallback);
  for (uint8_t panel_set_col_index = 0; panel_set_col_index<constants::panel_count_per_region_col_max; ++panel_set_col_index)
  {
    for (uint8_t panel_set_row_index = 0; panel_set_row_index<constants::panel_count_per_region_row_max; ++panel_set_row_index)
    {
      const uint8_t & pss_pin = constants::panel_set_select_pins[panel_set_row_index][panel_set_col_index];
      pinMode(pss_pin, OUTPUT);
      digitalWriteFast(pss_pin, HIGH);
    }
  }
}

uint8_t BSP::getPanelCountPerRegionRow()
{
  return constants::panel_count_per_region_row;
}

uint8_t BSP::getPanelCountPerRegionCol()
{
  return constants::panel_count_per_region_col;
}

uint8_t BSP::getRegionCountPerFrame()
{
  return constants::region_count_per_frame;
}

uint8_t BSP::getPanelCountPerFrameRow()
{
  return constants::panel_count_per_region_row;
}

uint8_t BSP::getPanelCountPerFrameCol()
{
  return constants::panel_count_per_region_col * constants::region_count_per_frame;
}

void BSP::fillFrameBufferWithAllOn(uint8_t * const buffer,
  bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  uint8_t stretch;
  if (grayscale)
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_grayscale;
    stretch = 1;
  }
  else
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_binary;
    stretch = 50;
  }
  uint16_t buffer_position = 0;
  for (uint8_t region_panel_col_index = 0; region_panel_col_index<constants::panel_count_per_region_col; ++region_panel_col_index)
  {
    for (uint8_t region_panel_row_index = 0; region_panel_row_index<constants::panel_count_per_region_row; ++region_panel_row_index)
    {
      for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
      {
        for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
        {
          for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
          {
            buffer[buffer_position++] = stretch;
            for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
            {
              for (uint8_t byte_index = 0; byte_index<byte_count_per_quarter_panel_row; ++byte_index)
              {
                buffer[buffer_position++] = 255;
                // uint8_t panel_col_index = region_panel_col_index + region_index * constants::panel_count_per_region_col_max;
              }
            }
          }
        }
      }
    }
  }
}

// uint8_t reverseBits(uint8_t b)
// {
//   uint8_t r = 0;
//   for (int i = 0; i<8; ++i)
//   {
//     if (b & 1)
//     {
//       r |= 1 << (7 - i);
//     }
//     b >>= 1;
//   }
//   return r;
// }

// uint8_t flipBits(uint8_t b)
// {
//   uint8_t left_shifted = b << 4;
//   uint8_t right_shifted = b >> 4;
//   uint8_t f = left_shifted | right_shifted;
//   return f;
// }

uint8_t remapColumnIndex(uint8_t column_index)
{
  if (column_index == 0)
  {
    return column_index;
  }
  return constants::panel_count_per_frame_col - column_index;
}

uint16_t BSP::decodePatternFrameBuffer(const uint8_t * const pattern_frame_buffer,
  bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  if (grayscale)
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_grayscale;
  }
  else
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_binary;
  }
  uint16_t pattern_frame_buffer_position = 0;
  // uint8_t row_signifier_check = 1;

  for (int8_t frame_panel_row_index = (constants::panel_count_per_frame_row - 1); frame_panel_row_index>=0; --frame_panel_row_index)
  {
    for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
    // for (int8_t quarter_panel_col_index = (constants::quarter_panel_count_per_panel_col - 1); quarter_panel_col_index>=0; --quarter_panel_col_index)
    {
      // for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
      for (int8_t quarter_panel_row_index = (constants::quarter_panel_count_per_panel_row - 1); quarter_panel_row_index>=0; --quarter_panel_row_index)
      {
        // uint8_t row_signifier = pattern_frame_buffer[pattern_frame_buffer_position++];
        ++pattern_frame_buffer_position; // skip row signifier
        // QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
        //   QS_U8(0, row_signifier_check);
        //   QS_U8(0, row_signifier);
        // QS_END()
        uint8_t region_index;
        uint8_t region_panel_col_index;
        uint8_t region_panel_row_index = frame_panel_row_index;
        uint8_t remapped_frame_panel_col_index;
        uint8_t stretch;
        for (uint8_t frame_panel_col_index = 0; frame_panel_col_index<constants::panel_count_per_frame_col; ++frame_panel_col_index)
        // for (int8_t frame_panel_col_index = (constants::panel_count_per_frame_col - 1); frame_panel_col_index>=0; --frame_panel_col_index)
        {
          remapped_frame_panel_col_index = remapColumnIndex(frame_panel_col_index);
          region_index = remapped_frame_panel_col_index / constants::panel_count_per_region_col_max;
          region_panel_col_index = remapped_frame_panel_col_index - region_index * constants::panel_count_per_region_col_max;
        // QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
        //   QS_U8(0, region_index);
        //   QS_U8(0, region_panel_col_index);
        // QS_END()
          QuarterPanel & quarter_panel = decoded_frame.regions[region_index].panels[region_panel_row_index][region_panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
          stretch = pattern_frame_buffer[pattern_frame_buffer_position++];
          quarter_panel.stretch = stretch;
          // QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
          //   QS_U8(0, stretch);
          // QS_END()
        }
        for (int8_t pixel_row_index = (constants::pixel_count_per_quarter_panel_row - 1); pixel_row_index>=0; --pixel_row_index)
        // for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
        {
          for (uint8_t byte_index = 0; byte_index<byte_count_per_quarter_panel_row; ++byte_index)
          // for (int8_t byte_index = (byte_count_per_quarter_panel_row - 1); byte_index>=0; --byte_index)
          {
            for (uint8_t frame_panel_col_index = 0; frame_panel_col_index<constants::panel_count_per_frame_col; ++frame_panel_col_index)
            // for (int8_t frame_panel_col_index = (constants::panel_count_per_frame_col - 1); frame_panel_col_index>=0; --frame_panel_col_index)
            {
              remapped_frame_panel_col_index = remapColumnIndex(frame_panel_col_index);
              region_index = remapped_frame_panel_col_index / constants::panel_count_per_region_col_max;
              region_panel_col_index = remapped_frame_panel_col_index - region_index * constants::panel_count_per_region_col_max;
              QuarterPanel & quarter_panel = decoded_frame.regions[region_index].panels[region_panel_row_index][region_panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
              quarter_panel.data[pixel_row_index][byte_index] = pattern_frame_buffer[pattern_frame_buffer_position++];
              // quarter_panel.data[pixel_row_index][byte_index] = reverseBits(pattern_frame_buffer[pattern_frame_buffer_position++]);
              // quarter_panel.data[pixel_row_index][byte_index] = flipBits(pattern_frame_buffer[pattern_frame_buffer_position++]);
            }
          }
        }
      }
    }
    // ++row_signifier_check;
  }
  return pattern_frame_buffer_position;
}

void BSP::fillFrameBufferWithDecodedFrame(uint8_t * const buffer,
  bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  if (grayscale)
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_grayscale;
  }
  else
  {
    byte_count_per_quarter_panel_row = constants::byte_count_per_quarter_panel_row_binary;
  }
  uint16_t buffer_position = 0;
  for (uint8_t region_panel_col_index = 0; region_panel_col_index<constants::panel_count_per_region_col; ++region_panel_col_index)
  {
    for (uint8_t region_panel_row_index = 0; region_panel_row_index<constants::panel_count_per_region_row; ++region_panel_row_index)
    {
      for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
      {
        for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
        {
          for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
          {
            QuarterPanel & quarter_panel = decoded_frame.regions[region_index].panels[region_panel_row_index][region_panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
            buffer[buffer_position++] = quarter_panel.stretch;
            for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
            {
              for (uint8_t byte_index = 0; byte_index<byte_count_per_quarter_panel_row; ++byte_index)
              {
                buffer[buffer_position++] = quarter_panel.data[pixel_row_index][byte_index];
              }
            }
          }
        }
      }
    }
  }
}

void BSP::enablePanelSetSelectPin(uint8_t row_index,
  uint8_t col_index)
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
    spi_ptr->beginTransaction(SPISettings(constants::spi_clock_speed, constants::spi_bit_order, constants::spi_data_mode));
  }
  const uint8_t & pss_pin = constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast(pss_pin, LOW);
}

void BSP::disablePanelSetSelectPin(uint8_t row_index,
  uint8_t col_index)
{
  const uint8_t & pss_pin = constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast(pss_pin, HIGH);
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
    spi_ptr->endTransaction();
  }
}

void BSP::transferPanelSet(const uint8_t * const buffer,
  uint16_t & buffer_position,
  uint8_t panel_byte_count)
{
  transfer_panel_complete_count = 0;
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
    spi_ptr->transfer((buffer + buffer_position), NULL, panel_byte_count, transfer_panel_complete_event);
    buffer_position += panel_byte_count;
  }
}

bool BSP::initializePatternCard()
{
  return sd.begin(SdioConfig(FIFO_SDIO));
}

uint64_t BSP::openPatternFileForReading(uint16_t pattern_id)
{
  char filename_str[constants::filename_str_len];
  sprintf(filename_str, "pat%0*d.pat", constants::pattern_id_str_len, pattern_id);
  pattern_file.open(filename_str, O_RDONLY);
  return pattern_file.fileSize();
}

void BSP::closePatternFile()
{
  pattern_file.close();
}

PatternHeader BSP::rewindPatternFileAndReadHeader()
{
  pattern_file.rewind();
  pattern_file.read(&pattern_header, constants::pattern_header_size);
  return pattern_header;
}

void BSP::readNextPatternFrameFromFileIntoBuffer(uint8_t * buffer,
  uint64_t byte_count_per_pattern_frame,
  bool positive_direction)
{
  if (positive_direction)
  {
    if ((pattern_file.curPosition() + byte_count_per_pattern_frame) > pattern_file.fileSize())
    {
      pattern_file.seek(constants::pattern_header_size);
    }
    pattern_file.read(buffer, byte_count_per_pattern_frame);
  }
  else
  {
    if (((int64_t)pattern_file.curPosition() - (int64_t)byte_count_per_pattern_frame) < (int64_t)0)
    {
      pattern_file.seek(pattern_file.fileSize());
    }
    pattern_file.seek(pattern_file.curPosition() - byte_count_per_pattern_frame);
    pattern_file.read(buffer, byte_count_per_pattern_frame);
    pattern_file.seek(pattern_file.curPosition() - byte_count_per_pattern_frame);
  }
  // QS_BEGIN_ID(USER_COMMENT, AO_Pattern->m_prio)
  //   QS_STR("pattern file position");
  //   QS_U16(5, pattern_file.curPosition());
  //   QS_STR("pattern file size");
  //   QS_U16(5, pattern_file.fileSize());
  // QS_END()
}

uint64_t BSP::getByteCountPerPatternFrameGrayscale()
{
  uint64_t byte_count_per_frame = constants::byte_count_per_panel_grayscale * \
    constants::panel_count_per_frame_row *                                    \
    constants::panel_count_per_frame_col +                                    \
    constants::pattern_row_signifier_byte_count_per_row *                     \
    constants::panel_count_per_frame_row;
  return byte_count_per_frame;
}

uint64_t BSP::getByteCountPerPatternFrameBinary()
{
  uint64_t byte_count_per_frame = constants::byte_count_per_panel_binary * \
    constants::panel_count_per_frame_row *                                 \
    constants::panel_count_per_frame_col +                                 \
    constants::pattern_row_signifier_byte_count_per_row *                  \
    constants::panel_count_per_frame_row;
  return byte_count_per_frame;
}

bool BSP::initializeAnalogOutput()
{
  return analog_output_chip.begin();
}

void BSP::setAnalogOutput(uint16_t value)
{
  analog_output_chip.setChannelValue(MCP4728_CHANNEL_A, value, MCP4728_VREF_INTERNAL,
    MCP4728_GAIN_2X);
}

//----------------------------------------------------------------------------
// QF callbacks...

//
// NOTE: The usual source of system clock tick in ARM Cortex-M (SysTick timer)
// is aready used by the Arduino library. Therefore, this code uses a different
// hardware Timer1 of the Teensy 4 board for providing the system clock tick.
//
// NOTE: You can re-define the macros to use a different ATSAM timer/channel.
//

#define TIMER1_CLCK_HZ  1000000
#define TIMER_HANDLER   T1_Handler

// interrupts.................................................................
void TIMER_HANDLER()
{
  QF::TICK_X(0, &l_BSP_ID); // process time events for tick rate 0
}
//............................................................................
void QF::onStartup()
{
  // configure the timer-counter channel........
  Timer1.initialize(TIMER1_CLCK_HZ / constants::ticks_per_second);
  Timer1.attachInterrupt(TIMER_HANDLER);
  // ...
}
//............................................................................
void QV::onIdle()
{ // called with interrupts DISABLED
#ifdef NDEBUG
  // Put the CPU and peripherals to the low-power mode. You might
  // need to customize the clock management for your application,
  // see the datasheet for your particular MCU.
  QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
#else
  QF_INT_ENABLE(); // simply re-enable interrupts

  // transmit QS outgoing data (QS-TX)
  uint16_t len = qs_serial_stream.availableForWrite();
  if (len > 0U)
  { // any space available in the output buffer?
    uint8_t const *buf = QS::getBlock(&len);
    if (buf)
    {
      qs_serial_stream.write(buf, len); // asynchronous and non-blocking
    }
  }

  // receive QS incoming data (QS-RX)
  len = qs_serial_stream.available();
  if (len > 0U)
  {
    do
    {
      QP::QS::rxPut(qs_serial_stream.read());
    } while (--len > 0U);
    QS::rxParse();
  }
#endif
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const *const module, int location)
{
  //
  // NOTE: add here your application-specific error handling
  //
  (void)module;
  (void)location;

  QF_INT_DISABLE(); // disable all interrupts
  BSP::ledOn();  // trun the LED on
  for (;;)
  { // freeze in an endless loop for now...
  }
}

//----------------------------------------------------------------------------
// QS callbacks...
//............................................................................
bool QP::QS::onStartup(void const *arg)
{
  static uint8_t qsTxBuf[2048]; // buffer for QS transmit channel (QS-TX)
  static uint8_t qsRxBuf[1024];  // buffer for QS receive channel (QS-RX)
  initBuf  (qsTxBuf, sizeof(qsTxBuf));
  rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
  qs_serial_stream.setTX(constants::qs_serial_stream_tx_pin);
  qs_serial_stream.setRX(constants::qs_serial_stream_rx_pin);
  qs_serial_stream.begin(constants::qs_serial_baud_rate);
  return true; // return success
}
//............................................................................
void QP::QS::onCommand(uint8_t cmdId, uint32_t param1,
  uint32_t param2, uint32_t param3)
{
}

//............................................................................
void QP::QS::onCleanup()
{
}
//............................................................................
QP::QSTimeCtr QP::QS::onGetTime()
{
  return millis();
}
//............................................................................
void QP::QS::onFlush()
{
  uint16_t len = 0xFFFFU; // big number to get as many bytes as available
  uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
  while (buf != nullptr)
  { // data available?
    qs_serial_stream.write(buf, len); // might poll until all bytes fit
    len = 0xFFFFU; // big number to get as many bytes as available
    buf = QS::getBlock(&len); // try to get more data
  }
  qs_serial_stream.flush(); // wait for the transmission of outgoing data to complete
}
//............................................................................
void QP::QS::onReset()
{
  SCB_AIRCR = 0x05FA0004;
  while(true);
}
