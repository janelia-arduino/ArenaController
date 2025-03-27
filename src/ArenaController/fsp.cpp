#include "fsp.hpp"


using namespace QP;

using namespace AC;

static QSpyId const l_FSP_ID = {0U}; // QSpy source ID

static CommandEvt const resetEvt = {RESET_SIG, 0U, 0U};
static CommandEvt const allOnEvt = {ALL_ON_SIG, 0U, 0U};
static CommandEvt const allOffEvt = {ALL_OFF_SIG, 0U, 0U};

static QEvt const activateSerialCommandInterfaceEvt = {ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateSerialCommandInterfaceEvt = {DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const serialReadyEvt = {SERIAL_READY_SIG, 0U, 0U};
static QEvt const serialCommandAvailableEvt = {SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const activateEthernetCommandInterfaceEvt = {ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateEthernetCommandInterfaceEvt = {DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const ethernetInitializedEvt = {ETHERNET_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetServerConnectedEvt = {ETHERNET_SERVER_CONNECTED_SIG, 0U, 0U};
// static QEvt const ethernetCommandAvailableEvt = {ETHERNET_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const deactivateDisplayEvt = {DEACTIVATE_DISPLAY_SIG, 0U, 0U};
static QEvt const frameTransferredEvt = {FRAME_TRANSFERRED_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = {COMMAND_PROCESSED_SIG, 0U, 0U};

//----------------------------------------------------------------------------
// Local functions

void FSP::ArenaController_setup()
{
  QF::init(); // initialize the framework

  QS_INIT(nullptr);

  BSP::init(); // initialize the BSP

  // object dictionaries for AOs...
  QS_OBJ_DICTIONARY(AO_Arena);
  QS_OBJ_DICTIONARY(AO_SerialCommandInterface);
  QS_OBJ_DICTIONARY(AO_EthernetCommandInterface);
  QS_OBJ_DICTIONARY(AO_Display);
  QS_OBJ_DICTIONARY(AO_Frame);
  QS_OBJ_DICTIONARY(AO_Watchdog);

  QS_OBJ_DICTIONARY(&l_FSP_ID);

  // signal dictionaries for globally published events...
  QS_SIG_DICTIONARY(RESET_SIG, nullptr);
  QS_SIG_DICTIONARY(ALL_ON_SIG, nullptr);
  QS_SIG_DICTIONARY(ALL_OFF_SIG, nullptr);
  QS_SIG_DICTIONARY(DEACTIVATE_DISPLAY_SIG, nullptr);
  QS_SIG_DICTIONARY(DISPLAY_FRAMES_SIG, nullptr);
  QS_SIG_DICTIONARY(TRANSFER_FRAME_SIG, nullptr);
  QS_SIG_DICTIONARY(SERIAL_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY(ETHERNET_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY(COMMAND_PROCESSED_SIG, nullptr);

  // user record dictionaries
  QS_USR_DICTIONARY(ETHERNET_LOG);
  QS_USR_DICTIONARY(USER_COMMENT);

  // setup the QS filters...
  // QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
  // QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
  QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records

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
  // AO_SerialCommandInterface->POST(&activateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&activateEthernetCommandInterfaceEvt, &l_FSP_ID);
}

void FSP::Arena_deactivateCommandInterfaces(QActive * const ao, QEvt const * e)
{
  // AO_SerialCommandInterface->POST(&deactivateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&deactivateEthernetCommandInterfaceEvt, &l_FSP_ID);
}

void FSP::Arena_deactivateDisplay(QActive * const ao, QEvt const * e)
{
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_displayAllOnFrames(QActive * const ao, QEvt const * e)
{
  static DisplayFramesEvt displayFramesEvt = {DISPLAY_FRAMES_SIG, 0U, 0U};
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

  QS_OBJ_DICTIONARY(&(display->display_time_evt_));
  QS_SIG_DICTIONARY(DISPLAY_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(SET_DISPLAY_FREQUENCY_SIG, ao);
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
  static TransferFrameEvt transferFrameEvt = {TRANSFER_FRAME_SIG, 0U, 0U};
  transferFrameEvt.panel_buffer = display->panel_buffer_;
  transferFrameEvt.panel_buffer_byte_count = display->panel_buffer_byte_count_;
  transferFrameEvt.region_row_panel_count = BSP::getRegionRowPanelCountMax();
  transferFrameEvt.region_col_panel_count = BSP::getRegionColPanelCountMax();
  QF::PUBLISH(&transferFrameEvt, ao);
}

void FSP::SerialCommandInterface_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);

  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  QS_OBJ_DICTIONARY(&(sci->serial_time_evt_));
  QS_SIG_DICTIONARY(SERIAL_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(SERIAL_READY_SIG, ao);
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
    AO_SerialCommandInterface->POST(&serialReadyEvt, &l_FSP_ID);
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
  BSP::readSerialStringCommand(sci->string_command_, (char)sci->first_command_byte_);
}

void FSP::SerialCommandInterface_processStringCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  FSP::processStringCommand(sci->string_command_, sci->string_response_);
}

void FSP::SerialCommandInterface_writeSerialStringResponse(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  BSP::writeSerialStringResponse(sci->string_response_);
}

void FSP::SerialCommandInterface_writeSerialBinaryResponse(QActive * const ao, QEvt const * e)
{
}

void FSP::EthernetCommandInterface_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);

  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  QS_OBJ_DICTIONARY(&(eci->ethernet_time_evt_));
  QS_SIG_DICTIONARY(ETHERNET_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(ETHERNET_INITIALIZED_SIG, ao);
  QS_SIG_DICTIONARY(ETHERNET_SERVER_CONNECTED_SIG, ao);
}

void FSP::EthernetCommandInterface_armEthernetTimer(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.armX(constants::ticks_per_second, constants::ticks_per_second/100);
}

void FSP::EthernetCommandInterface_disarmEthernetTimer(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.disarm();
}

void FSP::EthernetCommandInterface_initializeEthernet(QActive * const ao, QEvt const * e)
{
  bool ethernet_initialized = BSP::initializeEthernet();
  if (ethernet_initialized)
  {
    AO_EthernetCommandInterface->POST(&ethernetInitializedEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_pollEthernet(QActive * const ao, QEvt const * e)
{
  BSP::pollEthernet();
}

void FSP::EthernetCommandInterface_createServerConnection(QActive * const ao, QEvt const * e)
{
  bool server_connected = BSP::createEthernetServerConnection();
  if (server_connected)
  {
    AO_EthernetCommandInterface->POST(&ethernetServerConnectedEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_processBinaryCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  EthernetCommandEvt const * ece = static_cast<EthernetCommandEvt const *>(e);
  FSP::processBinaryCommand(ece->binary_command, ece->binary_command_byte_count);
}

// void FSP::EthernetCommandInterface_writeEthernetBinaryResponse(QActive * const ao, QEvt const * e)
// {
//   EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
//   // eci->string_command_ = BSP::readSerialStringCommand(sci->first_command_byte_);
// }

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

  Watchdog * const watchdog = static_cast<Watchdog * const>(ao);
  QS_OBJ_DICTIONARY(&(watchdog->watchdog_time_evt_));
  QS_SIG_DICTIONARY(WATCHDOG_TIMEOUT_SIG, ao);
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

// void FSP::processBinaryCommand(const void * command_buf,
//     size_t command_len,
//     void * response_buf,
//     size_t response_len)
void FSP::processBinaryCommand(uint8_t const * command_buf,
  size_t command_len)
{
  if (command_len < 2)
  {
    return;
  }
  uint8_t first_command_byte = (uint8_t)(*command_buf);
  if (first_command_byte > 32)
  {
    QS_BEGIN_ID(ETHERNET_LOG, AO_EthernetCommandInterface->m_prio)
      QS_STR("string?");
    QS_END()
      return;
  }
  else if (first_command_byte == 32)
  {
    QS_BEGIN_ID(ETHERNET_LOG, AO_EthernetCommandInterface->m_prio)
      QS_STR("stream frame");
    QS_END()
      return;
  }
  uint8_t second_command_byte = (uint8_t)(*(command_buf + 1));
  switch (second_command_byte)
  {
    case 0xFF: {
      QF::PUBLISH(&allOnEvt, &l_FSP_ID);
      break;
    }
    default:
      break;
  }
}

void FSP::processStringCommand(const char * command, char * response)
{
  strcpy(response, command);
  if (strcmp(command, "RESET") == 0)
  {
    QF::PUBLISH(&resetEvt, &l_FSP_ID);
  }
  if (strcmp(command, "LED_ON") == 0)
  {
    BSP::ledOn();
  }
  else if (strcmp(command, "LED_OFF") == 0)
  {
    BSP::ledOff();
  }
  else if (strcmp(command, "ALL_ON") == 0)
  {
    QF::PUBLISH(&allOnEvt, &l_FSP_ID);
  }
  else if (strcmp(command, "ALL_OFF") == 0)
  {
    QF::PUBLISH(&allOffEvt, &l_FSP_ID);
  }
  else if (strcmp(command, "EHS") == 0)
  {
    // BSP::getEthernetHardwareStatusString(response);
  }
  else if (strcmp(command, "ELS") == 0)
  {
    // BSP::getEthernetLinkStatusString(response);
  }
  else if (strcmp(command, "SIP") == 0)
  {
    // BSP::getServerIpAddressString(response);
  }
  else if (strcmp(command, "SET_DISPLAY_FREQUENCY") == 0)
  {
    //command.replace("SET_DISPLAY_FREQUENCY", "") == 0;
    //command.trim();
    //uint32_t frequency_hz = command.toInt();
    //BSP::setDisplayFrequency(frequency_hz);
  }
  QF::PUBLISH(&commandProcessedEvt, &l_FSP_ID);
}
