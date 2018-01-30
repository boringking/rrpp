/*
 *	Timer; icp_timer.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_TIMER_H_
#define _ICP_TIMER_H_

#include "list.h"

/*******************************************************************************
*	Structures
*******************************************************************************/
typedef struct _icp_timer
{
	struct list_head *p_timer_list;
	struct list_head timer_entry;
	unsigned long expires;
	
	void (*function)(unsigned long);
	unsigned long data;
}icp_timer;

/*******************************************************************************
*	Function Prototype
*******************************************************************************/
void setup_timer(icp_timer *timer, void(*function)(unsigned long), unsigned long data, struct list_head *p_timer_list);
int mod_timer(icp_timer *timer, unsigned long expires);
int del_timer(icp_timer *timer);
unsigned long icp_timer_value(icp_timer *timer);

#endif
