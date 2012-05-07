#ifdef __cplusplus
extern "C" {
#endif

// Callbacks are called from the ISR, so be careful! Return non-zero to wake up OS

typedef void* TimerHandle;
typedef int (*TimerCallback)(TimerHandle handle, void *context);

TimerHandle Timer_Create(TimerCallback callback, void *context, int delay, int repeats);
void Timer_Reset(TimerHandle *timer, int delay, int repeats);
void Timer_Change(TimerHandle *timer, TimerCallback callback, void *context);
void Timer_Destroy(TimerHandle *timer);
TimerCallback Timer_GetCallback(TimerHandle *timer);
void* Timer_GetContext(TimerHandle *timer);

// HAL interface
void Timer_Initialise(void);
int Timer_Interrupt(int prescale);

#ifdef __cplusplus
}
#endif
