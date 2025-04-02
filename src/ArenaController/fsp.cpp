#include "fsp.hpp"


using namespace QP;

using namespace AC;

static QSpyId const l_FSP_ID = {0U}; // QSpy source ID

static QEvt const resetEvt = {RESET_SIG, 0U, 0U};

static QEvt const deactivateDisplayEvt = {DEACTIVATE_DISPLAY_SIG, 0U, 0U};
static QEvt const frameFilledEvt = {FRAME_FILLED_SIG, 0U, 0U};
static QEvt const displayFramesEvt = {DISPLAY_FRAMES_SIG, 0U, 0U};
static QEvt const transferFrameEvt = {TRANSFER_FRAME_SIG, 0U, 0U};
static QEvt const frameTransferredEvt = {FRAME_TRANSFERRED_SIG, 0U, 0U};

static QEvt const processBinaryCommandEvt = {PROCESS_BINARY_COMMAND_SIG, 0U, 0U};
static QEvt const processStringCommandEvt = {PROCESS_STRING_COMMAND_SIG, 0U, 0U};
static QEvt const processStreamCommandEvt = {PROCESS_STREAM_COMMAND_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = {COMMAND_PROCESSED_SIG, 0U, 0U};

static QEvt const allOnEvt = {ALL_ON_SIG, 0U, 0U};
static QEvt const allOffEvt = {ALL_OFF_SIG, 0U, 0U};
static QEvt const streamFrameEvt = {STREAM_FRAME_SIG, 0U, 0U};

static QEvt const activateSerialCommandInterfaceEvt = {ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateSerialCommandInterfaceEvt = {DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const serialReadyEvt = {SERIAL_READY_SIG, 0U, 0U};
static QEvt const serialCommandAvailableEvt = {SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const activateEthernetCommandInterfaceEvt = {ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateEthernetCommandInterfaceEvt = {DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const ethernetInitializedEvt = {ETHERNET_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetServerConnectedEvt = {ETHERNET_SERVER_CONNECTED_SIG, 0U, 0U};

static QEvt const fillFrameBufferWithAllOnEvt = {FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, 0U, 0U};
static QEvt const fillFrameBufferWithStreamEvt = {FILL_FRAME_BUFFER_WITH_STREAM_SIG, 0U, 0U};

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
  QS_SIG_DICTIONARY(DEACTIVATE_DISPLAY_SIG, nullptr);
  QS_SIG_DICTIONARY(FRAME_FILLED_SIG, nullptr);
  QS_SIG_DICTIONARY(DISPLAY_FRAMES_SIG, nullptr);
  QS_SIG_DICTIONARY(TRANSFER_FRAME_SIG, nullptr);
  QS_SIG_DICTIONARY(SERIAL_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY(ETHERNET_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY(PROCESS_BINARY_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY(PROCESS_STRING_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY(PROCESS_STREAM_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY(COMMAND_PROCESSED_SIG, nullptr);

  // user record dictionaries
  QS_USR_DICTIONARY(ETHERNET_LOG);
  QS_USR_DICTIONARY(USER_COMMENT);

  // setup the QS filters...
  // QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records ON
  // QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records ON
  // QS_GLB_FILTER(QP::QS_QEP_STATE_ENTRY);
  // QS_GLB_FILTER(QP::QS_QEP_STATE_EXIT);
  // QS_GLB_FILTER(QP::QS_QEP_TRAN);
  // QS_GLB_FILTER(QP::QS_QEP_INTERN_TRAN);
  QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records ON
  // QS_GLB_FILTER(-QP::QS_U0_RECORDS); // ethernet records OFF
  // QS_GLB_FILTER(QP::QS_U1_RECORDS); // user records ON

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

  QS_LOC_FILTER(-AO_Watchdog->m_prio);
  QS_LOC_FILTER(-AO_Display->m_prio);
  QS_LOC_FILTER(-AO_Frame->m_prio);
  // QS_LOC_FILTER(-AO_EthernetCommandInterface->m_prio);
  QS_LOC_FILTER(-AO_SerialCommandInterface->m_prio);
}

void FSP::Arena_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  BSP::initializeArena();

  ao->subscribe(FRAME_FILLED_SIG);

  QS_SIG_DICTIONARY(ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY(ALL_OFF_SIG, ao);
  QS_SIG_DICTIONARY(STREAM_FRAME_SIG, ao);
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
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("deactivating display");
  QS_END()
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_displayFrames(QActive * const ao, QEvt const * e)
{
  QF::PUBLISH(&displayFramesEvt, ao);
}

void FSP::Arena_fillFrameBufferWithAllOn(QActive * const ao, QEvt const * e)
{
  AO_Frame->POST(&fillFrameBufferWithAllOnEvt, &l_FSP_ID);
}

void FSP::Arena_fillFrameBufferWithStream(QActive * const ao, QEvt const * e)
{
  AO_Frame->POST(&fillFrameBufferWithStreamEvt, &l_FSP_ID);
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
  QF::PUBLISH(&commandProcessedEvt, &l_FSP_ID);
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
  ao->subscribe(PROCESS_BINARY_COMMAND_SIG);
  ao->subscribe(PROCESS_STRING_COMMAND_SIG);
  ao->subscribe(PROCESS_STREAM_COMMAND_SIG);
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

void FSP::EthernetCommandInterface_analyzeCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  EthernetCommandEvt const * ece = static_cast<EthernetCommandEvt const *>(e);
  eci->connection_ = ece->connection;
  eci->binary_command_ = ece->binary_command;
  eci->binary_command_byte_count_ = ece->binary_command_byte_count;

  uint8_t first_command_byte = (uint8_t)(eci->binary_command_[0]);
  if (first_command_byte > constants::first_command_byte_max_value_binary)
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("string command");
    QS_END()
    QF::PUBLISH(&processStringCommandEvt, &l_FSP_ID);
  }
  else if (first_command_byte == constants::first_command_byte_max_value_binary)
  {
    uint32_t low_byte = (uint8_t)(eci->binary_command_[1]);
    uint32_t high_byte = (uint8_t)(eci->binary_command_[2]);
    eci->binary_command_byte_count_claim_ = (high_byte << constants::bit_count_per_byte) | low_byte;
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("stream command");
      QS_U32(8, eci->binary_command_byte_count_claim_);
    QS_END()
    QF::PUBLISH(&processStreamCommandEvt, &l_FSP_ID);
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("binary command");
    QS_END()
    QF::PUBLISH(&processBinaryCommandEvt, &l_FSP_ID);
  }
}

void FSP::EthernetCommandInterface_updateStreamCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  EthernetCommandEvt const * ece = static_cast<EthernetCommandEvt const *>(e);
  eci->binary_command_ = ece->binary_command;
  eci->binary_command_byte_count_ = ece->binary_command_byte_count;
}

bool FSP::EthernetCommandInterface_ifStreamCommandComplete(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  return (eci->binary_command_byte_count_ >= eci->binary_command_byte_count_claim_);
}

void FSP::EthernetCommandInterface_processBinaryCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->binary_response_byte_count_ = FSP::processBinaryCommand(eci->binary_command_,
    eci->binary_command_byte_count_,
    eci->binary_response_);
  QF::PUBLISH(&commandProcessedEvt, &l_FSP_ID);
}

void FSP::EthernetCommandInterface_processStreamCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->binary_response_byte_count_ = 3;
  eci->binary_response_[0] = 2;
  eci->binary_response_[1] = 0;
  eci->binary_response_[2] = constants::first_command_byte_max_value_binary;
  uint8_t const * command_buffer = eci->binary_command_ + constants::stream_header_byte_count;
  uint32_t command_byte_count = eci->binary_command_byte_count_ - constants::stream_header_byte_count;
  FSP::processStreamCommand(command_buffer, command_byte_count);
  QF::PUBLISH(&commandProcessedEvt, &l_FSP_ID);
}

void FSP::EthernetCommandInterface_writeBinaryResponse(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  BSP::writeEthernetBinaryResponse(eci->connection_, eci->binary_response_, eci->binary_response_byte_count_);
}

void FSP::Frame_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::initializeFrame();
  frame->buffer_ = BSP::getFrameBuffer();
  frame->buffer_byte_count_ = 0;
  ao->subscribe(DEACTIVATE_DISPLAY_SIG);
  ao->subscribe(TRANSFER_FRAME_SIG);
  ao->subscribe(PANEL_SET_TRANSFERRED_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);

  QS_SIG_DICTIONARY(FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY(FILL_FRAME_BUFFER_WITH_STREAM_SIG, ao);
}

void FSP::Frame_fillFrameBufferWithAllOn(QActive * const ao, QEvt const * e)
{
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("begin all on fill");
  QS_END()
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::fillFrameBufferWithAllOn(frame->buffer_,
    frame->buffer_byte_count_,
    frame->panel_byte_count_,
    frame->region_row_panel_count_,
    frame->region_col_panel_count_);
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("end all on fill");
    QS_U32(8, frame->buffer_byte_count_);
  QS_END()
  QF::PUBLISH(&frameFilledEvt, ao);
}

void FSP::Frame_fillFrameBufferWithStream(QActive * const ao, QEvt const * e)
{
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("begin stream fill");
  QS_END()
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::fillFrameBufferWithStream(frame->buffer_,
    frame->buffer_byte_count_,
    frame->panel_byte_count_,
    frame->region_row_panel_count_,
    frame->region_col_panel_count_);
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("end stream fill");
    QS_U32(8, frame->buffer_byte_count_);
  QS_END()
  QF::PUBLISH(&frameFilledEvt, ao);
}

void FSP::Frame_reset(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  frame->panel_set_row_index_ = 0;
  frame->panel_set_col_index_ = 0;
  frame->buffer_position_ = 0;
}

void FSP::Frame_beginTransferPanelSet(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::enablePanelSetSelectPin(frame->panel_set_row_index_, frame->panel_set_col_index_);
  BSP::transferPanelSet(frame->buffer_, frame->buffer_position_, frame->panel_byte_count_);
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
  BSP::initializeWatchdog();

  Watchdog * const watchdog = static_cast<Watchdog * const>(ao);
  QS_OBJ_DICTIONARY(&(watchdog->watchdog_time_evt_));
  QS_SIG_DICTIONARY(WATCHDOG_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(RESET_SIG, ao);
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


/**
  @brief Helper function: Appends a text message to a response buffer and updates the length byte

  Copy the `message` to the end of `response` and increase the overall count of the response 
  accordingly. Change the first byte of the message to the buffer length, but excluding that
  byte itself from the count. This should mirror the behavior of the G4 Host.

  @param[out] response The response buffer to append the message to
  @param[in,out] response_byte_count Current position in the buffer, updated after append
  @param[in] message Null-terminated string to append to the response
**/
static void appendMessage(uint8_t* response, uint8_t& response_byte_count, const char* message) {
  size_t msg_len = strlen(message);
  memcpy(&response[response_byte_count], message, msg_len);
  response_byte_count += msg_len;
  response[0] = response_byte_count - 1;
}

/**
  @brief Processes a binary command and generates an appropriate response

  Interprets the binary command from the command_buffer, performs the requested
  action by publishing the corresponding event, and generates a response message.
  The response format consists of:

  - Byte 0: Length of response (excluding length byte)
  - Byte 1: Status code (0 = success)
  - Byte 2: Echo of the command byte
  - Remaining bytes: Text message describing the action taken, using the
        responses from G4 Host.

  @param[in] command_buffer Buffer containing the binary command to process
  @param[in] command_byte_count Number of bytes in the command buffer
  @param[out] response Buffer to store the generated response

  @return Total number of bytes written to the response buffer
**/
uint8_t FSP::processBinaryCommand(uint8_t const * command_buffer,
    size_t command_byte_count,
    uint8_t response[constants::byte_count_per_response_max])
{
  uint8_t response_byte_count = 0;
  uint8_t second_command_byte = (uint8_t)(command_buffer[1]);
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = second_command_byte;
  switch (second_command_byte)
  {
    case 0x01:
    {
      AO_Watchdog->POST(&resetEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "Reset Command Sent to FPGA");
      break;
    }
    case 0x30:
    {
      AO_Arena->POST(&allOffEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "Display has been stopped");
      break;
    }
    case 0x00:
    {
      AO_Arena->POST(&allOffEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "All-Off Received");
      break;
    }
    case 0xFF:
    {
      AO_Arena->POST(&allOnEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "All-On Received");
      break;
    }
    default:
      break;
  }
  return response_byte_count;
}

void FSP::processStreamCommand(uint8_t const * command_buffer, uint32_t command_byte_count)
{
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("begin stream decode");
    QS_U32(8, command_byte_count);
  QS_END()
  uint16_t bytes_decoded = BSP::decodeStreamedFrame(command_buffer, command_byte_count);
  AO_Arena->POST(&streamFrameEvt, &l_FSP_ID);
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("end stream decode");
    QS_U32(8, bytes_decoded);
  QS_END()
}

void FSP::processStringCommand(const char * command, char * response)
{
  strcpy(response, command);
  if (strcmp(command, "RESET") == 0)
  {
    AO_Watchdog->POST(&resetEvt, &l_FSP_ID);
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
    AO_Arena->POST(&allOnEvt, &l_FSP_ID);
  }
  else if (strcmp(command, "ALL_OFF") == 0)
  {
    AO_Arena->POST(&allOffEvt, &l_FSP_ID);
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
}
