//$file${src::qf::qf_time.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qf::qf_time.cpp}
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
//$endhead${src::qf::qf_time.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define QP_IMPL             // this is QP implementation
#include "qp_port.hpp"      // QP port
#include "qp_pkg.hpp"       // QP package-scope interface
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_time")
} // unnamed namespace

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpcpp version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QTimeEvt} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QTimeEvt} ............................................................
QTimeEvt QTimeEvt::timeEvtHead_[QF_MAX_TICK_RATE];

#ifndef Q_UNSAFE
QPtrDis QTimeEvt::timeEvtHead_dis_[QF_MAX_TICK_RATE];
#endif // ndef Q_UNSAFE

//${QF::QTimeEvt::QTimeEvt} ..................................................
QTimeEvt::QTimeEvt(
    QActive * const act,
    QSignal const sig,
    std::uint_fast8_t const tickRate) noexcept
 :  QEvt(sig),
    m_next(nullptr),
#ifndef Q_UNSAFE
    m_next_dis(static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(nullptr))),
#endif
    m_act(act),
    m_ctr(0U),
#ifndef Q_UNSAFE
    m_ctr_dis(static_cast<QTimeEvtCtr>(~static_cast<QTimeEvtCtr>(0U))),
#endif
    m_interval(0U),
    m_tickRate(0U),
    m_flags(0U)
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(300, (sig != 0U)
        && (tickRate < QF_MAX_TICK_RATE));
    QF_CRIT_EXIT();

    // adjust the settings from the QEvt(sig) ctor
    evtTag_ = 0x0FU;
    refCtr_ = 0U;
}

//${QF::QTimeEvt::armX} ......................................................
void QTimeEvt::armX(
    std::uint32_t const nTicks,
    std::uint32_t const interval) noexcept
{
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    // dynamic range checks
    #if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(400, (nTicks < 0xFFU) && (interval < 0xFFU));
    #elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(400, (nTicks < 0xFFFFU) && (interval < 0xFFFFU));
    #endif

    Q_REQUIRE_INCRIT(401, verify_() && (nTicks != 0U));

    QTimeEvtCtr const ctr = m_ctr;
    std::uint8_t const tickRate = m_tickRate;
    #ifdef Q_SPY
    std::uint_fast8_t const qsId =
         static_cast<QActive const *>(m_act)->m_prio;
    #endif // def Q_SPY

    Q_REQUIRE_INCRIT(410, (ctr == 0U)
        && (m_act != nullptr)
        && (tickRate < QF_MAX_TICK_RATE));

    #ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(411, ctr == static_cast<QTimeEvtCtr>(~m_ctr_dis));
    #else
    Q_UNUSED_PAR(ctr);
    #endif // ndef Q_UNSAFE

    m_ctr = static_cast<QTimeEvtCtr>(nTicks);
    m_interval = static_cast<QTimeEvtCtr>(interval);
    #ifndef Q_UNSAFE
    m_ctr_dis = static_cast<QTimeEvtCtr>(~nTicks);
    #endif // ndef Q_UNSAFE

    // is the time event unlinked?
    // NOTE: For the duration of a single clock tick of the specified tick
    // rate a time event can be disarmed and yet still linked into the list
    // because un-linking is performed exclusively in the QF_tickX() function.
    if ((m_flags & QTE_FLAG_IS_LINKED) == 0U) {
        m_flags |= QTE_FLAG_IS_LINKED; // mark as linked

        // The time event is initially inserted into the separate
        // "freshly armed" list based on timeEvtHead_[tickRate].act.
        // Only later, inside QTimeEvt::tick(), the "freshly armed"
        // list is appended to the main list of armed time events based on
        // timeEvtHead_[tickRate].next. Again, this is to keep any
        // changes to the main list exclusively inside QTimeEvt::tick().
    #ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(420,
            Q_PTR2UINT_CAST_(m_next) ==
            static_cast<std::uintptr_t>(~m_next_dis));
        Q_INVARIANT_INCRIT(411,
            Q_PTR2UINT_CAST_(timeEvtHead_[tickRate].m_act) ==
            static_cast<std::uintptr_t>(~timeEvtHead_dis_[tickRate].m_ptr_dis));
    #endif
        m_next = timeEvtHead_[tickRate].toTimeEvt();
        timeEvtHead_[tickRate].m_act = this;
    #ifndef Q_UNSAFE
        m_next_dis =
            static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(m_next));
        timeEvtHead_dis_[tickRate].m_ptr_dis =
            static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(this));
    #endif // ndef Q_UNSAFE
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_ARM, qsId)
        QS_TIME_PRE();        // timestamp
        QS_OBJ_PRE(this);     // this time event object
        QS_OBJ_PRE(m_act);    // the active object
        QS_TEC_PRE(nTicks);   // the # ticks
        QS_TEC_PRE(interval); // the interval
        QS_U8_PRE(tickRate);  // tick rate
    QS_END_PRE()

    QF_MEM_APP();
    QF_CRIT_EXIT();
}

