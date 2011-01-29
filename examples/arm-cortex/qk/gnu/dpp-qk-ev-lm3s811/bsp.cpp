//////////////////////////////////////////////////////////////////////////////
// Product: "Dining Philosophers Problem" example, QK kernel
// Last Updated for Version: 4.1.04
// Date of the Last Update:  Mar 16, 2010
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2010 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

extern "C" {
    #include "lm3s_cmsis.h"
    #include "display96x16x1.h"
}

Q_DEFINE_THIS_FILE

enum ISR_Priorities {      // ISR priorities starting from the highest urgency
    GPIOPORTA_PRIO,
    SYSTICK_PRIO,
    // ...
};

#define UART_FR_TXFE            0x00000080
#define SYSTICK_COUNTFLAG       0x00000100

// Local-scope objects -------------------------------------------------------
static uint32_t l_delay = 0UL;    // limit for the loop counter in busyDelay()

#define PUSH_BUTTON             (1 << 4)
#define USER_LED                (1 << 5)

#ifdef Q_SPY

    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;

    #define UART_BAUD_RATE      115200
    #define UART_TXFIFO_DEPTH   16

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };

#endif

//............................................................................
extern "C" void SysTick_Handler(void) __attribute__((__interrupt__));
extern "C" void SysTick_Handler(void) {
    QK_ISR_ENTRY();                               // inform QK about ISR entry

#ifdef Q_SPY
    uint32_t dummy = SysTick->CTRL;          // clear the COUNTFLAG in SysTick
    QS_tickTime_ += QS_tickPeriod_;       // account for the clock rollover */
#endif

    QF::tick();                               // process all armed time events

    QK_ISR_EXIT();                                 // inform QK about ISR exit
}
//............................................................................
extern "C" void GPIOPortA_IRQHandler(void) __attribute__((__interrupt__));
extern "C" void GPIOPortA_IRQHandler(void) {
    QK_ISR_ENTRY();                         // infrom QK about entering an ISR

    AO_Table->postFIFO(Q_NEW(QEvent, MAX_PUB_SIG));             // for testing

    QK_ISR_EXIT();                           // infrom QK about exiting an ISR
}

