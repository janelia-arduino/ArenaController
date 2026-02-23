#include <Adafruit_ADS1X15.h>
#include <Adafruit_MCP4728.h>
#include <Arduino.h>
#include <EventResponder.h>
#include <SPI.h>
#include <SdFat.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <Watchdog_t4.h>
#include <Wire.h>

#include "ArenaController.hpp"

extern "C"
{
#include "mongoose_glue.h"
}

Q_DEFINE_THIS_FILE

using namespace QP;
using namespace AC;

namespace AC
{
namespace constants
{
static QP::QSpyId const bsp_id = { 1U }; // QSpy source ID

static QEvt const panel_set_transferred_evt
    = { PANEL_SET_TRANSFERRED_SIG, 0U, 0U };

// QS Serial Pins
// constexpr uint8_t qs_serial_stream_tx_pin = 53;
// constexpr uint8_t qs_serial_stream_rx_pin = 52;

// SPI Settings
constexpr uint8_t spi_bit_order = MSBFIRST;
constexpr uint8_t spi_data_mode = SPI_MODE0;
constexpr uint32_t spi_clock_speed = 4000000;

constexpr uint8_t reset_pin = 34;

// Ethernet Communication Interface
// static const char *ethernet_static_url = "tcp://192.168.10.62:62222";
const char *ethernet_dhcp_url = "tcp://0.0.0.0:62222";

// frame
constexpr uint8_t panel_count_per_frame_row_max_bsp = 5;
constexpr uint8_t panel_count_per_frame_col_max_bsp = 12;
constexpr uint8_t panel_count_per_frame_row = 2;
constexpr uint8_t panel_count_per_frame_col = 12;

// region
constexpr uint8_t region_count_per_frame = 2;
constexpr SPIClass *region_spi_ptrs[region_count_per_frame] = { &SPI, &SPI1 };
constexpr uint8_t region_cipo_pins[region_count_per_frame] = { 12, 1 };

constexpr uint8_t panel_count_per_region_row_max
    = panel_count_per_frame_row_max_bsp;
constexpr uint8_t panel_count_per_region_col_max
    = panel_count_per_frame_col_max_bsp / region_count_per_frame; // 6
constexpr uint8_t panel_count_per_region_row = panel_count_per_frame_row;
constexpr uint8_t panel_count_per_region_col
    = panel_count_per_frame_col / region_count_per_frame; // 6

constexpr uint8_t panel_set_select_pins[panel_count_per_region_row_max]
                                       [panel_count_per_region_col_max]
    = { { 0, 6, 24, 31, 20, 39 },
        { 2, 7, 25, 32, 17, 38 },
        { 3, 8, 28, 23, 16, 37 },
        { 4, 9, 29, 22, 41, 36 },
        { 5, 10, 30, 21, 40, 35 } };

// pattern files
constexpr uint8_t pattern_filename_log_str_len_max = 25;

// analog
constexpr adsGain_t analog_input_gain = GAIN_TWOTHIRDS;
constexpr uint16_t analog_input_mux = ADS1X15_REG_CONFIG_MUX_DIFF_0_1;
constexpr bool analog_input_continuous = false;
} // namespace constants
} // namespace AC

//----------------------------------------------------------------------------
// Static bsp_global variables
namespace bsp_global
{
//----------------------------------------------------------------------------
// QS facilities

static WDT_T4<WDT1> wdt;
static EventResponder transfer_panel_complete_event;
static uint8_t transfer_panel_complete_count;

// Serial Communication Interface
static HardwareSerial &serial_communication_interface_stream = Serial1;
static usb_serial_class &qs_serial_stream = Serial;
// static usb_serial_class &serial_communication_interface_stream = Serial;
// static HardwareSerialIMXRT &qs_serial_stream = Serial1;

// Ethernet Communication Interface
static char ethernet_ip_address[constants::ethernet_ip_address_length_max]
    = "";
static bool ip_announced = false;

// Log
static char log_str[constants::string_log_length_max];
static uint16_t log_str_pos = 0;

struct QuarterPanel
{
  uint8_t stretch;
  uint8_t data[constants::pixel_count_per_quarter_panel_row]
              [constants::byte_count_per_quarter_panel_row_grayscale];
};

struct Panel
{
  QuarterPanel quarter_panels[constants::quarter_panel_count_per_panel_row]
                             [constants::quarter_panel_count_per_panel_col];
};

struct Region
{
  Panel panels[constants::panel_count_per_region_row]
              [constants::panel_count_per_region_col];
};

struct DecodedFrame
{
  Region regions[constants::region_count_per_frame];
};
static DecodedFrame decoded_frame;

// Pattern
static PatternHeader pattern_header;

// SD Card
static SdFs pattern_sd;
static FsFile pattern_dir;
static FsFile pattern_file;

// Analog Output
static Adafruit_MCP4728 analog_output_chip;

// Analog Input
static Adafruit_ADS1015 analog_input_chip;
} // namespace bsp_global