//${QF::QTimeEvt::disarm} ....................................................
bool QTimeEvt::disarm() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    Q_REQUIRE_INCRIT(500, verify_());

    QTimeEvtCtr const ctr = m_ctr;
    #ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(501, ctr == static_cast<QTimeEvtCtr>(~m_ctr_dis));
    #endif // ndef Q_UNSAFE

    #ifdef Q_SPY
    std::uint_fast8_t const qsId = static_cast<QActive *>(m_act)->m_prio;
    #endif

    // was the time event actually armed?
    bool wasArmed;
    if (ctr != 0U) {
        wasArmed = true;
        m_flags |= QTE_FLAG_WAS_DISARMED;
        m_ctr = 0U; // schedule removal from the list
    #ifndef Q_UNSAFE
        m_ctr_dis = static_cast<QTimeEvtCtr>(~static_cast<QTimeEvtCtr>(0U));
    #endif // ndef Q_UNSAFE

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(this);         // this time event object
            QS_OBJ_PRE(m_act);        // the target AO
            QS_TEC_PRE(ctr);          // the # ticks
            QS_TEC_PRE(m_interval);   // the interval
            QS_U8_PRE(m_tickRate);    // tick rate
        QS_END_PRE()
    }
    else { // the time event was already disarmed automatically
        wasArmed = false;
        m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_WAS_DISARMED);

        QS_BEGIN_PRE(QS_QF_TIMEEVT_DISARM_ATTEMPT, qsId)
            QS_TIME_PRE();            // timestamp
            QS_OBJ_PRE(this);         // this time event object
            QS_OBJ_PRE(m_act);        // the target AO
            QS_U8_PRE(m_tickRate);    // tick rate
        QS_END_PRE()
    }

    QF_MEM_APP();
    QF_CRIT_EXIT();

    return wasArmed;
}

//${QF::QTimeEvt::rearm} .....................................................
bool QTimeEvt::rearm(std::uint32_t const nTicks) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    // dynamic range checks
    #if (QF_TIMEEVT_CTR_SIZE == 1U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFU);
    #elif (QF_TIMEEVT_CTR_SIZE == 2U)
    Q_REQUIRE_INCRIT(600, nTicks < 0xFFFFU);
    #endif

    Q_REQUIRE_INCRIT(601, verify_() && (nTicks != 0U));

    QTimeEvtCtr const ctr = m_ctr;
    std::uint8_t const tickRate = m_tickRate;
    #ifdef Q_SPY
    std::uint_fast8_t const qsId = static_cast<QActive *>(m_act)->m_prio;
    #endif

    Q_REQUIRE_INCRIT(610, (m_act != nullptr)
        && (tickRate < QF_MAX_TICK_RATE));

    #ifndef Q_UNSAFE
    Q_INVARIANT_INCRIT(602, ctr == static_cast<QTimeEvtCtr>(~m_ctr_dis));
    #endif // ndef Q_UNSAFE
    m_ctr = static_cast<QTimeEvtCtr>(nTicks);
    #ifndef Q_UNSAFE
    m_ctr_dis = static_cast<QTimeEvtCtr>(~nTicks);
    #endif // ndef Q_UNSAFE

    // is the time evt not running?
    bool wasArmed;
    if (ctr == 0U) {
        wasArmed = false;

        // NOTE: For a duration of a single clock tick of the specified
        // tick rate a time event can be disarmed and yet still linked into
        // the list, because unlinking is performed exclusively in the
        // QTimeEvt::tick() function.

        // was the time event unlinked?
        if ((m_flags & QTE_FLAG_IS_LINKED) == 0U) {
            m_flags |= QTE_FLAG_IS_LINKED; // mark as linked

            // The time event is initially inserted into the separate
            // "freshly armed" list based on timeEvtHead_[tickRate].act.
            // Only later, inside QTimeEvt::tick(), the "freshly armed"
            // list is appended to the main list of armed time events based on
            // timeEvtHead_[tickRate].next. Again, this is to keep any
            // changes to the main list exclusively inside QTimeEvt::tick().
    #ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(620,
            Q_PTR2UINT_CAST_(m_next) ==
            static_cast<std::uintptr_t>(~m_next_dis));
        Q_INVARIANT_INCRIT(611,
            Q_PTR2UINT_CAST_(timeEvtHead_[tickRate].m_act) ==
            static_cast<std::uintptr_t>(~timeEvtHead_dis_[tickRate].m_ptr_dis));
    #endif
            m_next = timeEvtHead_[tickRate].toTimeEvt();
            timeEvtHead_[tickRate].m_act = this;
    #ifndef Q_UNSAFE
            m_next_dis =
                static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(m_next));
            timeEvtHead_dis_[tickRate].m_ptr_dis =
                static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(this));
    #endif // ndef Q_UNSAFE
        }
    }
    else { // the time event was armed
        wasArmed = true;
    }

    QS_BEGIN_PRE(QS_QF_TIMEEVT_REARM, qsId)
        QS_TIME_PRE();            // timestamp
        QS_OBJ_PRE(this);         // this time event object
        QS_OBJ_PRE(m_act);        // the target AO
        QS_TEC_PRE(nTicks);       // the # ticks
        QS_TEC_PRE(m_interval);   // the interval
        QS_2U8_PRE(tickRate, (wasArmed ? 1U : 0U));
    QS_END_PRE()

    QF_MEM_APP();
    QF_CRIT_EXIT();

    return wasArmed;
}

