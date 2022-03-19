//.$file${.::philo.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp_bsp-sam.qm
// File:  ${.::philo.cpp}
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
//.$endhead${.::philo.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"   // QP-C++ framework
#include "dpp.hpp"     // DPP application
#include "bsp.hpp"     // Board Support Package

Q_DEFINE_THIS_FILE

// generate declaration of the active object ---------------------------------
//.$declare${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::Philo} .............................................................
class Philo : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    static Philo inst[N_PHILO];

public:
    Philo();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(thinking);
    Q_STATE_DECL(hungry);
    Q_STATE_DECL(eating);
};
//.$enddecl${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// generate definition of the opaque pointer to the AO -----------------------
//.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//. Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$define${AOs::AO_Philo[N_PHILO]} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::AO_Philo[N_PHILO]} .................................................
QP::QActive * const AO_Philo[N_PHILO] = { // "opaque" pointers to Philo AO
    &Philo::inst[0],
    &Philo::inst[1],
    &Philo::inst[2],
    &Philo::inst[3],
    &Philo::inst[4]
};
//.$enddef${AOs::AO_Philo[N_PHILO]} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// helper function to provide a randomized think time for Philos
inline QP::QTimeEvtCtr think_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + (BSP::TICKS_PER_SEC/2U));
}

// helper function to provide a randomized eat time for Philos
inline QP::QTimeEvtCtr eat_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + BSP::TICKS_PER_SEC);
}

// helper function to provide the ID of Philo
inline uint8_t PHILO_ID(Philo const * const philo) {
    return static_cast<uint8_t>(philo - &Philo::inst[0]);
}

// generate definition  of the AO --------------------------------------------
//.$define${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${AOs::Philo} .............................................................
Philo Philo::inst[N_PHILO];
//.${AOs::Philo::Philo} ......................................................
Philo::Philo()
  : QActive(Q_STATE_CAST(&Philo::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//.${AOs::Philo::SM} .........................................................
Q_STATE_DEF(Philo, initial) {
    //.${AOs::Philo::SM::initial}
    (void)e; // unused parameter

    subscribe(EAT_SIG);
    subscribe(TEST_SIG);

    // QS dictionaries only once for all Philos...
    static bool registered = false;
    if (!registered) { // first time through?
        registered = true;

        QS_OBJ_DICTIONARY(&Philo::inst[0]);
        QS_OBJ_DICTIONARY(&Philo::inst[0].m_timeEvt);
        QS_OBJ_DICTIONARY(&Philo::inst[1]);
        QS_OBJ_DICTIONARY(&Philo::inst[1].m_timeEvt);
        QS_OBJ_DICTIONARY(&Philo::inst[2]);
        QS_OBJ_DICTIONARY(&Philo::inst[2].m_timeEvt);
        QS_OBJ_DICTIONARY(&Philo::inst[3]);
        QS_OBJ_DICTIONARY(&Philo::inst[3].m_timeEvt);
        QS_OBJ_DICTIONARY(&Philo::inst[4]);
        QS_OBJ_DICTIONARY(&Philo::inst[4].m_timeEvt);

        QS_FUN_DICTIONARY(&Philo::initial);
        QS_FUN_DICTIONARY(&Philo::thinking);
        QS_FUN_DICTIONARY(&Philo::hungry);
        QS_FUN_DICTIONARY(&Philo::eating);

        QS_SIG_DICTIONARY(TIMEOUT_SIG, nullptr);
    }
    return tran(&thinking);
}
//.${AOs::Philo::SM::thinking} ...............................................
Q_STATE_DEF(Philo, thinking) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::thinking}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(think_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking}
        case Q_EXIT_SIG: {
            (void)m_timeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&hungry);
            break;
        }
        //.${AOs::Philo::SM::thinking::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking::TEST}
        case TEST_SIG: {
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${AOs::Philo::SM::hungry} .................................................
Q_STATE_DEF(Philo, hungry) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::hungry}
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoNum = PHILO_ID(this);
            AO_Table->POST(pe, this);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::hungry::EAT}
        case EAT_SIG: {
            //.${AOs::Philo::SM::hungry::EAT::[Q_EVT_CAST(TableEvt)->philoNum=~}
            if (Q_EVT_CAST(TableEvt)->philoNum == PHILO_ID(this)) {
                status_ = tran(&eating);
            }
            else {
                status_ = Q_RET_UNHANDLED;
            }
            break;
        }
        //.${AOs::Philo::SM::hungry::DONE}
        case DONE_SIG: {
            /* DONE must be for other Philos than this one */
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${AOs::Philo::SM::eating} .................................................
Q_STATE_DEF(Philo, eating) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::eating}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(eat_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::eating}
        case Q_EXIT_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoNum = PHILO_ID(this);
            QP::QF::PUBLISH(pe, this);
            (void)m_timeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::eating::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&thinking);
            break;
        }
        //.${AOs::Philo::SM::eating::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.$enddef${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