//----------------------------------------------------------------------------
// Local functions
void
watchdogCallback ()
{
}

//----------------------------------------------------------------------------
// BSP functions

void
BSP::init ()
{
  // initialize the hardware used in this sketch...
  // NOTE: interrupts are configured and started later in QF::onStartup()

  // setup pins
  pinMode (LED_BUILTIN, OUTPUT);
  ledOff ();

  // Optional performance probe pins
  pinMode (constants::perf_pin_refresh_tick, OUTPUT);
  pinMode (constants::perf_pin_frame_transfer, OUTPUT);
  pinMode (constants::perf_pin_fetch, OUTPUT);
  digitalWriteFast (constants::perf_pin_refresh_tick, LOW);
  digitalWriteFast (constants::perf_pin_frame_transfer, LOW);
  digitalWriteFast (constants::perf_pin_fetch, LOW);

#if defined(AC_ENABLE_PERF_PROBE)
  //  Boot-time signature pulses on the performance probe pins.
  //  This is a sanity check: if you do not see these on the scope, you are
  //  either probing the wrong pins or not running the expected firmware build.
  digitalWriteFast (constants::perf_pin_refresh_tick, HIGH);
  delay (20);
  digitalWriteFast (constants::perf_pin_refresh_tick, LOW);
  delay (20);
  digitalWriteFast (constants::perf_pin_frame_transfer, HIGH);
  delay (20);
  digitalWriteFast (constants::perf_pin_frame_transfer, LOW);
  delay (20);
  digitalWriteFast (constants::perf_pin_fetch, HIGH);
  delay (20);
  digitalWriteFast (constants::perf_pin_fetch, LOW);
#endif

  for (uint8_t region_index = 0;
       region_index < constants::region_count_per_frame; ++region_index)
    {
      pinMode (constants::region_cipo_pins[region_index], INPUT);
      SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
      spi_ptr->begin ();
    }
  QS_OBJ_DICTIONARY (&constants::bsp_id);
}

void
BSP::ledOff ()
{
  digitalWriteFast (LED_BUILTIN, LOW);
}

void
BSP::ledOn ()
{
  digitalWriteFast (LED_BUILTIN, HIGH);
}

void
BSP::initializeWatchdog ()
{
  WDT_timings_t config;
  config.trigger = constants::watchdog_trigger_seconds;
  config.timeout = constants::watchdog_timeout_seconds;
  config.callback = watchdogCallback;
  bsp_global::wdt.begin (config);
}

void
BSP::feedWatchdog ()
{
  bsp_global::wdt.feed ();
}

void
BSP::initializeArena ()
{
  pinMode (constants::reset_pin, OUTPUT);
  digitalWriteFast (constants::reset_pin, LOW);
}

bool
BSP::initializeSerial ()
{
  // Serial.begin() is optional on Teensy. USB hardware initialization is
  // performed before setup() runs.
  // bsp_global::serial_communication_interface_stream.begin(constants::serial_baud_rate);
  // bsp_global::serial_communication_interface_stream.setTimeout(constants::serial_timeout);
  return true;
}

bool
BSP::pollSerial ()
{
  return bsp_global::serial_communication_interface_stream.available ();
}

uint8_t
BSP::readSerialByte ()
{
  return bsp_global::serial_communication_interface_stream.read ();
}

void
BSP::writeSerialBinaryResponse (
    uint8_t response[AC::constants::byte_count_per_response_max],
    uint8_t response_byte_count)
{
  bsp_global::serial_communication_interface_stream.write (
      response, response_byte_count);
}

void
BSP::readSerialStringCommand (char *const command_str, char first_char)
{
  char command_tail[constants::string_command_length_max];
  size_t chars_read
      = bsp_global::serial_communication_interface_stream.readBytesUntil (
          constants::command_termination_character, command_tail,
          constants::string_command_length_max - 1);
  command_tail[chars_read] = '\0';
  command_str[0] = first_char;
  command_str[1] = '\0';
  strcat (command_str, command_tail);
}