//${QF::QTimeEvt::wasDisarmed} ...............................................
bool QTimeEvt::wasDisarmed() noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    bool const wasDisarmed = (m_flags & QTE_FLAG_WAS_DISARMED) != 0U;
    m_flags |= QTE_FLAG_WAS_DISARMED;

    QF_MEM_APP();
    QF_CRIT_EXIT();

    return wasDisarmed;
}

//${QF::QTimeEvt::tick} ......................................................
void QTimeEvt::tick(
    std::uint_fast8_t const tickRate,
    void const * const sender) noexcept
{
    #ifndef Q_SPY
    Q_UNUSED_PAR(sender);
    #endif

    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    QF_MEM_SYS();

    Q_REQUIRE_INCRIT(800, tickRate < Q_DIM(timeEvtHead_));

    QTimeEvt *prev = &timeEvtHead_[tickRate];

    QS_BEGIN_PRE(QS_QF_TICK, 0U)
        prev->m_ctr = (prev->m_ctr + 1U);
        QS_TEC_PRE(prev->m_ctr); // tick ctr
        QS_U8_PRE(tickRate);     // tick rate
    QS_END_PRE()

    // scan the linked-list of time events at this rate...
    std::uint_fast8_t lbound = 2U*QF_MAX_ACTIVE; // fixed upper loop bound
    for (; lbound > 0U; --lbound) {
        Q_ASSERT_INCRIT(810, prev != nullptr); // sanity check

        QTimeEvt *te = prev->m_next; // advance down the time evt. list
    #ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(811,
            Q_PTR2UINT_CAST_(te) ==
                static_cast<std::uintptr_t>(~prev->m_next_dis));
    #endif // ndef Q_UNSAFE

        if (te == nullptr) { // end of the list?

            // any new time events armed since the last QTimeEvt_tick_()?
            if (timeEvtHead_[tickRate].m_act != nullptr) {
    #ifndef Q_UNSAFE
                Q_INVARIANT_INCRIT(812,
                    Q_PTR2UINT_CAST_(timeEvtHead_[tickRate].m_act) ==
                    static_cast<std::uintptr_t>(
                        ~timeEvtHead_dis_[tickRate].m_ptr_dis));
    #endif // ndef Q_UNSAFE
                prev->m_next = timeEvtHead_[tickRate].toTimeEvt();
                timeEvtHead_[tickRate].m_act = nullptr;
    #ifndef Q_UNSAFE
                prev->m_next_dis =
                    static_cast<std::uintptr_t>(
                        ~Q_PTR2UINT_CAST_(prev->m_next));
                timeEvtHead_dis_[tickRate].m_ptr_dis =
                    static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(nullptr));
    #endif // ndef Q_UNSAFE

                te = prev->m_next; // switch to the new list
            }
            else { // all currently armed time events are processed
                break; // terminate the for-loop
            }
        }

        // the time event 'te' must be valid
        Q_ASSERT_INCRIT(820, te != nullptr);
        Q_INVARIANT_INCRIT(821, te->verify_());

        QTimeEvtCtr ctr = te->m_ctr;
    #ifndef Q_UNSAFE
        Q_INVARIANT_INCRIT(822, ctr ==
            static_cast<QTimeEvtCtr>(~te->m_ctr_dis));
    #endif // ndef Q_UNSAFE

        if (ctr == 0U) { // time event scheduled for removal?
            prev->m_next = te->m_next;
    #ifndef Q_UNSAFE
            prev->m_next_dis =
                static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(te->m_next));
    #endif // ndef Q_UNSAFE

            // mark time event 'te' as NOT linked
            te->m_flags &= static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);

            // do NOT advance the prev pointer
            QF_MEM_APP();
            QF_CRIT_EXIT(); // exit crit. section to reduce latency

            // NOTE: prevent merging critical sections
            // In some QF ports the critical section exit takes effect only
            // on the next machine instruction. If the next instruction is
            // another entry to a critical section, the critical section
            // might not be really exited, but rather the two adjacent
            // critical sections would be MERGED. The QF_CRIT_EXIT_NOP()
            // macro contains minimal code required to prevent such merging
            // of critical sections in QF ports, in which it can occur.
            QF_CRIT_EXIT_NOP();
        }
        else if (ctr == 1U) { // is time evt about to expire?
            QActive * const act = te->toActive();
            if (te->m_interval != 0U) { // periodic time evt?
                te->m_ctr = te->m_interval; // rearm the time event
    #ifndef Q_UNSAFE
                te->m_ctr_dis = static_cast<QTimeEvtCtr>(~te->m_interval);
    #endif // ndef Q_UNSAFE
                prev = te; // advance to this time event
            }
            else { // one-shot time event: automatically disarm
                te->m_ctr = 0U;
                prev->m_next = te->m_next;
    #ifndef Q_UNSAFE
                te->m_ctr_dis =
                    static_cast<QTimeEvtCtr>(~static_cast<QTimeEvtCtr>(0U));
                prev->m_next_dis =
                    static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(te->m_next));
    #endif // ndef Q_UNSAFE

                // mark time event 'te' as NOT linked
                te->m_flags &=
                    static_cast<std::uint8_t>(~QTE_FLAG_IS_LINKED);
                // do NOT advance the prev pointer

                QS_BEGIN_PRE(QS_QF_TIMEEVT_AUTO_DISARM, act->m_prio)
                    QS_OBJ_PRE(te);       // this time event object
                    QS_OBJ_PRE(act);      // the target AO
                    QS_U8_PRE(tickRate);  // tick rate
                QS_END_PRE()
            }

            QS_BEGIN_PRE(QS_QF_TIMEEVT_POST, act->m_prio)
                QS_TIME_PRE();            // timestamp
                QS_OBJ_PRE(te);           // the time event object
                QS_SIG_PRE(te->sig);      // signal of this time event
                QS_OBJ_PRE(act);          // the target AO
                QS_U8_PRE(tickRate);      // tick rate
            QS_END_PRE()

    #ifdef QXK_HPP_
            if (te->sig < Q_USER_SIG) {
                QXThread::timeout_(act);
                QF_MEM_APP();
                QF_CRIT_EXIT();
            }
            else {
                QF_MEM_APP();
                QF_CRIT_EXIT(); // exit crit. section before posting

                // act->POST() asserts if the queue overflows
                act->POST(te, sender);
            }
    #else
            QF_MEM_APP();
            QF_CRIT_EXIT(); // exit crit. section before posting

            // act->POST() asserts if the queue overflows
            act->POST(te, sender);
    #endif
        }
        else { // time event keeps timing out
            --ctr; // decrement the tick counter
            te->m_ctr = ctr; // update the original
    #ifndef Q_UNSAFE
            te->m_ctr_dis = static_cast<QTimeEvtCtr>(~ctr);
    #endif // ndef Q_UNSAFE

            prev = te; // advance to this time event

            QF_MEM_APP();
            QF_CRIT_EXIT(); // exit crit. section to reduce latency

            // prevent merging critical sections, see NOTE above
            QF_CRIT_EXIT_NOP();
        }
        QF_CRIT_ENTRY(); // re-enter crit. section to continue the loop
        QF_MEM_SYS();
    }

    Q_ENSURE_INCRIT(890, lbound > 0U);
    QF_MEM_APP();
    QF_CRIT_EXIT();
}

