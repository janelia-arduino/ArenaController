#include <Arduino.h>
#include <NativeEthernet.h>

#include "bsp.hpp"
#include "ArenaController.hpp"
#include "BspConstants.hpp"


using namespace QP;

//----------------------------------------------------------------------------
// QS facilities

// un-comment if QS instrumentation needed
//#define QS_ON

static QP::QSpyId const l_TIMER_ID = { 0U }; // QSpy source ID

//----------------------------------------------------------------------------
// Static global variables
static EthernetServer ethernet_server(AC::constants::port);
static EthernetClient ethernet_client;

//----------------------------------------------------------------------------
// Local functions
void getMacAddress(uint8_t * mac_address)
{
  for(uint8_t by=0; by<2; by++) mac_address[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
  for(uint8_t by=0; by<4; by++) mac_address[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
  // Serial.printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
}

//----------------------------------------------------------------------------
// BSP functions

//............................................................................
void BSP::init()
{
  // initialize the hardware used in this sketch...
  // NOTE: interrupts are configured and started later in QF::onStartup()

  // setup pins
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(AC::constants::reset_pin, OUTPUT);
  digitalWriteFast(AC::constants::reset_pin, LOW);

  for (uint8_t col_index = 0; col_index<AC::constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<AC::constants::panel_count_max_per_region_row; ++row_index)
    {
      const uint8_t & cs_pin = AC::constants::panel_select_pins[row_index][col_index];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
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
//............................................................................
void BSP::activateCommandInterfaces()
{
#ifndef QS_ON
  static QEvt const activateSerialCommandInterfaceEvt = { AC::ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
  QF::PUBLISH(&activateSerialCommandInterfaceEvt, &l_TIMER_ID);
#endif

  static QEvt const activateEthernetCommandInterfaceEvt = { AC::ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
  QF::PUBLISH(&activateEthernetCommandInterfaceEvt, &l_TIMER_ID);
}
//............................................................................
void BSP::deactivateCommandInterfaces()
{
#ifndef QS_ON
  static QEvt const deactivateSerialCommandInterfaceEvt = { AC::DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
  QF::PUBLISH(&deactivateSerialCommandInterfaceEvt, &l_TIMER_ID);
#endif

  static QEvt const deactivateEthernetCommandInterfaceEvt = { AC::DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
  QF::PUBLISH(&deactivateEthernetCommandInterfaceEvt, &l_TIMER_ID);
}
//............................................................................
void BSP::beginSerial()
{
  AC::constants::serial_communication_interface_stream.begin(AC::constants::SERIAL_COMMUNICATION_INTERFACE_BAUD_RATE);
  AC::constants::serial_communication_interface_stream.setTimeout(AC::constants::SERIAL_COMMUNICATION_INTERFACE_TIMEOUT);
  static QEvt const serialReadyEvt = { AC::SERIAL_READY_SIG, 0U, 0U};
  QF::PUBLISH(&serialReadyEvt, &l_TIMER_ID);
}
//............................................................................
void BSP::pollSerialCommand()
{
  if (AC::constants::serial_communication_interface_stream.available() > 0)
  {
    String command = AC::constants::serial_communication_interface_stream.readStringUntil('\n');
    if (command.equalsIgnoreCase("ALL_ON"))
    {
      static AC::CommandEvt const allOnEvt = { AC::ALL_ON_SIG, 0U, 0U};
      QF::PUBLISH(&allOnEvt, &l_TIMER_ID);
    }
    else if (command.equalsIgnoreCase("ALL_OFF"))
    {
      static AC::CommandEvt const allOffEvt = { AC::ALL_OFF_SIG, 0U, 0U};
      QF::PUBLISH(&allOffEvt, &l_TIMER_ID);
    }
  }
}
//............................................................................
void BSP::beginEthernet()
{
  uint8_t mac_address[AC::constants::mac_address_size];
  getMacAddress(mac_address);

  Ethernet.begin(mac_address, AC::constants::ip);
  static QEvt const ethernetInitializedEvt = { AC::ETHERNET_INITIALIZED_SIG, 0U, 0U};
  QF::PUBLISH(&ethernetInitializedEvt, &l_TIMER_ID);
}
//............................................................................
void BSP::beginEthernetServer()
{
  ethernet_server.begin();
  static QEvt const ethernetServerInitializedEvt = { AC::ETHERNET_SERVER_INITIALIZED_SIG, 0U, 0U};
  QF::PUBLISH(&ethernetServerInitializedEvt, &l_TIMER_ID);
}
//............................................................................
void BSP::checkForEthernetClient()
{
  ethernet_client = ethernet_server.available();
  if (ethernet_client)
  {
    static QEvt const ethernetClientConnectedEvt = { AC::ETHERNET_CLIENT_CONNECTED_SIG, 0U, 0U};
    QF::PUBLISH(&ethernetClientConnectedEvt, &l_TIMER_ID);
  }
  else
  {
    Serial.print("No Ethernet client connected. ");
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }
}
//............................................................................
void BSP::pollEthernetCommand()
{
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}
//............................................................................
void BSP::ledOff()
{
  digitalWriteFast(LED_BUILTIN, LOW);
}
//............................................................................
void BSP::ledOn()
{
  digitalWriteFast(LED_BUILTIN, HIGH);
}
//............................................................................
#include <TimerThree.h>  // Teensy Timer3 interface
void displayFrameTimerHandler()
{
  static QEvt const displayFrameTimeoutEvt = { AC::DISPLAY_FRAME_TIMEOUT_SIG, 0U, 0U};
  QF::PUBLISH(&displayFrameTimeoutEvt, &l_TIMER_ID);
}

void BSP::armDisplayFrameTimer(uint32_t frequency_hz)
{
  uint32_t period_us = AC::constants::MICROSECONDS_PER_SECOND / frequency_hz;
  Timer3.initialize(period_us);
  Timer3.attachInterrupt(displayFrameTimerHandler);
}
void BSP::disarmDisplayFrameTimer()
{
  Timer3.detachInterrupt();
}
//............................................................................
void BSP::displayFrame()
{
  ledOn();
  delay(2);
  static QEvt const frameDisplayedEvt = { AC::FRAME_DISPLAYED_SIG, 0U, 0U};
  QF::PUBLISH(&frameDisplayedEvt, &l_TIMER_ID);
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
#include <TimerOne.h>  // Teensy Timer1 interface

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
  uint16_t len = AC::constants::serial_communication_interface_stream.availableForWrite();
  if (len > 0U)
  { // any space available in the output buffer?
    uint8_t const *buf = QS::getBlock(&len);
    if (buf)
    {
      AC::constants::serial_communication_interface_stream.write(buf, len); // asynchronous and non-blocking
    }
  }

  // receive QS incoming data (QS-RX)
  len = AC::constants::serial_communication_interface_stream.available();
  if (len > 0U)
  {
    do
    {
      QP::QS::rxPut(AC::constants::serial_communication_interface_stream.read());
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
  Serial.begin(115200); // run serial port at 115200 baud rate
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
    Serial.write(buf, len); // might poll until all bytes fit
    len = 0xFFFFU; // big number to get as many bytes as available
    buf = QS::getBlock(&len); // try to get more data
  }
  Serial.flush(); // wait for the transmission of outgoing data to complete
#endif // QS_ON
}
//............................................................................
void QP::QS::onReset()
{
  //??? TBD for Teensy
}
