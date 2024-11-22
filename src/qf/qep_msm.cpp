//$file${src::qf::qep_msm.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qf::qep_msm.cpp}
//
// This code has been generated by QM 7.0.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// The QP/C++ software is dual-licensed under the terms of the open-source
// GNU General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for
// closed-source distribution.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qf::qep_msm.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

//============================================================================
//! @cond INTERNAL

// unnamed namespace for local definitions with internal linkage
namespace {

Q_DEFINE_THIS_MODULE("qep_msm")

// top-state object for QMsm-style state machines
QP::QMState const l_msm_top_s = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

// maximum depth of state nesting in a QMsm (including the top level)
static constexpr std::int_fast8_t QMSM_MAX_NEST_DEPTH_ {8};

// maximum length of transition-action array
static constexpr std::int_fast8_t QMSM_MAX_TRAN_LENGTH_ {2*QMSM_MAX_NEST_DEPTH_};

// maximum depth of entry levels in a MSM for tran. to history.
static constexpr std::int_fast8_t QMSM_MAX_ENTRY_DEPTH_ {4};

} // unnamed namespace

//! @endcond
//============================================================================

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpcpp version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QEP::QMsm} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QEP::QMsm} ...............................................................

//${QEP::QMsm::QMsm} .........................................................
QMsm::QMsm(QStateHandler const initial) noexcept
  : QAsm()
{
    m_state.obj = &l_msm_top_s; // the current state (top)
    m_temp.fun  = initial;      // the initial tran. handler
}

//${QEP::QMsm::init} .........................................................
void QMsm::init(
    void const * const e,
    std::uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(200, (m_temp.fun != nullptr)
                      && (m_state.obj == &l_msm_top_s));
    QF_CRIT_EXIT();

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    QF_CRIT_ENTRY();
    // the top-most initial tran. must be taken
    Q_ASSERT_INCRIT(210, r == Q_RET_TRAN_INIT);

    QS_MEM_SYS();
    QS_BEGIN_PRE(QS_QEP_STATE_INIT, qsId)
        QS_OBJ_PRE(this);  // this state machine object
        QS_FUN_PRE(m_state.obj->stateHandler);          // source state
        QS_FUN_PRE(m_temp.tatbl->target->stateHandler); // target state
    QS_END_PRE()
    QS_MEM_APP();

    QF_CRIT_EXIT();

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
    do {
        // execute the tran. table
        r = execTatbl_(m_temp.tatbl, qsId);
        --lbound;
    } while ((r >= Q_RET_TRAN_INIT) && (lbound > 0));

    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(290, lbound > 0);

    QS_MEM_SYS();
    QS_BEGIN_PRE(QS_QEP_INIT_TRAN, qsId)
        QS_TIME_PRE();    // time stamp
        QS_OBJ_PRE(this); // this state machine object
        QS_FUN_PRE(m_state.obj->stateHandler); // the new current state
    QS_END_PRE()
    QS_MEM_APP();

    QF_CRIT_EXIT();

    #ifndef Q_UNSAFE
    m_temp.uint = ~m_state.uint;
    #endif
}

