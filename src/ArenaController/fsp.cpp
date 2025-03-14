#include "fsp.hpp"


using namespace QP;

using namespace AC;

static QSpyId const l_COMMAND_ID = { 1U }; // QSpy source ID

static CommandEvt const resetEvt = { RESET_SIG, 0U, 0U};
static CommandEvt const allOnEvt = { ALL_ON_SIG, 0U, 0U};
static CommandEvt const allOffEvt = { ALL_OFF_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = { COMMAND_PROCESSED_SIG, 0U, 0U};

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

void FSP::Arena_InitialTransition(QP::QActive * const ao)
{
  BSP::initializeArena();
  ao->subscribe(RESET_SIG);
  ao->subscribe(ALL_ON_SIG);
  ao->subscribe(ALL_OFF_SIG);
}

void FSP::Arena_ArenaOn_entry(QP::QActive * const ao)
{
  BSP::activateCommandInterfaces();
}

void FSP::Arena_ArenaOn_exit(QP::QActive * const ao)
{
  BSP::deactivateCommandInterfaces();
}

void FSP::Arena_AllOff_entry(QP::QActive * const ao)
{
  static QEvt const deactivateDisplayEvt = { DEACTIVATE_DISPLAY_SIG, 0U, 0U};
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_AllOn_entry(QP::QActive * const ao)
{
  static DisplayFramesEvt displayFramesEvt = { DISPLAY_FRAMES_SIG, 0U, 0U};
  displayFramesEvt.panel_buffer = &constants::all_on_grayscale_pattern;
  displayFramesEvt.panel_buffer_byte_count = constants::byte_count_per_panel_grayscale;
  QF::PUBLISH(&displayFramesEvt, ao);
}

void FSP::SerialCommandInterface_InitialTransition(QActive * const ao)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);
}

void FSP::SerialCommandInterface_Active_entry(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.armX(BSP::TICKS_PER_SEC/2, BSP::TICKS_PER_SEC/50);
}

void FSP::SerialCommandInterface_Active_exit(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->serial_time_evt_.disarm();
}

void FSP::SerialCommandInterface_NotReady_entry(QActive * const ao)
{
  BSP::beginSerial();
}

void FSP::SerialCommandInterface_PollingForNewCommand_SERIAL_TIMEOUT(QActive * const ao)
{
  BSP::pollSerialCommand();
}

void FSP::SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->first_command_byte_ = BSP::readSerialByte();
}

bool FSP::SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE_if_guard(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  return (sci->first_command_byte_ <= constants::first_command_byte_max_value_binary);
}

void FSP::SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE_else_action(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->string_command_ = BSP::readSerialStringCommand(sci->first_command_byte_);
}

void FSP::SerialCommandInterface_ProcessingStringCommand_entry(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  sci->string_response_ = FSP::processStringCommand(sci->string_command_);
}

void FSP::SerialCommandInterface_ProcessingStringCommand_COMMAND_PROCESSED(QActive * const ao)
{
  SerialCommandInterface * const sci = static_cast<SerialCommandInterface * const>(ao);
  BSP::writeSerialStringResponse(sci->string_response_);
}

void FSP::SerialCommandInterface_ProcessingBinaryCommand_COMMAND_PROCESSED(QActive * const ao)
{
}

void FSP::EthernetCommandInterface_InitialTransition(QActive * const ao)
{
  ao->subscribe(SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe(ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe(COMMAND_PROCESSED_SIG);
}

void FSP::EthernetCommandInterface_Active_entry(QActive * const ao)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.armX(BSP::TICKS_PER_SEC/2, BSP::TICKS_PER_SEC/50);
}

void FSP::EthernetCommandInterface_Active_exit(QActive * const ao)
{
  EthernetCommandInterface * const eci = static_cast<EthernetCommandInterface * const>(ao);
  eci->ethernet_time_evt_.disarm();
}

void FSP::EthernetCommandInterface_Uninitialized_SERIAL_TIMEOUT(QActive * const ao)
{
  BSP::beginEthernet();
}

void FSP::EthernetCommandInterface_WaitingForIPAddress_SERIAL_TIMEOUT(QActive * const ao)
{
  BSP::checkForEthernetIPAddress();
}

void FSP::EthernetCommandInterface_IPAddressFound_SERIAL_TIMEOUT(QActive * const ao)
{
  BSP::beginEthernetServer();
}

void FSP::EthernetCommandInterface_PollingForNewCommand_SERIAL_TIMEOUT(QActive * const ao)
{
  BSP::pollEthernetCommand();
}

void FSP::Frame_InitialTransition(QP::QActive * const ao)
{
  BSP::initializeFrame();
  ao->subscribe(TRANSFER_FRAME_SIG);
  ao->subscribe(PANEL_SET_TRANSFERRED_SIG);
  ao->subscribe(FRAME_TRANSFERRED_SIG);
}

void FSP::Frame_Active_entry(QActive * const ao)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  frame->panel_set_row_index_ = 0;
  frame->panel_set_col_index_ = 0;
}

void FSP::Frame_TransferringPanelSet_entry(QActive * const ao)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  BSP::enablePanelSetSelectPin(frame->panel_set_row_index_, frame->panel_set_col_index_);
  BSP::transferPanelSet(frame->panel_buffer_, frame->panel_buffer_byte_count_);
}

void FSP::Frame_TransferringPanelSet_exit(QActive * const ao)
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

bool FSP::Frame_TransferringPanelSet_PANEL_SET_TRANSFERRED_if_guard(QP::QActive * const ao)
{
  Frame * const frame = static_cast<Frame * const>(ao);
  return (frame->panel_set_row_index_ != (frame->region_row_panel_count_-1)) ||
    (frame->panel_set_col_index_ != (frame->region_col_panel_count_-1));
}

void FSP::Frame_TransferringPanelSet_PANEL_SET_TRANSFERRED_else_action(QP::QActive * const ao)
{
  static QEvt const frameTransferredEvt = { FRAME_TRANSFERRED_SIG, 0U, 0U};
  QF::PUBLISH(&frameTransferredEvt, ao);
}

String FSP::processStringCommand(String command)
{
  command.trim();
  String response = command;
  if (command.equalsIgnoreCase("RESET"))
  {
    QF::PUBLISH(&resetEvt, &l_COMMAND_ID);
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
    QF::PUBLISH(&allOnEvt, &l_COMMAND_ID);
  }
  else if (command.equalsIgnoreCase("ALL_OFF"))
  {
    QF::PUBLISH(&allOffEvt, &l_COMMAND_ID);
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
  QF::PUBLISH(&commandProcessedEvt, &l_COMMAND_ID);
  return response;
}
