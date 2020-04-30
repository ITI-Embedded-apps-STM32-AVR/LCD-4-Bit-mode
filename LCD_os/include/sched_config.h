/*
 *
 *  Created on: Apr 2, 2020
 *      Author: SARA_MINA
 */

#ifndef SCHED_CONFIG_H_
#define SCHED_CONFIG_H_


#define  SCHED_MAX_TASKS   2

#define  SCHED_TICK_TIME   2

#define  SCHED_AHB_CLK     8000000


typedef struct
{
	SCHED_task_t const * apptask;
	u32                  delayMs;
} SCHED_systask_info_t;

#endif /* SCHED_CONFIG_H_ */
