/*
 *     SocialLedge.com -  Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * This contains the period callback functions for the periodic scheduler
 *
 * @warning
 * These callbacks should be used for hard real-time system, and the priority of these
 * tasks are above everything else in the system (above the PRIORITY_CRITICAL).
 * The period functions SHOULD NEVER block and SHOULD NEVER run over their time slot.
 * For example, the 1000Hz take slot runs periodically every 1ms, and whatever you
 * do must be completed within 1ms.  Running over the time slot will reset the system.
 */

#include <stdio.h>
#include <stdint.h>
#include "io.hpp"
#include "periodic_callback.h"
#include "gpio.hpp"
#include "eint.h"
#include "semphr.h"
#include "utilities.h"

/// This is the stack size used for each of the period tasks (1Hz, 10Hz, 100Hz, and 1000Hz)
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);

/**
 * This is the stack size of the dispatcher task that triggers the period tasks to run.
 * Minimum 1500 bytes are needed in order to write a debug file if the period tasks overrun.
 * This stack size is also used while calling the period_init() and period_reg_tlm(), and if you use
 * printf inside these functions, you need about 1500 bytes minimum
 */
const uint32_t PERIOD_MONITOR_TASK_STACK_SIZE_BYTES = (512 * 3);

SemaphoreHandle_t sem_handler = 0;
const int32_t bounce_count = 1;
int32_t cnt = 1;
GPIO *p2_0_ptr;
bool flag = false;
bool flag_500ms = false;
uint32_t local_count;

void onP2_1_event()
{
#if 0
    //Code to avoid bounce
    if (cnt > bounce_count)
    {
        return;
    }
    cnt += 1;
#else
    if (flag) {
        return;
    }
    flag = true;
#endif
    xSemaphoreGive(sem_handler);
}

/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
    vSemaphoreCreateBinary(sem_handler);
    p2_0_ptr = new GPIO(P2_0);
    p2_0_ptr->setAsOutput();
    flag_500ms = false;
    eint3_enable_port2(1, eint_falling_edge, onP2_1_event);
    return true; // Must return true upon success
}

/// Register any telemetry variables
bool period_reg_tlm(void)
{
    // Make sure "SYS_CFG_ENABLE_TLM" is enabled at sys_config.h to use Telemetry
    return true; // Must return true upon success
}

/**
 * Below are your periodic functions.
 * The argument 'count' is the number of times each periodic task is called.
 */

// Will be called every 1000ms
void period_1Hz(uint32_t count)
{
    LE.toggle(1);
}

// Will be called every 100ms
void period_10Hz(uint32_t count)
{
#if 1
    if (xSemaphoreTake(sem_handler, 0))
    {
        local_count = count + 5;
        flag_500ms = true;
        flag = false;
    }

    if (flag_500ms)
    {
        if (count < local_count) {
            p2_0_ptr->setHigh();
        } else {
            flag_500ms = false;
            p2_0_ptr->setLow();
        }
    }
    else {
        p2_0_ptr->setLow();
    }
#endif
#if 0
    if (p2_1_event)
    {
        p2_1_event = false;
        cnt = 1;
        flag = false;
        p2_0_ptr->setHigh();
    }
    else
    {
        p2_0_ptr->setLow();
    }
//    LE.toggle(2);
#endif
}

// Will be called every 10ms
void period_100Hz(uint32_t count)
{
    LE.toggle(3);
}

// 1Khz (1ms) is only run if Periodic Dispatcher was configured to run it at main():
// scheduler_add_task(new periodicSchedulerTask(run_1Khz = true));
void period_1000Hz(uint32_t count)
{
    LE.toggle(4);
}

