#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	adcHardwareConfiguration,
	adcLightLevel,
	adcBatteryLevel,
} Adc_Request_Type;

// Callbacks are called from the ISR, so be careful! Return non-zero to wake up OS

typedef int (*Adc_Callback)(Adc_Request_Type request, void *context, unsigned int value);

void Adc_Initialise(void);
void Adc_Request(Adc_Request_Type request, Adc_Callback callback, void *context);

unsigned int Adc_Battery_Average(void);
int Adc_Battery_Average_Available(void);

unsigned int Light_Level_Average(void);
int Light_Level_Average_Available(void);

#ifdef __cplusplus
}
#endif

#endif // __ADC_H__
