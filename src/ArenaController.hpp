//.$file${.::ArenaController.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: ArenaController.qm
// File:  ${.::ArenaController.hpp}
//
// This code has been generated by QM 5.1.3 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::ArenaController.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef ARENA_CONTROLLER_HPP
#define ARENA_CONTROLLER_HPP

#include "qpcpp.hpp"   // QP-C++ framework
#include "bsp.hpp"
#include "fsp.hpp"

namespace AC {

enum ArenaControllerSignals {

    // commands from serial or ethernet interface
    RESET_SIG = QP::Q_USER_SIG,
    ALL_ON_SIG,
    ALL_OFF_SIG,

    DEACTIVATE_DISPLAY_SIG,
    DISPLAY_FRAMES_SIG,
    TRANSFER_FRAME_SIG,
    PANEL_SET_TRANSFERRED_SIG,
    FRAME_TRANSFERRED_SIG,
    DISPLAY_FRAME_TIMEOUT_SIG,

    SERIAL_COMMAND_AVAILABLE_SIG,
    ETHERNET_COMMAND_AVAILABLE_SIG,
    COMMAND_PROCESSED_SIG,

    MAX_PUB_SIG,    // the last published signal

   // POST to SerialCommandInterface
    ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG,
    DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG,
    SERIAL_READY_SIG,

    // POST to EthernetCommandInterface
    ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG,
    DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG,
    ETHERNET_INITIALIZED_SIG,
    ETHERNET_IP_ADDRESS_FOUND_SIG,
    ETHERNET_SERVER_INITIALIZED_SIG,

    // POST to Display
    SET_DISPLAY_FREQUENCY_SIG,

    WATCHDOG_TIMEOUT_SIG, // signal for Watchdog timeout event
    SERIAL_TIMEOUT_SIG, // signal for SerialCommandInterface timeout event
    ETHERNET_TIMEOUT_SIG, // signal for EthernetCommandInterface timeout event

    MAX_SIG         // the last signal
};

} // namespace AC

namespace ArduinoInterface {

void setup();
void loop();

} // namespace ArduinoInterface

//.$declare${Shared} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace AC {

//.${Shared::CommandEvt} .....................................................
class CommandEvt : public QP::QEvt {};
//.${Shared::DisplayFramesEvt} ...............................................
class DisplayFramesEvt : public QP::QEvt {
public:
    std::uint8_t const (*panel_buffer)[];
    std::uint8_t panel_buffer_byte_count;
};
//.${Shared::TransferFrameEvt} ...............................................
class TransferFrameEvt : public QP::QEvt {
public:
    std::uint8_t const (*panel_buffer)[];
    std::uint8_t panel_buffer_byte_count;
    std::uint8_t region_row_panel_count;
    std::uint8_t region_col_panel_count;
};
//.${Shared::SetDisplayFrequencyEvt} .........................................
class SetDisplayFrequencyEvt : public QP::QEvt {
public:
    std::uint32_t display_frequency_hz;
};
extern QP::QActive * const AO_Arena;
extern QP::QActive * const AO_Display;
extern QP::QActive * const AO_SerialCommandInterface;
extern QP::QActive * const AO_EthernetCommandInterface;
extern QP::QActive * const AO_Frame;
extern QP::QActive * const AO_Watchdog;
//.${Shared::getRegionInstance} ..............................................
QP::QHsm * getRegionInstance(std::uint8_t id) ;

} // namespace AC
//.$enddecl${Shared} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif // ARENA_CONTROLLER_HPP
