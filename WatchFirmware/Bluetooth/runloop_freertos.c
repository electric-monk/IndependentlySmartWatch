#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>

#include "btstack/config.h"
#include "btstack/run_loop_private.h"
#include <btstack/linked_list.h>

#include "colindebug.h"

#define TASK_QUEUE_LEN			12
#define TASK_QUEUE_ITEM_SIZE	sizeof(timer_source_t*)

static xQueueHandle s_queue;
static linked_list_t s_data_sources;

static void TimerCallback(xTimerHandle timer)
{
	timer_source_t *source = pvTimerGetTimerID(timer);
	xQueueSend(s_queue, &source, portMAX_DELAY);
}

void embedded_trigger()
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	timer_source_t *source = NULL;
	xQueueSendFromISR(s_queue, &source, &xHigherPriorityTaskWoken);
	// If xHigherPriorityTaskWoken is true, we should wake up the OS - but
	// hal_lpm will set up a timer to do that when it's woken up by a timer anyway.
}

uint32_t embedded_get_ticks(void)
{
	return xTaskGetTickCountFromISR();
}

uint32_t embedded_ticks_for_ms(uint32_t time_in_ms)
{
	return portTICK_RATE_MS * time_in_ms;
}

void run_loop_set_timer(timer_source_t *ts, uint32_t timeout_in_ms)
{
    uint32_t ticks = embedded_ticks_for_ms(timeout_in_ms);
    if (ticks == 0)
    	ticks++;
    ts->timeout = ticks;
}

static void freertos_init(void)
{
	s_data_sources = NULL;
	s_queue = xQueueCreate(TASK_QUEUE_LEN, TASK_QUEUE_ITEM_SIZE);
}

static void freertos_add_data_source(data_source_t *ds)
{
    linked_list_add(&s_data_sources, (linked_item_t*)ds);
}

static int freertos_remove_data_source(data_source_t *ds)
{
    return linked_list_remove(&s_data_sources, (linked_item_t*)ds);
}

static void freertos_add_timer(timer_source_t *timer)
{
	xTimerHandle newTimer = xTimerCreate("runloop_timer", timer->timeout, 0, timer, TimerCallback);
	// Hack: Same as in btstack's Cocoa runloop
	timer->item.next = (struct linked_item*)newTimer;
	xTimerStart(newTimer, portMAX_DELAY);
}

static int freertos_remove_timer(timer_source_t *timer)
{
	if (timer->item.next == NULL)
		return 0;
	xTimerDelete((xTimerHandle)timer->item.next, 0);
	timer->item.next = NULL;
	return 0;
}

static void freertos_execute(void)
{
	timer_source_t *message;

	for (;;)
	{
		if (pdTRUE == xQueueReceive(s_queue, &message, portMAX_DELAY))
		{
			if (message == NULL)
			{
		        data_source_t *ds, *next;
//		        goodprintf("Hardware event fired?\n");
		        for (ds = (data_source_t*)s_data_sources; ds != NULL; ds = next)
		        {
		            next = (data_source_t*)ds->item.next; // cache pointer to next data_source to allow data source to remove itself
		            ds->process(ds);
		        }
			}
			else
			{
				goodprintf("Timer 0x%08x fired\n", message);
				message->process(message);
			}
		}
	}
}

static void freertos_dump_timer(void)
{
	// N/A
}

const run_loop_t run_loop_embedded = {
    &freertos_init,
    &freertos_add_data_source,
    &freertos_remove_data_source,
    &freertos_add_timer,
    &freertos_remove_timer,
    &freertos_execute,
    &freertos_dump_timer
};
