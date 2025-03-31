//.$file${.::Arena.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: ArenaController.qm
// File:  ${.::Arena.hpp}
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
//.$endhead${.::Arena.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef ARENA_HPP
#define ARENA_HPP

#include "ArenaController.hpp"


//============================================================================
// generate declaration of the active object
//.$declare${AOs::Arena} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace AC {

//.${AOs::Arena} .............................................................
class Arena : public QP::QActive {
public:
    static Arena instance;

public:
    Arena();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(ArenaOn);
    Q_STATE_DECL(AllOn);
    Q_STATE_DECL(AllOff);
    Q_STATE_DECL(StreamingFrame);
};

} // namespace AC
//.$enddecl${AOs::Arena} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif