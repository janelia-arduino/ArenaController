#include <Arduino.h>
#include <Watchdog_t4.h>
#include <QNEthernet.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <SPI.h>
#include <EventResponder.h>
#include <SdFat.h>

#include "bsp.hpp"
#include "ArenaController.hpp"


using namespace QP;
using namespace qindesign::network;

namespace AC
{
namespace constants
{
constexpr uint16_t byte_count_per_command_max = 16;
constexpr byte first_command_byte_max_value = 32;

// Ethernet Communication Interface
constexpr uint16_t port = 62222;


// Serial Communication Interface
//HardwareSerial & SERIAL_COMMUNICATION_INTERFACE_STREAM = Serial;
usb_serial_class & SERIAL_COMMUNICATION_INTERFACE_STREAM = Serial;
usb_serial_class & QS_SERIAL_STREAM = Serial;
constexpr uint32_t SERIAL_COMMUNICATION_INTERFACE_BAUD_RATE = 115200;
constexpr uint16_t SERIAL_COMMUNICATION_INTERFACE_TIMEOUT = 100;

// SPI Settings
constexpr uint32_t spi_clock_speed = 5000000;

constexpr uint8_t reset_pin = 34;

// frame
constexpr uint8_t panel_count_per_frame_row_max = 5;
constexpr uint8_t panel_count_per_frame_col_max = 12;
constexpr uint8_t panel_count_per_frame_max = \
  panel_count_per_frame_row_max * panel_count_per_frame_col_max; // 60
constexpr uint16_t byte_count_per_frame_grayscale_max = \
  panel_count_per_frame_max * \
  byte_count_per_panel_grayscale; // 7920

// region
constexpr uint8_t region_count_per_frame = 2;
constexpr SPIClass * region_spi_ptrs[region_count_per_frame] = {&SPI, &SPI1};

constexpr uint8_t region_row_panel_count_max = panel_count_per_frame_row_max;
constexpr uint8_t region_col_panel_count_max = \
  panel_count_per_frame_col_max/region_count_per_frame; // 6

constexpr uint8_t panel_set_select_pins[region_row_panel_count_max][region_col_panel_count_max] =
{
  {0, 6, 24, 31, 20, 39},
  {2, 7, 25, 32, 17, 38},
  {3, 8, 28, 23, 16, 37},
  {4, 9, 29, 22, 41, 36},
  {5, 10, 30, 21, 40, 35}
};

// files
constexpr char base_dir_str[] = "patterns";
constexpr uint8_t filename_length_max = 15;
constexpr uint16_t frame_count_y_max = 1;
constexpr uint16_t frame_count_x_max = 20;
} // namespace constants
} // namespace AC

//----------------------------------------------------------------------------
// QS facilities

// un-comment if QS instrumentation needed
//#define QS_ON

static QP::QSpyId const l_TIMER_ID = { 0U }; // QSpy source ID

//----------------------------------------------------------------------------
// Static global variables
static AC::CommandEvt const resetEvt = { AC::RESET_SIG, 0U, 0U};
static AC::CommandEvt const allOnEvt = { AC::ALL_ON_SIG, 0U, 0U};
static AC::CommandEvt const allOffEvt = { AC::ALL_OFF_SIG, 0U, 0U};