void
BSP::writeSerialStringResponse (char *const response)
{
  bsp_global::serial_communication_interface_stream.println (response);
}

void
log_fn (char ch, void *param)
{
  if ((ch == '\n')
      || (bsp_global::log_str_pos == (constants::string_log_length_max - 1)))
    {
      bsp_global::log_str[bsp_global::log_str_pos] = 0;
      QS_BEGIN_ID (ETHERNET_LOG, AO_EthernetCommandInterface->m_prio)
      QS_STR (bsp_global::log_str);
      QS_END ()
      bsp_global::log_str[0] = 0;
      bsp_global::log_str_pos = 0;
    }
  else if (ch != '\r')
    {
      bsp_global::log_str[bsp_global::log_str_pos++] = ch;
    }
}

bool
BSP::initializeEthernet ()
{
  mg_log_set_fn (log_fn, 0);
  ethernet_init ();
  mongoose_init ();
  return true;
}

void
BSP::pollEthernet ()
{
  mongoose_poll ();
}

void
sfn (struct mg_connection *c, int ev, void *ev_data)
{
  switch (ev)
    {
    case MG_EV_OPEN:
      if (c->is_listening == 1)
        {
          MG_INFO (("SERVER is listening"));
        }
      break;

    case MG_EV_ACCEPT:
      MG_INFO (("SERVER accepted a connection"));
      break;

    case MG_EV_READ:
      {
        struct mg_iobuf *r = &c->recv;
        MG_INFO (("SERVER got data: %lu bytes", r->len));

        CommandEvt *cev = Q_NEW (CommandEvt, ETHERNET_COMMAND_AVAILABLE_SIG);
        cev->connection = c;
        cev->binary_command = r->buf;
        cev->binary_command_byte_count = r->len;
        QF::PUBLISH (cev, &constants::bsp_id);
        break;
      }

    case MG_EV_WRITE:
      MG_INFO (("MG_EV_WRITE"));
      break;

    case MG_EV_CLOSE:
      MG_INFO (("SERVER disconnected"));
      break;

    case MG_EV_ERROR:
      MG_INFO (("SERVER error: %s", (char *)ev_data));
      break;

    case MG_EV_POLL:
      // DHCP done? g_mgr.ifp points to struct mg_tcpip_if
      if (!bsp_global::ip_announced && g_mgr.ifp && g_mgr.ifp->ip != 0)
        {
          mg_snprintf (bsp_global::ethernet_ip_address,
                       sizeof (bsp_global::ethernet_ip_address), "%M",
                       mg_print_ip, &g_mgr.ifp->ip);
          MG_INFO (("DHCP IP: %s", bsp_global::ethernet_ip_address));
          bsp_global::ip_announced = true;
        }
      break;

    default:
      MG_INFO (("event %lu", ev));
      break;
    }
}

bool
BSP::createEthernetServerConnection ()
{
  struct mg_connection *c
      = mg_listen (&g_mgr, constants::ethernet_dhcp_url, sfn, NULL);
  if (c == NULL)
    {
      MG_INFO (("SERVER cannot open a connection"));
      return false;
    }
  return true;
}

void
BSP::writeEthernetBinaryResponse (
    void *const connection,
    uint8_t response[constants::byte_count_per_response_max],
    uint8_t response_byte_count)
{
  struct mg_connection *c = (struct mg_connection *)connection;
  struct mg_iobuf *r = &c->recv;
  mg_send (c, response, response_byte_count);
  r->len = 0;
}

const char *
BSP::getEthernetIpAddress ()
{
  return bsp_global::ethernet_ip_address;
}

void
transferPanelCompleteCallback (EventResponderRef event_responder)
{
  ++bsp_global::transfer_panel_complete_count;
  if (bsp_global::transfer_panel_complete_count
      == constants::region_count_per_frame)
    {
      AO_Frame->POST (&constants::panel_set_transferred_evt,
                      &constants::bsp_id);
    }
}

void
BSP::armRefreshTimer (uint32_t frequency_hz, void (*isr) ())
{
  uint32_t microseconds = constants::microseconds_per_second / frequency_hz;
  Timer3.initialize (microseconds);
  Timer3.attachInterrupt (isr);
}

void
BSP::disarmRefreshTimer ()
{
  Timer3.stop ();
  Timer3.detachInterrupt ();
}

void
BSP::initializeFrame ()
{
  bsp_global::transfer_panel_complete_event.attachImmediate (
      &transferPanelCompleteCallback);
  for (uint8_t panel_set_col_index = 0;
       panel_set_col_index < constants::panel_count_per_region_col_max;
       ++panel_set_col_index)
    {
      for (uint8_t panel_set_row_index = 0;
           panel_set_row_index < constants::panel_count_per_region_row_max;
           ++panel_set_row_index)
        {
          const uint8_t &pss_pin
              = constants::panel_set_select_pins[panel_set_row_index]
                                                [panel_set_col_index];
          pinMode (pss_pin, OUTPUT);
          digitalWriteFast (pss_pin, HIGH);
        }
    }
}

uint8_t
BSP::getPanelCountPerRegionRow ()
{
  return constants::panel_count_per_region_row;
}

uint8_t
BSP::getPanelCountPerRegionCol ()
{
  return constants::panel_count_per_region_col;
}

uint8_t
BSP::getRegionCountPerFrame ()
{
  return constants::region_count_per_frame;
}

uint8_t
BSP::getPanelCountPerFrameRow ()
{
  return constants::panel_count_per_region_row;
}

uint8_t
BSP::getPanelCountPerFrameCol ()
{
  return constants::panel_count_per_region_col
         * constants::region_count_per_frame;
}