//${QF::QTimeEvt::noActive} ..................................................
bool QTimeEvt::noActive(std::uint_fast8_t const tickRate) noexcept {
    QF_CRIT_STAT
    QF_CRIT_ENTRY();
    Q_REQUIRE_INCRIT(900, tickRate < QF_MAX_TICK_RATE);
    QF_CRIT_EXIT();

    bool inactive;
    if (timeEvtHead_[tickRate].m_next != nullptr) {
        inactive = false;
    }
    else if (timeEvtHead_[tickRate].m_act != nullptr) {
        inactive = false;
    }
    else {
        inactive = true;
    }
    return inactive;
}

//${QF::QTimeEvt::QTimeEvt} ..................................................
QTimeEvt::QTimeEvt() noexcept
 :  QEvt(0U),
    m_next(nullptr),
#ifndef Q_UNSAFE
    m_next_dis(static_cast<std::uintptr_t>(~Q_PTR2UINT_CAST_(nullptr))),
#endif
    m_act(nullptr),
    m_ctr(0U),
#ifndef Q_UNSAFE
    m_ctr_dis(static_cast<QTimeEvtCtr>(~static_cast<QTimeEvtCtr>(0U))),
#endif
    m_interval(0U),
    m_tickRate(0U),
    m_flags(0U)
{}

} // namespace QP
//$enddef${QF::QTimeEvt} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
