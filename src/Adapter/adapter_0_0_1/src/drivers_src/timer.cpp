/*
 * timer.cpp
 *
 *  Created on: 2016年12月22日
 *      Author: Topon-Edison
 *
 *
 *
 */
#include "chip.h"
#include "timer.h"



/********************************************************************
 * function of the Timer
 ********************************************************************/

/**
 *   Constructor, according to the timer_t to assign the member timer_
 *  's value, and set the timer to one shot mode
 */
Timer::Timer(Timer_T timer_t):timer_tick_ms_(Chip_Clock_GetMainClockRate() / 1000),
		timer_tick_us_(timer_tick_ms_ / 1000),timer_type_(timer_t)
{

	switch(timer_t)
	{
	case Timer0:
		/* Set the timer point to the mrt channel one */
		timer_ = LPC_MRT_CH0;
		break;
	case Timer1:
		timer_ = LPC_MRT_CH1;
		break;
	case Timer2:
		timer_ = LPC_MRT_CH2;
		break;
	default : // Don't exist the type of timer;
		timer_ = 0;
		break;
	}

	/* If the timer_ isn't null,  set the timer work in one-shot mode and
	 * and disable the channel's interrupt */
	if(timer_)
	{
		Chip_MRT_SetMode(timer_,MRT_MODE_ONESHOT);
		Chip_MRT_SetDisabled(timer_);
	}
}


/* Start the timer for interval millisecond
 * the maximum interval is 233ms */
void Timer::Start_Millisecond(uint32_t interval)
{

	/* Calculate the counter value*/
	uint32_t val = timer_tick_ms_ * interval;
	/* Clear the timer INTFLAG in STAT register */
	Chip_MRT_IntClear(timer_);
	/* Set the IVALUE in INVAL register and load the value to timer immidiately */
	Chip_MRT_SetInterval_Immediately(timer_,val);

}



/* The timer STAT register's RUN bit is 1 for running state , 0 for idle state */
bool Timer::IsExpired()
{
	return (bool)(!Chip_MRT_Running(timer_));
}



/* The instance function of  the Timer singleton */
Timer * Timer::Instance(Timer_T timer_t)
{
	static Timer timer0(Timer0); // the Timer use MRT  channel 0
    static Timer timer1(Timer1); // the Timer use MRT  channel 1
    static Timer timer2(Timer2); // the Timer use MRT  channel 2
    switch (timer_t)
    {
    case Timer0:
    	return &timer0;
    case Timer1:
    	return &timer1;
    case Timer2:
    	return &timer2;
    default:
    	return 0;
    }

}



/********************************************************************
 * function of the LongTimer
 ********************************************************************/

/* The RIT interrupt callback funtion */
extern "C" void RIT_IRQHandler(void)
{
	/* Clear the RITEN bit, and set the RITENBR bit, set the RITENCLR bit for clearing
	 * the timer to 0 when counter reach the condition, set 1 for RITINT to clear the
	 * interrupt flag
	 */
    LPC_RITIMER->CTRL &= 0x07;
    LongTimer::Instance()->SetExpired();
}


/* Constructor */
LongTimer::LongTimer(): is_expired_(true),timer_(LPC_RITIMER),timer_tick_ms_(Chip_Clock_GetMainClockRate() / 1000)
{
	/**
	 *  Clear the RITEN bit, and set the RITENBR bit, set the RITENCLR bit for clearing
	 * the timer to 0 when counter reach the condition, set 1 for RITINT to clear the
	 * interrupt flag
	 */
    timer_->CTRL &= 0x07;

    /**
     * Set the all the mask bit to 0, so have no bit in compare register
     * was masked
     */
    timer_->MASK = 0x00000000UL;
    timer_->MASK_H = 0x00000000UL;

    /**
     * Enable the RIT interrupt handler
     */
    NVIC_EnableIRQ(RITIMER_IRQn);
}



/* Start the LongTimer */
void LongTimer::Start_Millisecond(uint64_t interval)
{
	interval = timer_tick_ms_* interval;

	// disable the counting and clear the interrupt flage
	timer_->CTRL = 0x07;
	//reload the counter to zero for a new counting circle
	timer_->COUNTER = timer_->COUNTER_H = 0;

	// set the compare value
	timer_->COMPVAL = interval & 0xFFFFFFFF;
	timer_->COMPVAL_H = interval >> 32;

	// starting counting
	timer_->CTRL = 0x0F;

	is_expired_ =  false;

}


void LongTimer::SetExpired()
{
	is_expired_ = true;
}


/* Is end of the counting */
bool LongTimer::IsExpired()
{
	return is_expired_;
}


/* The factory to implement the  singleton */
LongTimer * LongTimer::Instance()
{
	static LongTimer longtimer;
	return &longtimer;
}