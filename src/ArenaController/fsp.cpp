#include "fsp.hpp"


using namespace QP;

static QP::QSpyId const l_COMMAND_ID = { 1U }; // QSpy source ID

static AC::CommandEvt const resetEvt = { AC::RESET_SIG, 0U, 0U};
static AC::CommandEvt const allOnEvt = { AC::ALL_ON_SIG, 0U, 0U};
static AC::CommandEvt const allOffEvt = { AC::ALL_OFF_SIG, 0U, 0U};
static QEvt const commandProcessedEvt = { AC::COMMAND_PROCESSED_SIG, 0U, 0U};

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