//${QEP::QMsm::dispatch} .....................................................
void QMsm::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QMState const *s = m_state.obj; // store the current state
    QMState const *t = s;

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300, (e != nullptr) && (s != nullptr));
    Q_INVARIANT_INCRIT(301,
        e->verify_()
        && (m_state.uint == static_cast<std::uintptr_t>(~m_temp.uint)));

    QS_MEM_SYS();
    QS_BEGIN_PRE(QS_QEP_DISPATCH, qsId)
        QS_TIME_PRE();               // time stamp
        QS_SIG_PRE(e->sig);          // the signal of the event
        QS_OBJ_PRE(this);            // this state machine object
        QS_FUN_PRE(s->stateHandler); // the current state handler
    QS_END_PRE()
    QS_MEM_APP();

    QF_CRIT_EXIT();

    // scan the state hierarchy up to the top state...
    QState r;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
    do {
        r = (*t->stateHandler)(this, e); // call state handler function

        // event handled? (the most frequent case)
        if (r >= Q_RET_HANDLED) {
            break; // done scanning the state hierarchy
        }
        // event unhandled and passed to the superstate?
        else if (r == Q_RET_SUPER) {
            t = t->superstate; // advance to the superstate
        }
        else { // event unhandled due to a guard
            QF_CRIT_ENTRY();
            // event must be unhandled due to a guard evaluating to 'false'
            Q_ASSERT_INCRIT(310, r == Q_RET_UNHANDLED);

            QS_MEM_SYS();
            QS_BEGIN_PRE(QS_QEP_UNHANDLED, qsId)
                QS_SIG_PRE(e->sig);  // the signal of the event
                QS_OBJ_PRE(this);    // this state machine object
                QS_FUN_PRE(t->stateHandler); // the current state
            QS_END_PRE()
            QS_MEM_APP();

            QF_CRIT_EXIT();

            t = t->superstate; // advance to the superstate
        }
        --lbound;
    } while ((t != nullptr) && (lbound > 0));
    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(320, lbound > 0);
    QF_CRIT_EXIT();

    if (r >= Q_RET_TRAN) { // any kind of tran. taken?
        QF_CRIT_ENTRY();
        // the tran. source state must not be nullptr
        Q_ASSERT_INCRIT(330, t != nullptr);
        QF_CRIT_EXIT();

    #ifdef Q_SPY
        QMState const * const ts = t; // for saving tran. table
    #endif // Q_SPY
        QMTranActTable const *tatbl;

        if (r == Q_RET_TRAN_HIST) { // was it tran. to history?
            QMState const * const hist = m_state.obj; // save history
            m_state.obj = s; // restore the original state

            QS_CRIT_ENTRY();
            QS_MEM_SYS();
            QS_BEGIN_PRE(QS_QEP_TRAN_HIST, qsId)
                QS_OBJ_PRE(this); // this state machine object
                QS_FUN_PRE(t->stateHandler);    // source state handler
                QS_FUN_PRE(hist->stateHandler); // target state handler
            QS_END_PRE()
            QS_MEM_APP();
            QS_CRIT_EXIT();

            // save the tran-action table before it gets clobbered
            tatbl = m_temp.tatbl;
            exitToTranSource_(s, t, qsId);
            static_cast<void>(execTatbl_(tatbl, qsId));
            r = enterHistory_(hist, qsId);
            s = m_state.obj;
            t = s; // set target to the current state
        }

        lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
        while ((r >= Q_RET_TRAN) && (lbound > 0)) {
            // save the tran-action table before it gets clobbered
            tatbl = m_temp.tatbl;
            m_temp.obj = nullptr; // clear
            exitToTranSource_(s, t, qsId);
            r = execTatbl_(tatbl, qsId);
            s = m_state.obj;
            t = s; // set target to the current state
            --lbound;
        }

        QF_CRIT_ENTRY();
        Q_ENSURE_INCRIT(360, lbound > 0);

        QS_MEM_SYS();
        QS_BEGIN_PRE(QS_QEP_TRAN, qsId)
            QS_TIME_PRE();                 // time stamp
            QS_SIG_PRE(e->sig);            // the signal of the event
            QS_OBJ_PRE(this);              // this state machine object
            QS_FUN_PRE(ts->stateHandler);  // the tran. source
            QS_FUN_PRE(s->stateHandler);   // the new active state
        QS_END_PRE()
        QS_MEM_APP();

        QF_CRIT_EXIT();
    }

    #ifdef Q_SPY
    // was the event handled?
    else if (r == Q_RET_HANDLED) {
        QF_CRIT_ENTRY();
        // internal tran. source can't be nullptr
        Q_ASSERT_INCRIT(380, t != nullptr);

        QS_MEM_SYS();
        QS_BEGIN_PRE(QS_QEP_INTERN_TRAN, qsId)
            QS_TIME_PRE();                 // time stamp
            QS_SIG_PRE(e->sig);            // the signal of the event
            QS_OBJ_PRE(this);              // this state machine object
            QS_FUN_PRE(t->stateHandler);   // the source state
        QS_END_PRE()
        QS_MEM_APP();

        QF_CRIT_EXIT();
    }
    // event bubbled to the 'top' state?
    else if (t == nullptr) {
        QS_CRIT_ENTRY();
        QS_MEM_SYS();
        QS_BEGIN_PRE(QS_QEP_IGNORED, qsId)
            QS_TIME_PRE();                 // time stamp
            QS_SIG_PRE(e->sig);            // the signal of the event
            QS_OBJ_PRE(this);              // this state machine object
            QS_FUN_PRE(s->stateHandler);   // the current state
        QS_END_PRE()
        QS_MEM_APP();
        QS_CRIT_EXIT();
    }
    #endif // Q_SPY
    else {
        // empty
    }

    #ifndef Q_UNSAFE
    m_temp.uint = ~m_state.uint;
    #endif
}

