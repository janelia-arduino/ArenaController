#include "fsp.hpp"


using namespace QP;

static QSpyId const l_COMMAND_ID = { 1U }; // QSpy source ID

static AC::CommandEvt const resetEvt = { AC::RESET_SIG, 0U, 0U};
static AC::CommandEvt const allOnEvt = { AC::ALL_ON_SIG, 0U, 0U};
static AC::CommandEvt const allOffEvt = { AC::ALL_OFF_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = { AC::COMMAND_PROCESSED_SIG, 0U, 0U};

void FSP::ArenaController_setup()
{
  QF::init(); // initialize the framework
  BSP::init(); // initialize the BSP

  // init publish-subscribe
  static QSubscrList subscrSto[AC::MAX_PUB_SIG];
  QF::psInit(subscrSto, Q_DIM(subscrSto));

  // statically allocate event queues for the AOs and start them...
  static QEvt const *watchdog_queueSto[2];
  AC::AO_Watchdog->start(1U, // priority
    watchdog_queueSto, Q_DIM(watchdog_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *serial_command_interface_queueSto[10];
  AC::AO_SerialCommandInterface->start(2U, // priority
    serial_command_interface_queueSto, Q_DIM(serial_command_interface_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *ethernet_command_interface_queueSto[10];
  AC::AO_EthernetCommandInterface->start(3U, // priority
    ethernet_command_interface_queueSto, Q_DIM(ethernet_command_interface_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *arena_queueSto[10];
  AC::AO_Arena->start(4U, // priority
    arena_queueSto, Q_DIM(arena_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *display_queueSto[10];
  AC::AO_Display->start(5U, // priority
    display_queueSto, Q_DIM(display_queueSto),
    (void *)0, 0U); // no stack

  static QEvt const *frame_queueSto[10];
  AC::AO_Frame->start(6U, // priority
    frame_queueSto, Q_DIM(frame_queueSto),
    (void *)0, 0U); // no stack

  //...
}

void FSP::Arena_InitialTransition(QActive * const ao)
{
  BSP::initializeArena();
  ao->subscribe(AC::RESET_SIG);
  ao->subscribe(AC::ALL_ON_SIG);
  ao->subscribe(AC::ALL_OFF_SIG);
}

void FSP::Arena_ArenaOn_entry(QActive * const ao)
{
  BSP::activateCommandInterfaces();
}

void FSP::Arena_ArenaOn_exit(QActive * const ao)
{
  BSP::deactivateCommandInterfaces();
}

void FSP::Arena_ArenaOn_AllOff_entry(QActive * const ao)
{
  static QEvt const deactivateDisplayEvt = { AC::DEACTIVATE_DISPLAY_SIG, 0U, 0U};
  QF::PUBLISH(&deactivateDisplayEvt, ao);
}

void FSP::Arena_ArenaOn_AllOn_entry(QActive * const ao)
{
  static AC::DisplayFramesEvt displayFramesEvt = { AC::DISPLAY_FRAMES_SIG, 0U, 0U};
  displayFramesEvt.panel_buffer = &AC::constants::all_on_grayscale_pattern;
  displayFramesEvt.panel_buffer_byte_count = AC::constants::byte_count_per_panel_grayscale;
  QF::PUBLISH(&displayFramesEvt, ao);
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
