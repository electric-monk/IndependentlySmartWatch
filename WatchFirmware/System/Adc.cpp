#include <string.h>
#include "hal_board_type.h"
#include "hal_lpm.h"
#include "hal_calibration.h"
#include "macro.h"
#include "Adc.h"
#include "Timer.h"
#include "colindebug.h"

#include "FixedMaths.h"

// As most stuff happens in an ISR, it's easier to use a static list than a linked list
// (but certainly not impossible, if it's necessary in future)
#define MAX_REQUESTS			10
#define MAX_SAMPLES				10

#define ENABLE_REFERENCE()
#define DISABLE_REFERENCE()

const double CONVERSION_FACTOR_BATTERY = ((24300.0+38300.0)*2.5*1000.0)/(4095.0*24300.0);
const double CONVERSION_FACTOR =  2.5*10000.0/4096.0;

#ifdef USE_FLOATING_POINT

static unsigned int AdcCountsToBatteryVoltage(unsigned int Counts)
{
	return ((unsigned int)(CONVERSION_FACTOR_BATTERY*(double)Counts));
}

static unsigned int AdcCountsToVoltage(unsigned int Counts)
{
	return ((unsigned int)(CONVERSION_FACTOR*(double)Counts));
}

#else

#ifdef ENABLE_FLOATS
static const Fixed FIXED_CONVERSION_FACTOR_BATTERY(CONVERSION_FACTOR_BATTERY);
static const Fixed FIXED_CONVERSION_FACTOR(CONVERSION_FACTOR);

void ReportFixedAdcs(void)
{
#define REPORT(x, y)		goodprintf("static const Fixed " ## #x ## "(Fixed::RAW, %l);\t// %s = %F\n", x.Raw(), #y, x)
	REPORT(FIXED_CONVERSION_FACTOR_BATTERY, CONVERSION_FACTOR_BATTERY);
	REPORT(FIXED_CONVERSION_FACTOR, CONVERSION_FACTOR);
}
#else
static const Fixed FIXED_CONVERSION_FACTOR_BATTERY(Fixed::RAW, 103070);	// CONVERSION_FACTOR_BATTERY = 1.57
static const Fixed FIXED_CONVERSION_FACTOR(Fixed::RAW, 400000);	// CONVERSION_FACTOR = 6.10
#endif

static unsigned int AdcCountsToBatteryVoltage(unsigned int Counts)
{
	return (int)(Fixed((int)Counts) * FIXED_CONVERSION_FACTOR_BATTERY);
}

static unsigned int AdcCountsToVoltage(unsigned int Counts)
{
	return (int)(Fixed((int)Counts) * FIXED_CONVERSION_FACTOR);
}

#endif

template<int count> class AdcAveraging
{
public:
	AdcAveraging()
	{
		valid = false;
		index = 0;
	}

	void AddSample(unsigned int value)
	{
		samples[index++] = value;
		if (index >= count)
		{
			valid = true;
			index = 0;
		}
	}

	unsigned int Value(void)
	{
		unsigned int total = 0;
		for (int i = 0; i < count; i++)
			total += samples[i];
		return total / count;
	}

	bool IsValid(void)
	{
		return valid;
	}

private:
	unsigned int samples[count];
	int index;
	bool valid;
};

class AdcRequest
{
public:
	virtual void Initiate(void) = 0;
	virtual unsigned int Complete(void) = 0;
};

// Make this a template, so the functions will be all-ROM to save some RAM (it
// should be simple to swap around to store the channels/etc. in RAM if you'd
// rather get the ROM back)
template<unsigned short input, unsigned short channel> class AdcConversion : public AdcRequest
{
public:
protected:
	static void Config(void)
	{
		(&ADC12MCTL0)[input] = channel;
		ADC12IE |= 1 << input;
	}

	void StartADC(void)
	{
		// Set ADC channel
		ADC12CTL1 &= ~0xF000;
		ADC12CTL1 |= input << 12;

		// Turn on ADC
		ADC12CTL0 |= ADC12ON;
		ADC12CTL0 |= ADC12ENC | ADC12SC;
	}

	unsigned short ReadADC(void)
	{
		return (&ADC12MEM0)[input];
	}
};
#define HARDWARE_CFG_INPUT_CHANNEL  ( ADC12INCH_13 )
#define BATTERY_SENSE_INPUT_CHANNEL ( ADC12INCH_15 )
#define LIGHT_SENSE_INPUT_CHANNEL   ( ADC12INCH_1 )

template <unsigned short input, unsigned short channel> class HardwareConfig : public AdcConversion<input, channel>
{
public:
	static void Setup(void)
	{
		HARDWARE_CFG_SENSE_INIT();
		AdcConversion<input, channel>::Config();
	}

	void Initiate(void)
	{
		HARDWARE_CFG_SENSE_ENABLE();
		AdcConversion<input, channel>::StartADC();
	}

	unsigned int Complete(void)
	{
		unsigned int result = AdcCountsToVoltage(AdcConversion<input, channel>::ReadADC());
		HARDWARE_CFG_SENSE_DISABLE();
		return result;
	}
};

template <unsigned short input, unsigned short channel> class Battery : public AdcConversion<input, channel>
{
public:
	static void Setup(void)
	{
		BATTERY_SENSE_INIT();
		AdcConversion<input, channel>::Config();
	}

	void Initiate(void)
	{
		BATTERY_SENSE_ENABLE();
		AdcConversion<input, channel>::StartADC();
	}

	unsigned int Complete(void)
	{
		unsigned int result = AdcCountsToBatteryVoltage(AdcConversion<input, channel>::ReadADC());
		BATTERY_SENSE_DISABLE();
		if (QueryCalibrationValid())
			result += GetBatteryCalibrationValue();
		Average.AddSample(result);
		return result;
	}

	AdcAveraging<MAX_SAMPLES> Average;
};

template <unsigned short input, unsigned short channel> class LightSensor : public AdcConversion<input, channel>
{
public:
	static void Setup(void)
	{
		LIGHT_SENSE_INIT();
		AdcConversion<input, channel>::Config();
	}

	void Initiate(void)
	{
		LIGHT_SENSOR_L_GAIN();
		// Let the light sensor stabilise
		Timer_Create(DelayStartAdc, this, 10, false);
	}

	unsigned int Complete(void)
	{
		unsigned int result = AdcCountsToVoltage(AdcConversion<input, channel>::ReadADC());
		LIGHT_SENSOR_SHUTDOWN();
		Average.AddSample(result);
		return result;
	}

	AdcAveraging<MAX_SAMPLES> Average;

private:
	static int DelayStartAdc(TimerHandle handle, void *context)
	{
		((LightSensor<input, channel>*)context)->StartADC();
		Timer_Destroy(handle);
		return 0;
	}
};

static HardwareConfig<0, ADC12INCH_13> s_hardwareConfig;
static Battery<1, ADC12INCH_15> s_batteryLevel;
static LightSensor<2, ADC12INCH_1> s_lightSensor;

typedef struct {
	Adc_Request_Type request;
	Adc_Callback callback;
	void *context;
} ADC_REQUEST;

static ADC_REQUEST requests[MAX_REQUESTS];
static int currentRequest, nextRequest;
static bool requestActive;

static AdcRequest* GetRequester(Adc_Request_Type request)
{
	switch (request)
	{
		case adcHardwareConfiguration:
			return &s_hardwareConfig;
		case adcLightLevel:
			return &s_lightSensor;
		case adcBatteryLevel:
			return &s_batteryLevel;
		default:
			return NULL;
	}
}

extern "C" void Adc_Initialise(void)
{
	// Prepare all types of inputs
	s_hardwareConfig.Setup();
	s_batteryLevel.Setup();
	s_lightSensor.Setup();

	// Setup from Metawatch

	/* enable the 2.5V reference */
	ADC12CTL0 = ADC12REFON + ADC12REF2_5V;

	/* select ADC12SC bit as sample and hold source (00)
	 * and use pulse mode
	 * use ACLK so that ADCCLK < 2.7 MHz and so that SMCLK does not have to be used
	 */
	ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHP + ADC12SSEL_1;

	/* 12 bit resolution, only use reference when doing a conversion,
	 * use low power mode because sample rate is < 50 ksps
	 */
	ADC12CTL2 = ADC12TCOFF + ADC12RES_2 + ADC12REFBURST + ADC12SR;

	// Set up request queue
	memset(requests, 0, sizeof(requests));
	nextRequest = 0;
	requestActive = false;
}

static void Launch(int req)
{
	if (requests[req].callback == NULL)
	{
		requestActive = false;
		return;
	}
	currentRequest = req;
	requestActive = true;
	ENABLE_REFERENCE();
	GetRequester(requests[req].request)->Initiate();
}

static bool HandleResult(int req)
{
	// Compute result value
	ADC_REQUEST *request = &requests[req];
	unsigned int result = GetRequester(request->request)->Complete();
	// Shut off ADC
	ADC12CTL0 &= ~ADC12ENC;
	ADC12CTL0 &= ~ADC12ON;	// Potentially shut down only if no more requests?
	DISABLE_REFERENCE();
	// Send result
	bool endIsr = request->callback(request->request, request->context, result) != 0;
	request->callback = NULL;

	return endIsr;
}

extern "C" void Adc_Request(Adc_Request_Type request, Adc_Callback callback, void *context)
{
	ENTER_CRITICAL_REGION_QUICK();
	requests[nextRequest].request = request;
	requests[nextRequest].callback = callback;
	requests[nextRequest].context = context;
	if (!requestActive)
		Launch(nextRequest);
	nextRequest++;
	if (nextRequest >= MAX_REQUESTS)
		nextRequest = 0;
	LEAVE_CRITICAL_REGION_QUICK();
}

extern "C" unsigned int Adc_Battery_Average(void)
{
	return s_batteryLevel.Average.Value();
}

extern "C" int Adc_Battery_Average_Available(void)
{
	return s_batteryLevel.Average.IsValid() ? 1 : 0;
}

extern "C" unsigned int Light_Level_Average(void)
{
	return s_lightSensor.Average.Value();
}

extern "C" int Light_Level_Average_Available(void)
{
	return s_lightSensor.Average.IsValid() ? 1 : 0;
}

#pragma vector=ADC12_VECTOR
__interrupt void isrAnalogToDigital(void)
{
	ADC12IV = 0;	// We should only ever get an expected interrupt

	if (HandleResult(currentRequest))
	{
		EXIT_LPM_ISR();
	}
	currentRequest++;
	if (currentRequest >= MAX_REQUESTS)
		currentRequest = 0;
	Launch(currentRequest);
}
