//============================================================================
// QP/C++ Real-Time Embedded Framework (RTEF)
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
// Select the CPU at which the QP Framework will be attached
#define CONFIG_QP_PINNED_TO_CORE_0
//#define CONFIG_QP_PINNED_TO_CORE_1

// QActive customization for FreeRTOS
#define QACTIVE_EQUEUE_TYPE  QueueHandle_t
#define QACTIVE_OS_OBJ_TYPE  StaticQueue_t
#define QACTIVE_THREAD_TYPE  StaticTask_t

// FreeRTOS requires the "FromISR" API in QP/C++
#define QF_ISR_API           1

// QF interrupt disabling/enabling (task level)
#define QF_INT_DISABLE()     taskDISABLE_INTERRUPTS()
#define QF_INT_ENABLE()      taskENABLE_INTERRUPTS()

/* QF critical section for FreeRTOS-ESP32 (task level), see NOTE2 */
/* #define QF_CRIT_STAT_TYPE not defined */
#define QF_CRIT_ENTRY(dummy)  portENTER_CRITICAL(&QF_esp32mux)
#define QF_CRIT_EXIT(dummy)   portEXIT_CRITICAL(&QF_esp32mux)

// include files -------------------------------------------------------------
#include "freertos/FreeRTOS.h"  // FreeRTOS master include file, see NOTE3
#include "freertos/task.h"      // FreeRTOS task management
#include "freertos/queue.h"     // FreeRTOS queue management

#include "qequeue.hpp" // QP event queue (for deferring events)
#include "qmpool.hpp"  // QP memory pool (for event pools)
#include "qp.hpp"      // QP platform-independent public interface

/* global spinlock "mutex" for all critical sections in QF (see NOTE3) */
extern PRIVILEGED_DATA portMUX_TYPE QF_esp32mux;

#if defined( CONFIG_QP_PINNED_TO_CORE_0 )
    #define QP_CPU_NUM         PRO_CPU_NUM
#elif defined( CONFIG_QP_PINNED_TO_CORE_1 )
    #define QP_CPU_NUM         APP_CPU_NUM
#else
    /* Defaults to APP_CPU */
    #define QP_CPU_NUM         APP_CPU_NUM
#endif

/* the "FromISR" versions of the QF APIs, see NOTE4 */
#ifdef Q_SPY
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        publishFromISR((e_), (pxHigherPrioTaskWoken_), (sender_))

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR((e_), QP::QF::NO_MARGIN, \
                     (pxHigherPrioTaskWoken_), (sender_))

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, sender_) \
        postFromISR((e_), (margin_), (pxHigherPrioTaskWoken_), (sender_))

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, sender_) \
        tickFromISR((tickRate_), (pxHigherPrioTaskWoken_), (sender_))
#else
    #define PUBLISH_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        publishFromISR((e_), (pxHigherPrioTaskWoken_), nullptr)

    #define POST_FROM_ISR(e_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR((e_), QP::QF::NO_MARGIN, (pxHigherPrioTaskWoken_), \
                     nullptr)

    #define POST_X_FROM_ISR(e_, margin_, pxHigherPrioTaskWoken_, dummy) \
        postFromISR((e_), (margin_), (pxHigherPrioTaskWoken_), nullptr)

    #define TICK_X_FROM_ISR(tickRate_, pxHigherPrioTaskWoken_, dummy) \
        tickFromISR((tickRate_), (pxHigherPrioTaskWoken_), nullptr)
#endif

#define TICK_FROM_ISR(pxHigherPrioTaskWoken_, sender_) \
    TICK_X_FROM_ISR(0U, (pxHigherPrioTaskWoken_), (sender_))

#ifdef QEVT_DYN_CTOR
    #define Q_NEW_FROM_ISR(evtT_, sig_, ...) \
        (new(QP::QF::newXfromISR_(sizeof(evtT_), QP::QF::NO_MARGIN, (sig_))) \
            evtT_(__VA_ARGS__))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_, ...) do {        \
        (e_) = static_cast<evtT_ *>(                                    \
                  QP::QF::newXfromISR_(static_cast<std::uint_fast16_t>( \
                      sizeof(evtT_)), (margin_), (sig_)));              \
        if ((e_) != nullptr) {                                          \
            new((e_)) evtT_(__VA_ARGS__);                               \
        }                                                               \
     } while (false)

