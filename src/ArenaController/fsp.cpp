#include "fsp.hpp"
#include "commands.hpp"


using namespace QP;

using namespace AC;

//----------------------------------------------------------------------------
// Static global variables
static QSpyId const l_FSP_ID = {0U}; // QSpy source ID

static QEvt const resetEvt = {RESET_SIG, 0U, 0U};

static QEvt const deactivateDisplayEvt = {DEACTIVATE_DISPLAY_SIG, 0U, 0U};
static QEvt const displayFrameEvt = {DISPLAY_FRAME_SIG, 0U, 0U};
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
static QEvt const serialInitializedEvt = {SERIAL_INITIALIZED_SIG, 0U, 0U};
static QEvt const serialCommandAvailableEvt = {SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U};

static QEvt const activateEthernetCommandInterfaceEvt = {ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const deactivateEthernetCommandInterfaceEvt = {DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U};
static QEvt const ethernetInitializedEvt = {ETHERNET_INITIALIZED_SIG, 0U, 0U};
static QEvt const ethernetServerConnectedEvt = {ETHERNET_SERVER_CONNECTED_SIG, 0U, 0U};

static QEvt const refreshTimeoutEvt = {REFRESH_TIMEOUT_SIG, 0U, 0U};

static QEvt const fillFrameBufferWithAllOnEvt = {FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, 0U, 0U};
static QEvt const fillFrameBufferWithDecodedFrameEvt = {FILL_FRAME_BUFFER_WITH_DECODED_FRAME_SIG, 0U, 0U};

static QEvt const beginDisplayingPatternEvt = {BEGIN_DISPLAYING_PATTERN_SIG, 0U, 0U};
static QEvt const endDisplayingPatternEvt = {END_DISPLAYING_PATTERN_SIG, 0U, 0U};
static QEvt const cardFoundEvt = {CARD_FOUND_SIG, 0U, 0U};
static QEvt const cardNotFoundEvt = {CARD_NOT_FOUND_SIG, 0U, 0U};
static QEvt const fileValidEvt = {FILE_VALID_SIG, 0U, 0U};
static QEvt const fileNotValidEvt = {FILE_NOT_VALID_SIG, 0U, 0U};
static QEvt const patternValidEvt = {PATTERN_VALID_SIG, 0U, 0U};
static QEvt const patternNotValidEvt = {PATTERN_NOT_VALID_SIG, 0U, 0U};
static QEvt const frameDecodedEvt = {FRAME_DECODED_SIG, 0U, 0U};

//----------------------------------------------------------------------------
// Local functions

void FSP::ArenaController_setup()
{
  static QF_MPOOL_EL(SetParameterEvt) smlPoolSto[constants::set_parameter_event_pool_count];
  static QF_MPOOL_EL(CommandEvt) medPoolSto[constants::command_event_pool_count];
  static QF_MPOOL_EL(FrameEvt) lrgPoolSto[constants::frame_event_pool_count];

  QF::init(); // initialize the framework

  QS_INIT(nullptr);

  BSP::init(); // initialize the BSP

  // initialize the event pools...
  QP::QF::poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
  QP::QF::poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));
  QP::QF::poolInit(lrgPoolSto, sizeof(lrgPoolSto), sizeof(lrgPoolSto[0]));

  // object dictionaries for AOs...
  QS_OBJ_DICTIONARY(AO_Arena);
  QS_OBJ_DICTIONARY(AO_SerialCommandInterface);
  QS_OBJ_DICTIONARY(AO_EthernetCommandInterface);
  QS_OBJ_DICTIONARY(AO_Display);
  QS_OBJ_DICTIONARY(AO_Frame);
  QS_OBJ_DICTIONARY(AO_Watchdog);
  QS_OBJ_DICTIONARY(AO_Pattern);

  QS_OBJ_DICTIONARY(&l_FSP_ID);

  // signal dictionaries for globally published events...
  QS_SIG_DICTIONARY(DEACTIVATE_DISPLAY_SIG, nullptr);
  QS_SIG_DICTIONARY(FRAME_FILLED_SIG, nullptr);
  QS_SIG_DICTIONARY(FRAME_TRANSFERRED_SIG, nullptr);
  QS_SIG_DICTIONARY(DISPLAY_PATTERN_SIG, nullptr);
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
  // QS_GLB_FILTER(QP::QS_QF_NEW);QS_QF_EQUEUE_POST
  // QS_GLB_FILTER(QP::QS_QF_EQUEUE_POST);
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

  static QEvt const *pattern_queueSto[10];
  AO_Pattern->start(4U, // priority
    pattern_queueSto, Q_DIM(pattern_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *arena_queueSto[10];
  AO_Arena->start(5U, // priority
    arena_queueSto, Q_DIM(arena_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *display_queueSto[10];
  AO_Display->start(6U, // priority
    display_queueSto, Q_DIM(display_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *frame_queueSto[10];
  AO_Frame->start(7U, // priority
    frame_queueSto, Q_DIM(frame_queueSto),
    (void *)0, 0U); // no stack

  QS_LOC_FILTER(-AO_Watchdog->m_prio);
  // QS_LOC_FILTER(-AO_Display->m_prio);
  // QS_LOC_FILTER(-AO_Frame->m_prio);
  // QS_LOC_FILTER(-AO_EthernetCommandInterface->m_prio);
  // QS_LOC_FILTER(-AO_SerialCommandInterface->m_prio);
}

void FSP::Arena_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  BSP::initializeArena();

  ao->subscribe(DISPLAY_PATTERN_SIG);
  ao->subscribe(FRAME_FILLED_SIG);

  QS_SIG_DICTIONARY(ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY(ALL_OFF_SIG, ao);
  QS_SIG_DICTIONARY(STREAM_FRAME_SIG, ao);
  QS_SIG_DICTIONARY(FRAME_FILLED_SIG, ao);

  Arena * const arena = static_cast<Arena * const>(ao);
  arena->frames_streamed_ = 0;
}

void FSP::Arena_activateCommandInterfaces(QActive * const ao, QEvt const * e)
{
  AO_SerialCommandInterface->POST(&activateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&activateEthernetCommandInterfaceEvt, ao);
}

void FSP::Arena_deactivateCommandInterfaces(QActive * const ao, QEvt const * e)
{
  AO_SerialCommandInterface->POST(&deactivateSerialCommandInterfaceEvt, &l_FSP_ID);
  AO_EthernetCommandInterface->POST(&deactivateEthernetCommandInterfaceEvt, ao);
}

void FSP::Arena_deactivateDisplay(QActive * const ao, QEvt const * e)
{
  QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
    QS_STR("deactivating display");
  QS_END()
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_displayFrame(QActive * const ao, QEvt const * e)
{
  AO_Display->POST(&displayFrameEvt, ao);
}

void FSP::Arena_fillFrameBufferWithAllOn(QActive * const ao, QEvt const * e)
{
  AO_Frame->POST(&fillFrameBufferWithAllOnEvt, ao);
}

void FSP::Arena_fillFrameBufferWithDecodedFrame(QActive * const ao, QEvt const * e)
{
  AO_Frame->POST(&fillFrameBufferWithDecodedFrameEvt, ao);
}

void FSP::Arena_endDisplayingPattern(QActive * const ao, QEvt const * e)
{
  AO_Pattern->POST(&endDisplayingPatternEvt, ao);
}

void FSP::Arena_allOffTransition(QP::QActive * const ao, QP::QEvt const * e)
{
  Arena * const arena = static_cast<Arena * const>(ao);
  arena->frames_streamed_ = 0;
}

void FSP::Arena_allOnTransition(QP::QActive * const ao, QP::QEvt const * e)
{
  Arena_deactivateDisplay(ao, e);
  Arena * const arena = static_cast<Arena * const>(ao);
  arena->frames_streamed_ = 0;
}

void FSP::Arena_streamFrameTransition(QP::QActive * const ao, QP::QEvt const * e)
{
  Arena_deactivateDisplay(ao, e);
}

void FSP::Arena_displayPatternTransition(QP::QActive * const ao, QP::QEvt const * e)
{
  Arena_deactivateDisplay(ao, e);
  Arena * const arena = static_cast<Arena * const>(ao);
  arena->frames_streamed_ = 0;
}

void FSP::Display_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  ao->subscribe(DEACTIVATE_DISPLAY_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);
  display->refresh_rate_hz_ = constants::refresh_rate_grayscale_default;

  static QEvt const * display_refresh_queue_store[constants::display_refresh_queue_size];
  display->refresh_queue_.init(display_refresh_queue_store, Q_DIM(display_refresh_queue_store));

  QS_SIG_DICTIONARY(DISPLAY_FRAME_SIG, ao);
  QS_SIG_DICTIONARY(REFRESH_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(SET_REFRESH_RATE_SIG, ao);
}

void FSP::Display_setRefreshRate(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  SetParameterEvt const * spev = static_cast<SetParameterEvt const *>(e);
  display->refresh_rate_hz_ = spev->value;
  QS_BEGIN_ID(USER_COMMENT, AO_Display->m_prio)
    QS_STR("set refresh rate");
    QS_U16(5, display->refresh_rate_hz_);
  QS_END()
}

void postRefreshTimeout()
{
  AO_Display->POST(&refreshTimeoutEvt, &l_FSP_ID);
}

void FSP::Display_armRefreshTimer(QActive * const ao, QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  BSP::armRefreshTimer(display->refresh_rate_hz_, postRefreshTimeout);
  // QS_BEGIN_ID(USER_COMMENT, AO_Display->m_prio)
  //   QS_STR("armRefreshTimer");
  //   QS_U16(5, display->refresh_rate_hz_);
  // QS_END()
}

void FSP::Display_disarmRefreshTimer(QActive * const ao, QEvt const * e)
{
  BSP::disarmRefreshTimer();
  // QS_BEGIN_ID(USER_COMMENT, AO_Display->m_prio)
  //   QS_STR("disarmRefreshTimer");
  // QS_END()
}

void FSP::Display_transferFrame(QActive * const ao, QEvt const * e)
{
  AO_Frame->POST(&transferFrameEvt, ao);
}

void FSP::Display_defer(QP::QActive * const ao, QP::QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  if (display->refresh_queue_.getNFree() > 0)
  {
    display->defer(&display->refresh_queue_, e);
  }
}

void FSP::Display_recall(QP::QActive * const ao, QP::QEvt const * e)
{
  Display * const display = static_cast<Display * const>(ao);
  display->recall(&display->refresh_queue_);
}

void FSP::SerialCommandInterface_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(PROCESS_BINARY_COMMAND_SIG);
  ao->subscribe(PROCESS_STRING_COMMAND_SIG);
  ao->subscribe(PROCESS_STREAM_COMMAND_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);

  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  QS_OBJ_DICTIONARY(&(sci->serial_time_evt_));
  QS_SIG_DICTIONARY(SERIAL_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY(SERIAL_INITIALIZED_SIG, ao);
}

void FSP::SerialCommandInterface_armSerialTimer(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.armX(constants::ticks_per_second, constants::ticks_per_second/constants::serial_timer_frequency_hz);
}

void FSP::SerialCommandInterface_disarmSerialTimer(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.disarm();
}

void FSP::SerialCommandInterface_initializeSerial(QActive * const ao, QEvt const * e)
{
  bool serial_ready = BSP::initializeSerial();
  if (serial_ready)
  {
    AO_SerialCommandInterface->POST(&serialInitializedEvt, ao);
  }
}

void FSP::SerialCommandInterface_pollSerial(QActive * const ao, QEvt const * e)
{
  bool bytes_available = BSP::pollSerial();
  if (bytes_available)
  {
    QF::PUBLISH(&serialCommandAvailableEvt, ao);
  }
}

void FSP::SerialCommandInterface_analyzeCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);

  uint8_t first_command_byte = BSP::readSerialByte();
  uint8_t command_buffer_position = 0;
  command_buffer_position += sizeof(first_command_byte);
  sci->binary_command_byte_count_ = 0;
  sci->binary_command_[sci->binary_command_byte_count_++] = first_command_byte;
  if (first_command_byte > constants::first_command_byte_max_value_binary)
  {
    QS_BEGIN_ID(USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR("string command");
    QS_END()
    QF::PUBLISH(&processStringCommandEvt, ao);
  }
  else if (first_command_byte == STREAM_FRAME_CMD)
  {
    uint16_t binary_command_byte_count_claim;
    for (uint8_t position=0; position<sizeof(binary_command_byte_count_claim); ++position)
    {
      sci->binary_command_[sci->binary_command_byte_count_++] = BSP::readSerialByte();
    }
    memcpy(&binary_command_byte_count_claim, sci->binary_command_ + command_buffer_position, sizeof(binary_command_byte_count_claim));
    sci->binary_command_byte_count_claim_ = binary_command_byte_count_claim;
    QS_BEGIN_ID(USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR("stream command");
      QS_U32(8, sci->binary_command_byte_count_claim_);
    QS_END()
    QF::PUBLISH(&processStreamCommandEvt, ao);
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR("binary command");
    QS_END()
    QF::PUBLISH(&processBinaryCommandEvt, ao);
  }
}

void FSP::SerialCommandInterface_processBinaryCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  uint8_t binary_command_byte_count = sci->binary_command_[0];
  for (uint8_t position=0; position<binary_command_byte_count; ++position)
  {
    sci->binary_command_[sci->binary_command_byte_count_++] = BSP::readSerialByte();
  }
  sci->binary_response_byte_count_ = FSP::processBinaryCommand(sci->binary_command_,
    sci->binary_command_byte_count_,
    sci->binary_response_);
  QF::PUBLISH(&commandProcessedEvt, ao);
}

void FSP::SerialCommandInterface_writeBinaryResponse(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  BSP::writeSerialBinaryResponse(sci->binary_response_, sci->binary_response_byte_count_);
}

void FSP::SerialCommandInterface_updateStreamCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  while (BSP::pollSerial())
  {
    sci->binary_command_[sci->binary_command_byte_count_++] = BSP::readSerialByte();
  }
}

bool FSP::SerialCommandInterface_ifStreamCommandComplete(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  return (sci->binary_command_byte_count_ >= sci->binary_command_byte_count_claim_);
}

void FSP::SerialCommandInterface_processStreamCommand(QActive * const ao, QEvt const * e)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->binary_response_byte_count_ = 3;
  sci->binary_response_[0] = 2;
  sci->binary_response_[1] = 0;
  sci->binary_response_[2] = STREAM_FRAME_CMD;
  uint8_t const * buffer = sci->binary_command_ + constants::stream_header_byte_count;
  uint32_t frame_byte_count = sci->binary_command_byte_count_ - constants::stream_header_byte_count;
  FSP::processStreamCommand(buffer, frame_byte_count);
  QF::PUBLISH(&commandProcessedEvt, ao);
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
  eci->ethernet_time_evt_.armX(constants::ticks_per_second, constants::ticks_per_second/constants::ethernet_timer_frequency_hz);
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
    AO_EthernetCommandInterface->POST(&ethernetInitializedEvt, ao);
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
    AO_EthernetCommandInterface->POST(&ethernetServerConnectedEvt, ao);
  }
}

void FSP::EthernetCommandInterface_analyzeCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  CommandEvt const * cev = static_cast<CommandEvt const *>(e);
  eci->connection_ = cev->connection;
  eci->binary_command_ = cev->binary_command;
  eci->binary_command_byte_count_ = cev->binary_command_byte_count;

  uint8_t command_buffer_position = 0;
  uint8_t first_command_byte;
  memcpy(&first_command_byte, eci->binary_command_ + command_buffer_position, sizeof(first_command_byte));
  command_buffer_position += sizeof(first_command_byte);
  if (first_command_byte > constants::first_command_byte_max_value_binary)
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("string command");
    QS_END()
    QF::PUBLISH(&processStringCommandEvt, ao);
  }
  else if (first_command_byte == STREAM_FRAME_CMD)
  {
    uint16_t binary_command_byte_count_claim;
    memcpy(&binary_command_byte_count_claim, eci->binary_command_ + command_buffer_position, sizeof(binary_command_byte_count_claim));
    eci->binary_command_byte_count_claim_ = binary_command_byte_count_claim;
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("stream command");
      QS_U32(8, eci->binary_command_byte_count_claim_);
    QS_END()
    QF::PUBLISH(&processStreamCommandEvt, ao);
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("binary command");
    QS_END()
    QF::PUBLISH(&processBinaryCommandEvt, ao);
  }
}

