#include "qpcpp.hpp"   // QP-C++ framework
#include "blinky.hpp"  // Blinky application interface
#include "bsp.hpp"     // Board Support Package (BSP)

using namespace QP;

//............................................................................
void setup() {
    QF::init(); // initialize the framework
    BSP::init(); // initialize the BSP

    // statically allocate event queues for the AOs and start them...
    static QEvt const *blinky_queueSto[10];
    AO_Blinky->start(1U, // priority
                     blinky_queueSto, Q_DIM(blinky_queueSto),
                     (void *)0, 0U); // no stack
    //...
}

//............................................................................
void loop() {
    QF::run(); // run the QF/C++ framework
}

//============================================================================
// generate declarations and definitions of all AO classes (state machines)...
//.$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::Blinky} ............................................................
class Blinky : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    static Blinky instance;

public:
    Blinky();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
};
//.$enddecl${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//. Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$define${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::Blinky} ............................................................
Blinky Blinky::instance;
//.${AOs::Blinky::Blinky} ....................................................
Blinky::Blinky()
: QActive(Q_STATE_CAST(&Blinky::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//.${AOs::Blinky::SM} ........................................................
Q_STATE_DEF(Blinky, initial) {
    //.${AOs::Blinky::SM::initial}
    (void)e; // unused parameter
    m_timeEvt.armX(BSP::TICKS_PER_SEC/2, BSP::TICKS_PER_SEC/2);

    QS_OBJ_DICTIONARY(&Blinky::instance);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, nullptr);

    QS_FUN_DICTIONARY(&Blinky::off);
    QS_FUN_DICTIONARY(&Blinky::on);

    return tran(&off);
}
//.${AOs::Blinky::SM::off} ...................................................
Q_STATE_DEF(Blinky, off) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Blinky::SM::off}
        case Q_ENTRY_SIG: {
            BSP::ledOff();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Blinky::SM::off::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&on);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${AOs::Blinky::SM::on} ....................................................
Q_STATE_DEF(Blinky, on) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Blinky::SM::on}
        case Q_ENTRY_SIG: {
            BSP::ledOn();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Blinky::SM::on::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&off);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.$enddef${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//...

//============================================================================
// generate definitions of all AO opaque pointers...
//.$define${AOs::AO_Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::AO_Blinky} .........................................................
QP::QActive * const AO_Blinky = &Blinky::instance;
//.$enddef${AOs::AO_Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//...
