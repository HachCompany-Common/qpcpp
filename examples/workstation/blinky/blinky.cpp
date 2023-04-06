//$file${.::blinky.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: blinky.qm
// File:  ${.::blinky.cpp}
//
// This code has been generated by QM 5.2.5 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::blinky.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include <iostream>
#include <cstdlib> // for exit()

using namespace std;
using namespace QP;

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

enum { BSP_TICKS_PER_SEC = 100 };

void BSP_ledOff(void) {
    cout << "LED OFF" << endl;
}
void BSP_ledOn(void) {
    cout << "LED ON" << endl;
}
extern "C" void Q_onAssert(char const * const module, int loc) {
    cout << "Assertion failed in " << module << ':' << loc << endl;
    exit(-1);
}
void QF::onStartup(void) {}
void QF::onCleanup(void) {}
void QF::onClockTick(void) {
    QTimeEvt::TICK_X(0U, nullptr);  // perform the QF clock tick processing
}

enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG,
    MAX_SIG
};

//=============== ask QM to declare the Blinky class ==================
//$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Blinky} .............................................................
class Blinky : public QP::QMActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    Blinky();

protected:
    QM_STATE_DECL( initial);
    QM_STATE_DECL( off);
    QM_ACTION_DECL(off_e);
    QM_STATE_DECL( on);
    QM_ACTION_DECL(on_e);
}; // class Blinky
//$enddecl${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

static Blinky l_blinky;
QActive * const AO_Blinky = &l_blinky;

int main() {
    // statically allocate event queue buffer for the Blinky AO
    static QEvt const *blinky_queueSto[10];

    QF::init(); // initialize the framework
    AO_Blinky->start(1U, // priority
                     blinky_queueSto, Q_DIM(blinky_queueSto),
                     nullptr, 0U); // no stack
    return QF::run(); // run the QF application
}

//================ ask QM to define the Blinky class ==================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 700U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 7.0.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Blinky} .............................................................

//${AOs::Blinky::Blinky} .....................................................
Blinky::Blinky()
: QMActive(Q_STATE_CAST(&Blinky::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//${AOs::Blinky::SM} .........................................................
QM_STATE_DEF(Blinky, initial) {
    //${AOs::Blinky::SM::initial}
    m_timeEvt.armX(BSP_TICKS_PER_SEC/2, BSP_TICKS_PER_SEC/2);
    (void)e; // unused parameter

    QS_FUN_DICTIONARY(&Blinky::off);
    QS_FUN_DICTIONARY(&Blinky::on);

    static struct {
        QP::QMState const *target;
        QP::QActionHandler act[2];
    } const tatbl_ = { // tran-action table
        &off_s, // target state
        {
            &off_e, // entry
            Q_ACTION_NULL // zero terminator
        }
    };
    return qm_tran_init(&tatbl_);
}

//${AOs::Blinky::SM::off} ....................................................
QP::QMState const Blinky::off_s = {
    QM_STATE_NULL, // superstate (top)
    &Blinky::off,
    &Blinky::off_e,
    Q_ACTION_NULL, // no exit action
    Q_ACTION_NULL  // no initial tran.
};
//${AOs::Blinky::SM::off}
QM_ACTION_DEF(Blinky, off_e) {
    BSP_ledOff();
    return qm_entry(&off_s);
}
//${AOs::Blinky::SM::off}
QM_STATE_DEF(Blinky, off) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::off::TIMEOUT}
        case TIMEOUT_SIG: {

            static struct {
                QP::QMState const *target;
                QP::QActionHandler act[2];
            } const tatbl_ = { // tran-action table
                &on_s, // target state
                {
                    &on_e, // entry
                    Q_ACTION_NULL // zero terminator
                }
            };
            status_ = qm_tran(&tatbl_);
            break;
        }
        default: {
            status_ = Q_RET_SUPER;
            break;
        }
    }
    return status_;
}

//${AOs::Blinky::SM::on} .....................................................
QP::QMState const Blinky::on_s = {
    QM_STATE_NULL, // superstate (top)
    &Blinky::on,
    &Blinky::on_e,
    Q_ACTION_NULL, // no exit action
    Q_ACTION_NULL  // no initial tran.
};
//${AOs::Blinky::SM::on}
QM_ACTION_DEF(Blinky, on_e) {
    BSP_ledOn();
    return qm_entry(&on_s);
}
//${AOs::Blinky::SM::on}
QM_STATE_DEF(Blinky, on) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::on::TIMEOUT}
        case TIMEOUT_SIG: {

            static struct {
                QP::QMState const *target;
                QP::QActionHandler act[2];
            } const tatbl_ = { // tran-action table
                &off_s, // target state
                {
                    &off_e, // entry
                    Q_ACTION_NULL // zero terminator
                }
            };
            status_ = qm_tran(&tatbl_);
            break;
        }
        default: {
            status_ = Q_RET_SUPER;
            break;
        }
    }
    return status_;
}
//$enddef${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