void
BSP::fillFrameBufferWithAllOn (uint8_t *const buffer, bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  uint8_t stretch;
  if (grayscale)
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_grayscale;
      stretch = 1;
    }
  else
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_binary;
      stretch = 50;
    }
  uint16_t buffer_position = 0;
  for (uint8_t region_panel_col_index = 0;
       region_panel_col_index < constants::panel_count_per_region_col;
       ++region_panel_col_index)
    {
      for (uint8_t region_panel_row_index = 0;
           region_panel_row_index < constants::panel_count_per_region_row;
           ++region_panel_row_index)
        {
          for (uint8_t region_index = 0;
               region_index < constants::region_count_per_frame;
               ++region_index)
            {
              for (uint8_t quarter_panel_col_index = 0;
                   quarter_panel_col_index
                   < constants::quarter_panel_count_per_panel_col;
                   ++quarter_panel_col_index)
                {
                  for (uint8_t quarter_panel_row_index = 0;
                       quarter_panel_row_index
                       < constants::quarter_panel_count_per_panel_row;
                       ++quarter_panel_row_index)
                    {
                      buffer[buffer_position++] = stretch;
                      for (uint8_t pixel_row_index = 0;
                           pixel_row_index
                           < constants::pixel_count_per_quarter_panel_row;
                           ++pixel_row_index)
                        {
                          for (uint8_t byte_index = 0;
                               byte_index < byte_count_per_quarter_panel_row;
                               ++byte_index)
                            {
                              buffer[buffer_position++] = 255;
                              // uint8_t panel_col_index =
                              // region_panel_col_index + region_index *
                              // constants::panel_count_per_region_col_max;
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

uint8_t
remapColumnIndex (uint8_t column_index)
{
  if (column_index == 0)
    {
      return column_index;
    }
  return constants::panel_count_per_frame_col - column_index;
}

uint16_t
BSP::decodePatternFrameBuffer (const uint8_t *const pattern_frame_buffer,
                               bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  if (grayscale)
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_grayscale;
    }
  else
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_binary;
    }
  uint16_t pattern_frame_buffer_position = 0;
  // uint8_t row_signifier_check = 1;

  for (int8_t frame_panel_row_index
       = (constants::panel_count_per_frame_row - 1);
       frame_panel_row_index >= 0; --frame_panel_row_index)
    {
      for (uint8_t quarter_panel_col_index = 0;
           quarter_panel_col_index
           < constants::quarter_panel_count_per_panel_col;
           ++quarter_panel_col_index)
        // for (int8_t quarter_panel_col_index =
        // (constants::quarter_panel_count_per_panel_col - 1);
        // quarter_panel_col_index>=0; --quarter_panel_col_index)
        {
          // for (uint8_t quarter_panel_row_index = 0;
          // quarter_panel_row_index<constants::quarter_panel_count_per_panel_row;
          // ++quarter_panel_row_index)
          for (int8_t quarter_panel_row_index
               = (constants::quarter_panel_count_per_panel_row - 1);
               quarter_panel_row_index >= 0; --quarter_panel_row_index)
            {
              // uint8_t row_signifier =
              // pattern_frame_buffer[pattern_frame_buffer_position++];
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
              for (uint8_t frame_panel_col_index = 0;
                   frame_panel_col_index
                   < constants::panel_count_per_frame_col;
                   ++frame_panel_col_index)
                // for (int8_t frame_panel_col_index =
                // (constants::panel_count_per_frame_col - 1);
                // frame_panel_col_index>=0;
                // --frame_panel_col_index)
                {
                  remapped_frame_panel_col_index
                      = remapColumnIndex (frame_panel_col_index);
                  region_index = remapped_frame_panel_col_index
                                 / constants::panel_count_per_region_col_max;
                  region_panel_col_index
                      = remapped_frame_panel_col_index
                        - region_index
                              * constants::panel_count_per_region_col_max;
                  // QS_BEGIN_ID(USER_COMMENT,
                  // AO_EthernetCommandInterface->m_prio)
                  //   QS_U8(0, region_index);
                  //   QS_U8(0, region_panel_col_index);
                  // QS_END()
                  bsp_global::QuarterPanel &quarter_panel
                      = bsp_global::decoded_frame.regions[region_index]
                            .panels[region_panel_row_index]
                                   [region_panel_col_index]
                            .quarter_panels[quarter_panel_row_index]
                                           [quarter_panel_col_index];
                  stretch
                      = pattern_frame_buffer[pattern_frame_buffer_position++];
                  quarter_panel.stretch = stretch;
                  // QS_BEGIN_ID(USER_COMMENT,
                  // AO_EthernetCommandInterface->m_prio)
                  //   QS_U8(0, stretch);
                  // QS_END()
                }
              for (int8_t pixel_row_index
                   = (constants::pixel_count_per_quarter_panel_row - 1);
                   pixel_row_index >= 0; --pixel_row_index)
                // for (uint8_t pixel_row_index = 0;
                // pixel_row_index<constants::pixel_count_per_quarter_panel_row;
                // ++pixel_row_index)
                {
                  for (uint8_t byte_index = 0;
                       byte_index < byte_count_per_quarter_panel_row;
                       ++byte_index)
                    // for (int8_t byte_index =
                    // (byte_count_per_quarter_panel_row - 1); byte_index>=0;
                    // --byte_index)
                    {
                      for (uint8_t frame_panel_col_index = 0;
                           frame_panel_col_index
                           < constants::panel_count_per_frame_col;
                           ++frame_panel_col_index)
                        // for (int8_t frame_panel_col_index =
                        // (constants::panel_count_per_frame_col - 1);
                        // frame_panel_col_index>=0; --frame_panel_col_index)
                        {
                          remapped_frame_panel_col_index
                              = remapColumnIndex (frame_panel_col_index);
                          region_index
                              = remapped_frame_panel_col_index
                                / constants::panel_count_per_region_col_max;
                          region_panel_col_index
                              = remapped_frame_panel_col_index
                                - region_index
                                      * constants::
                                          panel_count_per_region_col_max;
                          bsp_global::QuarterPanel &quarter_panel
                              = bsp_global::decoded_frame.regions[region_index]
                                    .panels[region_panel_row_index]
                                           [region_panel_col_index]
                                    .quarter_panels[quarter_panel_row_index]
                                                   [quarter_panel_col_index];
                          quarter_panel.data[pixel_row_index][byte_index]
                              = pattern_frame_buffer
                                  [pattern_frame_buffer_position++];
                          // quarter_panel.data[pixel_row_index][byte_index] =
                          // reverseBits(pattern_frame_buffer[pattern_frame_buffer_position++]);
                          // quarter_panel.data[pixel_row_index][byte_index] =
                          // flipBits(pattern_frame_buffer[pattern_frame_buffer_position++]);
                        }
                    }
                }
            }
        }
      // ++row_signifier_check;
    }
  return pattern_frame_buffer_position;
}

void
BSP::fillFrameBufferWithDecodedFrame (uint8_t *const buffer, bool grayscale)
{
  uint8_t byte_count_per_quarter_panel_row;
  if (grayscale)
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_grayscale;
    }
  else
    {
      byte_count_per_quarter_panel_row
          = constants::byte_count_per_quarter_panel_row_binary;
    }
  uint16_t buffer_position = 0;
  for (uint8_t region_panel_col_index = 0;
       region_panel_col_index < constants::panel_count_per_region_col;
       ++region_panel_col_index)
    {
      for (uint8_t region_panel_row_index = 0;
           region_panel_row_index < constants::panel_count_per_region_row;
           ++region_panel_row_index)
        {
          for (uint8_t region_index = 0;
               region_index < constants::region_count_per_frame;
               ++region_index)
            {
              for (uint8_t quarter_panel_col_index = 0;
                   quarter_panel_col_index
                   < constants::quarter_panel_count_per_panel_col;
                   ++quarter_panel_col_index)
                {
                  for (uint8_t quarter_panel_row_index = 0;
                       quarter_panel_row_index
                       < constants::quarter_panel_count_per_panel_row;
                       ++quarter_panel_row_index)
                    {
                      bsp_global::QuarterPanel &quarter_panel
                          = bsp_global::decoded_frame.regions[region_index]
                                .panels[region_panel_row_index]
                                       [region_panel_col_index]
                                .quarter_panels[quarter_panel_row_index]
                                               [quarter_panel_col_index];
                      buffer[buffer_position++] = quarter_panel.stretch;
                      for (uint8_t pixel_row_index = 0;
                           pixel_row_index
                           < constants::pixel_count_per_quarter_panel_row;
                           ++pixel_row_index)
                        {
                          for (uint8_t byte_index = 0;
                               byte_index < byte_count_per_quarter_panel_row;
                               ++byte_index)
                            {
                              buffer[buffer_position++]
                                  = quarter_panel
                                        .data[pixel_row_index][byte_index];
                            }
                        }
                    }
                }
            }
        }
    }
}

void
BSP::enablePanelSetSelectPin (uint8_t row_index, uint8_t col_index)
{
  for (uint8_t region_index = 0;
       region_index < constants::region_count_per_frame; ++region_index)
    {
      SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
      spi_ptr->beginTransaction (SPISettings (constants::spi_clock_speed,
                                              constants::spi_bit_order,
                                              constants::spi_data_mode));
    }
  const uint8_t &pss_pin
      = constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast (pss_pin, LOW);
}

void
BSP::disablePanelSetSelectPin (uint8_t row_index, uint8_t col_index)
{
  const uint8_t &pss_pin
      = constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast (pss_pin, HIGH);
  for (uint8_t region_index = 0;
       region_index < constants::region_count_per_frame; ++region_index)
    {
      SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
      spi_ptr->endTransaction ();
    }
}

void
BSP::transferPanelSet (const uint8_t *const buffer, uint16_t &buffer_position,
                       uint8_t panel_byte_count)
{
  bsp_global::transfer_panel_complete_count = 0;
  for (uint8_t region_index = 0;
       region_index < constants::region_count_per_frame; ++region_index)
    {
      SPIClass *spi_ptr = constants::region_spi_ptrs[region_index];
      spi_ptr->transfer ((buffer + buffer_position), NULL, panel_byte_count,
                         bsp_global::transfer_panel_complete_event);
      buffer_position += panel_byte_count;
    }
}

bool
BSP::findPatternCard ()
{
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("Attempting to find pattern card");
  QS_END ()

  bool result = bsp_global::pattern_sd.begin (SdioConfig (FIFO_SDIO));

  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("Pattern card found");
  QS_U8 (0, result);
  QS_END ()
  QS_FLUSH ();

  return result;
}

bool
BSP::openPatternDirectory ()
{
  bool directory_opened
      = bsp_global::pattern_dir.open (constants::pattern_dir_str);
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("pattern directory opened");
  QS_END ()
  return directory_opened;
}

void
BSP::scanPatternDirectory ()
{
  // assumes pattern_dir is opened
  FsFile f;
  uint32_t dir_index;
  while (f.openNext (&bsp_global::pattern_dir, O_RDONLY))
    {
      if (!f.isDir ())
        {
          dir_index = f.dirIndex ();
          char name_log[constants::pattern_filename_log_str_len_max];
          f.getName (name_log, sizeof (name_log));
          QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
          QS_STR ("dir-index");
          QS_U16 (5, dir_index);
          QS_STR ("pattern-id");
          QS_U16 (5, dir_index - 1);
          QS_STR ("name");
          QS_STR (name_log);
          QS_END ()
        }
      f.close ();
    }
}

uint64_t
BSP::openPatternFileForReading (uint16_t pattern_id)
{
  // pattern_id is the SdFat directory entry index (FsFile::dirIndex())
  // within bsp_global::pattern_dir (which must already be opened).

  // If you want to keep 0 as "invalid / not set", uncomment this.
  // NOTE: On exFAT the first file can have dirIndex()==0, so only do this
  // if you also offset IDs elsewhere (dirIndex+1), or you accept that index 0
  // can never be opened.
  // if (pattern_id == 0) {
  //   return 0;
  // }

  if (!bsp_global::pattern_dir || !bsp_global::pattern_dir.isDir ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("openPatternFileForReading: pattern_dir not open");
      QS_END ()
      return 0;
    }

  // Close any previously open pattern file.
  bsp_global::pattern_file.close ();

  // Open by directory entry index.
  bool ok = bsp_global::pattern_file.open (
      &bsp_global::pattern_dir, static_cast<uint32_t> (pattern_id + 1),
      O_RDONLY);
  if (!ok)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("openPatternFileForReading: open by dirIndex failed");
      QS_STR ("dirIndex");
      QS_U16 (5, pattern_id);
      QS_STR ("sdErrorCode");
      QS_U8 (0, bsp_global::pattern_sd.sdErrorCode ());
      QS_STR ("sdErrorData");
      QS_U8 (0, bsp_global::pattern_sd.sdErrorData ());
      QS_END ()
      return 0;
    }

  // Reject directories (dirIndex can refer to a subdirectory too).
  if (bsp_global::pattern_file.isDir ())
    {
      bsp_global::pattern_file.close ();
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("openPatternFileForReading: dirIndex is a directory");
      QS_U16 (5, pattern_id);
      QS_END ()
      return 0;
    }

  // Optional: log the (possibly truncated) filename for debugging.
  char name_log[constants::pattern_filename_log_str_len_max];
  bsp_global::pattern_file.getName (name_log, sizeof (name_log));
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("opened pattern by dirIndex");
  QS_U16 (5, pattern_id);
  QS_STR ("name");
  QS_STR (name_log);
  QS_END ()

  return bsp_global::pattern_file.fileSize ();
}

void
BSP::closePatternFile ()
{
  bsp_global::pattern_file.close ();
}

PatternHeader
BSP::rewindPatternFileAndReadHeader ()
{
  bsp_global::pattern_file.rewind ();
  bsp_global::pattern_file.read (&bsp_global::pattern_header,
                                 constants::pattern_header_size);
  return bsp_global::pattern_header;
}

void
BSP::readPatternFrameFromFileIntoBuffer (uint8_t *buffer, uint16_t frame_index,
                                         uint64_t byte_count_per_pattern_frame)
{
  uint32_t file_position = constants::pattern_header_size
                           + frame_index * byte_count_per_pattern_frame;
  Q_ASSERT ((file_position + byte_count_per_pattern_frame)
            <= bsp_global::pattern_file.fileSize ());
  bsp_global::pattern_file.seek (file_position);
  bsp_global::pattern_file.read (buffer, byte_count_per_pattern_frame);
  // QS_BEGIN_ID(USER_COMMENT, AO_Pattern->m_prio)
  //   QS_STR("pattern file position");
  //   QS_U16(5, file_position);
  //   QS_STR("pattern file size");
  //   QS_U16(5, bsp_global::pattern_file.fileSize());
  // QS_END()
}

uint64_t
BSP::getByteCountPerPatternFrameGrayscale ()
{
  uint64_t byte_count_per_frame
      = constants::byte_count_per_panel_grayscale
            * constants::panel_count_per_frame_row
            * constants::panel_count_per_frame_col
        + constants::pattern_row_signifier_byte_count_per_row
              * constants::panel_count_per_frame_row;
  return byte_count_per_frame;
}

uint64_t
BSP::getByteCountPerPatternFrameBinary ()
{
  uint64_t byte_count_per_frame
      = constants::byte_count_per_panel_binary
            * constants::panel_count_per_frame_row
            * constants::panel_count_per_frame_col
        + constants::pattern_row_signifier_byte_count_per_row
              * constants::panel_count_per_frame_row;
  return byte_count_per_frame;
}

bool
BSP::initializeAnalogOutput ()
{
  return bsp_global::analog_output_chip.begin ();
}

void
BSP::setAnalogOutput (uint16_t value)
{
  bsp_global::analog_output_chip.setChannelValue (
      MCP4728_CHANNEL_A, value, MCP4728_VREF_VDD, MCP4728_GAIN_1X);
}

bool
BSP::initializeAnalogInput ()
{
  return bsp_global::analog_input_chip.begin ();
}

void
BSP::setAnalogInputGainAndStartReading ()
{
  bsp_global::analog_input_chip.setGain (constants::analog_input_gain);
  bsp_global::analog_input_chip.startADCReading (
      constants::analog_input_mux, constants::analog_input_continuous);
}

bool
BSP::analogInputDataAvailable ()
{
  return bsp_global::analog_input_chip.conversionComplete ();
}

int16_t
BSP::getAnalogInputMillivolts ()
{
  int16_t analog_input_value
      = bsp_global::analog_input_chip.getLastConversionResults ();
  bsp_global::analog_input_chip.startADCReading (
      constants::analog_input_mux, constants::analog_input_continuous);
  int16_t analog_input_millivolts
      = 1000 * bsp_global::analog_input_chip.computeVolts (analog_input_value);
  return analog_input_millivolts;
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

#define TIMER1_CLCK_HZ 1000000
#define TIMER_HANDLER T1_Handler

// interrupts.................................................................
void
TIMER_HANDLER ()
{
  QF::TICK_X (0, &constants::bsp_id); // process time events for tick rate 0
}
//............................................................................
void
QF::onStartup ()
{
  // configure the timer-counter channel........
  Timer1.initialize (TIMER1_CLCK_HZ / constants::ticks_per_second);
  Timer1.attachInterrupt (TIMER_HANDLER);
  // ...
}
//............................................................................
void
QV::onIdle ()
{ // called with interrupts DISABLED
#ifdef NDEBUG
  // Put the CPU and peripherals to the low-power mode. You
  // might
  // need to customize the clock management for your application,
  // see the datasheet for your particular MCU.
  QV_CPU_SLEEP (); // atomically go to sleep and enable interrupts
#else
  QF_INT_ENABLE (); // simply re-enable interrupts

  // transmit QS outgoing data (QS-TX)
  uint16_t len = bsp_global::qs_serial_stream.availableForWrite ();
  if (len > 0U)
    { // any space available in the output buffer?
      uint8_t const *buf = QS::getBlock (&len);
      if (buf)
        {
          bsp_global::qs_serial_stream.write (
              buf,
              len); // asynchronous and non-blocking
        }
    }

  // receive QS incoming data (QS-RX)
  len = bsp_global::qs_serial_stream.available ();
  if (len > 0U)
    {
      do
        {
          QP::QS::rxPut (bsp_global::qs_serial_stream.read ());
        }
      while (--len > 0U);
      QS::rxParse ();
    }
#endif
}
//............................................................................
extern "C" Q_NORETURN
Q_onAssert (char const *const module, int location)
{
  //
  // NOTE: add here your application-specific error handling
  //
  (void)module;
  (void)location;

  QF_INT_DISABLE (); // disable all interrupts
  BSP::ledOn ();     // trun the LED on
  for (;;)
    { // freeze in an endless loop for now...
    }
}

//----------------------------------------------------------------------------
// QS callbacks...
//............................................................................
bool
QP::QS::onStartup (void const *arg)
{
  static uint8_t qsTxBuf[2048]; // buffer for QS transmit channel (QS-TX)
  static uint8_t qsRxBuf[1024]; // buffer for QS receive channel (QS-RX)
  initBuf (qsTxBuf, sizeof (qsTxBuf));
  rxInitBuf (qsRxBuf, sizeof (qsRxBuf));
  // bsp_global::qs_serial_stream.setTX (constants::qs_serial_stream_tx_pin);
  // bsp_global::qs_serial_stream.setRX (constants::qs_serial_stream_rx_pin);
  bsp_global::qs_serial_stream.begin (constants::qs_serial_baud_rate);
  return true; // return success
}
//............................................................................
void
QP::QS::onCommand (uint8_t cmdId, uint32_t param1, uint32_t param2,
                   uint32_t param3)
{
}

//............................................................................
void
QP::QS::onCleanup ()
{
}
//............................................................................
QP::QSTimeCtr
QP::QS::onGetTime ()
{
  // Use microsecond timestamps for tighter inter-frame-interval/jitter
  // measurements. Note: micros() wraps in ~71 minutes (2^32 us).
  return micros ();
}
//............................................................................
void
QP::QS::onFlush ()
{
  uint16_t len = 0xFFFFU; // big number to get as many bytes as available
  uint8_t const *buf = QS::getBlock (&len); // get continguous block of data
  while (buf != nullptr)
    { // data available?
      bsp_global::qs_serial_stream.write (
          buf,
          len);      // might poll until all bytes fit
      len = 0xFFFFU; // big number to get as many bytes as available
      buf = QS::getBlock (&len); // try to get more data
    }
  bsp_global::qs_serial_stream
      .flush (); // wait for the transmission of outgoing data to complete
}
//............................................................................
void
QP::QS::onReset ()
{
  SCB_AIRCR = 0x05FA0004;
  while (true)
    ;
}
