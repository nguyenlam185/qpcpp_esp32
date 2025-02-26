#include "qpcpp.hpp"   // QP-C++ framework
#include "dpp.hpp"     // DPP application
#include "bsp.hpp"     // Board Support Package
#include <Arduino.h>
#include "esp_freertos_hooks.h"

#ifndef LED_BUILTIN  //If current ESP32 board does not define LED_BUILTIN
static constexpr unsigned LED_BUILTIN=2U;
#endif

using namespace QP;
//----------------------------------------------------------------------------
// BSP functions
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
void QF::onStartup(void) {
    esp_register_freertos_tick_hook_for_cpu(tickHook_ESP32, QP_CPU_NUM);
}
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