//============================================================================
// QP/C++ Real-Time Event Framework (RTEF)
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open-source GNU
// General Public License (GPL) or under the terms of one of the closed-
// source Quantum Leaps commercial licenses.
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// NOTE:
// The GPL (see <www.gnu.org/licenses/gpl-3.0>) does NOT permit the
// incorporation of the QP/C software into proprietary programs. Please
// contact Quantum Leaps for commercial licensing options, which expressly
// supersede the GPL and are designed explicitly for licensees interested
// in using QP/C in closed-source proprietary applications.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef QP_PORT_HPP_
#define QP_PORT_HPP_

#include <cstdint>        // Exact-width types. C++11 Standard
#include "qp_config.hpp"  // QP configuration from the application

// no-return function specifier (C++11 Standard)
#define Q_NORETURN  [[ noreturn ]] void

// QActive event queue type
#define QACTIVE_EQUEUE_TYPE  QEQueue

// QF interrupt disable/enable
#define QF_INT_DISABLE()     (++QP::QF::intLock_)
#define QF_INT_ENABLE()      (--QP::QF::intLock_)

// QF critical section
#define QF_CRIT_STAT
#define QF_CRIT_ENTRY()      QF_INT_DISABLE()
#define QF_CRIT_EXIT()       QF_INT_ENABLE()

// QF_LOG2 not defined -- use the internal LOG2() implementation

// include files -------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#elif defined(__GNUC__)
    #include <msp430.h>
    #include "in430.h"
#endif

#include "qequeue.hpp"   // QUTest port uses QEQueue event-queue
#include "qmpool.hpp"    // QUTest port uses QMPool memory-pool
#include "qp.hpp"        // QP platform-independent public interface

//============================================================================
// interface used only inside QF implementation, but not in applications

#ifdef QP_IMPL

    // QUTest scheduler locking (not used)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) (static_cast<void>(0))
    #define QF_SCHED_UNLOCK_()    (static_cast<void>(0))

    // native QEQueue operations
    #define QACTIVE_EQUEUE_WAIT_(me_) (static_cast<void>(0))
    #define QACTIVE_EQUEUE_SIGNAL_(me_)    \
        (QP::QS::tstPriv_.readySet.insert( \
            static_cast<std::uint_fast8_t>((me_)->m_prio)))

    // native QMPool operations
    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
    #define QF_EPOOL_PUT_(p_, e_, qsId_)  ((p_).put((e_), (qsId_)))

#endif // QP_IMPL

#endif // QP_PORT_HPP_