void FSP::EthernetCommandInterface_updateStreamCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  CommandEvt const * cev = static_cast<CommandEvt const *>(e);
  eci->binary_command_ = cev->binary_command;
  eci->binary_command_byte_count_ = cev->binary_command_byte_count;
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
  QF::PUBLISH(&commandProcessedEvt, ao);
}

void FSP::EthernetCommandInterface_processStreamCommand(QActive * const ao, QEvt const * e)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->binary_response_byte_count_ = 3;
  eci->binary_response_[0] = 2;
  eci->binary_response_[1] = 0;
  eci->binary_response_[2] = STREAM_FRAME_CMD;
  uint8_t const * buffer = eci->binary_command_ + constants::stream_header_byte_count;
  uint32_t frame_byte_count = eci->binary_command_byte_count_ - constants::stream_header_byte_count;
  FSP::processStreamCommand(buffer, frame_byte_count);
  QF::PUBLISH(&commandProcessedEvt, ao);
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
  frame->frame_ = nullptr;
  frame->grayscale_ = true;

  static QEvt const * frame_event_queue_store[constants::frame_event_queue_size];
  frame->event_queue_.init(frame_event_queue_store, Q_DIM(frame_event_queue_store));

  ao->subscribe(FRAME_FILLED_SIG);
  ao->subscribe(DEACTIVATE_DISPLAY_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);

  QS_SIG_DICTIONARY(TRANSFER_FRAME_SIG, ao);
  QS_SIG_DICTIONARY(PANEL_SET_TRANSFERRED_SIG, ao);
  QS_SIG_DICTIONARY(FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY(FILL_FRAME_BUFFER_WITH_DECODED_FRAME_SIG, ao);
  QS_SIG_DICTIONARY(SWITCH_GRAYSCALE_SIG, ao);
}