static QEvt const activateSerialCommandInterfaceEvt = { AC::ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const activateEthernetCommandInterfaceEvt = { AC::ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateSerialCommandInterfaceEvt = { AC::DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateEthernetCommandInterfaceEvt = { AC::DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const serialReadyEvt = { AC::SERIAL_READY_SIG, 0U, 0U};
static QEvt const ethernetInitializedEvt = { AC::ETHERNET_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetIPAddressFoundEvt = { AC::ETHERNET_IP_ADDRESS_FOUND_SIG, 0U, 0U};
static QEvt const ethernetServerInitializedEvt = { AC::ETHERNET_SERVER_INITIALIZED_SIG, 0U, 0U};
static QEvt const serialCommandAvailableEvt = { AC::SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U};
static QEvt const ethernetCommandAvailableEvt = { AC::ETHERNET_COMMAND_AVAILABLE_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = { AC::COMMAND_PROCESSED_SIG, 0U, 0U};

static QEvt const displayFrameTimeoutEvt = { AC::DISPLAY_FRAME_TIMEOUT_SIG, 0U, 0U};
static QEvt const panelSetTransferredEvt = { AC::PANEL_SET_TRANSFERRED_SIG, 0U, 0U};

static WDT_T4<WDT1> wdt;
static EventResponder transfer_panel_complete_event;
static uint8_t transfer_panel_complete_count;
// static uint8_t frame_buffer[AC::constants::];

static EthernetServer ethernet_server{AC::constants::port};
static IPAddress static_ip{192, 168, 10, 62};
static IPAddress subnet_mask{255, 255, 255, 0};
static IPAddress gateway{192, 168, 10, 1};

static byte command_byte_array[AC::constants::byte_count_per_command_max];

//----------------------------------------------------------------------------
// Local functions
String ipAddressToString(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]);
}

String processCommandString(String command)
{
  command.trim();
  String response = command;
  if (command.equalsIgnoreCase("RESET"))
  {
    QF::PUBLISH(&resetEvt, &l_TIMER_ID);
  }
  if (command.equalsIgnoreCase("LED_ON"))
  {
    BSP::ledOn();
  }
  else if (command.equalsIgnoreCase("LED_OFF"))
  {
    BSP::ledOff();
  }
  else if (command.equalsIgnoreCase("ALL_ON"))
  {
    QF::PUBLISH(&allOnEvt, &l_TIMER_ID);
  }
  else if (command.equalsIgnoreCase("ALL_OFF"))
  {
    QF::PUBLISH(&allOffEvt, &l_TIMER_ID);
  }
  else if (command.equalsIgnoreCase("EHS"))
  {
    response = String(Ethernet.hardwareStatus());
  }
  else if (command.equalsIgnoreCase("ELS"))
  {
    response = String(Ethernet.linkStatus());
  }
  else if (command.equalsIgnoreCase("GET_IP_ADDRESS"))
  {
    response = ipAddressToString(Ethernet.localIP());
  }
  else if (command.startsWith("SET_DISPLAY_FREQUENCY"))
  {
    command.replace("SET_DISPLAY_FREQUENCY", "");
    command.trim();
    uint32_t frequency_hz = command.toInt();
    BSP::setDisplayFrequency(frequency_hz);
  }
  return response;
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

  for (uint8_t region_index = 0; region_index<AC::constants::region_count_per_frame; ++region_index)
  {
    SPIClass * spi_ptr = AC::constants::region_spi_ptrs[region_index];
    spi_ptr->begin();
  }


#ifdef QS_ON
  QS_INIT(nullptr);

  // output QS dictionaries...
  QS_OBJ_DICTIONARY(&l_TIMER_ID);

  // setup the QS filters...
  QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
  QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
  QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
#endif
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
  config.trigger = AC::constants::watchdog_delay_s * 10; /* in seconds, 0->128 */
  config.timeout = AC::constants::watchdog_delay_s; /* in seconds, 0->128 */
  wdt.begin(config);
}

void BSP::feedWatchdog()
{
  wdt.feed();
}

void BSP::initializeArena()
{
  pinMode(AC::constants::reset_pin, OUTPUT);
  digitalWriteFast(AC::constants::reset_pin, LOW);
}

void BSP::initializeDisplay()
{
  uint32_t period_us = AC::constants::microseconds_per_second;
  Timer3.initialize(period_us);
  Timer3.stop();
}

void transferPanelCompleteCallback(EventResponderRef event_responder)
{
  ++transfer_panel_complete_count;
  if (transfer_panel_complete_count == AC::constants::region_count_per_frame)
  {
    QF::PUBLISH(&panelSetTransferredEvt, &l_TIMER_ID);
  }
}

void BSP::initializeFrame()
{
  transfer_panel_complete_event.attachImmediate(&transferPanelCompleteCallback);
  for (uint8_t panel_set_col_index = 0; panel_set_col_index<AC::constants::region_col_panel_count_max; ++panel_set_col_index)
  {
    for (uint8_t panel_set_row_index = 0; panel_set_row_index<AC::constants::region_row_panel_count_max; ++panel_set_row_index)
    {
      const uint8_t & pss_pin = AC::constants::panel_set_select_pins[panel_set_row_index][panel_set_col_index];
      pinMode(pss_pin, OUTPUT);
      digitalWriteFast(pss_pin, HIGH);
    }
  }
}

void BSP::activateCommandInterfaces()
{
#ifndef QS_ON
  AC::AO_SerialCommandInterface->POST(&activateSerialCommandInterfaceEvt, &l_TIMER_ID);
#endif

  AC::AO_EthernetCommandInterface->POST(&activateEthernetCommandInterfaceEvt, &l_TIMER_ID);
}

void BSP::deactivateCommandInterfaces()
{
#ifndef QS_ON
  AC::AO_SerialCommandInterface->POST(&deactivateSerialCommandInterfaceEvt, &l_TIMER_ID);
#endif

  AC::AO_EthernetCommandInterface->POST(&deactivateEthernetCommandInterfaceEvt, &l_TIMER_ID);
}

void BSP::beginSerial()
{
  AC::constants::SERIAL_COMMUNICATION_INTERFACE_STREAM.begin(AC::constants::SERIAL_COMMUNICATION_INTERFACE_BAUD_RATE);
  AC::constants::SERIAL_COMMUNICATION_INTERFACE_STREAM.setTimeout(AC::constants::SERIAL_COMMUNICATION_INTERFACE_TIMEOUT);
  AC::AO_SerialCommandInterface->POST(&serialReadyEvt, &l_TIMER_ID);
}

void BSP::pollSerialCommand()
{
  size_t bytes_available = AC::constants::SERIAL_COMMUNICATION_INTERFACE_STREAM.available();
  if (bytes_available)
  {
    QF::PUBLISH(&serialCommandAvailableEvt, &l_TIMER_ID);
  }
}

void BSP::readSerialCommand()
{
  byte first_byte = Serial.read();
  Serial.println(first_byte);
  // check to see if command is string
  if (first_byte > AC::constants::first_command_byte_max_value)
  {
    size_t bytes_available = AC::constants::SERIAL_COMMUNICATION_INTERFACE_STREAM.available();
    byte command_byte_array[bytes_available];
    Serial.readBytes(command_byte_array, bytes_available);

    char command_str[bytes_available + 2];
    command_str[0] = first_byte;
    // memcpy(command_str + 1, command_byte_array, bytes_available);
    // memcpy(command_str, command_byte_array, bytes_available);
    command_str[bytes_available] = 0;
    String command_string = String(command_str);
    String response = processCommandString(command_string);
    Serial.println(response);
    QF::PUBLISH(&commandProcessedEvt, &l_TIMER_ID);
   return;
  }

  // size_t bytes_available = AC::constants::SERIAL_COMMUNICATION_INTERFACE_STREAM.available();
  // if (bytes_available)
  // {

    // byte command_byte_array[bytes_available];
    // Serial.readBytes(command_byte_array, bytes_available);
  // }
}

void BSP::beginEthernet()
{
  if (Ethernet.begin(static_ip, subnet_mask, gateway))
  {
    AC::AO_EthernetCommandInterface->POST(&ethernetInitializedEvt, &l_TIMER_ID);
  }
}

void BSP::checkForEthernetIPAddress()
{
  IPAddress ip_address = Ethernet.localIP();
  if (ip_address)
  {
    AC::AO_EthernetCommandInterface->POST(&ethernetIPAddressFoundEvt, &l_TIMER_ID);
  }
}

void BSP::beginEthernetServer()
{
  ethernet_server.begin();
  if (ethernet_server)
  {
    AC::AO_EthernetCommandInterface->POST(&ethernetServerInitializedEvt, &l_TIMER_ID);
  }
}

void BSP::pollEthernetCommand()
{
  EthernetClient ethernet_client = ethernet_server.accept();
  if (ethernet_client && ethernet_client.available())
  {
    QF::PUBLISH(&ethernetCommandAvailableEvt, &l_TIMER_ID);
    // String command = ethernet_client.readStringUntil('\n');
    // String response = processCommandString(command);
    // ethernet_client.write(response.c_str());
  }
}

void displayFrameTimerHandler()
{
  QF::PUBLISH(&displayFrameTimeoutEvt, &l_TIMER_ID);
}

void BSP::armDisplayFrameTimer(uint32_t frequency_hz)
{
  uint32_t period_us = AC::constants::microseconds_per_second / frequency_hz;
  Timer3.stop();
  Timer3.setPeriod(period_us);
  Timer3.attachInterrupt(displayFrameTimerHandler);
  Timer3.start();
}

void BSP::disarmDisplayFrameTimer()
{
  Timer3.stop();
  Timer3.detachInterrupt();
}

void BSP::displayFrame()
{
  delay(2);
  // QF::PUBLISH(&frameDisplayedEvt, &l_TIMER_ID);
}

void BSP::setDisplayFrequency(uint32_t frequency_hz)
{
  static AC::SetDisplayFrequencyEvt setDisplayFrequencyEvt = { AC::SET_DISPLAY_FREQUENCY_SIG, 0U, 0U};
  setDisplayFrequencyEvt.display_frequency_hz = frequency_hz;
  AC::AO_Display->POST(&setDisplayFrequencyEvt, &l_TIMER_ID);
}

uint8_t BSP::getRegionRowPanelCountMax()
{
  return AC::constants::region_row_panel_count_max;
}

uint8_t BSP::getRegionColPanelCountMax()
{
  return AC::constants::region_col_panel_count_max;
}

void BSP::enablePanelSetSelectPin(uint8_t row_index, uint8_t col_index)
{
  for (uint8_t region_index = 0; region_index<AC::constants::region_count_per_frame; ++region_index)
  {
    SPIClass * spi_ptr = AC::constants::region_spi_ptrs[region_index];
    spi_ptr->beginTransaction(SPISettings(AC::constants::spi_clock_speed, AC::constants::spi_bit_order, AC::constants::spi_data_mode));
  }
  const uint8_t & pss_pin = AC::constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast(pss_pin, LOW);
  // Serial.print("setting ");
  // Serial.print(pss_pin);
  // Serial.println(" LOW");
}

void BSP::disablePanelSetSelectPin(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & pss_pin = AC::constants::panel_set_select_pins[row_index][col_index];
  digitalWriteFast(pss_pin, HIGH);
  for (uint8_t region_index = 0; region_index<AC::constants::region_count_per_frame; ++region_index)
  {
    SPIClass * spi_ptr = AC::constants::region_spi_ptrs[region_index];
    spi_ptr->endTransaction();
  }
  // Serial.print("setting ");
  // Serial.print(pss_pin);
  // Serial.println(" HIGH");
}

void BSP::transferPanelSet(const uint8_t (*panel_buffer)[], uint8_t panel_buffer_byte_count)
{
  transfer_panel_complete_count = 0;
  for (uint8_t region_index = 0; region_index<AC::constants::region_count_per_frame; ++region_index)
  {
    SPIClass * spi_ptr = AC::constants::region_spi_ptrs[region_index];
    spi_ptr->transfer(panel_buffer, NULL, panel_buffer_byte_count, transfer_panel_complete_event);
  }
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
  QF::TICK_X(0, &l_TIMER_ID); // process time events for tick rate 0
}
//............................................................................
void QF::onStartup()
{
  // configure the timer-counter channel........
  Timer1.initialize(TIMER1_CLCK_HZ / BSP::TICKS_PER_SEC);
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

#ifdef QS_ON

  // transmit QS outgoing data (QS-TX)
  uint16_t len = AC::constants::QS_SERIAL_STREAM.availableForWrite();
  if (len > 0U)
  { // any space available in the output buffer?
    uint8_t const *buf = QS::getBlock(&len);
    if (buf)
    {
      AC::constants::QS_SERIAL_STREAM.write(buf, len); // asynchronous and non-blocking
    }
  }

  // receive QS incoming data (QS-RX)
  len = AC::constants::QS_SERIAL_STREAM.available();
  if (len > 0U)
  {
    do
    {
      QP::QS::rxPut(AC::constants::QS_SERIAL_STREAM.read());
    } while (--len > 0U);
    QS::rxParse();
  }

#endif // QS_ON

#endif
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int location)
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
#ifdef QS_ON

//............................................................................
bool QP::QS::onStartup(void const * arg)
{
  static uint8_t qsTxBuf[1024]; // buffer for QS transmit channel (QS-TX)
  static uint8_t qsRxBuf[128];  // buffer for QS receive channel (QS-RX)
  initBuf  (qsTxBuf, sizeof(qsTxBuf));
  rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
  AC::constants::QS_SERIAL_STREAM.begin(115200); // run serial port at 115200 baud rate
  return true; // return success
}
//............................................................................
void QP::QS::onCommand(uint8_t cmdId, uint32_t param1,
  uint32_t param2, uint32_t param3)
{
}

#endif // QS_ON

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
#ifdef QS_ON
  uint16_t len = 0xFFFFU; // big number to get as many bytes as available
  uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
  while (buf != nullptr)
  { // data available?
    AC::constants::QS_SERIAL_STREAM.write(buf, len); // might poll until all bytes fit
    len = 0xFFFFU; // big number to get as many bytes as available
    buf = QS::getBlock(&len); // try to get more data
  }
  AC::constants::QS_SERIAL_STREAM.flush(); // wait for the transmission of outgoing data to complete
#endif // QS_ON
}
//............................................................................
void QP::QS::onReset()
{
  //??? TBD for Teensy
}