//${QEP::QMsm::isIn} .........................................................
bool QMsm::isIn(QStateHandler const state) noexcept {
    bool inState = false; // assume that this SM is not in 'state'

    QMState const *s = m_state.obj;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
    for (; (s != nullptr) && (lbound > 0); --lbound) {
        if (s->stateHandler == state) { // match found?
            inState = true;
            break;
        }
        else {
            s = s->superstate; // advance to the superstate
        }
    }

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(490, lbound > 0);
    QF_CRIT_EXIT();

    return inState;
}

//${QEP::QMsm::childStateObj} ................................................
QMState const * QMsm::childStateObj(QMState const * const parent) const noexcept {
    QMState const *child = m_state.obj;
    bool isFound = false; // start with the child not found
    QMState const *s;

    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
    for (s = m_state.obj;
         (s != nullptr) && (lbound > 0);
         s = s->superstate)
    {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        else {
            child = s;
        }
        --lbound;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(680, lbound > 0);
    QF_CRIT_EXIT();

    if (!isFound) { // still not found?
        lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
        for (s = m_temp.obj;
             (s != nullptr) && (lbound > 0);
             s = s->superstate)
        {
            if (s == parent) {
                isFound = true; // child is found
                break;
            }
            else {
                child = s;
            }
            --lbound;
        }
    }

    QF_CRIT_ENTRY();
    // NOTE: the following postcondition can only succeed when
    // (lbound > 0), so no extra check is necessary.
    Q_ENSURE_INCRIT(690, isFound);
    QF_CRIT_EXIT();

    return child; // return the child
}

//${QEP::QMsm::execTatbl_} ...................................................
QState QMsm::execTatbl_(
    QMTranActTable const * const tatbl,
    std::uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    // precondition:
    // - the tran-action table pointer must not be NULL
    Q_REQUIRE_INCRIT(700, tatbl != nullptr);
    QF_CRIT_EXIT();

    QState r = Q_RET_NULL;
    std::int_fast8_t lbound = QMSM_MAX_TRAN_LENGTH_; // fixed upper loop bound
    QActionHandler const *a = &tatbl->act[0];
    for (; (*a != nullptr) && (lbound > 0); ++a) {
        r = (*(*a))(this); // call the action through the 'a' pointer
        --lbound;
    #ifdef Q_SPY
        QS_CRIT_ENTRY();
        QS_MEM_SYS();
        if (r == Q_RET_ENTRY) {
            QS_BEGIN_PRE(QS_QEP_STATE_ENTRY, qsId)
                QS_OBJ_PRE(this); // this state machine object
                QS_FUN_PRE(m_temp.obj->stateHandler); // entered state
            QS_END_PRE()
        }
        else if (r == Q_RET_EXIT) {
            QS_BEGIN_PRE(QS_QEP_STATE_EXIT, qsId)
                QS_OBJ_PRE(this); // this state machine object
                QS_FUN_PRE(m_temp.obj->stateHandler); // exited state
            QS_END_PRE()
        }
        else if (r == Q_RET_TRAN_INIT) {
            QS_BEGIN_PRE(QS_QEP_STATE_INIT, qsId)
                QS_OBJ_PRE(this); // this state machine object
                QS_FUN_PRE(tatbl->target->stateHandler);        // source
                QS_FUN_PRE(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE()
        }
        else {
            // empty
        }
        QS_MEM_APP();
        QS_CRIT_EXIT();
    #endif // Q_SPY
    }
    QF_CRIT_ENTRY();
    // NOTE: the following postcondition can only succeed when
    // (lbound > 0), so no extra check is necessary.
    Q_ENSURE_INCRIT(790, *a == nullptr);
    QF_CRIT_EXIT();

    m_state.obj = (r >= Q_RET_TRAN)
        ? m_temp.tatbl->target
        : tatbl->target;
    return r;
}

//${QEP::QMsm::exitToTranSource_} ............................................
void QMsm::exitToTranSource_(
    QMState const * const cs,
    QMState const * const ts,
    std::uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    QF_CRIT_STAT

    // exit states from the current state to the tran. source state
    QMState const *s = cs;
    std::int_fast8_t lbound = QMSM_MAX_NEST_DEPTH_; // fixed upper loop bound
    for (; (s != ts) && (lbound > 0); --lbound) {
        // exit action provided in state 's'?
        if (s->exitAction != nullptr) {
            // execute the exit action
            static_cast<void>((*s->exitAction)(this));

            QS_CRIT_ENTRY();
            QS_MEM_SYS();
            QS_BEGIN_PRE(QS_QEP_STATE_EXIT, qsId)
                QS_OBJ_PRE(this);            // this state machine object
                QS_FUN_PRE(s->stateHandler); // the exited state handler
            QS_END_PRE()
            QS_MEM_APP();
            QS_CRIT_EXIT();
        }

        s = s->superstate; // advance to the superstate
    }
    QF_CRIT_ENTRY();
    Q_ENSURE_INCRIT(890, lbound > 0);
    QF_CRIT_EXIT();
}

//${QEP::QMsm::enterHistory_} ................................................
QState QMsm::enterHistory_(
    QMState const * const hist,
    std::uint_fast8_t const qsId)
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(qsId);
    #endif

    // record the entry path from current state to history
    QMState const *epath[QMSM_MAX_ENTRY_DEPTH_];
    QMState const *s = hist;
    std::int_fast8_t i = 0; // tran. entry path index
    while ((s != m_state.obj) && (i < (QMSM_MAX_ENTRY_DEPTH_ - 1))) {
        if (s->entryAction != nullptr) {
            epath[i] = s;
            ++i;
        }
        s = s->superstate;
    }
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_ASSERT_INCRIT(910, s == m_state.obj);
    QF_CRIT_EXIT();

    // retrace the entry path in reverse (desired) order...
    while (i > 0) {
        --i;
        (*epath[i]->entryAction)(this); // run entry action in epath[i]

        QS_CRIT_ENTRY();
        QS_MEM_SYS();
        QS_BEGIN_PRE(QS_QEP_STATE_ENTRY, qsId)
            QS_OBJ_PRE(this);
            QS_FUN_PRE(epath[i]->stateHandler); // entered state handler
        QS_END_PRE()
        QS_MEM_APP();
        QS_CRIT_EXIT();
    }

    m_state.obj = hist; // set current state to the tran. target

    // initial tran. present?
    QState r;
    if (hist->initAction != nullptr) {
        r = (*hist->initAction)(this); // execute the tran. action
        QS_CRIT_ENTRY();
        QS_MEM_SYS();
        QS_BEGIN_PRE(QS_QEP_STATE_INIT, qsId)
            QS_OBJ_PRE(this); // this state machine object
            QS_FUN_PRE(hist->stateHandler);                 // source
            QS_FUN_PRE(m_temp.tatbl->target->stateHandler); // target
        QS_END_PRE()
        QS_MEM_APP();
        QS_CRIT_EXIT();
    }
    else {
        r = Q_RET_NULL;
    }

    return r;
}

//${QEP::QMsm::topQMState} ...................................................
QMState const * QMsm::topQMState() const noexcept {
    return &l_msm_top_s;
}

} // namespace QP
//$enddef${QEP::QMsm} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