void FSP::Frame_fillFrameBufferWithAllOn(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  FrameEvt * fe = Q_NEW(FrameEvt, FRAME_FILLED_SIG);

  BSP::fillFrameBufferWithAllOn(fe->buffer,
    frame->grayscale_);
  if (frame->grayscale_)
  {
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("filled frame buffer with grayscale all on");
  QS_END()
  }
  else
  {
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("filled frame buffer with binary all on");
  QS_END()
  }
  QF::PUBLISH(fe, ao);
}

void FSP::Frame_fillFrameBufferWithDecodedFrame(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  FrameEvt * fe = Q_NEW(FrameEvt, FRAME_FILLED_SIG);

  BSP::fillFrameBufferWithDecodedFrame(fe->buffer,
    frame->grayscale_);
  QF::PUBLISH(fe, ao);
}

void FSP::Frame_saveFrameReference(QP::QActive * const ao, QP::QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  Q_NEW_REF(frame->frame_, FrameEvt);
}

void FSP::Frame_deleteFrameReference(QP::QActive * const ao, QP::QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  if (frame->frame_)
  {
    Q_DELETE_REF(frame->frame_);
    frame->frame_ = nullptr;
  }
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
  uint8_t panel_byte_count;
  if (frame->grayscale_)
  {
    panel_byte_count = constants::byte_count_per_panel_grayscale;
  }
  else
  {
    panel_byte_count = constants::byte_count_per_panel_binary;
  }
  BSP::transferPanelSet(frame->frame_->buffer, frame->buffer_position_, panel_byte_count);
}