//............................................................................
void BSP_init(void) {
    // set the system clock as specified in lm3s_config.h (20MHz from PLL)
    SystemInit();

    // enable clock to the peripherals used by the application
    SYSCTL->RCGC2 |= (1 <<  0) | (1 <<  2);      // enable clock to GPIOA & C
    __NOP();                                     // wait after enabling clocks
    __NOP();
    __NOP();

    // configure the LED and push button
    GPIOC->DIR |= USER_LED;                           // set direction: output
    GPIOC->DEN |= USER_LED;                                  // digital enable
    GPIOC->DATA_Bits[USER_LED] = 0;                   // turn the User LED off

    GPIOC->DIR &= ~PUSH_BUTTON;                       //  set direction: input
    GPIOC->DEN |= PUSH_BUTTON;                               // digital enable

    Display96x16x1Init(1);                      // initialize the OLED display
    Display96x16x1StringDraw("Dining Philos", 0, 0);
    Display96x16x1StringDraw("0 ,1 ,2 ,3 ,4", 0, 1);

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    char str[2];
    str[0] = stat[0];
    str[1] = '\0';
    Display96x16x1StringDraw(str, (3*6*n + 6), 1);

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void BSP_driveLED(uint8_t state) {
    if (state != 0) {
        GPIOC->DATA_Bits[USER_LED] = USER_LED;        // turn the User LED on
    }
    else {
        GPIOC->DATA_Bits[USER_LED] = 0;               // turn the User LED off
    }
}
//............................................................................
void BSP_busyDelay(void) {
    uint32_t volatile i = l_delay;
    while (i-- > 0UL) {                                      // busy-wait loop
    }
}

//............................................................................
void QF::onStartup(void) {
                 // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    SysTick_Config(SystemFrequency / BSP_TICKS_PER_SEC);

                          // set priorities of all interrupts in the system...
    NVIC_SetPriority(SysTick_IRQn,   SYSTICK_PRIO);
    NVIC_SetPriority(GPIOPortA_IRQn, GPIOPORTA_PRIO);

    NVIC_EnableIRQ(GPIOPortA_IRQn);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {

    // toggle the User LED on and then off, see NOTE01
    QF_INT_LOCK(dummy);
    GPIOC->DATA_Bits[USER_LED] = USER_LED;            // turn the User LED on
    GPIOC->DATA_Bits[USER_LED] = 0;                   // turn the User LED off
    QF_INT_UNLOCK(dummy);

#ifdef Q_SPY
    if ((UART0->FR & UART_FR_TXFE) != 0) {                         // TX done?
        uint16_t fifo = UART_TXFIFO_DEPTH;          // max bytes we can accept
        uint8_t const *block;
        QF_INT_LOCK(dummy);
        block = QS::getBlock(&fifo);      // try to get next block to transmit
        QF_INT_UNLOCK(dummy);
        while (fifo-- != 0) {                       // any bytes in the block?
            UART0->DR = *block++;                         // put into the FIFO
        }
    }
#elif defined NDEBUG
    // put the CPU and peripherals to the low-power mode
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M3 MCU.
    __WFI();                                             // Wait-For-Interrupt
#endif
}

//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                      // avoid compiler warning
    (void)line;                                      // avoid compiler warning
    QF_INT_LOCK(dummy);         // make sure that all interrupts are disabled
    for (;;) {          // NOTE: replace the loop with reset for final version
    }
}
//............................................................................
// error routine that is called if the CMSIS library encounters an error
extern "C" void assert_failed(char const *file, int line) {
    Q_onAssert(file, line);
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
uint8_t QS::onStartup(void const *arg) {
    static uint8_t qsBuf[6*256];                     // buffer for Quantum Spy
    uint32_t tmp;
    initBuf(qsBuf, sizeof(qsBuf));

                                   // enable the peripherals used by the UART0
    SYSCTL->RCGC1 |= (1 << 0);                        // enable clock to UART0
    SYSCTL->RCGC2 |= (1 << 0);                        // enable clock to GPIOA
    __NOP();                                     // wait after enabling clocks
    __NOP();
    __NOP();

                                    // configure UART0 pins for UART operation
    tmp = (1 << 0) | (1 << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->AFSEL |= tmp;
    GPIOA->DR2R  |= tmp;           // set 2mA drive, DR4R and DR8R are cleared
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->DEN   |= tmp;

              // configure the UART for the desired baud rate, 8-N-1 operation
    tmp = (((SystemFrequency * 8) / UART_BAUD_RATE) + 1) / 2;
    UART0->IBRD   = tmp / 64;
    UART0->FBRD   = tmp % 64;
    UART0->LCRH   = 0x60;                         // configure 8-N-1 operation
    UART0->LCRH  |= 0x10;
    UART0->CTL   |= (1 << 0) | (1 << 8) | (1 << 9);

    QS_tickPeriod_ = SystemFrequency / BSP_TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_;           // to start the timestamp at zero

                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_IGNORED);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_INT_LOCK);
    QS_FILTER_OFF(QS_QF_INT_UNLOCK);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;                                       // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    if ((SysTick->CTRL & SYSTICK_COUNTFLAG) == 0) {       // COUNTFLAG no set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else {        // the rollover occured, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t fifo = UART_TXFIFO_DEPTH;                        // Tx FIFO depth
    uint8_t const *block;
    QF_INT_LOCK(dummy);
    while ((block = QS::getBlock(&fifo)) != (uint8_t *)0) {
        QF_INT_UNLOCK(dummy);
                                              // busy-wait until TX FIFO empty
        while ((UART0->FR & UART_FR_TXFE) == 0) {
        }

        while (fifo-- != 0) {                       // any bytes in the block?
            UART0->DR = *block++;                      // put into the TX FIFO
        }
        fifo = UART_TXFIFO_DEPTH;                 // re-load the Tx FIFO depth
        QF_INT_LOCK(dummy);
    }
    QF_INT_UNLOCK(dummy);
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