#else // QEvt is a POD (Plain Old Datatype)
    #define Q_NEW_FROM_ISR(evtT_, sig_)                         \
        (static_cast<evtT_ *>(QP::QF::newXfromISR_(             \
                static_cast<std::uint_fast16_t>(sizeof(evtT_)), \
                QP::QF::NO_MARGIN, (sig_))))

    #define Q_NEW_X_FROM_ISR(e_, evtT_, margin_, sig_)          \
        ((e_) = static_cast<evtT_ *>(                           \
        QP::QF::newXfromISR_(sizeof(evtT_), (margin_), (sig_))))
#endif

namespace QP {

enum FreeRTOS_TaskAttrs {
    TASK_NAME_ATTR
};

} // namespace QP

// FreeRTOS hooks prototypes (not provided by FreeRTOS)
extern "C" {
#if (configUSE_IDLE_HOOK > 0)
    void vApplicationIdleHook(void);
#endif
#if (configUSE_TICK_HOOK > 0)
    void vApplicationTickHook(void);
#endif
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
#endif
#if (configSUPPORT_STATIC_ALLOCATION > 0)
    void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                       StackType_t **ppxIdleTaskStackBuffer,
                                       uint32_t *pulIdleTaskStackSize);
#endif
} // extern "C"

//============================================================================
// interface used only inside QF, but not in applications

#ifdef QP_IMPL
    #define FREERTOS_TASK_PRIO(qp_prio_) \
        ((UBaseType_t)((qp_prio_) + tskIDLE_PRIORITY))

    // FreeRTOS scheduler locking for QF_publish_() (task context only)
    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(prio_) (vTaskSuspendAll())
    #define QF_SCHED_UNLOCK_()    ((void)xTaskResumeAll())

    // native QF event pool customization
    #define QF_EPOOL_TYPE_        QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_)  ((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_, qsId_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_), (qsId_))))
    #define QF_EPOOL_PUT_(p_, e_, qsId_) ((p_).put((e_), (qsId_)))

#endif // def QP_IMPL

//============================================================================
// NOTE0:
// This is the "experimental" port to the [Espressif ESP-IDF][1]
// IoT Framework, which is loosely based on the [FreeRTOS kernel][2].
//
// "Experimental" means that the port has not been thouroughly tested at
// Quantum Leaps and no working examples are provided.
//
// The [Espressif ESP-IDF][1] is based on a significantly changed version
// of the FreeRTOS kernel developed by Espressif to support the ESP32 multi-core
// CPUs (see [ESP-IDF][1]).
//
// The Espressif version of FreeRTOS is __NOT__ compatible with the baseline
// FreeRTOS and it needs to be treated as a separate RTOS kernel.
// According to the comments in the Espressif source code, FreeRTOS-ESP-IDF
// is based on FreeRTOS V8.2.0, but apparently FreeRTOS-ESP-IDF has been
// updated with the newer features introduced to the original FreeRTOS in the
// later versions. For example, FreeRTOS-ESP32 supports the "static allocation",
// first introduced in baseline FreeRTOS V9.x. 
//
// [1]: https://www.espressif.com/en/products/sdks/esp-idf
// [2]: https://freertos.org
//
// NOTE1:
// The maximum number of active objects QF_MAX_ACTIVE can be increased to 64,
// inclusive, but it can be reduced to save some memory. Also, the number of
// active objects cannot exceed the number of FreeRTOS task priorities,
// because each QP active object requires a unique priority level.
//
// NOTE2:
// The critical section definition applies only to the FreeRTOS "task level"
// APIs. The "FromISR" APIs are defined separately.
//
// NOTE3:
// This QF port to FreeRTOS-ESP32 uses the FreeRTOS-ESP32 spin lock "mutex",
// similar to the internal implementation of FreeRTOS-ESP32 (see tasks.c).
// However, the QF port uses its own "mutex" object QF_esp32mux.
// 
// NOTE4:
// This QP/C++ port used as reference the esp-idf QP/C port counterpart. The main 
// difference implementation-wise is that instead of using xTaskCreateStaticPinnedToCore,
// it uses xTaskCreatePinnedToCore. The reason is that this port was designed to work
// with the Arduino SDK, which does not include xTaskCreateStaticPinnedToCore.
//
// NOTE5:
// The design of FreeRTOS requires using different APIs inside the ISRs
// (the "FromISR" variant) than at the task level. Accordingly, this port
// provides the "FromISR" variants for QP functions and "FROM_ISR" variants
// for QP macros to be used inside ISRs. ONLY THESE "FROM_ISR" VARIANTS
// ARE ALLOWED INSIDE ISRs AND CALLING THE TASK-LEVEL APIs IS AN ERROR.

#endif // QP_PORT_HPP_