void FSP::Frame_endTransferPanelSet(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::disablePanelSetSelectPin(frame->panel_set_row_index_, frame->panel_set_col_index_);
  ++frame->panel_set_row_index_;
  if (frame->panel_set_row_index_ == BSP::getPanelCountPerRegionRow())
  {
    frame->panel_set_row_index_ = 0;
    ++frame->panel_set_col_index_;
  }
  if (frame->panel_set_col_index_ == BSP::getPanelCountPerRegionCol())
  {
    frame->panel_set_col_index_ = 0;
  }
}

bool FSP::Frame_ifFrameNotTransferred(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  return (frame->panel_set_row_index_ != (BSP::getPanelCountPerRegionRow()-1)) ||
    (frame->panel_set_col_index_ != (BSP::getPanelCountPerRegionCol()-1));
}

void FSP::Frame_publishFrameTransferred(QActive * const ao, QEvt const * e)
{
  QF::PUBLISH(&frameTransferredEvt, ao);
}

void FSP::Frame_switchGrayscale(QActive * const ao, QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  SetParameterEvt const * spev = static_cast<SetParameterEvt const *>(e);

  uint8_t grayscale_index = spev->value;

  if (grayscale_index == constants::switch_grayscale_command_value_grayscale)
  {
    frame->grayscale_ = true;
    QS_BEGIN_ID(USER_COMMENT, AO_Frame->m_prio)
      QS_STR("switch grayscale to grayscale");
    QS_END()
  }
  else if (grayscale_index == constants::switch_grayscale_command_value_binary)
  {
    frame->grayscale_ = false;
    QS_BEGIN_ID(USER_COMMENT, AO_Frame->m_prio)
      QS_STR("switch grayscale to binary");
    QS_END()
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, AO_Frame->m_prio)
      QS_STR("invalid switch grayscale value");
    QS_END()
  }
}

