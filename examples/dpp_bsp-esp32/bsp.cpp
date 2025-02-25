//$file${.::bsp.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp_bsp-esp32.qm
// File:  ${.::bsp.cpp}
//
// This code has been generated by QM 7.0.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// Copyright (c) 2005 Quantum Leaps, LLC. All rights reserved.
//
//                 ____________________________________
//                /                                   /
//               /    GGGGGGG    PPPPPPPP   LL       /
//              /   GG     GG   PP     PP  LL       /
//             /   GG          PP     PP  LL       /
//            /   GG   GGGGG  PPPPPPPP   LL       /
//           /   GG      GG  PP         LL       /
//          /     GGGGGGG   PP         LLLLLLL  /
//         /___________________________________/
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open-source software licensed under the GNU
// General Public License (GPL) as published by the Free Software Foundation
// (see <https://www.gnu.org/licenses>).
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
//$endhead${.::bsp.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"   // QP-C++ framework
#include "dpp.hpp"     // DPP application
#include "bsp.hpp"     // Board Support Package
#include <Arduino.h>
#include "esp_freertos_hooks.h"

#ifndef LED_BUILTIN  //If current ESP32 board does not define LED_BUILTIN
static constexpr unsigned LED_BUILTIN=13U;
#endif

using namespace QP;

//............................................................................
// QS facilities

#ifdef Q_SPY
enum AppRecords { // application-specific QS trace records
    PHILO_STAT = QP::QS_USER,
};

static QP::QSpyId const l_TIMER_ID = { 0U }; // QSpy source ID
#endif // Q_SPY

//----------------------------------------------------------------------------
// BSP functions

static void tickHook_ESP32(void); /*Tick hook for QP */

static void tickHook_ESP32(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* process time events for rate 0 */
    QTimeEvt::TICK_FROM_ISR(&xHigherPriorityTaskWoken, &l_TIMER_ID);
    /* notify FreeRTOS to perform context switch from ISR, if needed */
    if(xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
#ifndef Q_SPY
    if (Serial.available() > 0) {
        switch (Serial.read()) { // read the incoming byte
            case 'p':
            case 'P':
                static QEvt const pauseEvt(PAUSE_SIG);
                QF::PUBLISH(&pauseEvt, &l_TIMER_ID);
                break;
            case 's':
            case 'S':
                static QEvt const serveEvt(SERVE_SIG);
                QF::PUBLISH(&serveEvt, &l_TIMER_ID);
                break;
        }
    }
#endif
}

void BSP::init(void) {
    // initialize the hardware used in this sketch...
    // NOTE: interrupts are configured and started later in QF::onStartup()
    pinMode(LED_BUILTIN, OUTPUT);
    randomSeed(1234); // seed the Random Number Generator
    Serial.begin(115200); // set the highest stanard baud rate of 115200 bps
    QS_INIT(nullptr);
#ifdef Q_SPY
    // output QS dictionaries
    QS_OBJ_DICTIONARY(&l_TIMER_ID);
    QS_USR_DICTIONARY(PHILO_STAT);

    // setup the QS filters...
    QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
    QS_GLB_FILTER(QP::QS_UA_RECORDS); // all user records
#else
    Serial.print("QP-C++: ");
    Serial.print(QP_VERSION_STR);
    Serial.println("");
#endif
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char_t const *stat) {
#ifdef Q_SPY
    QS_BEGIN_ID(PHILO_STAT, AO_Philo[n]->m_prio) // app-specific record begin
        QS_U8(1, n);  // Philo number
        QS_STR(stat); // Philo status
    QS_END()
#else
    Serial.print("Philosopher ");
    Serial.write(48+n);
    Serial.print(" ");
    Serial.println(stat);
#endif
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    char const *msg = paused ? "Paused ON" : "Paused OFF";
#ifndef Q_SPY
    Serial.println(msg);
#endif
}
//............................................................................
void BSP::ledOff(void) {
    digitalWrite(LED_BUILTIN, LOW);
}
//............................................................................
void BSP::ledOn(void) {
    digitalWrite(LED_BUILTIN, HIGH);
}

//............................................................................
static uint32_t l_rnd; // random seed

void BSP::randomSeed(uint32_t seed) {
    l_rnd = seed;
}
//............................................................................
uint32_t BSP::random(void) { // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    return (rnd >> 8);
}

//............................................................................
#ifdef Q_SPY
void QSpy_Task(void *) {
  while(1)
  {
    // transmit QS outgoing data (QS-TX)
    uint16_t len = Serial.availableForWrite();
    if (len > 0U) { // any space available in the output buffer?
        uint8_t const *buf = QS::getBlock(&len);
        if (buf) {
            Serial.write(buf, len); // asynchronous and non-blocking
        }
    }

    // receive QS incoming data (QS-RX)
    len = Serial.available();
    if (len > 0U) {
        do {
            QP::QS::rxPut(Serial.read());
        } while (--len > 0U);
        QS::rxParse();
    }
    delay(100);
  };
}
#endif

void QF::onStartup(void) {
    esp_register_freertos_tick_hook_for_cpu(tickHook_ESP32, QP_CPU_NUM);
#ifdef Q_SPY
    xTaskCreatePinnedToCore(
                    QSpy_Task,   /* Function to implement the task */
                    "QSPY", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    configMAX_PRIORITIES-1,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    QP_CPU_NUM);  /* Core where the task should run */
#endif
}
//............................................................................

//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int location) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)location;
    Serial.print("QP Assert module: ");
    Serial.print(module);
    Serial.print("@ ");
    Serial.println(location);
    QF_INT_DISABLE(); // disable all interrupts
    for (;;) { // sit in an endless loop for now
    }
}

//----------------------------------------------------------------------------
// QS callbacks...
//............................................................................
#ifdef Q_SPY
namespace QS {
bool onStartup(void const * arg) {
    static uint8_t qsTxBuf[1024]; // buffer for QS transmit channel (QS-TX)
    static uint8_t qsRxBuf[128];  // buffer for QS receive channel (QS-RX)
    initBuf  (qsTxBuf, sizeof(qsTxBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
    return true; // return success
}
//............................................................................
void onCommand(uint8_t cmdId, uint32_t param1,
                       uint32_t param2, uint32_t param3)
{
}
//............................................................................
void onCleanup(void) {
}
//............................................................................
QP::QSTimeCtr onGetTime(void) {
#ifdef Q_SPY
    return millis();
#else
    return 0;
#endif

}
//............................................................................
void onFlush(void) {
#ifdef Q_SPY
    uint16_t len = 0xFFFFU; // big number to get as many bytes as available
    uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
    while (buf != nullptr) { // data available?
        Serial.write(buf, len); // might poll until all bytes fit
        len = 0xFFFFU; // big number to get as many bytes as available
        buf = QS::getBlock(&len); // try to get more data
    }
    Serial.flush(); // wait for the transmission of outgoing data to complete
#endif // Q_SPY
}
//............................................................................
void onReset(void) {
    esp_restart();
}
} // namespace QS
#endif // Q_SPY



