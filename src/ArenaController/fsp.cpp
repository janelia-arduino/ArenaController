#include "fsp.hpp"


using namespace QP;

using namespace AC;

static QSpyId const l_FSP_ID = { 1U }; // QSpy source ID

static CommandEvt const resetEvt = { RESET_SIG, 0U, 0U};
static CommandEvt const allOnEvt = { ALL_ON_SIG, 0U, 0U};
static CommandEvt const allOffEvt = { ALL_OFF_SIG, 0U, 0U};

static QEvt const activateSerialCommandInterfaceEvt = {ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateSerialCommandInterfaceEvt = {DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const serialReadyEvt = {SERIAL_READY_SIG, 0U, 0U};
static QEvt const serialCommandAvailableEvt = {SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const activateEthernetCommandInterfaceEvt = {ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateEthernetCommandInterfaceEvt = {DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const ethernetInitializedEvt = {ETHERNET_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetIPAddressFoundEvt = {ETHERNET_IP_ADDRESS_FOUND_SIG, 0U, 0U};
static QEvt const ethernetServerInitializedEvt = {ETHERNET_SERVER_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetClientConnectedEvt = {ETHERNET_CLIENT_CONNECTED_SIG, 0U, 0U};
static QEvt const ethernetCommandAvailableEvt = {ETHERNET_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const deactivateDisplayEvt = {DEACTIVATE_DISPLAY_SIG, 0U, 0U};
static QEvt const frameTransferredEvt = {FRAME_TRANSFERRED_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = {COMMAND_PROCESSED_SIG, 0U, 0U};

//----------------------------------------------------------------------------
// Local functions

void FSP::ArenaController_setup()
{
  QF::init(); // initialize the framework
  BSP::init(); // initialize the BSP

  // init publish-subscribe
  static QSubscrList subscrSto[MAX_PUB_SIG];
  QF::psInit(subscrSto, Q_DIM(subscrSto));

  // statically allocate event queues for the AOs and start them...
  static QEvt const *watchdog_queueSto[2];
  AO_Watchdog->start(1U, // priority
    watchdog_queueSto, Q_DIM(watchdog_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *serial_command_interface_queueSto[10];
  AO_SerialCommandInterface->start(2U, // priority
    serial_command_interface_queueSto, Q_DIM(serial_command_interface_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *ethernet_command_interface_queueSto[10];
  AO_EthernetCommandInterface->start(3U, // priority
    ethernet_command_interface_queueSto, Q_DIM(ethernet_command_interface_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *arena_queueSto[10];
  AO_Arena->start(4U, // priority
    arena_queueSto, Q_DIM(arena_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *display_queueSto[10];
  AO_Display->start(5U, // priority
    display_queueSto, Q_DIM(display_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *frame_queueSto[10];
  AO_Frame->start(6U, // priority
    frame_queueSto, Q_DIM(frame_queueSto),
    (void *)0, 0U); // no stack

  //...
}

void FSP::Arena_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  BSP::initializeArena();
  ao->subscribe(RESET_SIG);
  ao->subscribe(ALL_ON_SIG);
  ao->subscribe(ALL_OFF_SIG);
}

void FSP::Arena_activateCommandInterfaces(QActive * const ao, QEvt const * e)
{
  AO_SerialCommandInterface->POST(&activateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&activateEthernetCommandInterfaceEvt, &l_FSP_ID);
}

void FSP::Arena_deactivateCommandInterfaces(QActive * const ao, QEvt const * e)
{
  AO_SerialCommandInterface->POST(&deactivateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&deactivateEthernetCommandInterfaceEvt, &l_FSP_ID);
}

void FSP::Arena_deactivateDisplay(QActive * const ao, QEvt const * e)
{
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_displayAllOnFrames(QActive * const ao, QEvt const * e)
{
  static DisplayFramesEvt displayFramesEvt = { DISPLAY_FRAMES_SIG, 0U, 0U};
  displayFramesEvt.panel_buffer = &constants::all_on_grayscale_pattern;
  displayFramesEvt.panel_buffer_byte_count = constants::byte_count_per_panel_grayscale;
  QF::PUBLISH(&displayFramesEvt, ao);
}

void FSP::Display_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  ao->subscribe(DEACTIVATE_DISPLAY_SIG);
  ao->subscribe(DISPLAY_FRAMES_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);
  display->display_frequency_hz_ = constants::display_frequency_hz_default;
}

void FSP::Display_setDisplayFrequency(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  display->display_frequency_hz_ = Q_EVT_CAST(SetDisplayFrequencyEvt)->display_frequency_hz;
}

void FSP::Display_displayFrames(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  display->panel_buffer_ = Q_EVT_CAST(DisplayFramesEvt)->panel_buffer;
  display->panel_buffer_byte_count_ = Q_EVT_CAST(DisplayFramesEvt)->panel_buffer_byte_count;
}

void FSP::Display_armDisplayFrameTimer(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  display->display_time_evt_.armX(constants::ticks_per_second/display->display_frequency_hz_,
    constants::ticks_per_second/display->display_frequency_hz_);
}

void FSP::Display_disarmDisplayFrameTimer(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  display->display_time_evt_.disarm();
}

void FSP::Display_transferFrame(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  static TransferFrameEvt transferFrameEvt = { TRANSFER_FRAME_SIG, 0U, 0U};
  transferFrameEvt.panel_buffer = display->panel_buffer_;
  transferFrameEvt.panel_buffer_byte_count = display->panel_buffer_byte_count_;
  transferFrameEvt.region_row_panel_count = BSP::getRegionRowPanelCountMax();
  transferFrameEvt.region_col_panel_count = BSP::getRegionColPanelCountMax();
  QF::PUBLISH(&transferFrameEvt, ao);
}

void FSP::SerialCommandInterface_subscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);
}

void FSP::SerialCommandInterface_armSerialTimer(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.armX(constants::ticks_per_second/2, constants::ticks_per_second/50);
}

void FSP::SerialCommandInterface_disarmSerialTimer(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.disarm();
}

void FSP::SerialCommandInterface_beginSerial(QActive * const ao, QEvt const * e)
{
  bool serial_ready = BSP::beginSerial();
  if (serial_ready)
  {
    AC::AO_SerialCommandInterface->POST(&serialReadyEvt, &l_FSP_ID);
  }
}

void FSP::SerialCommandInterface_pollSerialCommand(QActive * const ao, QEvt const * e)
{
  bool bytes_available = BSP::pollSerialCommand();
  if (bytes_available)
  {
    QF::PUBLISH(&serialCommandAvailableEvt, &l_FSP_ID);
  }
}

void FSP::SerialCommandInterface_readFirstByte(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->first_command_byte_ = BSP::readSerialByte();
}

bool FSP::SerialCommandInterface_ifBinaryCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  return (sci->first_command_byte_ <= constants::first_command_byte_max_value_binary);
}

void FSP::SerialCommandInterface_readSerialStringCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->string_command_ = BSP::readSerialStringCommand(sci->first_command_byte_);
}

void FSP::SerialCommandInterface_processStringCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->string_response_ = FSP::processStringCommand(sci->string_command_);
}

void FSP::SerialCommandInterface_writeSerialStringResponse(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  BSP::writeSerialStringResponse(sci->string_response_);
}

void FSP::SerialCommandInterface_writeSerialBinaryResponse(QActive * const ao, QEvt const * e)
{
}

void FSP::EthernetCommandInterface_subscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);
}

void FSP::EthernetCommandInterface_armEthernetTimer(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.armX(constants::ticks_per_second/2, constants::ticks_per_second/50);
}

void FSP::EthernetCommandInterface_disarmEthernetTimer(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.disarm();
}

void FSP::EthernetCommandInterface_beginEthernet(QActive * const ao, QEvt const * e)
{
  bool ethernet_begun = BSP::beginEthernet();
  if (ethernet_begun)
  {
    AC::AO_EthernetCommandInterface->POST(&ethernetInitializedEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_checkForIPAddress(QActive * const ao, QEvt const * e)
{
  bool ip_address_found = BSP::checkForEthernetIPAddress();
  if (ip_address_found)
  {
    AO_EthernetCommandInterface->POST(&ethernetIPAddressFoundEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_beginServer(QActive * const ao, QEvt const * e)
{
  bool ethernet_server_begun = BSP::beginEthernetServer();
  if (ethernet_server_begun)
  {
    AO_EthernetCommandInterface->POST(&ethernetServerInitializedEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_checkForClient(QActive * const ao, QEvt const * e)
{
  bool ethernet_client_connected = BSP::checkForEthernetClient();
  if (ethernet_client_connected)
  {
    QF::PUBLISH(&ethernetClientConnectedEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_pollEthernetCommand(QActive * const ao, QEvt const * e)
{
  bool bytes_available = BSP::pollEthernetCommand();
  if (bytes_available)
  {
    QF::PUBLISH(&ethernetCommandAvailableEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_readEthernetBinaryCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  BSP::readEthernetBinaryCommand();
  // eci->binary_command_ = BSP::readEthernetBinaryCommand(sci->first_command_byte_);
}

void FSP::EthernetCommandInterface_writeEthernetBinaryResponse(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  // eci->string_command_ = BSP::readSerialStringCommand(sci->first_command_byte_);
}

void FSP::Frame_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  BSP::initializeFrame();
  ao->subscribe(TRANSFER_FRAME_SIG);
  ao->subscribe(PANEL_SET_TRANSFERRED_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);
}

void FSP::Frame_resetIndicies(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  frame->panel_set_row_index_ = 0;
  frame->panel_set_col_index_ = 0;
}

void FSP::Frame_beginTransferPanelSet(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::enablePanelSetSelectPin(frame->panel_set_row_index_, frame->panel_set_col_index_);
  BSP::transferPanelSet(frame->panel_buffer_, frame->panel_buffer_byte_count_);
}

void FSP::Frame_endTransferPanelSet(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::disablePanelSetSelectPin(frame->panel_set_row_index_, frame->panel_set_col_index_);
  ++frame->panel_set_row_index_;
  if (frame->panel_set_row_index_ == frame->region_row_panel_count_)
  {
    frame->panel_set_row_index_ = 0;
    ++frame->panel_set_col_index_;
  }
  if (frame->panel_set_col_index_ == frame->region_col_panel_count_)
  {
    frame->panel_set_col_index_ = 0;
  }
}

bool FSP::Frame_ifFrameNotTransferred(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  return (frame->panel_set_row_index_ != (frame->region_row_panel_count_-1)) ||
    (frame->panel_set_col_index_ != (frame->region_col_panel_count_-1));
}

void FSP::Frame_publishFrameTransferred(QActive * const ao, QEvt const * e)
{
  QF::PUBLISH(&frameTransferredEvt, ao);
}

void FSP::Watchdog_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(RESET_SIG);
  BSP::initializeWatchdog();
}

void FSP::Watchdog_armWatchdogTimer(QActive * const ao, QEvt const * e)
{
  Watchdog * const watchdog = static_cast<Watchdog * const>(ao);
  watchdog->watchdog_time_evt_.armX(constants::ticks_per_second, constants::ticks_per_second);
}

void FSP::Watchdog_disarmWatchdogTimer(QActive * const ao, QEvt const * e)
{
  Watchdog * const watchdog = static_cast<Watchdog * const>(ao);
  watchdog->watchdog_time_evt_.disarm();
}

void FSP::Watchdog_feedWatchdog(QActive * const ao, QEvt const * e)
{
  BSP::feedWatchdog();
}

String FSP::processStringCommand(String command)
{
  command.trim();
  String response = command;
  if (command.equalsIgnoreCase("RESET"))
  {
    QF::PUBLISH(&resetEvt, &l_FSP_ID);
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
    QF::PUBLISH(&allOnEvt, &l_FSP_ID);
  }
  else if (command.equalsIgnoreCase("ALL_OFF"))
  {
    QF::PUBLISH(&allOffEvt, &l_FSP_ID);
  }
  else if (command.equalsIgnoreCase("EHS"))
  {
    //response = String(Ethernet.hardwareStatus());
  }
  else if (command.equalsIgnoreCase("ELS"))
  {
    //response = String(Ethernet.linkStatus());
  }
  else if (command.equalsIgnoreCase("GET_IP_ADDRESS"))
  {
    //response = ipAddressToString(Ethernet.localIP());
  }
  else if (command.startsWith("SET_DISPLAY_FREQUENCY"))
  {
    //command.replace("SET_DISPLAY_FREQUENCY", "");
    //command.trim();
    //uint32_t frequency_hz = command.toInt();
    //BSP::setDisplayFrequency(frequency_hz);
  }
  QF::PUBLISH(&commandProcessedEvt, &l_FSP_ID);
  return response;
}