void FSP::Frame_defer(QP::QActive * const ao, QP::QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  if (frame->event_queue_.getNFree() > 0)
  {
    frame->defer(&frame->event_queue_, e);
  }
}

void FSP::Frame_recall(QP::QActive * const ao, QP::QEvt const * e)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  frame->recall(&frame->event_queue_);
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

void FSP::Pattern_initializeAndSubscribe(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->frame_ = nullptr;
  pattern->file_size_ = 0;
  pattern->byte_count_per_frame_ = 0;
  pattern->positive_direction_ = true;

  static QEvt const * pattern_frame_rate_queue_store[constants::pattern_frame_rate_queue_size];
  pattern->frame_rate_queue_.init(pattern_frame_rate_queue_store, Q_DIM(pattern_frame_rate_queue_store));

  ao->subscribe(DISPLAY_PATTERN_SIG);
  ao->subscribe(FRAME_FILLED_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);

  QS_SIG_DICTIONARY(FRAME_RATE_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(RUNTIME_DURATION_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY(BEGIN_DISPLAYING_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY(END_DISPLAYING_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY(CARD_FOUND_SIG, ao);
  QS_SIG_DICTIONARY(CARD_NOT_FOUND_SIG, ao);
  QS_SIG_DICTIONARY(FILE_VALID_SIG, ao);
  QS_SIG_DICTIONARY(FILE_NOT_VALID_SIG, ao);
  QS_SIG_DICTIONARY(PATTERN_VALID_SIG, ao);
  QS_SIG_DICTIONARY(PATTERN_NOT_VALID_SIG, ao);
  QS_SIG_DICTIONARY(FRAME_READ_FROM_FILE_SIG, ao);
  QS_SIG_DICTIONARY(FRAME_DECODED_SIG, ao);
}

void FSP::Pattern_checkAndStoreParameters(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  DisplayPatternEvt const * dpev = static_cast<DisplayPatternEvt const *>(e);

  if (dpev->frame_rate == 0)
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("invalid frame rate");
    QS_END()
    AO_Arena->POST(&allOffEvt, ao);
    return;
  }
  else if (dpev->frame_rate < 0)
  {
    pattern->positive_direction_ = false;
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("valid negative frame rate");
    QS_END()
  }
  else
  {
    pattern->positive_direction_ = true;
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("valid positive frame rate");
    QS_END()
  }
  pattern->id_ = dpev->pattern_id;
  pattern->frame_rate_hz_ = abs(dpev->frame_rate);
  pattern->runtime_duration_ms_ = dpev->runtime_duration * constants::milliseconds_per_runtime_duration_unit;
  QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
    QS_STR("check and store parameters");
  // QS_U8(0, control_mode);
  QS_U16(5, pattern->id_);
  QS_U16(5, pattern->frame_rate_hz_);
  // QS_U16(5, init_pos);
  // QS_U16(5, gain);
  QS_U16(5, pattern->runtime_duration_ms_);
  QS_END()
  AO_Pattern->POST(&beginDisplayingPatternEvt, ao);
}

void FSP::Pattern_armInitializeCardTimer(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->initialize_card_time_evt_.armX((constants::ticks_per_second * constants::initialize_card_timeout_duration) / constants::milliseconds_per_second);
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("initializing card may cause reboot if card not found...");
  QS_END()
}

void FSP::Pattern_initializeCard(QActive * const ao, QEvt const * e)
{
  if (BSP::initializePatternCard())
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("pattern card found");
    QS_END()
    AO_Pattern->POST(&cardFoundEvt, ao);
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("pattern card not found");
    QS_END()
    AO_Pattern->POST(&cardNotFoundEvt, ao);
  }
}

void FSP::Pattern_postAllOff(QActive * const ao, QEvt const * e)
{
  AO_Arena->POST(&allOffEvt, ao);
}

void FSP::Pattern_openFile(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->file_size_ = BSP::openPatternFileForReading(pattern->id_);
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("file opened");
  QS_END()
}

void FSP::Pattern_closeFile(QActive * const ao, QEvt const * e)
{
  BSP::closePatternFile();
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("file closed");
  QS_END()
}

void FSP::Pattern_checkFile(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  if (pattern->file_size_)
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("file valid");
      QS_U32(8, pattern->id_);
      QS_U32(8, pattern->file_size_);
    QS_END()
    AO_Pattern->POST(&fileValidEvt, ao);
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("file not valid");
      QS_U32(8, pattern->id_);
    QS_END()
    AO_Pattern->POST(&fileNotValidEvt, ao);
  }
}

void FSP::Pattern_checkPattern(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  Frame * const frame = static_cast<Frame * const>(AO_Frame);

  PatternHeader pattern_header = BSP::rewindPatternFileAndReadHeader();
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("frame_count_x");
    QS_U16(5, pattern_header.frame_count_x);
    QS_STR("frame_count_y");
    QS_U16(5, pattern_header.frame_count_y);
    QS_STR("grayscale_value");
    QS_U8(0, pattern_header.grayscale_value);
    QS_STR("panel_count_per_frame_row");
    QS_U8(0, pattern_header.panel_count_per_frame_row);
    QS_STR("panel_count_per_frame_col");
    QS_U8(0, pattern_header.panel_count_per_frame_col);
  QS_END()

  if (pattern_header.panel_count_per_frame_row != BSP::getPanelCountPerFrameRow())
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("pattern frame has incorrect number of rows:");
      QS_STR("header row count");
      QS_U8(0, pattern_header.panel_count_per_frame_row);
      QS_STR("bsp row count");
      QS_U8(0, BSP::getPanelCountPerRegionRow());
    QS_END()
    AO_Pattern->POST(&patternNotValidEvt, ao);
    return;
  }
  if (pattern_header.panel_count_per_frame_col != BSP::getPanelCountPerFrameCol())
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("pattern frame has incorrect number of cols:");
      QS_STR("header col count");
      QS_U8(0, pattern_header.panel_count_per_frame_col);
      QS_STR("bsp col count");
      QS_U8(0, BSP::getPanelCountPerRegionCol() * BSP::getRegionCountPerFrame());
    QS_END()
    AO_Pattern->POST(&patternNotValidEvt, ao);
    return;
  }

  uint64_t byte_count_per_frame;
  switch (pattern_header.grayscale_value)
  {
    case constants::pattern_grayscale_value:
    {
      frame->grayscale_ = true;
      byte_count_per_frame = BSP::getByteCountPerPatternFrameGrayscale();
      break;
    }
    case constants::pattern_binary_value:
    {
      frame->grayscale_ = false;
      byte_count_per_frame = BSP::getByteCountPerPatternFrameBinary();
      break;
    }
    default:
      QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
        QS_STR("pattern has invalid grayscale value");
      QS_END()
      AO_Pattern->POST(&patternNotValidEvt, ao);
      return;
  }

  if ((uint64_t)(pattern_header.frame_count_x * byte_count_per_frame + constants::pattern_header_size) != pattern->file_size_)
  {
    QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
      QS_STR("pattern frame has incorrect file size");
    QS_END()
    AO_Pattern->POST(&patternNotValidEvt, ao);
    return;
  }

  pattern->byte_count_per_frame_ = byte_count_per_frame;
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("byte count per pattern frame");
    QS_U16(5, byte_count_per_frame);
  QS_END()

  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("pattern valid");
  QS_END()
  AO_Pattern->POST(&patternValidEvt, ao);
}

void FSP::Pattern_armTimers(QActive * const ao, QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->frame_rate_time_evt_.armX(constants::ticks_per_second / pattern->frame_rate_hz_, constants::ticks_per_second / pattern->frame_rate_hz_);
  pattern->runtime_duration_time_evt_.armX((constants::ticks_per_second * pattern->runtime_duration_ms_) / constants::milliseconds_per_second);
  QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
    QS_STR("arming pattern timers");
    QS_STR("frame rate ticks");
    QS_U32(8, (constants::ticks_per_second / pattern->frame_rate_hz_));
    QS_STR("runtime duration ticks");
    QS_U32(8, ((constants::ticks_per_second * pattern->runtime_duration_ms_) / constants::milliseconds_per_second));
  QS_END()
}

void FSP::Pattern_disarmTimers(QActive * const ao, QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("disarming pattern timers");
  // QS_END()
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->frame_rate_time_evt_.disarm();
  pattern->runtime_duration_time_evt_.disarm();
}

void FSP::Pattern_deactivateDisplay(QActive * const ao, QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("deactivating display");
  // QS_END()
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Pattern_readNextFrameFromFile(QP::QActive * const ao, QP::QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("reading next pattern frame from file");
  // QS_END()
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  FrameEvt * fe = Q_NEW(FrameEvt, FRAME_READ_FROM_FILE_SIG);
  BSP::readNextPatternFrameFromFileIntoBuffer(fe->buffer,
    pattern->byte_count_per_frame_,
    pattern->positive_direction_);
  AO_Pattern->POST(fe, ao);
}

void FSP::Pattern_saveFrameReference(QP::QActive * const ao, QP::QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  Q_NEW_REF(pattern->frame_, FrameEvt);
}

void FSP::Pattern_deleteFrameReference(QP::QActive * const ao, QP::QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  if (pattern->frame_)
  {
    Q_DELETE_REF(pattern->frame_);
    pattern->frame_ = nullptr;
  }
}

void FSP::Pattern_decodeFrame(QActive * const ao, QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("decoding pattern frame");
  // QS_END()
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  Frame * const frame = static_cast<Frame * const>(AO_Frame);
  BSP::decodePatternFrameBuffer(pattern->frame_->buffer, frame->grayscale_);
  // uint16_t bytes_decoded = BSP::decodePatternFrameBuffer(pattern->frame_->buffer, frame->grayscale_);
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("bytes decoded");
  //   QS_U32(8, bytes_decoded);
  // QS_END()
  AO_Pattern->POST(&frameDecodedEvt, ao);
}

void FSP::Pattern_fillFrameBufferWithDecodedFrame(QActive * const ao, QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("filling frame buffer with decoded frame");
  // QS_END()
  AO_Frame->POST(&fillFrameBufferWithDecodedFrameEvt, ao);
}

void FSP::Pattern_defer(QP::QActive * const ao, QP::QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  if (pattern->frame_rate_queue_.getNFree() > 0)
  {
    pattern->defer(&pattern->frame_rate_queue_, e);
  }
}

void FSP::Pattern_recall(QP::QActive * const ao, QP::QEvt const * e)
{
  Pattern * const pattern = static_cast<Pattern * const>(ao);
  pattern->recall(&pattern->frame_rate_queue_);
}

void FSP::Pattern_displayFrame(QActive * const ao, QEvt const * e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("displaying pattern frame");
  // QS_END()
  AO_Display->POST(&displayFrameEvt, ao);
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

  uint8_t command_buffer_position = 0;
  uint8_t command_byte_count_claim = command_buffer[command_buffer_position++];

  if ((command_byte_count - 1) != command_byte_count_claim)
  {
    appendMessage(response, response_byte_count, "Invalid-Command");
    QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
      QS_STR("invalid command");
    QS_END()
    return response_byte_count;
  }

  uint8_t command_byte = command_buffer[command_buffer_position++];
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = command_byte;
  switch (command_byte)
  {
    case ALL_OFF_CMD:
    {
      AO_Arena->POST(&allOffEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "All-Off Received");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("all-off command");
      QS_END()
      break;
    }
    case DISPLAY_RESET_CMD:
    {
      AO_Watchdog->POST(&resetEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "Reset Command Sent to FPGA");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("display-reset command");
      QS_END()
      break;
    }
    case SWITCH_GRAYSCALE_CMD:
    {
      uint8_t grayscale_index;
      memcpy(&grayscale_index, command_buffer + command_buffer_position, sizeof(grayscale_index));

      AO_Arena->POST(&allOffEvt, &l_FSP_ID);

      SetParameterEvt *spev = Q_NEW(SetParameterEvt, SWITCH_GRAYSCALE_SIG);
      spev->value = grayscale_index;
      AO_Frame->POST(spev, &l_FSP_ID);

      appendMessage(response, response_byte_count, "");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("switch-grayscale command");
      QS_END()
      break;
    }
    case TRIAL_PARAMS_CMD:
    {
      uint8_t control_mode;
      memcpy(&control_mode, command_buffer + command_buffer_position, sizeof(control_mode));
      command_buffer_position += sizeof(control_mode);

      uint16_t pattern_id;
      memcpy(&pattern_id, command_buffer + command_buffer_position, sizeof(pattern_id));
      command_buffer_position += sizeof(pattern_id);

      int16_t frame_rate;
      memcpy(&frame_rate, command_buffer + command_buffer_position, sizeof(frame_rate));
      command_buffer_position += sizeof(frame_rate);

      uint16_t init_pos;
      memcpy(&init_pos, command_buffer + command_buffer_position, sizeof(init_pos));
      command_buffer_position += sizeof(init_pos);

      uint16_t gain;
      memcpy(&gain, command_buffer + command_buffer_position, sizeof(gain));
      command_buffer_position += sizeof(gain);

      uint16_t runtime_duration;
      memcpy(&runtime_duration, command_buffer + command_buffer_position, sizeof(runtime_duration));
      command_buffer_position += sizeof(runtime_duration);

      DisplayPatternEvt *dpev = Q_NEW(DisplayPatternEvt, DISPLAY_PATTERN_SIG);
      dpev->pattern_id = pattern_id;
      dpev->frame_rate = frame_rate;
      dpev->runtime_duration = runtime_duration;
      QF::PUBLISH(dpev, &l_FSP_ID);

      appendMessage(response, response_byte_count, "");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("trial-params command");
        // QS_U8(0, control_mode);
        // QS_U16(5, pattern_id);
        // QS_U16(5, frame_rate);
        // QS_U16(5, init_pos);
        // QS_U16(5, gain);
        // QS_U16(5, runtime_duration);
      QS_END()
      break;
    }
    case SET_REFRESH_RATE_CMD:
    {
      uint16_t refresh_rate;
      memcpy(&refresh_rate, command_buffer + command_buffer_position, sizeof(refresh_rate));

      SetParameterEvt *spev = Q_NEW(SetParameterEvt, SET_REFRESH_RATE_SIG);
      spev->value = refresh_rate;
      AO_Display->POST(spev, &l_FSP_ID);
      appendMessage(response, response_byte_count, "");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("set-refresh-rate command");
      QS_END()
      break;
    }
    case STOP_DISPLAY_CMD:
    {
      AO_Arena->POST(&allOffEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "Display has been stopped");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("stop-display command");
      QS_END()
      break;
    }
    case ALL_ON_CMD:
    {
      AO_Arena->POST(&allOnEvt, &l_FSP_ID);
      appendMessage(response, response_byte_count, "All-On Received");
      QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
        QS_STR("all-on command");
      QS_END()
      break;
    }
    default:
      break;
  }
  return response_byte_count;
}

void FSP::processStreamCommand(uint8_t const * buffer, uint32_t frame_byte_count)
{
  Frame * const frame = static_cast<Frame * const>(AO_Frame);

  if (frame_byte_count == BSP::getByteCountPerPatternFrameGrayscale())
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("streamed frame has grayscale size");
    QS_END()
    frame->grayscale_ = true;
  }
  else if (frame_byte_count == BSP::getByteCountPerPatternFrameBinary())
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("streamed frame has binary size");
    QS_END()
    frame->grayscale_ = false;
  }
  else
  {
    QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR("streamed frame has invalid size");
      QS_U32(8, frame_byte_count);
      QS_U32(8, BSP::getByteCountPerPatternFrameGrayscale());
      QS_U32(8, BSP::getByteCountPerPatternFrameBinary());
    QS_END()
    AO_Arena->POST(&allOffEvt, &l_FSP_ID);
    return;
  }
  uint16_t bytes_decoded = BSP::decodePatternFrameBuffer(buffer, frame->grayscale_);
  AO_Arena->POST(&streamFrameEvt, &l_FSP_ID);
  Arena * const arena = static_cast<Arena * const>(AO_Arena);
  arena->frames_streamed_ = arena->frames_streamed_ + 1;
  QS_BEGIN_ID(USER_COMMENT, AO_EthernetCommandInterface->m_prio)
    QS_STR("processed stream command");
    QS_U32(8, bytes_decoded);
    QS_STR("frames streamed");
    QS_U32(8, arena->frames_streamed_);
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
  else if (strcmp(command, "SET_REFRESH_RATE") == 0)
  {
    //command.replace("SET_REFRESH_RATE", "") == 0;
    //command.trim();
    //uint32_t frequency_hz = command.toInt();
    //BSP::setRefreshRate(frequency_hz);
  }
}
