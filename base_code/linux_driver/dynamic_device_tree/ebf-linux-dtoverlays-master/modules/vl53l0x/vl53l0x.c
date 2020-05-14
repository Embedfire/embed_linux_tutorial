/*
 * vl53l0x.c
 * Library for ranging sensor.
 *
 * Copyright (c) 2018 seeed technology inc.
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/sysfs.h>


/**************************************************************************************************************/
/**************************************************************************************************************/
/***************************************************driver*****************************************************/


struct i2c_client *vl53l0x_client;
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef uint32_t FixPoint1616_t;
typedef uint8_t VL53L0X_DeviceError;
typedef uint8_t VL53L0X_GpioFunctionality;
typedef int8_t  VL53L0X_Error;
typedef uint8_t VL53L0X_DeviceModes;
typedef uint8_t VL53L0X_PowerModes;
typedef uint8_t VL53L0X_State;
typedef uint8_t VL53L0X_InterruptPolarity;
typedef uint8_t VL53L0X_VcselPeriod;

#define LOG_FUNCTION_START(fmt, ...) \
	_LOG_FUNCTION_START(TRACE_MODULE_API, fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_END(status, ...) \
	_LOG_FUNCTION_END(TRACE_MODULE_API, status, ##__VA_ARGS__)
#define LOG_FUNCTION_END_FMT(status, fmt, ...) \
	_LOG_FUNCTION_END_FMT(TRACE_MODULE_API, status, fmt, ##__VA_ARGS__)

#define VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE           0
#define VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE     1
#define VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP             2
#define VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD      3
#define VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC            4
#define VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE       5

#define VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS            6
#define VERSION_REQUIRED_MAJOR 1
#define VERSION_REQUIRED_MINOR 0
#define VERSION_REQUIRED_BUILD 1
#define REF_ARRAY_SPAD_0  0
#define REF_ARRAY_SPAD_5  5
#define REF_ARRAY_SPAD_10 10

#define I2C_BUFFER_CONFIG 1
#define VL53L0X_MAX_I2C_XFER_SIZE   64 /* Maximum buffer size to be used in i2c */

#if I2C_BUFFER_CONFIG == 0
	uint8_t i2c_global_buffer[VL53L0X_MAX_I2C_XFER_SIZE];
	#define DECL_I2C_BUFFER
	#define VL53L0X_GetLocalBuffer(Dev, n_byte)  i2c_global_buffer

#elif I2C_BUFFER_CONFIG == 1
	#define DECL_I2C_BUFFER  uint8_t LocBuffer[VL53L0X_MAX_I2C_XFER_SIZE];
	#define VL53L0X_GetLocalBuffer(Dev, n_byte)  LocBuffer
#elif I2C_BUFFER_CONFIG == 2
	/* user define buffer type declare DECL_I2C_BUFFER  as access  via VL53L0X_GetLocalBuffer */
	#define DECL_I2C_BUFFER
#else
	#error "invalid I2C_BUFFER_CONFIG "
#endif


#define VL53L0X_I2C_USER_VAR         /* none but could be for a flag var to get/pass to mutex interruptible  return flags and try again */
#define VL53L0X_GetI2CAccess(Dev)    /* todo mutex acquire */
#define VL53L0X_DoneI2CAcces(Dev)    /* todo mutex release */

#define VL53L0X10_SPECIFICATION_VER_MAJOR   1
/** PAL SPECIFICATION minor version */
#define VL53L0X10_SPECIFICATION_VER_MINOR   2
/** PAL SPECIFICATION sub version */
#define VL53L0X10_SPECIFICATION_VER_SUB	   7
/** PAL SPECIFICATION sub version */
#define VL53L0X10_SPECIFICATION_VER_REVISION 1440

/** VL53L0X PAL IMPLEMENTATION major version */
#define VL53L0X10_IMPLEMENTATION_VER_MAJOR	1
/** VL53L0X PAL IMPLEMENTATION minor version */
#define VL53L0X10_IMPLEMENTATION_VER_MINOR	0
/** VL53L0X PAL IMPLEMENTATION sub version */
#define VL53L0X10_IMPLEMENTATION_VER_SUB		9
/** VL53L0X PAL IMPLEMENTATION sub version */
#define VL53L0X10_IMPLEMENTATION_VER_REVISION	3673

/** PAL SPECIFICATION major version */
#define VL53L0X_SPECIFICATION_VER_MAJOR	 1
/** PAL SPECIFICATION minor version */
#define VL53L0X_SPECIFICATION_VER_MINOR	 2
/** PAL SPECIFICATION sub version */
#define VL53L0X_SPECIFICATION_VER_SUB	 7
/** PAL SPECIFICATION sub version */
#define VL53L0X_SPECIFICATION_VER_REVISION 1440

/** VL53L0X PAL IMPLEMENTATION major version */
#define VL53L0X_IMPLEMENTATION_VER_MAJOR	  1
/** VL53L0X PAL IMPLEMENTATION minor version */
#define VL53L0X_IMPLEMENTATION_VER_MINOR	  0
/** VL53L0X PAL IMPLEMENTATION sub version */
#define VL53L0X_IMPLEMENTATION_VER_SUB	  1
/** VL53L0X PAL IMPLEMENTATION sub version */
#define VL53L0X_IMPLEMENTATION_VER_REVISION	  4606
#define VL53L0X_DEFAULT_MAX_LOOP 200
#define VL53L0X_MAX_STRING_LENGTH 32

#define VL53L0X_HISTOGRAM_BUFFER_SIZE 24

/** @brief Defines the parameters of the Get Version Functions
 */
typedef struct {
	uint32_t	 revision; /*!< revision number */
	uint8_t		 major;	   /*!< major number */
	uint8_t		 minor;	   /*!< minor number */
	uint8_t		 build;	   /*!< build number */
} VL53L0X_Version_t;

/** @brief Defines the parameters of the Get Device Info Functions
 */
typedef struct {
	char Name[VL53L0X_MAX_STRING_LENGTH];
	char Type[VL53L0X_MAX_STRING_LENGTH];
	char ProductId[VL53L0X_MAX_STRING_LENGTH];
	uint8_t ProductType;
	uint8_t ProductRevisionMajor;
	uint8_t ProductRevisionMinor;
} VL53L0X_DeviceInfo_t;


/** @defgroup VL53L0X_define_Error_group Error and Warning code returned by API
 *	The following DEFINE are used to identify the PAL ERROR
 *	@{
 */

#define VL53L0X_ERROR_NONE		((VL53L0X_Error)	0)
#define VL53L0X_ERROR_CALIBRATION_WARNING	((VL53L0X_Error) -1)
#define VL53L0X_ERROR_MIN_CLIPPED			((VL53L0X_Error) -2)
	/*!< Warning parameter passed was clipped to min before to be applied */

#define VL53L0X_ERROR_UNDEFINED				((VL53L0X_Error) -3)
	/*!< Unqualified error */
#define VL53L0X_ERROR_INVALID_PARAMS			((VL53L0X_Error) -4)
	/*!< Parameter passed is invalid or out of range */
#define VL53L0X_ERROR_NOT_SUPPORTED			((VL53L0X_Error) -5)
	/*!< Function is not supported in current mode or configuration */
#define VL53L0X_ERROR_RANGE_ERROR			((VL53L0X_Error) -6)
	/*!< Device report a ranging error interrupt status */
#define VL53L0X_ERROR_TIME_OUT				((VL53L0X_Error) -7)
	/*!< Aborted due to time out */
#define VL53L0X_ERROR_MODE_NOT_SUPPORTED			((VL53L0X_Error) -8)
	/*!< Asked mode is not supported by the device */
#define VL53L0X_ERROR_BUFFER_TOO_SMALL			((VL53L0X_Error) -9)
	/*!< ... */
#define VL53L0X_ERROR_GPIO_NOT_EXISTING			((VL53L0X_Error) -10)
	/*!< User tried to setup a non-existing GPIO pin */
#define VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED  ((VL53L0X_Error) -11)
	/*!< unsupported GPIO functionality */
#define VL53L0X_ERROR_INTERRUPT_NOT_CLEARED		((VL53L0X_Error) -12)
	/*!< Error during interrupt clear */
#define VL53L0X_ERROR_CONTROL_INTERFACE			((VL53L0X_Error) -20)
	/*!< error reported from IO functions */
#define VL53L0X_ERROR_INVALID_COMMAND			((VL53L0X_Error) -30)
	/*!< The command is not allowed in the current device state
	 *	(power down) */
#define VL53L0X_ERROR_DIVISION_BY_ZERO			((VL53L0X_Error) -40)
	/*!< In the function a division by zero occurs */
#define VL53L0X_ERROR_REF_SPAD_INIT			((VL53L0X_Error) -50)
	/*!< Error during reference SPAD initialization */
#define VL53L0X_ERROR_NOT_IMPLEMENTED			((VL53L0X_Error) -99)
	/*!< Tells requested functionality has not been implemented yet or
	 * not compatible with the device */
/** @} VL53L0X_define_Error_group */



#define VL53L0X_DEVICEMODE_SINGLE_RANGING	((VL53L0X_DeviceModes)  0)
#define VL53L0X_DEVICEMODE_CONTINUOUS_RANGING	((VL53L0X_DeviceModes)  1)
#define VL53L0X_DEVICEMODE_SINGLE_HISTOGRAM	((VL53L0X_DeviceModes)  2)
#define VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING ((VL53L0X_DeviceModes) 3)
#define VL53L0X_DEVICEMODE_SINGLE_ALS		((VL53L0X_DeviceModes) 10)
#define VL53L0X_DEVICEMODE_GPIO_DRIVE		((VL53L0X_DeviceModes) 20)
#define VL53L0X_DEVICEMODE_GPIO_OSC		((VL53L0X_DeviceModes) 21)

/** @defgroup VL53L0X_define_HistogramModes_group Defines Histogram modes
 *	Defines all possible Histogram modes for the device
 *	@{
 */
typedef uint8_t VL53L0X_HistogramModes;

#define VL53L0X_HISTOGRAMMODE_DISABLED		((VL53L0X_HistogramModes) 0)
	/*!< Histogram Disabled */
#define VL53L0X_HISTOGRAMMODE_REFERENCE_ONLY	((VL53L0X_HistogramModes) 1)
	/*!< Histogram Reference array only */
#define VL53L0X_HISTOGRAMMODE_RETURN_ONLY	((VL53L0X_HistogramModes) 2)
	/*!< Histogram Return array only */
#define VL53L0X_HISTOGRAMMODE_BOTH		((VL53L0X_HistogramModes) 3)
	/*!< Histogram both Reference and Return Arrays */
	/* ... Modes to be added depending on device */
/** @} VL53L0X_define_HistogramModes_group */

#define VL53L0X_POWERMODE_STANDBY_LEVEL1 ((VL53L0X_PowerModes) 0)
	/*!< Standby level 1 */
#define VL53L0X_POWERMODE_STANDBY_LEVEL2 ((VL53L0X_PowerModes) 1)
	/*!< Standby level 2 */
#define VL53L0X_POWERMODE_IDLE_LEVEL1	((VL53L0X_PowerModes) 2)
	/*!< Idle level 1 */
#define VL53L0X_POWERMODE_IDLE_LEVEL2	((VL53L0X_PowerModes) 3)
	/*!< Idle level 2 */

/** @} VL53L0X_define_PowerModes_group */

/** @brief Defines all parameters for the device
 */
typedef struct {
	VL53L0X_DeviceModes DeviceMode;
	/*!< Defines type of measurement to be done for the next measure */
	VL53L0X_HistogramModes HistogramMode;
	/*!< Defines type of histogram measurement to be done for the next
	 *	measure */
	uint32_t MeasurementTimingBudgetMicroSeconds;
	/*!< Defines the allowed total time for a single measurement */
	uint32_t InterMeasurementPeriodMilliSeconds;
	/*!< Defines time between two consecutive measurements (between two
	 *	measurement starts). If set to 0 means back-to-back mode */
	uint8_t XTalkCompensationEnable;
	/*!< Tells if Crosstalk compensation shall be enable or not	 */
	uint16_t XTalkCompensationRangeMilliMeter;
	/*!< CrossTalk compensation range in millimeter	 */
	FixPoint1616_t XTalkCompensationRateMegaCps;
	/*!< CrossTalk compensation rate in Mega counts per seconds.
	 *	Expressed in 16.16 fixed point format.	*/
	int32_t RangeOffsetMicroMeters;
	/*!< Range offset adjustment (mm).	*/

	uint8_t LimitChecksEnable[VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS];
	/*!< This Array store all the Limit Check enable for this device. */
	uint8_t LimitChecksStatus[VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS];
	/*!< This Array store all the Status of the check linked to last
	* measurement. */
	FixPoint1616_t LimitChecksValue[VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS];
	/*!< This Array store all the Limit Check value for this device */

	uint8_t WrapAroundCheckEnable;
	/*!< Tells if Wrap Around Check shall be enable or not */
} VL53L0X_DeviceParameters_t;


/** @defgroup VL53L0X_define_State_group Defines the current status of the device
 *	Defines the current status of the device
 *	@{
 */

#define VL53L0X_STATE_POWERDOWN		 ((VL53L0X_State)  0)
	/*!< Device is in HW reset	*/
#define VL53L0X_STATE_WAIT_STATICINIT ((VL53L0X_State)  1)
	/*!< Device is initialized and wait for static initialization  */
#define VL53L0X_STATE_STANDBY		 ((VL53L0X_State)  2)
	/*!< Device is in Low power Standby mode   */
#define VL53L0X_STATE_IDLE			 ((VL53L0X_State)  3)
	/*!< Device has been initialized and ready to do measurements  */
#define VL53L0X_STATE_RUNNING		 ((VL53L0X_State)  4)
	/*!< Device is performing measurement */
#define VL53L0X_STATE_UNKNOWN		 ((VL53L0X_State)  98)
	/*!< Device is in unknown state and need to be rebooted	 */
#define VL53L0X_STATE_ERROR			 ((VL53L0X_State)  99)
	/*!< Device is in error state and need to be rebooted  */


/** @brief Structure containing the Dmax computation parameters and data
 */
typedef struct {
	int32_t AmbTuningWindowFactor_K;
	int32_t RetSignalAt0mm;
} VL53L0X_DMaxData_t;

/**
 * @struct VL53L0X_RangeData_t
 * @brief Range measurement data.
 */
typedef struct {
	uint32_t TimeStamp;		/*!< 32-bit time stamp. */
	uint32_t MeasurementTimeUsec;

	uint16_t RangeMilliMeter;	/*!< range distance in millimeter. */
	uint16_t RangeDMaxMilliMeter;
	FixPoint1616_t SignalRateRtnMegaCps;
	FixPoint1616_t AmbientRateRtnMegaCps;
	uint16_t EffectiveSpadRtnCount;
	uint8_t ZoneId;
	uint8_t RangeFractionalPart;
	uint8_t RangeStatus;
} VL53L0X_RangingMeasurementData_t;



/**
 * @struct VL53L0X_HistogramData_t
 * @brief Histogram measurement data.
 */
typedef struct {
	/* Histogram Measurement data */
	uint32_t HistogramData[VL53L0X_HISTOGRAM_BUFFER_SIZE];
	/*!< Histogram data */
	uint8_t HistogramType; /*!< Indicate the types of histogram data :
	Return only, Reference only, both Return and Reference */
	uint8_t FirstBin; /*!< First Bin value */
	uint8_t BufferSize; /*!< Buffer Size - Set by the user.*/
	uint8_t NumberOfBins;
	/*!< Number of bins filled by the histogram measurement */

	VL53L0X_DeviceError ErrorStatus;
	/*!< Error status of the current measurement. \n
	see @a ::VL53L0X_DeviceError @a VL53L0X_GetStatusErrorString() */
} VL53L0X_HistogramMeasurementData_t;

#define VL53L0X_REF_SPAD_BUFFER_SIZE 6

/**
 * @struct VL53L0X_SpadData_t
 * @brief Spad Configuration Data.
 */
typedef struct {
	uint8_t RefSpadEnables[VL53L0X_REF_SPAD_BUFFER_SIZE];
	/*!< Reference Spad Enables */
	uint8_t RefGoodSpadMap[VL53L0X_REF_SPAD_BUFFER_SIZE];
	/*!< Reference Spad Good Spad Map */
} VL53L0X_SpadData_t;

typedef struct {
	FixPoint1616_t OscFrequencyMHz; /* Frequency used */

	uint16_t LastEncodedTimeout;
	/* last encoded Time out used for timing budget*/

	VL53L0X_GpioFunctionality Pin0GpioFunctionality;
	/* store the functionality of the GPIO: pin0 */

	uint32_t FinalRangeTimeoutMicroSecs;
	 /*!< Execution time of the final range*/
	uint8_t FinalRangeVcselPulsePeriod;
	 /*!< Vcsel pulse period (pll clocks) for the final range measurement*/
	uint32_t PreRangeTimeoutMicroSecs;
	 /*!< Execution time of the final range*/
	uint8_t PreRangeVcselPulsePeriod;
	 /*!< Vcsel pulse period (pll clocks) for the pre-range measurement*/

	uint16_t SigmaEstRefArray;
	 /*!< Reference array sigma value in 1/100th of [mm] e.g. 100 = 1mm */
	uint16_t SigmaEstEffPulseWidth;
	 /*!< Effective Pulse width for sigma estimate in 1/100th
	  * of ns e.g. 900 = 9.0ns */
	uint16_t SigmaEstEffAmbWidth;
	 /*!< Effective Ambient width for sigma estimate in 1/100th of ns
	  * e.g. 500 = 5.0ns */


	uint8_t ReadDataFromDeviceDone; /* Indicate if read from device has
	been done (==1) or not (==0) */
	uint8_t ModuleId; /* Module ID */
	uint8_t Revision; /* test Revision */
	char ProductId[VL53L0X_MAX_STRING_LENGTH];
		/* Product Identifier String  */
	uint8_t ReferenceSpadCount; /* used for ref spad management */
	uint8_t ReferenceSpadType;	/* used for ref spad management */
	uint8_t RefSpadsInitialised; /* reports if ref spads are initialised. */
	uint32_t PartUIDUpper; /*!< Unique Part ID Upper */
	uint32_t PartUIDLower; /*!< Unique Part ID Lower */
	FixPoint1616_t SignalRateMeasFixed400mm; /*!< Peek Signal rate
	at 400 mm*/

} VL53L0X_DeviceSpecificParameters_t;

/**
 * @struct VL53L0X_DevData_t
 *
 * @brief VL53L0X PAL device ST private data structure \n
 * End user should never access any of these field directly
 *
 * These must never access directly but only via macro
 */
typedef struct {
	VL53L0X_DMaxData_t DMaxData;
	/*!< Dmax Data */
	int32_t	 Part2PartOffsetNVMMicroMeter;
	/*!< backed up NVM value */
	int32_t	 Part2PartOffsetAdjustmentNVMMicroMeter;
	/*!< backed up NVM value representing additional offset adjustment */
	VL53L0X_DeviceParameters_t CurrentParameters;
	/*!< Current Device Parameter */
	VL53L0X_RangingMeasurementData_t LastRangeMeasure;
	/*!< Ranging Data */
	VL53L0X_HistogramMeasurementData_t LastHistogramMeasure;
	/*!< Histogram Data */
	VL53L0X_DeviceSpecificParameters_t DeviceSpecificParameters;
	/*!< Parameters specific to the device */
	VL53L0X_SpadData_t SpadData;
	/*!< Spad Data */
	uint8_t SequenceConfig;
	/*!< Internal value for the sequence config */
	uint8_t RangeFractionalEnable;
	/*!< Enable/Disable fractional part of ranging data */
	VL53L0X_State PalState;
	/*!< Current state of the PAL for this device */
	VL53L0X_PowerModes PowerMode;
	/*!< Current Power Mode	 */
	uint16_t SigmaEstRefArray;
	/*!< Reference array sigma value in 1/100th of [mm] e.g. 100 = 1mm */
	uint16_t SigmaEstEffPulseWidth;
	/*!< Effective Pulse width for sigma estimate in 1/100th
	* of ns e.g. 900 = 9.0ns */
	uint16_t SigmaEstEffAmbWidth;
	/*!< Effective Ambient width for sigma estimate in 1/100th of ns
	* e.g. 500 = 5.0ns */
	uint8_t StopVariable;
	/*!< StopVariable used during the stop sequence */
	uint16_t targetRefRate;
	/*!< Target Ambient Rate for Ref spad management */
	FixPoint1616_t SigmaEstimate;
	/*!< Sigma Estimate - based on ambient & VCSEL rates and
	* signal_total_events */
	FixPoint1616_t SignalEstimate;
	/*!< Signal Estimate - based on ambient & VCSEL rates and cross talk */
	FixPoint1616_t LastSignalRefMcps;
	/*!< Latest Signal ref in Mcps */
	uint8_t *pTuningSettingsPointer;
	/*!< Pointer for Tuning Settings table */
	uint8_t UseInternalTuningSettings;
	/*!< Indicate if we use	 Tuning Settings table */
	uint16_t LinearityCorrectiveGain;
	/*!< Linearity Corrective Gain value in x1000 */
	uint16_t DmaxCalRangeMilliMeter;
	/*!< Dmax Calibration Range millimeter */
	FixPoint1616_t DmaxCalSignalRateRtnMegaCps;
	/*!< Dmax Calibration Signal Rate Return MegaCps */

} VL53L0X_DevData_t;


/** @defgroup VL53L0X_define_InterruptPolarity_group Defines the Polarity
 * of the Interrupt
 *	Defines the Polarity of the Interrupt
 *	@{
 */

#define VL53L0X_INTERRUPTPOLARITY_LOW	   ((VL53L0X_InterruptPolarity)	0)
/*!< Set active low polarity best setup for falling edge. */
#define VL53L0X_INTERRUPTPOLARITY_HIGH	   ((VL53L0X_InterruptPolarity)	1)
/*!< Set active high polarity best setup for rising edge. */

/** @defgroup VL53L0X_define_VcselPeriod_group Vcsel Period Defines
 *	Defines the range measurement for which to access the vcsel period.
 *	@{
 */


#define VL53L0X_VCSEL_PERIOD_PRE_RANGE	((VL53L0X_VcselPeriod) 0)
/*!<Identifies the pre-range vcsel period. */
#define VL53L0X_VCSEL_PERIOD_FINAL_RANGE ((VL53L0X_VcselPeriod) 1)
/*!<Identifies the final range vcsel period. */

/** @} VL53L0X_define_VcselPeriod_group */

/** @defgroup VL53L0X_define_SchedulerSequence_group Defines the steps
 * carried out by the scheduler during a range measurement.
 *	@{
 *	Defines the states of all the steps in the scheduler
 *	i.e. enabled/disabled.
 */
typedef struct {
	uint8_t		 TccOn;	   /*!<Reports if Target Centre Check On  */
	uint8_t		 MsrcOn;	   /*!<Reports if MSRC On  */
	uint8_t		 DssOn;		   /*!<Reports if DSS On  */
	uint8_t		 PreRangeOn;   /*!<Reports if Pre-Range On	*/
	uint8_t		 FinalRangeOn; /*!<Reports if Final-Range On  */
} VL53L0X_SchedulerSequenceSteps_t;

/** @} VL53L0X_define_SchedulerSequence_group */

/** @defgroup VL53L0X_define_SequenceStepId_group Defines the Polarity
 *	of the Interrupt
 *	Defines the the sequence steps performed during ranging..
 *	@{
 */
typedef uint8_t VL53L0X_SequenceStepId;

#define	 VL53L0X_SEQUENCESTEP_TCC		 ((VL53L0X_VcselPeriod) 0)
/*!<Target CentreCheck identifier. */
#define	 VL53L0X_SEQUENCESTEP_DSS		 ((VL53L0X_VcselPeriod) 1)
/*!<Dynamic Spad Selection function Identifier. */
#define	 VL53L0X_SEQUENCESTEP_MSRC		 ((VL53L0X_VcselPeriod) 2)
/*!<Minimum Signal Rate Check function Identifier. */
#define	 VL53L0X_SEQUENCESTEP_PRE_RANGE	 ((VL53L0X_VcselPeriod) 3)
/*!<Pre-Range check Identifier. */
#define	 VL53L0X_SEQUENCESTEP_FINAL_RANGE ((VL53L0X_VcselPeriod) 4)
/*!<Final Range Check Identifier. */

#define	 VL53L0X_SEQUENCESTEP_NUMBER_OF_CHECKS			 5
/*!<Number of Sequence Step Managed by the API. */

/** @} VL53L0X_define_SequenceStepId_group */


/* MACRO Definitions */
/** @defgroup VL53L0X_define_GeneralMacro_group General Macro Defines
 *	General Macro Defines
 *	@{
 */

/* Defines */
#define VL53L0X_SETPARAMETERFIELD(Dev, field, value) \
	PALDevDataSet(Dev, CurrentParameters.field, value)

#define VL53L0X_GETPARAMETERFIELD(Dev, field, variable) \
	variable = PALDevDataGet(Dev, CurrentParameters).field


#define VL53L0X_SETARRAYPARAMETERFIELD(Dev, field, index, value) \
	PALDevDataSet(Dev, CurrentParameters.field[index], value)

#define VL53L0X_GETARRAYPARAMETERFIELD(Dev, field, index, variable) \
	variable = PALDevDataGet(Dev, CurrentParameters).field[index]


#define VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, field, value) \
		PALDevDataSet(Dev, DeviceSpecificParameters.field, value)

#define VL53L0X_GETDEVICESPECIFICPARAMETER(Dev, field) \
		PALDevDataGet(Dev, DeviceSpecificParameters).field


#define VL53L0X_FIXPOINT1616TOFIXPOINT97(Value) \
	(uint16_t)((Value>>9)&0xFFFF)
#define VL53L0X_FIXPOINT97TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<9)

#define VL53L0X_FIXPOINT1616TOFIXPOINT88(Value) \
	(uint16_t)((Value>>8)&0xFFFF)
#define VL53L0X_FIXPOINT88TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<8)

#define VL53L0X_FIXPOINT1616TOFIXPOINT412(Value) \
	(uint16_t)((Value>>4)&0xFFFF)
#define VL53L0X_FIXPOINT412TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<4)

#define VL53L0X_FIXPOINT1616TOFIXPOINT313(Value) \
	(uint16_t)((Value>>3)&0xFFFF)
#define VL53L0X_FIXPOINT313TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<3)

#define VL53L0X_FIXPOINT1616TOFIXPOINT08(Value) \
	(uint8_t)((Value>>8)&0x00FF)
#define VL53L0X_FIXPOINT08TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<8)

#define VL53L0X_FIXPOINT1616TOFIXPOINT53(Value) \
	(uint8_t)((Value>>13)&0x00FF)
#define VL53L0X_FIXPOINT53TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<13)

#define VL53L0X_FIXPOINT1616TOFIXPOINT102(Value) \
	(uint16_t)((Value>>14)&0x0FFF)
#define VL53L0X_FIXPOINT102TOFIXPOINT1616(Value) \
	(FixPoint1616_t)(Value<<12)

#define VL53L0X_MAKEUINT16(lsb, msb) (uint16_t)((((uint16_t)msb)<<8) + \
		(uint16_t)lsb)


typedef struct {
    VL53L0X_DevData_t Data;               /*!< embed ST Ewok Dev  data as "Data"*/

    /*!< user specific field */
    uint8_t   I2cDevAddr;                /*!< i2c device address user specific field */
    uint8_t   comms_type;                /*!< Type of comms : VL53L0X_COMMS_I2C or VL53L0X_COMMS_SPI */
    uint16_t  comms_speed_khz;           /*!< Comms speed [kHz] : typically 400kHz for I2C           */

} VL53L0X_Dev_t;


typedef VL53L0X_Dev_t* VL53L0X_DEV;

#define PALDevDataGet(Dev, field) (Dev->Data.field)
#define PALDevDataSet(Dev, field, data) (Dev->Data.field)=(data)

#ifdef USE_EMPTY_STRING
	#define  VL53L0X_STRING_DEVICE_INFO_NAME                             ""
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS0                         ""
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS1                         ""
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS2                         ""
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_ES1                         ""
	#define  VL53L0X_STRING_DEVICE_INFO_TYPE                             ""

	/* PAL ERROR strings */
	#define  VL53L0X_STRING_ERROR_NONE                                   ""
	#define  VL53L0X_STRING_ERROR_CALIBRATION_WARNING                    ""
	#define  VL53L0X_STRING_ERROR_MIN_CLIPPED                            ""
	#define  VL53L0X_STRING_ERROR_UNDEFINED                              ""
	#define  VL53L0X_STRING_ERROR_INVALID_PARAMS                         ""
	#define  VL53L0X_STRING_ERROR_NOT_SUPPORTED                          ""
	#define  VL53L0X_STRING_ERROR_RANGE_ERROR                            ""
	#define  VL53L0X_STRING_ERROR_TIME_OUT                               ""
	#define  VL53L0X_STRING_ERROR_MODE_NOT_SUPPORTED                     ""
	#define  VL53L0X_STRING_ERROR_BUFFER_TOO_SMALL                       ""
	#define  VL53L0X_STRING_ERROR_GPIO_NOT_EXISTING                      ""
	#define  VL53L0X_STRING_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED       ""
	#define  VL53L0X_STRING_ERROR_CONTROL_INTERFACE                      ""
	#define  VL53L0X_STRING_ERROR_INVALID_COMMAND                        ""
	#define  VL53L0X_STRING_ERROR_DIVISION_BY_ZERO                       ""
	#define  VL53L0X_STRING_ERROR_REF_SPAD_INIT                          ""
	#define  VL53L0X_STRING_ERROR_NOT_IMPLEMENTED                        ""

	#define  VL53L0X_STRING_UNKNOW_ERROR_CODE                            ""



	/* Range Status */
	#define  VL53L0X_STRING_RANGESTATUS_NONE                             ""
	#define  VL53L0X_STRING_RANGESTATUS_RANGEVALID                       ""
	#define  VL53L0X_STRING_RANGESTATUS_SIGMA                            ""
	#define  VL53L0X_STRING_RANGESTATUS_SIGNAL                           ""
	#define  VL53L0X_STRING_RANGESTATUS_MINRANGE                         ""
	#define  VL53L0X_STRING_RANGESTATUS_PHASE                            ""
	#define  VL53L0X_STRING_RANGESTATUS_HW                               ""


	/* Range Status */
	#define  VL53L0X_STRING_STATE_POWERDOWN                              ""
	#define  VL53L0X_STRING_STATE_WAIT_STATICINIT                        ""
	#define  VL53L0X_STRING_STATE_STANDBY                                ""
	#define  VL53L0X_STRING_STATE_IDLE                                   ""
	#define  VL53L0X_STRING_STATE_RUNNING                                ""
	#define  VL53L0X_STRING_STATE_UNKNOWN                                ""
	#define  VL53L0X_STRING_STATE_ERROR                                  ""


	/* Device Specific */
	#define  VL53L0X_STRING_DEVICEERROR_NONE                             ""
	#define  VL53L0X_STRING_DEVICEERROR_VCSELCONTINUITYTESTFAILURE       ""
	#define  VL53L0X_STRING_DEVICEERROR_VCSELWATCHDOGTESTFAILURE         ""
	#define  VL53L0X_STRING_DEVICEERROR_NOVHVVALUEFOUND                  ""
	#define  VL53L0X_STRING_DEVICEERROR_MSRCNOTARGET                     ""
	#define  VL53L0X_STRING_DEVICEERROR_SNRCHECK                         ""
	#define  VL53L0X_STRING_DEVICEERROR_RANGEPHASECHECK                  ""
	#define  VL53L0X_STRING_DEVICEERROR_SIGMATHRESHOLDCHECK              ""
	#define  VL53L0X_STRING_DEVICEERROR_TCC                              ""
	#define  VL53L0X_STRING_DEVICEERROR_PHASECONSISTENCY                 ""
	#define  VL53L0X_STRING_DEVICEERROR_MINCLIP                          ""
	#define  VL53L0X_STRING_DEVICEERROR_RANGECOMPLETE                    ""
	#define  VL53L0X_STRING_DEVICEERROR_ALGOUNDERFLOW                    ""
	#define  VL53L0X_STRING_DEVICEERROR_ALGOOVERFLOW                     ""
	#define  VL53L0X_STRING_DEVICEERROR_RANGEIGNORETHRESHOLD             ""
	#define  VL53L0X_STRING_DEVICEERROR_UNKNOWN                          ""

	/* Check Enable */
	#define  VL53L0X_STRING_CHECKENABLE_SIGMA_FINAL_RANGE                ""
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE          ""
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_REF_CLIP                  ""
	#define  VL53L0X_STRING_CHECKENABLE_RANGE_IGNORE_THRESHOLD           ""

	/* Sequence Step */
	#define  VL53L0X_STRING_SEQUENCESTEP_TCC                             ""
	#define  VL53L0X_STRING_SEQUENCESTEP_DSS                             ""
	#define  VL53L0X_STRING_SEQUENCESTEP_MSRC                            ""
	#define  VL53L0X_STRING_SEQUENCESTEP_PRE_RANGE                       ""
	#define  VL53L0X_STRING_SEQUENCESTEP_FINAL_RANGE                     ""
#else
	#define  VL53L0X_STRING_DEVICE_INFO_NAME          "VL53L0X cut1.0"
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS0      "VL53L0X TS0"
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS1      "VL53L0X TS1"
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_TS2      "VL53L0X TS2"
	#define  VL53L0X_STRING_DEVICE_INFO_NAME_ES1      "VL53L0X ES1 or later"
	#define  VL53L0X_STRING_DEVICE_INFO_TYPE          "VL53L0X"

	/* PAL ERROR strings */
	#define  VL53L0X_STRING_ERROR_NONE \
			"No Error"
	#define  VL53L0X_STRING_ERROR_CALIBRATION_WARNING \
			"Calibration Warning Error"
	#define  VL53L0X_STRING_ERROR_MIN_CLIPPED \
			"Min clipped error"
	#define  VL53L0X_STRING_ERROR_UNDEFINED \
			"Undefined error"
	#define  VL53L0X_STRING_ERROR_INVALID_PARAMS \
			"Invalid parameters error"
	#define  VL53L0X_STRING_ERROR_NOT_SUPPORTED \
			"Not supported error"
	#define  VL53L0X_STRING_ERROR_RANGE_ERROR \
			"Range error"
	#define  VL53L0X_STRING_ERROR_TIME_OUT \
			"Time out error"
	#define  VL53L0X_STRING_ERROR_MODE_NOT_SUPPORTED \
			"Mode not supported error"
	#define  VL53L0X_STRING_ERROR_BUFFER_TOO_SMALL \
			"Buffer too small"
	#define  VL53L0X_STRING_ERROR_GPIO_NOT_EXISTING \
			"GPIO not existing"
	#define  VL53L0X_STRING_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED \
			"GPIO funct not supported"
	#define  VL53L0X_STRING_ERROR_INTERRUPT_NOT_CLEARED \
			"Interrupt not Cleared"
	#define  VL53L0X_STRING_ERROR_CONTROL_INTERFACE \
			"Control Interface Error"
	#define  VL53L0X_STRING_ERROR_INVALID_COMMAND \
			"Invalid Command Error"
	#define  VL53L0X_STRING_ERROR_DIVISION_BY_ZERO \
			"Division by zero Error"
	#define  VL53L0X_STRING_ERROR_REF_SPAD_INIT \
			"Reference Spad Init Error"
	#define  VL53L0X_STRING_ERROR_NOT_IMPLEMENTED \
			"Not implemented error"

	#define  VL53L0X_STRING_UNKNOW_ERROR_CODE \
			"Unknown Error Code"



	/* Range Status */
	#define  VL53L0X_STRING_RANGESTATUS_NONE                 "No Update"
	#define  VL53L0X_STRING_RANGESTATUS_RANGEVALID           "Range Valid"
	#define  VL53L0X_STRING_RANGESTATUS_SIGMA                "Sigma Fail"
	#define  VL53L0X_STRING_RANGESTATUS_SIGNAL               "Signal Fail"
	#define  VL53L0X_STRING_RANGESTATUS_MINRANGE             "Min Range Fail"
	#define  VL53L0X_STRING_RANGESTATUS_PHASE                "Phase Fail"
	#define  VL53L0X_STRING_RANGESTATUS_HW                   "Hardware Fail"


	/* Range Status */
	#define  VL53L0X_STRING_STATE_POWERDOWN               "POWERDOWN State"
	#define  VL53L0X_STRING_STATE_WAIT_STATICINIT \
			"Wait for staticinit State"
	#define  VL53L0X_STRING_STATE_STANDBY                 "STANDBY State"
	#define  VL53L0X_STRING_STATE_IDLE                    "IDLE State"
	#define  VL53L0X_STRING_STATE_RUNNING                 "RUNNING State"
	#define  VL53L0X_STRING_STATE_UNKNOWN                 "UNKNOWN State"
	#define  VL53L0X_STRING_STATE_ERROR                   "ERROR State"


	/* Device Specific */
	#define  VL53L0X_STRING_DEVICEERROR_NONE                   "No Update"
	#define  VL53L0X_STRING_DEVICEERROR_VCSELCONTINUITYTESTFAILURE \
			"VCSEL Continuity Test Failure"
	#define  VL53L0X_STRING_DEVICEERROR_VCSELWATCHDOGTESTFAILURE \
			"VCSEL Watchdog Test Failure"
	#define  VL53L0X_STRING_DEVICEERROR_NOVHVVALUEFOUND \
			"No VHV Value found"
	#define  VL53L0X_STRING_DEVICEERROR_MSRCNOTARGET \
			"MSRC No Target Error"
	#define  VL53L0X_STRING_DEVICEERROR_SNRCHECK \
			"SNR Check Exit"
	#define  VL53L0X_STRING_DEVICEERROR_RANGEPHASECHECK \
			"Range Phase Check Error"
	#define  VL53L0X_STRING_DEVICEERROR_SIGMATHRESHOLDCHECK \
			"Sigma Threshold Check Error"
	#define  VL53L0X_STRING_DEVICEERROR_TCC \
			"TCC Error"
	#define  VL53L0X_STRING_DEVICEERROR_PHASECONSISTENCY \
			"Phase Consistency Error"
	#define  VL53L0X_STRING_DEVICEERROR_MINCLIP \
			"Min Clip Error"
	#define  VL53L0X_STRING_DEVICEERROR_RANGECOMPLETE \
			"Range Complete"
	#define  VL53L0X_STRING_DEVICEERROR_ALGOUNDERFLOW \
			"Range Algo Underflow Error"
	#define  VL53L0X_STRING_DEVICEERROR_ALGOOVERFLOW \
			"Range Algo Overlow Error"
	#define  VL53L0X_STRING_DEVICEERROR_RANGEIGNORETHRESHOLD \
			"Range Ignore Threshold Error"
	#define  VL53L0X_STRING_DEVICEERROR_UNKNOWN \
			"Unknown error code"

	/* Check Enable */
	#define  VL53L0X_STRING_CHECKENABLE_SIGMA_FINAL_RANGE \
			"SIGMA FINAL RANGE"
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE \
			"SIGNAL RATE FINAL RANGE"
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_REF_CLIP \
			"SIGNAL REF CLIP"
	#define  VL53L0X_STRING_CHECKENABLE_RANGE_IGNORE_THRESHOLD \
			"RANGE IGNORE THRESHOLD"
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_MSRC \
			"SIGNAL RATE MSRC"
	#define  VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_PRE_RANGE \
			"SIGNAL RATE PRE RANGE"

	/* Sequence Step */
	#define  VL53L0X_STRING_SEQUENCESTEP_TCC                   "TCC"
	#define  VL53L0X_STRING_SEQUENCESTEP_DSS                   "DSS"
	#define  VL53L0X_STRING_SEQUENCESTEP_MSRC                  "MSRC"
	#define  VL53L0X_STRING_SEQUENCESTEP_PRE_RANGE             "PRE RANGE"
	#define  VL53L0X_STRING_SEQUENCESTEP_FINAL_RANGE           "FINAL RANGE"
#endif /* USE_EMPTY_STRING */




#define VL53L0X_DEVICEERROR_NONE                        ((VL53L0X_DeviceError) 0)
	/*!< 0  NoError  */
#define VL53L0X_DEVICEERROR_VCSELCONTINUITYTESTFAILURE  ((VL53L0X_DeviceError) 1)
#define VL53L0X_DEVICEERROR_VCSELWATCHDOGTESTFAILURE    ((VL53L0X_DeviceError) 2)
#define VL53L0X_DEVICEERROR_NOVHVVALUEFOUND             ((VL53L0X_DeviceError) 3)
#define VL53L0X_DEVICEERROR_MSRCNOTARGET                ((VL53L0X_DeviceError) 4)
#define VL53L0X_DEVICEERROR_SNRCHECK                    ((VL53L0X_DeviceError) 5)
#define VL53L0X_DEVICEERROR_RANGEPHASECHECK             ((VL53L0X_DeviceError) 6)
#define VL53L0X_DEVICEERROR_SIGMATHRESHOLDCHECK         ((VL53L0X_DeviceError) 7)
#define VL53L0X_DEVICEERROR_TCC                         ((VL53L0X_DeviceError) 8)
#define VL53L0X_DEVICEERROR_PHASECONSISTENCY            ((VL53L0X_DeviceError) 9)
#define VL53L0X_DEVICEERROR_MINCLIP                     ((VL53L0X_DeviceError) 10)
#define VL53L0X_DEVICEERROR_RANGECOMPLETE               ((VL53L0X_DeviceError) 11)
#define VL53L0X_DEVICEERROR_ALGOUNDERFLOW               ((VL53L0X_DeviceError) 12)
#define VL53L0X_DEVICEERROR_ALGOOVERFLOW                ((VL53L0X_DeviceError) 13)
#define VL53L0X_DEVICEERROR_RANGEIGNORETHRESHOLD        ((VL53L0X_DeviceError) 14)

/** @defgroup VL53L0X_GpioFunctionality_group Gpio Functionality
 *  @brief Defines the different functionalities for the device GPIO(s)
 *  @{
 */

#define VL53L0X_GPIOFUNCTIONALITY_OFF                     \
	((VL53L0X_GpioFunctionality)  0) /*!< NO Interrupt  */
#define VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW   \
	((VL53L0X_GpioFunctionality)  1) /*!< Level Low (value < thresh_low)  */
#define VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH   \
	((VL53L0X_GpioFunctionality)  2) /*!< Level High (value > thresh_high) */
#define VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT    \
	((VL53L0X_GpioFunctionality)  3)
	/*!< Out Of Window (value < thresh_low OR value > thresh_high)  */
#define VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY        \
	((VL53L0X_GpioFunctionality)  4) /*!< New Sample Ready  */

/* Device register map */

/** @defgroup VL53L0X_DefineRegisters_group Define Registers
 *  @brief List of all the defined registers
 *  @{
 */
#define VL53L0X_REG_SYSRANGE_START                        0x000
	/** mask existing bit in #VL53L0X_REG_SYSRANGE_START*/
	#define VL53L0X_REG_SYSRANGE_MODE_MASK          0x0F
	/** bit 0 in #VL53L0X_REG_SYSRANGE_START write 1 toggle state in
	 * continuous mode and arm next shot in single shot mode */
	#define VL53L0X_REG_SYSRANGE_MODE_START_STOP    0x01
	/** bit 1 write 0 in #VL53L0X_REG_SYSRANGE_START set single shot mode */
	#define VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT    0x00
	/** bit 1 write 1 in #VL53L0X_REG_SYSRANGE_START set back-to-back
	 *  operation mode */
	#define VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK    0x02
	/** bit 2 write 1 in #VL53L0X_REG_SYSRANGE_START set timed operation
	 *  mode */
	#define VL53L0X_REG_SYSRANGE_MODE_TIMED         0x04
	/** bit 3 write 1 in #VL53L0X_REG_SYSRANGE_START set histogram operation
	 *  mode */
	#define VL53L0X_REG_SYSRANGE_MODE_HISTOGRAM     0x08


#define VL53L0X_REG_SYSTEM_THRESH_HIGH               0x000C
#define VL53L0X_REG_SYSTEM_THRESH_LOW                0x000E


#define VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG		0x0001
#define VL53L0X_REG_SYSTEM_RANGE_CONFIG			0x0009
#define VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD	0x0004


#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO               0x000A
	#define VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_DISABLED	0x00
	#define VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_LEVEL_LOW	0x01
	#define VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_LEVEL_HIGH	0x02
	#define VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_OUT_OF_WINDOW	0x03
	#define VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY	0x04

#define VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH          0x0084


#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR           0x000B

/* Result registers */
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS          0x0013
#define VL53L0X_REG_RESULT_RANGE_STATUS              0x0014

#define VL53L0X_REG_RESULT_CORE_PAGE  1
#define VL53L0X_REG_RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN   0x00BC
#define VL53L0X_REG_RESULT_CORE_RANGING_TOTAL_EVENTS_RTN    0x00C0
#define VL53L0X_REG_RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF   0x00D0
#define VL53L0X_REG_RESULT_CORE_RANGING_TOTAL_EVENTS_REF    0x00D4
#define VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_REF             0x00B6

/* Algo register */

#define VL53L0X_REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM       0x0028

#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS                0x008a

/* Check Limit registers */
#define VL53L0X_REG_MSRC_CONFIG_CONTROL                     0x0060

#define VL53L0X_REG_PRE_RANGE_CONFIG_MIN_SNR                      0X0027
#define VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW              0x0056
#define VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH             0x0057
#define VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT            0x0064

#define VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_SNR                    0X0067
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW            0x0047
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH           0x0048
#define VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT   0x0044


#define VL53L0X_REG_PRE_RANGE_CONFIG_SIGMA_THRESH_HI              0X0061
#define VL53L0X_REG_PRE_RANGE_CONFIG_SIGMA_THRESH_LO              0X0062

/* PRE RANGE registers */
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD                 0x0050
#define VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI            0x0051
#define VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO            0x0052

#define VL53L0X_REG_SYSTEM_HISTOGRAM_BIN                          0x0081
#define VL53L0X_REG_HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT         0x0033
#define VL53L0X_REG_HISTOGRAM_CONFIG_READOUT_CTRL                 0x0055

#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD               0x0070
#define VL53L0X_REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI          0x0071
#define VL53L0X_REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO          0x0072
#define VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS         0x0020

#define VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP                    0x0046


#define VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N	                 0x00bf
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID                       0x00c0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID                    0x00c2

#define VL53L0X_REG_OSC_CALIBRATE_VAL                             0x00f8


#define VL53L0X_SIGMA_ESTIMATE_MAX_VALUE                          65535
/* equivalent to a range sigma of 655.35mm */

#define VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH          0x032
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0   0x0B0
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_1   0x0B1
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_2   0x0B2
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_3   0x0B3
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_4   0x0B4
#define VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_5   0x0B5

#define VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START_SELECT   0xB6
#define VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD 0x4E /* 0x14E */
#define VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET    0x4F /* 0x14F */
#define VL53L0X_REG_POWER_MANAGEMENT_GO1_POWER_FORCE    0x80

/*
 * Speed of light in um per 1E-10 Seconds
 */

#define VL53L0X_SPEED_OF_LIGHT_IN_AIR 2997

#define VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV     0x0089

#define VL53L0X_REG_ALGO_PHASECAL_LIM                         0x0030 /* 0x130 */
#define VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT              0x0030

enum {
	TRACE_LEVEL_NONE,
	TRACE_LEVEL_ERRORS,
	TRACE_LEVEL_WARNING,
	TRACE_LEVEL_INFO,
	TRACE_LEVEL_DEBUG,
	TRACE_LEVEL_ALL,
	TRACE_LEVEL_IGNORE
};

enum {
	TRACE_FUNCTION_NONE = 0,
	TRACE_FUNCTION_I2C  = 1,
	TRACE_FUNCTION_ALL  = 0x7fffffff //all bits except sign
};

enum {
	TRACE_MODULE_NONE              = 0x0,
	TRACE_MODULE_API               = 0x1,
	TRACE_MODULE_PLATFORM          = 0x2,
	TRACE_MODULE_ALL               = 0x7fffffff //all bits except sign
};

#ifdef VL53L0X_LOG_ENABLE

extern uint32_t _trace_level;

int32_t VL53L0X_trace_config(char *filename, uint32_t modules, uint32_t level, uint32_t functions);

void trace_print_module_function(uint32_t module, uint32_t level, uint32_t function, const char *format, ...);

#define LOG_GET_TIME() (int)clock()

#define _LOG_FUNCTION_START(module, fmt, ... ) \
        trace_print_module_function(module, _trace_level, TRACE_FUNCTION_ALL, "%ld <START> %s "fmt"\n", LOG_GET_TIME(), __FUNCTION__, ##__VA_ARGS__);

#define _LOG_FUNCTION_END(module, status, ... )\
        trace_print_module_function(module, _trace_level, TRACE_FUNCTION_ALL, "%ld <END> %s %d\n", LOG_GET_TIME(), __FUNCTION__, (int)status, ##__VA_ARGS__)

#define _LOG_FUNCTION_END_FMT(module, status, fmt, ... )\
        trace_print_module_function(module, _trace_level, TRACE_FUNCTION_ALL, "%ld <END> %s %d "fmt"\n", LOG_GET_TIME(),  __FUNCTION__, (int)status,##__VA_ARGS__)


#else /* VL53L0X_LOG_ENABLE no logging */
    #define VL53L0X_ErrLog(...) (void)0
    #define _LOG_FUNCTION_START(module, fmt, ... ) (void)0
    #define _LOG_FUNCTION_END(module, status, ... ) (void)0
    #define _LOG_FUNCTION_END_FMT(module, status, fmt, ... ) (void)0
#endif /* else */

#define VL53L0X_COPYSTRING(str, ...) strcpy(str, ##__VA_ARGS__)


uint8_t DefaultTuningSettings[] = {

	/* update 02/11/2015_v36 */
	0x01, 0xFF, 0x01,
	0x01, 0x00, 0x00,

	0x01, 0xFF, 0x00,
	0x01, 0x09, 0x00,
	0x01, 0x10, 0x00,
	0x01, 0x11, 0x00,

	0x01, 0x24, 0x01,
	0x01, 0x25, 0xff,
	0x01, 0x75, 0x00,

	0x01, 0xFF, 0x01,
	0x01, 0x4e, 0x2c,
	0x01, 0x48, 0x00,
	0x01, 0x30, 0x20,

	0x01, 0xFF, 0x00,
	0x01, 0x30, 0x09, /* mja changed from 0x64. */
	0x01, 0x54, 0x00,
	0x01, 0x31, 0x04,
	0x01, 0x32, 0x03,
	0x01, 0x40, 0x83,
	0x01, 0x46, 0x25,
	0x01, 0x60, 0x00,
	0x01, 0x27, 0x00,
	0x01, 0x50, 0x06,
	0x01, 0x51, 0x00,
	0x01, 0x52, 0x96,
	0x01, 0x56, 0x08,
	0x01, 0x57, 0x30,
	0x01, 0x61, 0x00,
	0x01, 0x62, 0x00,
	0x01, 0x64, 0x00,
	0x01, 0x65, 0x00,
	0x01, 0x66, 0xa0,

	0x01, 0xFF, 0x01,
	0x01, 0x22, 0x32,
	0x01, 0x47, 0x14,
	0x01, 0x49, 0xff,
	0x01, 0x4a, 0x00,

	0x01, 0xFF, 0x00,
	0x01, 0x7a, 0x0a,
	0x01, 0x7b, 0x00,
	0x01, 0x78, 0x21,

	0x01, 0xFF, 0x01,
	0x01, 0x23, 0x34,
	0x01, 0x42, 0x00,
	0x01, 0x44, 0xff,
	0x01, 0x45, 0x26,
	0x01, 0x46, 0x05,
	0x01, 0x40, 0x40,
	0x01, 0x0E, 0x06,
	0x01, 0x20, 0x1a,
	0x01, 0x43, 0x40,

	0x01, 0xFF, 0x00,
	0x01, 0x34, 0x03,
	0x01, 0x35, 0x44,

	0x01, 0xFF, 0x01,
	0x01, 0x31, 0x04,
	0x01, 0x4b, 0x09,
	0x01, 0x4c, 0x05,
	0x01, 0x4d, 0x04,


	0x01, 0xFF, 0x00,
	0x01, 0x44, 0x00,
	0x01, 0x45, 0x20,
	0x01, 0x47, 0x08,
	0x01, 0x48, 0x28,
	0x01, 0x67, 0x00,
	0x01, 0x70, 0x04,
	0x01, 0x71, 0x01,
	0x01, 0x72, 0xfe,
	0x01, 0x76, 0x00,
	0x01, 0x77, 0x00,

	0x01, 0xFF, 0x01,
	0x01, 0x0d, 0x01,

	0x01, 0xFF, 0x00,
	0x01, 0x80, 0x01,
	0x01, 0x01, 0xF8,

	0x01, 0xFF, 0x01,
	0x01, 0x8e, 0x01,
	0x01, 0x00, 0x01,
	0x01, 0xFF, 0x00,
	0x01, 0x80, 0x00,

	0x00, 0x00, 0x00
};

uint8_t InterruptThresholdSettings[] = {

	/* Start of Interrupt Threshold Settings */
	0x1, 0xff, 0x00,
	0x1, 0x80, 0x01,
	0x1, 0xff, 0x01,
	0x1, 0x00, 0x00,
	0x1, 0xff, 0x01,
	0x1, 0x4f, 0x02,
	0x1, 0xFF, 0x0E,
	0x1, 0x00, 0x03,
	0x1, 0x01, 0x84,
	0x1, 0x02, 0x0A,
	0x1, 0x03, 0x03,
	0x1, 0x04, 0x08,
	0x1, 0x05, 0xC8,
	0x1, 0x06, 0x03,
	0x1, 0x07, 0x8D,
	0x1, 0x08, 0x08,
	0x1, 0x09, 0xC6,
	0x1, 0x0A, 0x01,
	0x1, 0x0B, 0x02,
	0x1, 0x0C, 0x00,
	0x1, 0x0D, 0xD5,
	0x1, 0x0E, 0x18,
	0x1, 0x0F, 0x12,
	0x1, 0x10, 0x01,
	0x1, 0x11, 0x82,
	0x1, 0x12, 0x00,
	0x1, 0x13, 0xD5,
	0x1, 0x14, 0x18,
	0x1, 0x15, 0x13,
	0x1, 0x16, 0x03,
	0x1, 0x17, 0x86,
	0x1, 0x18, 0x0A,
	0x1, 0x19, 0x09,
	0x1, 0x1A, 0x08,
	0x1, 0x1B, 0xC2,
	0x1, 0x1C, 0x03,
	0x1, 0x1D, 0x8F,
	0x1, 0x1E, 0x0A,
	0x1, 0x1F, 0x06,
	0x1, 0x20, 0x01,
	0x1, 0x21, 0x02,
	0x1, 0x22, 0x00,
	0x1, 0x23, 0xD5,
	0x1, 0x24, 0x18,
	0x1, 0x25, 0x22,
	0x1, 0x26, 0x01,
	0x1, 0x27, 0x82,
	0x1, 0x28, 0x00,
	0x1, 0x29, 0xD5,
	0x1, 0x2A, 0x18,
	0x1, 0x2B, 0x0B,
	0x1, 0x2C, 0x28,
	0x1, 0x2D, 0x78,
	0x1, 0x2E, 0x28,
	0x1, 0x2F, 0x91,
	0x1, 0x30, 0x00,
	0x1, 0x31, 0x0B,
	0x1, 0x32, 0x00,
	0x1, 0x33, 0x0B,
	0x1, 0x34, 0x00,
	0x1, 0x35, 0xA1,
	0x1, 0x36, 0x00,
	0x1, 0x37, 0xA0,
	0x1, 0x38, 0x00,
	0x1, 0x39, 0x04,
	0x1, 0x3A, 0x28,
	0x1, 0x3B, 0x30,
	0x1, 0x3C, 0x0C,
	0x1, 0x3D, 0x04,
	0x1, 0x3E, 0x0F,
	0x1, 0x3F, 0x79,
	0x1, 0x40, 0x28,
	0x1, 0x41, 0x1E,
	0x1, 0x42, 0x2F,
	0x1, 0x43, 0x87,
	0x1, 0x44, 0x00,
	0x1, 0x45, 0x0B,
	0x1, 0x46, 0x00,
	0x1, 0x47, 0x0B,
	0x1, 0x48, 0x00,
	0x1, 0x49, 0xA7,
	0x1, 0x4A, 0x00,
	0x1, 0x4B, 0xA6,
	0x1, 0x4C, 0x00,
	0x1, 0x4D, 0x04,
	0x1, 0x4E, 0x01,
	0x1, 0x4F, 0x00,
	0x1, 0x50, 0x00,
	0x1, 0x51, 0x80,
	0x1, 0x52, 0x09,
	0x1, 0x53, 0x08,
	0x1, 0x54, 0x01,
	0x1, 0x55, 0x00,
	0x1, 0x56, 0x0F,
	0x1, 0x57, 0x79,
	0x1, 0x58, 0x09,
	0x1, 0x59, 0x05,
	0x1, 0x5A, 0x00,
	0x1, 0x5B, 0x60,
	0x1, 0x5C, 0x05,
	0x1, 0x5D, 0xD1,
	0x1, 0x5E, 0x0C,
	0x1, 0x5F, 0x3C,
	0x1, 0x60, 0x00,
	0x1, 0x61, 0xD0,
	0x1, 0x62, 0x0B,
	0x1, 0x63, 0x03,
	0x1, 0x64, 0x28,
	0x1, 0x65, 0x10,
	0x1, 0x66, 0x2A,
	0x1, 0x67, 0x39,
	0x1, 0x68, 0x0B,
	0x1, 0x69, 0x02,
	0x1, 0x6A, 0x28,
	0x1, 0x6B, 0x10,
	0x1, 0x6C, 0x2A,
	0x1, 0x6D, 0x61,
	0x1, 0x6E, 0x0C,
	0x1, 0x6F, 0x00,
	0x1, 0x70, 0x0F,
	0x1, 0x71, 0x79,
	0x1, 0x72, 0x00,
	0x1, 0x73, 0x0B,
	0x1, 0x74, 0x00,
	0x1, 0x75, 0x0B,
	0x1, 0x76, 0x00,
	0x1, 0x77, 0xA1,
	0x1, 0x78, 0x00,
	0x1, 0x79, 0xA0,
	0x1, 0x7A, 0x00,
	0x1, 0x7B, 0x04,
	0x1, 0xFF, 0x04,
	0x1, 0x79, 0x1D,
	0x1, 0x7B, 0x27,
	0x1, 0x96, 0x0E,
	0x1, 0x97, 0xFE,
	0x1, 0x98, 0x03,
	0x1, 0x99, 0xEF,
	0x1, 0x9A, 0x02,
	0x1, 0x9B, 0x44,
	0x1, 0x73, 0x07,
	0x1, 0x70, 0x01,
	0x1, 0xff, 0x01,
	0x1, 0x00, 0x01,
	0x1, 0xff, 0x00,
	0x00, 0x00, 0x00
};


#ifdef _MSC_VER
#   ifdef VL53L0X_API_EXPORTS
#       define VL53L0X_API  __declspec(dllexport)
#   else
#       define VL53L0X_API
#   endif
#else
#   define VL53L0X_API
#endif

VL53L0X_Error VL53L0X_get_device_info(VL53L0X_DEV Dev,
			VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo);
VL53L0X_Error VL53L0X_get_device_error_string(VL53L0X_DeviceError ErrorCode,
		char *pDeviceErrorString);
VL53L0X_Error VL53L0X_get_range_status_string(uint8_t RangeStatus,
		char *pRangeStatusString);
VL53L0X_Error VL53L0X_get_pal_error_string(VL53L0X_Error PalErrorCode,
		char *pPalErrorString);
VL53L0X_Error VL53L0X_get_pal_state_string(VL53L0X_State PalStateCode,
		char *pPalStateString);
VL53L0X_Error VL53L0X_get_sequence_steps_info(
		VL53L0X_SequenceStepId SequenceStepId,
		char *pSequenceStepsString);
VL53L0X_Error VL53L0X_get_limit_check_info(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	char *pLimitCheckString);
VL53L0X_API VL53L0X_Error VL53L0X_GetVersion(VL53L0X_Version_t *pVersion);

VL53L0X_API VL53L0X_Error VL53L0X_GetPalSpecVersion(
	VL53L0X_Version_t *pPalSpecVersion);
VL53L0X_API VL53L0X_Error VL53L0X_GetProductRevision(VL53L0X_DEV Dev,
	uint8_t *pProductRevisionMajor, uint8_t *pProductRevisionMinor);
VL53L0X_API VL53L0X_Error VL53L0X_GetDeviceInfo(VL53L0X_DEV Dev,
	VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo);
VL53L0X_API VL53L0X_Error VL53L0X_GetDeviceErrorStatus(VL53L0X_DEV Dev,
	VL53L0X_DeviceError *pDeviceErrorStatus);
VL53L0X_API VL53L0X_Error VL53L0X_GetRangeStatusString(uint8_t RangeStatus,
	char *pRangeStatusString);
VL53L0X_API VL53L0X_Error VL53L0X_GetDeviceErrorString(
	VL53L0X_DeviceError ErrorCode, char *pDeviceErrorString);
VL53L0X_API VL53L0X_Error VL53L0X_GetPalErrorString(VL53L0X_Error PalErrorCode,
	char *pPalErrorString);
VL53L0X_API VL53L0X_Error VL53L0X_GetPalStateString(VL53L0X_State PalStateCode,
	char *pPalStateString);
VL53L0X_API VL53L0X_Error VL53L0X_GetPalState(VL53L0X_DEV Dev,
	VL53L0X_State *pPalState);
VL53L0X_API VL53L0X_Error VL53L0X_SetPowerMode(VL53L0X_DEV Dev,
	VL53L0X_PowerModes PowerMode);
VL53L0X_API VL53L0X_Error VL53L0X_GetPowerMode(VL53L0X_DEV Dev,
	VL53L0X_PowerModes *pPowerMode);
VL53L0X_API VL53L0X_Error VL53L0X_SetOffsetCalibrationDataMicroMeter(
	VL53L0X_DEV Dev, int32_t OffsetCalibrationDataMicroMeter);
VL53L0X_API VL53L0X_Error VL53L0X_GetOffsetCalibrationDataMicroMeter(
	VL53L0X_DEV Dev, int32_t *pOffsetCalibrationDataMicroMeter);
VL53L0X_API VL53L0X_Error VL53L0X_SetLinearityCorrectiveGain(VL53L0X_DEV Dev,
	int16_t LinearityCorrectiveGain);
VL53L0X_API VL53L0X_Error VL53L0X_GetLinearityCorrectiveGain(VL53L0X_DEV Dev,
	uint16_t *pLinearityCorrectiveGain);
VL53L0X_API VL53L0X_Error VL53L0X_SetGroupParamHold(VL53L0X_DEV Dev,
	uint8_t GroupParamHold);
VL53L0X_API VL53L0X_Error VL53L0X_GetUpperLimitMilliMeter(VL53L0X_DEV Dev,
	uint16_t *pUpperLimitMilliMeter);
VL53L0X_Error VL53L0X_GetTotalSignalRate(VL53L0X_DEV Dev,
	FixPoint1616_t *pTotalSignalRate);
VL53L0X_API VL53L0X_Error VL53L0X_SetDeviceAddress(VL53L0X_DEV Dev,
	uint8_t DeviceAddress);
VL53L0X_API VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_SetTuningSettingBuffer(VL53L0X_DEV Dev,
	uint8_t *pTuningSettingBuffer, uint8_t UseInternalTuningSettings);
VL53L0X_API VL53L0X_Error VL53L0X_GetTuningSettingBuffer(VL53L0X_DEV Dev,
	uint8_t **ppTuningSettingBuffer, uint8_t *pUseInternalTuningSettings);
VL53L0X_API VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_WaitDeviceBooted(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_ResetDevice(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_SetDeviceParameters(VL53L0X_DEV Dev,
	const VL53L0X_DeviceParameters_t *pDeviceParameters);
VL53L0X_API VL53L0X_Error VL53L0X_GetDeviceParameters(VL53L0X_DEV Dev,
	VL53L0X_DeviceParameters_t *pDeviceParameters);
VL53L0X_API VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes DeviceMode);
VL53L0X_API VL53L0X_Error VL53L0X_GetDeviceMode(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes *pDeviceMode);
VL53L0X_API VL53L0X_Error VL53L0X_SetRangeFractionEnable(VL53L0X_DEV Dev,
	uint8_t Enable);
VL53L0X_API VL53L0X_Error VL53L0X_GetFractionEnable(VL53L0X_DEV Dev,
	uint8_t *pEnable);
VL53L0X_API VL53L0X_Error VL53L0X_SetHistogramMode(VL53L0X_DEV Dev,
	VL53L0X_HistogramModes HistogramMode);
VL53L0X_API VL53L0X_Error VL53L0X_GetHistogramMode(VL53L0X_DEV Dev,
	VL53L0X_HistogramModes *pHistogramMode);
VL53L0X_API VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
	VL53L0X_DEV Dev, uint32_t MeasurementTimingBudgetMicroSeconds);
VL53L0X_API VL53L0X_Error VL53L0X_GetMeasurementTimingBudgetMicroSeconds(
	VL53L0X_DEV Dev, uint32_t *pMeasurementTimingBudgetMicroSeconds);
VL53L0X_API VL53L0X_Error VL53L0X_GetVcselPulsePeriod(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t *pVCSELPulsePeriod);
VL53L0X_API VL53L0X_Error VL53L0X_SetVcselPulsePeriod(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t VCSELPulsePeriod);
VL53L0X_API VL53L0X_Error VL53L0X_SetSequenceStepEnable(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, uint8_t SequenceStepEnabled);
VL53L0X_API VL53L0X_Error VL53L0X_GetSequenceStepEnable(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, uint8_t *pSequenceStepEnabled);
VL53L0X_API VL53L0X_Error VL53L0X_GetSequenceStepEnables(VL53L0X_DEV Dev,
	VL53L0X_SchedulerSequenceSteps_t *pSchedulerSequenceSteps);
VL53L0X_API VL53L0X_Error VL53L0X_SetSequenceStepTimeout(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, FixPoint1616_t TimeOutMilliSecs);
VL53L0X_API VL53L0X_Error VL53L0X_GetSequenceStepTimeout(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId,
	FixPoint1616_t *pTimeOutMilliSecs);
VL53L0X_API VL53L0X_Error VL53L0X_GetNumberOfSequenceSteps(VL53L0X_DEV Dev,
	uint8_t *pNumberOfSequenceSteps);
VL53L0X_API VL53L0X_Error VL53L0X_GetSequenceStepsInfo(
	VL53L0X_SequenceStepId SequenceStepId, char *pSequenceStepsString);
VL53L0X_API VL53L0X_Error VL53L0X_SetInterMeasurementPeriodMilliSeconds(
	VL53L0X_DEV Dev, uint32_t InterMeasurementPeriodMilliSeconds);
VL53L0X_API VL53L0X_Error VL53L0X_GetInterMeasurementPeriodMilliSeconds(
	VL53L0X_DEV Dev, uint32_t *pInterMeasurementPeriodMilliSeconds);
VL53L0X_API VL53L0X_Error VL53L0X_SetXTalkCompensationEnable(VL53L0X_DEV Dev,
	uint8_t XTalkCompensationEnable);
VL53L0X_API VL53L0X_Error VL53L0X_GetXTalkCompensationEnable(VL53L0X_DEV Dev,
	uint8_t *pXTalkCompensationEnable);
VL53L0X_API VL53L0X_Error VL53L0X_SetXTalkCompensationRateMegaCps(VL53L0X_DEV Dev,
	FixPoint1616_t XTalkCompensationRateMegaCps);
VL53L0X_API VL53L0X_Error VL53L0X_GetXTalkCompensationRateMegaCps(VL53L0X_DEV Dev,
	FixPoint1616_t *pXTalkCompensationRateMegaCps);
VL53L0X_API VL53L0X_Error VL53L0X_SetRefCalibration(VL53L0X_DEV Dev,
	uint8_t VhvSettings, uint8_t PhaseCal);
VL53L0X_API VL53L0X_Error VL53L0X_GetRefCalibration(VL53L0X_DEV Dev,
	uint8_t *pVhvSettings, uint8_t *pPhaseCal);
VL53L0X_API VL53L0X_Error VL53L0X_GetNumberOfLimitCheck(
	uint16_t *pNumberOfLimitCheck);
VL53L0X_API VL53L0X_Error VL53L0X_GetLimitCheckInfo(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, char *pLimitCheckString);
VL53L0X_API VL53L0X_Error VL53L0X_GetLimitCheckStatus(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, uint8_t *pLimitCheckStatus);
VL53L0X_API VL53L0X_Error VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, uint8_t LimitCheckEnable);
VL53L0X_API VL53L0X_Error VL53L0X_GetLimitCheckEnable(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, uint8_t *pLimitCheckEnable);
VL53L0X_API VL53L0X_Error VL53L0X_SetLimitCheckValue(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t LimitCheckValue);
VL53L0X_API VL53L0X_Error VL53L0X_GetLimitCheckValue(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckValue);
VL53L0X_API VL53L0X_Error VL53L0X_GetLimitCheckCurrent(VL53L0X_DEV Dev,
	uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckCurrent);
VL53L0X_API VL53L0X_Error VL53L0X_SetWrapAroundCheckEnable(VL53L0X_DEV Dev,
		uint8_t WrapAroundCheckEnable);
VL53L0X_API VL53L0X_Error VL53L0X_GetWrapAroundCheckEnable(VL53L0X_DEV Dev,
		uint8_t *pWrapAroundCheckEnable);
VL53L0X_API VL53L0X_Error VL53L0X_SetDmaxCalParameters(VL53L0X_DEV Dev,
		uint16_t RangeMilliMeter, FixPoint1616_t SignalRateRtnMegaCps);
VL53L0X_API VL53L0X_Error VL53L0X_GetDmaxCalParameters(VL53L0X_DEV Dev,
	uint16_t *pRangeMilliMeter, FixPoint1616_t *pSignalRateRtnMegaCps);
VL53L0X_API VL53L0X_Error VL53L0X_PerformSingleMeasurement(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_PerformRefCalibration(VL53L0X_DEV Dev,
	uint8_t *pVhvSettings, uint8_t *pPhaseCal);
VL53L0X_API VL53L0X_Error VL53L0X_PerformXTalkMeasurement(VL53L0X_DEV Dev,
	uint32_t TimeoutMs, FixPoint1616_t *pXtalkPerSpad,
	uint8_t *pAmbientTooHigh);
VL53L0X_API VL53L0X_Error VL53L0X_PerformXTalkCalibration(VL53L0X_DEV Dev,
	FixPoint1616_t XTalkCalDistance,
	FixPoint1616_t *pXTalkCompensationRateMegaCps);
VL53L0X_API VL53L0X_Error VL53L0X_PerformOffsetCalibration(VL53L0X_DEV Dev,
	FixPoint1616_t CalDistanceMilliMeter, int32_t *pOffsetMicroMeter);
VL53L0X_API VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev);
VL53L0X_API VL53L0X_Error VL53L0X_GetMeasurementDataReady(VL53L0X_DEV Dev,
	uint8_t *pMeasurementDataReady);
VL53L0X_API VL53L0X_Error VL53L0X_WaitDeviceReadyForNewMeasurement(VL53L0X_DEV Dev,
	uint32_t MaxLoop);
VL53L0X_API VL53L0X_Error VL53L0X_GetMeasurementRefSignal(VL53L0X_DEV Dev,
	FixPoint1616_t *pMeasurementRefSignal);
VL53L0X_API VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData);
VL53L0X_API VL53L0X_Error VL53L0X_GetHistogramMeasurementData(VL53L0X_DEV Dev,
	VL53L0X_HistogramMeasurementData_t *pHistogramMeasurementData);
VL53L0X_API VL53L0X_Error VL53L0X_PerformSingleRangingMeasurement(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData);
VL53L0X_API VL53L0X_Error VL53L0X_PerformSingleHistogramMeasurement(VL53L0X_DEV Dev,
	VL53L0X_HistogramMeasurementData_t *pHistogramMeasurementData);
VL53L0X_API VL53L0X_Error VL53L0X_SetNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t NumberOfROIZones);
VL53L0X_API VL53L0X_Error VL53L0X_GetNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t *pNumberOfROIZones);
VL53L0X_API VL53L0X_Error VL53L0X_GetMaxNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t *pMaxNumberOfROIZones);
VL53L0X_API VL53L0X_Error VL53L0X_SetGpioConfig(VL53L0X_DEV Dev, uint8_t Pin,
	VL53L0X_DeviceModes DeviceMode, VL53L0X_GpioFunctionality Functionality,
	VL53L0X_InterruptPolarity Polarity);
VL53L0X_API VL53L0X_Error VL53L0X_GetGpioConfig(VL53L0X_DEV Dev, uint8_t Pin,
	VL53L0X_DeviceModes *pDeviceMode,
	VL53L0X_GpioFunctionality *pFunctionality,
	VL53L0X_InterruptPolarity *pPolarity);
VL53L0X_API VL53L0X_Error VL53L0X_SetInterruptThresholds(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes DeviceMode, FixPoint1616_t ThresholdLow,
	FixPoint1616_t ThresholdHigh);
VL53L0X_API VL53L0X_Error VL53L0X_GetInterruptThresholds(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes DeviceMode, FixPoint1616_t *pThresholdLow,
	FixPoint1616_t *pThresholdHigh);
VL53L0X_API VL53L0X_Error VL53L0X_GetStopCompletedStatus(VL53L0X_DEV Dev,
	uint32_t *pStopStatus);
VL53L0X_API VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev,
	uint32_t InterruptMask);
VL53L0X_API VL53L0X_Error VL53L0X_GetInterruptMaskStatus(VL53L0X_DEV Dev,
	uint32_t *pInterruptMaskStatus);
VL53L0X_API VL53L0X_Error VL53L0X_EnableInterruptMask(VL53L0X_DEV Dev,
	uint32_t InterruptMask);
VL53L0X_API VL53L0X_Error VL53L0X_SetSpadAmbientDamperThreshold(VL53L0X_DEV Dev,
	uint16_t SpadAmbientDamperThreshold);
VL53L0X_API VL53L0X_Error VL53L0X_GetSpadAmbientDamperThreshold(VL53L0X_DEV Dev,
	uint16_t *pSpadAmbientDamperThreshold);
VL53L0X_API VL53L0X_Error VL53L0X_SetSpadAmbientDamperFactor(VL53L0X_DEV Dev,
	uint16_t SpadAmbientDamperFactor);
VL53L0X_API VL53L0X_Error VL53L0X_GetSpadAmbientDamperFactor(VL53L0X_DEV Dev,
	uint16_t *pSpadAmbientDamperFactor);
VL53L0X_API VL53L0X_Error VL53L0X_PerformRefSpadManagement(VL53L0X_DEV Dev,
	uint32_t *refSpadCount, uint8_t *isApertureSpads);
VL53L0X_API VL53L0X_Error VL53L0X_SetReferenceSpads(VL53L0X_DEV Dev,
	uint32_t refSpadCount, uint8_t isApertureSpads);
VL53L0X_API VL53L0X_Error VL53L0X_GetReferenceSpads(VL53L0X_DEV Dev,
	uint32_t *refSpadCount, uint8_t *isApertureSpads);
VL53L0X_Error VL53L0X_perform_phase_calibration(VL53L0X_DEV Dev,
	uint8_t *pPhaseCal, const uint8_t get_data_enable,
	const uint8_t restore_config);
VL53L0X_Error VL53L0X_perform_ref_calibration(VL53L0X_DEV Dev,
	uint8_t *pVhvSettings, uint8_t *pPhaseCal, uint8_t get_data_enable);
VL53L0X_Error VL53L0X_set_offset_calibration_data_micro_meter(VL53L0X_DEV Dev,
		int32_t OffsetCalibrationDataMicroMeter);
VL53L0X_Error VL53L0X_perform_ref_spad_management(VL53L0X_DEV Dev,
				uint32_t *refSpadCount,
				uint8_t *isApertureSpads);
VL53L0X_Error VL53L0X_set_reference_spads(VL53L0X_DEV Dev,
				 uint32_t count, uint8_t isApertureSpads);
VL53L0X_Error VL53L0X_get_offset_calibration_data_micro_meter(VL53L0X_DEV Dev,
		int32_t *pOffsetCalibrationDataMicroMeter);
VL53L0X_Error VL53L0X_set_ref_calibration(VL53L0X_DEV Dev,
		uint8_t VhvSettings, uint8_t PhaseCal);
VL53L0X_Error VL53L0X_get_ref_calibration(VL53L0X_DEV Dev,
		uint8_t *pVhvSettings, uint8_t *pPhaseCal);
VL53L0X_Error VL53L0X_perform_xtalk_calibration(VL53L0X_DEV Dev,
			FixPoint1616_t XTalkCalDistance,
			FixPoint1616_t *pXTalkCompensationRateMegaCps);
VL53L0X_Error VL53L0X_get_reference_spads(VL53L0X_DEV Dev,
			uint32_t *pSpadCount, uint8_t *pIsApertureSpads);
VL53L0X_Error VL53L0X_perform_offset_calibration(VL53L0X_DEV Dev,
			FixPoint1616_t CalDistanceMilliMeter,
			int32_t *pOffsetMicroMeter);
uint32_t refArrayQuadrants[4] = {REF_ARRAY_SPAD_10, REF_ARRAY_SPAD_5,
		REF_ARRAY_SPAD_0, REF_ARRAY_SPAD_5 };
VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev);
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count);
VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data);
VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data);
VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data);
VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data);
VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data);
VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data);
VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData);
VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev); /* usually best implemented as a real function */
VL53L0X_Error VL53L0X_reverse_bytes(uint8_t *data, uint32_t size);
VL53L0X_Error VL53L0X_measurement_poll_for_completion(VL53L0X_DEV Dev);
uint8_t VL53L0X_encode_vcsel_period(uint8_t vcsel_period_pclks);
uint8_t VL53L0X_decode_vcsel_period(uint8_t vcsel_period_reg);
uint32_t VL53L0X_isqrt(uint32_t num);
uint32_t VL53L0X_quadrature_sum(uint32_t a, uint32_t b);
VL53L0X_Error VL53L0X_get_info_from_device(VL53L0X_DEV Dev, uint8_t option);
VL53L0X_Error VL53L0X_set_vcsel_pulse_period(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t VCSELPulsePeriodPCLK);
VL53L0X_Error VL53L0X_get_vcsel_pulse_period(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t *pVCSELPulsePeriodPCLK);
uint32_t VL53L0X_decode_timeout(uint16_t encoded_timeout);
VL53L0X_Error get_sequence_step_timeout(VL53L0X_DEV Dev,
			VL53L0X_SequenceStepId SequenceStepId,
			uint32_t *pTimeOutMicroSecs);
VL53L0X_Error set_sequence_step_timeout(VL53L0X_DEV Dev,
			VL53L0X_SequenceStepId SequenceStepId,
			uint32_t TimeOutMicroSecs);
VL53L0X_Error VL53L0X_set_measurement_timing_budget_micro_seconds(VL53L0X_DEV Dev,
	uint32_t MeasurementTimingBudgetMicroSeconds);
VL53L0X_Error VL53L0X_get_measurement_timing_budget_micro_seconds(VL53L0X_DEV Dev,
		uint32_t *pMeasurementTimingBudgetMicroSeconds);
VL53L0X_Error VL53L0X_load_tuning_settings(VL53L0X_DEV Dev,
		uint8_t *pTuningSettingBuffer);
VL53L0X_Error VL53L0X_calc_sigma_estimate(VL53L0X_DEV Dev,
		VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
		FixPoint1616_t *pSigmaEstimate, uint32_t *pDmax_mm);
VL53L0X_Error VL53L0X_get_total_xtalk_rate(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
	FixPoint1616_t *ptotal_xtalk_rate_mcps);
VL53L0X_Error VL53L0X_get_total_signal_rate(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
	FixPoint1616_t *ptotal_signal_rate_mcps);
VL53L0X_Error VL53L0X_get_pal_range_status(VL53L0X_DEV Dev,
		 uint8_t DeviceRangeStatus,
		 FixPoint1616_t SignalRate,
		 uint16_t EffectiveSpadRtnCount,
		 VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
		 uint8_t *pPalRangeStatus);
uint32_t VL53L0X_calc_timeout_mclks(VL53L0X_DEV Dev,
	uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
uint16_t VL53L0X_encode_timeout(uint32_t timeout_macro_clks);


int VL53L0X_i2c_init(void);
int VL53L0X_write_multi(uint8_t deviceAddress, uint8_t index, uint8_t *pdata, uint32_t count);
int VL53L0X_read_multi(uint8_t deviceAddress, uint8_t index, uint8_t *pdata, uint32_t count);
int VL53L0X_write_byte(uint8_t deviceAddress, uint8_t index, uint8_t data);
int VL53L0X_write_word(uint8_t deviceAddress, uint8_t index, uint16_t data);
int VL53L0X_write_dword(uint8_t deviceAddress, uint8_t index, uint32_t data);
int VL53L0X_read_byte(uint8_t deviceAddress, uint8_t index, uint8_t *data);
int VL53L0X_read_word(uint8_t deviceAddress, uint8_t index, uint16_t *data);
int VL53L0X_read_dword(uint8_t deviceAddress, uint8_t index, uint32_t *data);

/***************************************************i2c comms******************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

int VL53L0X_i2c_init(void) 
{
	printk(KERN_INFO "init i2c,i2c addr = %d\n",vl53l0x_client->addr);
	return VL53L0X_ERROR_NONE;
}

int VL53L0X_write_multi(uint8_t deviceAddress, uint8_t index, uint8_t *pdata, uint32_t count) 
{
	deviceAddress = deviceAddress;
	if(i2c_smbus_write_i2c_block_data(vl53l0x_client,index,count,pdata)){
		printk(KERN_ALERT "Smbus send bytes failed\n");
	}
	return VL53L0X_ERROR_NONE;
}

int VL53L0X_write_byte(uint8_t deviceAddress, uint8_t index, uint8_t data) 
{
	// return VL53L0X_write_multi(deviceAddress, index, &data, 1);
	if(i2c_smbus_write_byte_data(vl53l0x_client,index,data)){
		printk(KERN_ALERT "Smbus send byte failed\n");
	}
	return VL53L0X_ERROR_NONE;
}

int VL53L0X_write_word(uint8_t deviceAddress, uint8_t index, uint16_t data) 
{

	uint8_t buff[2];
	buff[1] = data & 0xFF;
	buff[0] = data >> 8;
	return VL53L0X_write_multi(deviceAddress, index, buff, 2);
}

int VL53L0X_write_dword(uint8_t deviceAddress, uint8_t index, uint32_t data) 
{
	uint8_t buff[4];

	buff[3] = data & 0xFF;
	buff[2] = data >> 8;
	buff[1] = data >> 16;
	buff[0] = data >> 24;

	return VL53L0X_write_multi(deviceAddress, index, buff, 4);
}

int VL53L0X_read_multi(uint8_t deviceAddress, uint8_t index, uint8_t *pdata, uint32_t count) 
{
	if(i2c_smbus_read_i2c_block_data(vl53l0x_client,index,count,pdata) < 0 ){
		printk(KERN_ALERT "Smbus read mutil failed\n");
		return VL53L0X_ERROR_UNDEFINED;
	}
	return VL53L0X_ERROR_NONE;
}

int VL53L0X_read_byte(uint8_t deviceAddress, uint8_t index, uint8_t *data) 
{
	return VL53L0X_read_multi(0, index, data, 1);
	// uint8_t read_data = 0;
	// read_data = i2c_smbus_read_byte_data(vl53l0x_client,index);
	// if(read_data < 0){
	//   printk(KERN_ALERT "Smbus read byte failed\n");
	// }
	// *data = read_data ;
	// return VL53L0X_ERROR_NONE;
}

int VL53L0X_read_word(uint8_t deviceAddress, uint8_t index, uint16_t *data) 
{
	uint8_t buff[2];
	int r = VL53L0X_read_multi(deviceAddress, index, buff, 2);

	uint16_t tmp;
	tmp = buff[0];
	tmp <<= 8;
	tmp |= buff[1];
	*data = tmp;
	return r;
}

int VL53L0X_read_dword(uint8_t deviceAddress, uint8_t index, uint32_t *data) 
{
	uint8_t buff[4];
	int r = VL53L0X_read_multi(deviceAddress, index, buff, 4);

	uint32_t tmp;
	tmp = buff[0];
	tmp <<= 8;
	tmp |= buff[1];
	tmp <<= 8;
	tmp |= buff[2];
	tmp <<= 8;
	tmp |= buff[3];
	*data = tmp;

	return r;
}

/**************************************************************************************************************/
/**************************************************platform****************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	return Status;
}

VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	return Status;
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count){

	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int = 0;
	uint8_t deviceAddress;

	if (count>=VL53L0X_MAX_I2C_XFER_SIZE){
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}

	deviceAddress = Dev->I2cDevAddr;
	status_int = VL53L0X_write_multi(deviceAddress, index, pdata, count);
	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;
	return Status;
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
	VL53L0X_I2C_USER_VAR
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	if (count>=VL53L0X_MAX_I2C_XFER_SIZE){
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}
	deviceAddress = Dev->I2cDevAddr;
	status_int = VL53L0X_read_multi(deviceAddress, index, pdata, count);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;
	return Status;
}


VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_write_byte(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_write_word(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_write_dword(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;
	uint8_t data;

	deviceAddress = Dev->I2cDevAddr;
	status_int = VL53L0X_read_byte(deviceAddress, index, &data);

	if (status_int != 0)
	Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	if (Status == VL53L0X_ERROR_NONE) {
		data = (data & AndData) | OrData;
		status_int = VL53L0X_write_byte(deviceAddress, index, data);

		if (status_int != 0)
			Status = VL53L0X_ERROR_CONTROL_INTERFACE;
	}
	return Status;
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_read_byte(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_read_word(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

VL53L0X_Error  VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;
	uint8_t deviceAddress;

	deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_read_dword(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

	return Status;
}

#define VL53L0X_POLLINGDELAY_LOOPNB  250
VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev)
{
	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	volatile uint32_t i;
	LOG_FUNCTION_START("");

	for(i=0;i<VL53L0X_POLLINGDELAY_LOOPNB;i++){
		asm("nop");
	}

	LOG_FUNCTION_END(status);
	return status;
}


/**************************************************api_strings*************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/

VL53L0X_Error VL53L0X_check_part_used(VL53L0X_DEV Dev,
		uint8_t *Revision,
		VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t ModuleIdInt;
	char *ProductId_tmp;

	LOG_FUNCTION_START("");

	Status = VL53L0X_get_info_from_device(Dev, 2);

	if (Status == VL53L0X_ERROR_NONE) {
		ModuleIdInt = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev, ModuleId);

	if (ModuleIdInt == 0) {
		*Revision = 0;
		VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->ProductId, "");
	} else {
		*Revision = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev, Revision);
		ProductId_tmp = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
			ProductId);
		VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->ProductId, ProductId_tmp);
	}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}


VL53L0X_Error VL53L0X_get_device_info(VL53L0X_DEV Dev,
				VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t revision_id;
	uint8_t Revision;

	Status = VL53L0X_check_part_used(Dev, &Revision, pVL53L0X_DeviceInfo);
	if (Status == VL53L0X_ERROR_NONE) {
		if (Revision == 0) {
			VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->Name,
					VL53L0X_STRING_DEVICE_INFO_NAME_TS0);
		} else if ((Revision <= 34) && (Revision != 32)) {
			VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->Name,
					VL53L0X_STRING_DEVICE_INFO_NAME_TS1);
		} else if (Revision < 39) {
			VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->Name,
					VL53L0X_STRING_DEVICE_INFO_NAME_TS2);
		} else {
			VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->Name,
					VL53L0X_STRING_DEVICE_INFO_NAME_ES1);
		}

		VL53L0X_COPYSTRING(pVL53L0X_DeviceInfo->Type,
				VL53L0X_STRING_DEVICE_INFO_TYPE);

	}

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_RdByte(Dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID,
				&pVL53L0X_DeviceInfo->ProductType);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_IDENTIFICATION_REVISION_ID,
				&revision_id);
		pVL53L0X_DeviceInfo->ProductRevisionMajor = 1;
		pVL53L0X_DeviceInfo->ProductRevisionMinor =
					(revision_id & 0xF0) >> 4;
	}

	return Status;
}


VL53L0X_Error VL53L0X_get_device_error_string(VL53L0X_DeviceError ErrorCode,
		char *pDeviceErrorString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	switch (ErrorCode) {
	case VL53L0X_DEVICEERROR_NONE:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_NONE);
	break;
	case VL53L0X_DEVICEERROR_VCSELCONTINUITYTESTFAILURE:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_VCSELCONTINUITYTESTFAILURE);
	break;
	case VL53L0X_DEVICEERROR_VCSELWATCHDOGTESTFAILURE:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_VCSELWATCHDOGTESTFAILURE);
	break;
	case VL53L0X_DEVICEERROR_NOVHVVALUEFOUND:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_NOVHVVALUEFOUND);
	break;
	case VL53L0X_DEVICEERROR_MSRCNOTARGET:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_MSRCNOTARGET);
	break;
	case VL53L0X_DEVICEERROR_SNRCHECK:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_SNRCHECK);
	break;
	case VL53L0X_DEVICEERROR_RANGEPHASECHECK:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_RANGEPHASECHECK);
	break;
	case VL53L0X_DEVICEERROR_SIGMATHRESHOLDCHECK:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_SIGMATHRESHOLDCHECK);
	break;
	case VL53L0X_DEVICEERROR_TCC:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_TCC);
	break;
	case VL53L0X_DEVICEERROR_PHASECONSISTENCY:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_PHASECONSISTENCY);
	break;
	case VL53L0X_DEVICEERROR_MINCLIP:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_MINCLIP);
	break;
	case VL53L0X_DEVICEERROR_RANGECOMPLETE:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_RANGECOMPLETE);
	break;
	case VL53L0X_DEVICEERROR_ALGOUNDERFLOW:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_ALGOUNDERFLOW);
	break;
	case VL53L0X_DEVICEERROR_ALGOOVERFLOW:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_ALGOOVERFLOW);
	break;
	case VL53L0X_DEVICEERROR_RANGEIGNORETHRESHOLD:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_DEVICEERROR_RANGEIGNORETHRESHOLD);
	break;

	default:
		VL53L0X_COPYSTRING(pDeviceErrorString,
			VL53L0X_STRING_UNKNOW_ERROR_CODE);

	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_range_status_string(uint8_t RangeStatus,
		char *pRangeStatusString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	switch (RangeStatus) {
	case 0:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_RANGEVALID);
	break;
	case 1:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_SIGMA);
	break;
	case 2:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_SIGNAL);
	break;
	case 3:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_MINRANGE);
	break;
	case 4:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_PHASE);
	break;
	case 5:
		VL53L0X_COPYSTRING(pRangeStatusString,
			VL53L0X_STRING_RANGESTATUS_HW);
	break;

	default: /**/
		VL53L0X_COPYSTRING(pRangeStatusString,
				VL53L0X_STRING_RANGESTATUS_NONE);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_pal_error_string(VL53L0X_Error PalErrorCode,
		char *pPalErrorString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	switch (PalErrorCode) {
	case VL53L0X_ERROR_NONE:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_NONE);
	break;
	case VL53L0X_ERROR_CALIBRATION_WARNING:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_CALIBRATION_WARNING);
	break;
	case VL53L0X_ERROR_MIN_CLIPPED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_MIN_CLIPPED);
	break;
	case VL53L0X_ERROR_UNDEFINED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_UNDEFINED);
	break;
	case VL53L0X_ERROR_INVALID_PARAMS:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_INVALID_PARAMS);
	break;
	case VL53L0X_ERROR_NOT_SUPPORTED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_NOT_SUPPORTED);
	break;
	case VL53L0X_ERROR_INTERRUPT_NOT_CLEARED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_INTERRUPT_NOT_CLEARED);
	break;
	case VL53L0X_ERROR_RANGE_ERROR:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_RANGE_ERROR);
	break;
	case VL53L0X_ERROR_TIME_OUT:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_TIME_OUT);
	break;
	case VL53L0X_ERROR_MODE_NOT_SUPPORTED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_MODE_NOT_SUPPORTED);
	break;
	case VL53L0X_ERROR_BUFFER_TOO_SMALL:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_BUFFER_TOO_SMALL);
	break;
	case VL53L0X_ERROR_GPIO_NOT_EXISTING:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_GPIO_NOT_EXISTING);
	break;
	case VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED);
	break;
	case VL53L0X_ERROR_CONTROL_INTERFACE:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_CONTROL_INTERFACE);
	break;
	case VL53L0X_ERROR_INVALID_COMMAND:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_INVALID_COMMAND);
	break;
	case VL53L0X_ERROR_DIVISION_BY_ZERO:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_DIVISION_BY_ZERO);
	break;
	case VL53L0X_ERROR_REF_SPAD_INIT:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_REF_SPAD_INIT);
	break;
	case VL53L0X_ERROR_NOT_IMPLEMENTED:
		VL53L0X_COPYSTRING(pPalErrorString,
			VL53L0X_STRING_ERROR_NOT_IMPLEMENTED);
	break;

	default:
		VL53L0X_COPYSTRING(pPalErrorString,
				VL53L0X_STRING_UNKNOW_ERROR_CODE);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_pal_state_string(VL53L0X_State PalStateCode,
		char *pPalStateString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	switch (PalStateCode) {
	case VL53L0X_STATE_POWERDOWN:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_POWERDOWN);
	break;
	case VL53L0X_STATE_WAIT_STATICINIT:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_WAIT_STATICINIT);
	break;
	case VL53L0X_STATE_STANDBY:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_STANDBY);
	break;
	case VL53L0X_STATE_IDLE:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_IDLE);
	break;
	case VL53L0X_STATE_RUNNING:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_RUNNING);
	break;
	case VL53L0X_STATE_UNKNOWN:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_UNKNOWN);
	break;
	case VL53L0X_STATE_ERROR:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_ERROR);
	break;

	default:
		VL53L0X_COPYSTRING(pPalStateString,
			VL53L0X_STRING_STATE_UNKNOWN);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_sequence_steps_info(
		VL53L0X_SequenceStepId SequenceStepId,
		char *pSequenceStepsString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	switch (SequenceStepId) {
	case VL53L0X_SEQUENCESTEP_TCC:
		VL53L0X_COPYSTRING(pSequenceStepsString,
			VL53L0X_STRING_SEQUENCESTEP_TCC);
	break;
	case VL53L0X_SEQUENCESTEP_DSS:
		VL53L0X_COPYSTRING(pSequenceStepsString,
			VL53L0X_STRING_SEQUENCESTEP_DSS);
	break;
	case VL53L0X_SEQUENCESTEP_MSRC:
		VL53L0X_COPYSTRING(pSequenceStepsString,
			VL53L0X_STRING_SEQUENCESTEP_MSRC);
	break;
	case VL53L0X_SEQUENCESTEP_PRE_RANGE:
		VL53L0X_COPYSTRING(pSequenceStepsString,
			VL53L0X_STRING_SEQUENCESTEP_PRE_RANGE);
	break;
	case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
		VL53L0X_COPYSTRING(pSequenceStepsString,
			VL53L0X_STRING_SEQUENCESTEP_FINAL_RANGE);
	break;

	default:
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}

	LOG_FUNCTION_END(Status);

	return Status;
}


VL53L0X_Error VL53L0X_get_limit_check_info(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	char *pLimitCheckString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	switch (LimitCheckId) {
	case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_SIGMA_FINAL_RANGE);
	break;
	case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE);
	break;
	case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_SIGNAL_REF_CLIP);
	break;
	case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_RANGE_IGNORE_THRESHOLD);
	break;

	case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_MSRC);
	break;

	case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_CHECKENABLE_SIGNAL_RATE_PRE_RANGE);
	break;

	default:
		VL53L0X_COPYSTRING(pLimitCheckString,
			VL53L0X_STRING_UNKNOW_ERROR_CODE);

	}

	LOG_FUNCTION_END(Status);
	return Status;
}



/******************************************************api_core***************************************************/
/******************************************************api_core***************************************************/
/******************************************************api_core***************************************************/

VL53L0X_Error VL53L0X_reverse_bytes(uint8_t *data, uint32_t size)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t tempData;
	uint32_t mirrorIndex;
	uint32_t middle = size/2;
	uint32_t index;

	for (index = 0; index < middle; index++) {
		mirrorIndex		 = size - index - 1;
		tempData		 = data[index];
		data[index]		 = data[mirrorIndex];
		data[mirrorIndex] = tempData;
	}
	return Status;
}

VL53L0X_Error VL53L0X_measurement_poll_for_completion(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t NewDataReady = 0;
	uint32_t LoopNb;

	LOG_FUNCTION_START("");

	LoopNb = 0;

	do {
		Status = VL53L0X_GetMeasurementDataReady(Dev, &NewDataReady);
		if (Status != 0)
			break; /* the error is set */

		if (NewDataReady == 1)
			break; /* done note that status == 0 */

		LoopNb++;
		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			Status = VL53L0X_ERROR_TIME_OUT;
			break;
		}

		VL53L0X_PollingDelay(Dev);
	} while (1);

	LOG_FUNCTION_END(Status);

	return Status;
}


uint8_t VL53L0X_decode_vcsel_period(uint8_t vcsel_period_reg)
{
	/*!
	 * Converts the encoded VCSEL period register value into the real
	 * period in PLL clocks
	 */

	uint8_t vcsel_period_pclks = 0;

	vcsel_period_pclks = (vcsel_period_reg + 1) << 1;

	return vcsel_period_pclks;
}

uint8_t VL53L0X_encode_vcsel_period(uint8_t vcsel_period_pclks)
{
	/*!
	 * Converts the encoded VCSEL period register value into the real period
	 * in PLL clocks
	 */

	uint8_t vcsel_period_reg = 0;

	vcsel_period_reg = (vcsel_period_pclks >> 1) - 1;

	return vcsel_period_reg;
}


uint32_t VL53L0X_isqrt(uint32_t num)
{
	/*
	 * Implements an integer square root
	 *
	 * From: http://en.wikipedia.org/wiki/Methods_of_computing_square_roots
	 */

	uint32_t  res = 0;
	uint32_t  bit = 1 << 30;
	/* The second-to-top bit is set:
	 *	1 << 14 for 16-bits, 1 << 30 for 32 bits */

	 /* "bit" starts at the highest power of four <= the argument. */
	while (bit > num)
		bit >>= 2;


	while (bit != 0) {
		if (num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;

		bit >>= 2;
	}

	return res;
}


uint32_t VL53L0X_quadrature_sum(uint32_t a, uint32_t b)
{
	/*
	 * Implements a quadrature sum
	 *
	 * rea = sqrt(a^2 + b^2)
	 *
	 * Trap overflow case max input value is 65535 (16-bit value)
	 * as internal calc are 32-bit wide
	 *
	 * If overflow then seta output to maximum
	 */
	uint32_t  res = 0;

	if (a > 65535 || b > 65535)
		res = 65535;
	else
		res = VL53L0X_isqrt(a * a + b * b);

	return res;
}


VL53L0X_Error VL53L0X_device_read_strobe(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t strobe;
	uint32_t LoopNb;
	LOG_FUNCTION_START("");

	Status |= VL53L0X_WrByte(Dev, 0x83, 0x00);

	/* polling
	 * use timeout to avoid deadlock*/
	if (Status == VL53L0X_ERROR_NONE) {
		LoopNb = 0;
		do {
			Status = VL53L0X_RdByte(Dev, 0x83, &strobe);
			if ((strobe != 0x00) || Status != VL53L0X_ERROR_NONE)
					break;

			LoopNb = LoopNb + 1;
		} while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
			Status = VL53L0X_ERROR_TIME_OUT;

	}

	Status |= VL53L0X_WrByte(Dev, 0x83, 0x01);

	LOG_FUNCTION_END(Status);
	return Status;

}

VL53L0X_Error VL53L0X_get_info_from_device(VL53L0X_DEV Dev, uint8_t option)
{

	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t byte;
	uint32_t TmpDWord;
	uint8_t ModuleId;
	uint8_t Revision;
	uint8_t ReferenceSpadCount = 0;
	uint8_t ReferenceSpadType = 0;
	uint32_t PartUIDUpper = 0;
	uint32_t PartUIDLower = 0;
	uint32_t OffsetFixed1104_mm = 0;
	int16_t OffsetMicroMeters = 0;
	uint32_t DistMeasTgtFixed1104_mm = 400 << 4;
	uint32_t DistMeasFixed1104_400_mm = 0;
	uint32_t SignalRateMeasFixed1104_400_mm = 0;
	char ProductId[19];
	char *ProductId_tmp;
	uint8_t ReadDataFromDeviceDone;
	FixPoint1616_t SignalRateMeasFixed400mmFix = 0;
	uint8_t NvmRefGoodSpadMap[VL53L0X_REF_SPAD_BUFFER_SIZE];
	int i;


	LOG_FUNCTION_START("");

	ReadDataFromDeviceDone = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
			ReadDataFromDeviceDone);

	/* This access is done only once after that a GetDeviceInfo or
	 * datainit is done*/
	if (ReadDataFromDeviceDone != 7) {

		Status |= VL53L0X_WrByte(Dev, 0x80, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0x00, 0x00);

		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x06);
		Status |= VL53L0X_RdByte(Dev, 0x83, &byte);
		Status |= VL53L0X_WrByte(Dev, 0x83, byte|4);
		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x07);
		Status |= VL53L0X_WrByte(Dev, 0x81, 0x01);

		Status |= VL53L0X_PollingDelay(Dev);

		Status |= VL53L0X_WrByte(Dev, 0x80, 0x01);

		if (((option & 1) == 1) &&
			((ReadDataFromDeviceDone & 1) == 0)) {
			Status |= VL53L0X_WrByte(Dev, 0x94, 0x6b);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			ReferenceSpadCount = (uint8_t)((TmpDWord >> 8) & 0x07f);
			ReferenceSpadType  = (uint8_t)((TmpDWord >> 15) & 0x01);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x24);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);


			NvmRefGoodSpadMap[0] = (uint8_t)((TmpDWord >> 24)
				& 0xff);
			NvmRefGoodSpadMap[1] = (uint8_t)((TmpDWord >> 16)
				& 0xff);
			NvmRefGoodSpadMap[2] = (uint8_t)((TmpDWord >> 8)
				& 0xff);
			NvmRefGoodSpadMap[3] = (uint8_t)(TmpDWord & 0xff);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x25);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			NvmRefGoodSpadMap[4] = (uint8_t)((TmpDWord >> 24)
				& 0xff);
			NvmRefGoodSpadMap[5] = (uint8_t)((TmpDWord >> 16)
				& 0xff);
		}

		if (((option & 2) == 2) &&
			((ReadDataFromDeviceDone & 2) == 0)) {

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x02);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdByte(Dev, 0x90, &ModuleId);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x7B);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdByte(Dev, 0x90, &Revision);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x77);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			ProductId[0] = (char)((TmpDWord >> 25) & 0x07f);
			ProductId[1] = (char)((TmpDWord >> 18) & 0x07f);
			ProductId[2] = (char)((TmpDWord >> 11) & 0x07f);
			ProductId[3] = (char)((TmpDWord >> 4) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x00f) << 3);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x78);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			ProductId[4] = (char)(byte +
					((TmpDWord >> 29) & 0x07f));
			ProductId[5] = (char)((TmpDWord >> 22) & 0x07f);
			ProductId[6] = (char)((TmpDWord >> 15) & 0x07f);
			ProductId[7] = (char)((TmpDWord >> 8) & 0x07f);
			ProductId[8] = (char)((TmpDWord >> 1) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x001) << 6);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x79);

			Status |= VL53L0X_device_read_strobe(Dev);

			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			ProductId[9] = (char)(byte +
					((TmpDWord >> 26) & 0x07f));
			ProductId[10] = (char)((TmpDWord >> 19) & 0x07f);
			ProductId[11] = (char)((TmpDWord >> 12) & 0x07f);
			ProductId[12] = (char)((TmpDWord >> 5) & 0x07f);

			byte = (uint8_t)((TmpDWord & 0x01f) << 2);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x7A);

			Status |= VL53L0X_device_read_strobe(Dev);

			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			ProductId[13] = (char)(byte +
					((TmpDWord >> 30) & 0x07f));
			ProductId[14] = (char)((TmpDWord >> 23) & 0x07f);
			ProductId[15] = (char)((TmpDWord >> 16) & 0x07f);
			ProductId[16] = (char)((TmpDWord >> 9) & 0x07f);
			ProductId[17] = (char)((TmpDWord >> 2) & 0x07f);
			ProductId[18] = '\0';

		}

		if (((option & 4) == 4) &&
			((ReadDataFromDeviceDone & 4) == 0)) {

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x7B);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &PartUIDUpper);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x7C);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &PartUIDLower);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x73);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			SignalRateMeasFixed1104_400_mm = (TmpDWord &
				0x0000000ff) << 8;

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x74);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			SignalRateMeasFixed1104_400_mm |= ((TmpDWord &
				0xff000000) >> 24);

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x75);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			DistMeasFixed1104_400_mm = (TmpDWord & 0x0000000ff)
							<< 8;

			Status |= VL53L0X_WrByte(Dev, 0x94, 0x76);
			Status |= VL53L0X_device_read_strobe(Dev);
			Status |= VL53L0X_RdDWord(Dev, 0x90, &TmpDWord);

			DistMeasFixed1104_400_mm |= ((TmpDWord & 0xff000000)
							>> 24);
		}

		Status |= VL53L0X_WrByte(Dev, 0x81, 0x00);
		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x06);
		Status |= VL53L0X_RdByte(Dev, 0x83, &byte);
		Status |= VL53L0X_WrByte(Dev, 0x83, byte&0xfb);
		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0x00, 0x01);

		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);
		Status |= VL53L0X_WrByte(Dev, 0x80, 0x00);
	}

	if ((Status == VL53L0X_ERROR_NONE) &&
		(ReadDataFromDeviceDone != 7)) {
		/* Assign to variable if status is ok */
		if (((option & 1) == 1) &&
			((ReadDataFromDeviceDone & 1) == 0)) {
			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
				ReferenceSpadCount, ReferenceSpadCount);

			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
				ReferenceSpadType, ReferenceSpadType);

			for (i = 0; i < VL53L0X_REF_SPAD_BUFFER_SIZE; i++) {
				Dev->Data.SpadData.RefGoodSpadMap[i] =
					NvmRefGoodSpadMap[i];
			}
		}

		if (((option & 2) == 2) &&
			((ReadDataFromDeviceDone & 2) == 0)) {
			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					ModuleId, ModuleId);

			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					Revision, Revision);

			ProductId_tmp = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
					ProductId);
			VL53L0X_COPYSTRING(ProductId_tmp, ProductId);

		}

		if (((option & 4) == 4) &&
			((ReadDataFromDeviceDone & 4) == 0)) {
			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
						PartUIDUpper, PartUIDUpper);

			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
						PartUIDLower, PartUIDLower);

			SignalRateMeasFixed400mmFix =
				VL53L0X_FIXPOINT97TOFIXPOINT1616(
					SignalRateMeasFixed1104_400_mm);

			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
				SignalRateMeasFixed400mm,
				SignalRateMeasFixed400mmFix);

			OffsetMicroMeters = 0;
			if (DistMeasFixed1104_400_mm != 0) {
					OffsetFixed1104_mm =
						DistMeasFixed1104_400_mm -
						DistMeasTgtFixed1104_mm;
					OffsetMicroMeters = (OffsetFixed1104_mm
						* 1000) >> 4;
					OffsetMicroMeters *= -1;
			}

			PALDevDataSet(Dev,
				Part2PartOffsetAdjustmentNVMMicroMeter,
				OffsetMicroMeters);
		}
		byte = (uint8_t)(ReadDataFromDeviceDone|option);
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, ReadDataFromDeviceDone,
				byte);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}


uint32_t VL53L0X_calc_macro_period_ps(VL53L0X_DEV Dev, uint8_t vcsel_period_pclks)
{
	uint64_t PLL_period_ps;
	uint32_t macro_period_vclks;
	uint32_t macro_period_ps;

	LOG_FUNCTION_START("");

	/* The above calculation will produce rounding errors,
	   therefore set fixed value
	*/
	PLL_period_ps = 1655;

	macro_period_vclks = 2304;
	macro_period_ps = (uint32_t)(macro_period_vclks
			* vcsel_period_pclks * PLL_period_ps);

	LOG_FUNCTION_END("");
	return macro_period_ps;
}

uint16_t VL53L0X_encode_timeout(uint32_t timeout_macro_clks)
{
	/*!
	 * Encode timeout in macro periods in (LSByte * 2^MSByte) + 1 format
	 */

	uint16_t encoded_timeout = 0;
	uint32_t ls_byte = 0;
	uint16_t ms_byte = 0;

	if (timeout_macro_clks > 0) {
		ls_byte = timeout_macro_clks - 1;

		while ((ls_byte & 0xFFFFFF00) > 0) {
			ls_byte = ls_byte >> 1;
			ms_byte++;
		}

		encoded_timeout = (ms_byte << 8)
				+ (uint16_t) (ls_byte & 0x000000FF);
	}

	return encoded_timeout;

}

uint32_t VL53L0X_decode_timeout(uint16_t encoded_timeout)
{
	/*!
	 * Decode 16-bit timeout register value - format (LSByte * 2^MSByte) + 1
	 */

	uint32_t timeout_macro_clks = 0;

	timeout_macro_clks = ((uint32_t) (encoded_timeout & 0x00FF)
			<< (uint32_t) ((encoded_timeout & 0xFF00) >> 8)) + 1;

	return timeout_macro_clks;
}


/* To convert ms into register value */
uint32_t VL53L0X_calc_timeout_mclks(VL53L0X_DEV Dev,
		uint32_t timeout_period_us,
		uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ps;
	uint32_t macro_period_ns;
	uint32_t timeout_period_mclks = 0;

	macro_period_ps = VL53L0X_calc_macro_period_ps(Dev, vcsel_period_pclks);
	macro_period_ns = (macro_period_ps + 500) / 1000;

	timeout_period_mclks =
		(uint32_t) (((timeout_period_us * 1000)
		+ (macro_period_ns / 2)) / macro_period_ns);

	return timeout_period_mclks;
}

/* To convert register value into us */
uint32_t VL53L0X_calc_timeout_us(VL53L0X_DEV Dev,
		uint16_t timeout_period_mclks,
		uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ps;
	uint32_t macro_period_ns;
	uint32_t actual_timeout_period_us = 0;

	macro_period_ps = VL53L0X_calc_macro_period_ps(Dev, vcsel_period_pclks);
	macro_period_ns = (macro_period_ps + 500) / 1000;

	actual_timeout_period_us =
		((timeout_period_mclks * macro_period_ns)
		+ (macro_period_ns / 2)) / 1000;

	return actual_timeout_period_us;
}


VL53L0X_Error get_sequence_step_timeout(VL53L0X_DEV Dev,
				VL53L0X_SequenceStepId SequenceStepId,
				uint32_t *pTimeOutMicroSecs)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t CurrentVCSELPulsePeriodPClk;
	uint8_t EncodedTimeOutByte = 0;
	uint32_t TimeoutMicroSeconds = 0;
	uint16_t PreRangeEncodedTimeOut = 0;
	uint16_t MsrcTimeOutMClks;
	uint16_t PreRangeTimeOutMClks;
	uint16_t FinalRangeTimeOutMClks = 0;
	uint16_t FinalRangeEncodedTimeOut;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;

	if ((SequenceStepId == VL53L0X_SEQUENCESTEP_TCC)	 ||
		(SequenceStepId == VL53L0X_SEQUENCESTEP_DSS)	 ||
		(SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)) {

		Status = VL53L0X_GetVcselPulsePeriod(Dev,
					VL53L0X_VCSEL_PERIOD_PRE_RANGE,
					&CurrentVCSELPulsePeriodPClk);
		if (Status == VL53L0X_ERROR_NONE) {
			Status = VL53L0X_RdByte(Dev,
					VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP,
					&EncodedTimeOutByte);
		}
		MsrcTimeOutMClks = VL53L0X_decode_timeout(EncodedTimeOutByte);

		TimeoutMicroSeconds = VL53L0X_calc_timeout_us(Dev,
						MsrcTimeOutMClks,
						CurrentVCSELPulsePeriodPClk);
	} else if (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE) {
		/* Retrieve PRE-RANGE VCSEL Period */
		Status = VL53L0X_GetVcselPulsePeriod(Dev,
						VL53L0X_VCSEL_PERIOD_PRE_RANGE,
						&CurrentVCSELPulsePeriodPClk);

		/* Retrieve PRE-RANGE Timeout in Macro periods (MCLKS) */
		if (Status == VL53L0X_ERROR_NONE) {

			/* Retrieve PRE-RANGE VCSEL Period */
			Status = VL53L0X_GetVcselPulsePeriod(Dev,
					VL53L0X_VCSEL_PERIOD_PRE_RANGE,
					&CurrentVCSELPulsePeriodPClk);

			if (Status == VL53L0X_ERROR_NONE) {
				Status = VL53L0X_RdWord(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
				&PreRangeEncodedTimeOut);
			}

			PreRangeTimeOutMClks = VL53L0X_decode_timeout(
					PreRangeEncodedTimeOut);

			TimeoutMicroSeconds = VL53L0X_calc_timeout_us(Dev,
					PreRangeTimeOutMClks,
					CurrentVCSELPulsePeriodPClk);
		}
	} else if (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE) {

		VL53L0X_GetSequenceStepEnables(Dev, &SchedulerSequenceSteps);
		PreRangeTimeOutMClks = 0;

		if (SchedulerSequenceSteps.PreRangeOn) {
			/* Retrieve PRE-RANGE VCSEL Period */
			Status = VL53L0X_GetVcselPulsePeriod(Dev,
				VL53L0X_VCSEL_PERIOD_PRE_RANGE,
				&CurrentVCSELPulsePeriodPClk);

			/* Retrieve PRE-RANGE Timeout in Macro periods
			 * (MCLKS) */
			if (Status == VL53L0X_ERROR_NONE) {
				Status = VL53L0X_RdWord(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
				&PreRangeEncodedTimeOut);
				PreRangeTimeOutMClks = VL53L0X_decode_timeout(
						PreRangeEncodedTimeOut);
			}
		}

		if (Status == VL53L0X_ERROR_NONE) {
			/* Retrieve FINAL-RANGE VCSEL Period */
			Status = VL53L0X_GetVcselPulsePeriod(Dev,
					VL53L0X_VCSEL_PERIOD_FINAL_RANGE,
					&CurrentVCSELPulsePeriodPClk);
		}

		/* Retrieve FINAL-RANGE Timeout in Macro periods (MCLKS) */
		if (Status == VL53L0X_ERROR_NONE) {
			Status = VL53L0X_RdWord(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
				&FinalRangeEncodedTimeOut);
			FinalRangeTimeOutMClks = VL53L0X_decode_timeout(
					FinalRangeEncodedTimeOut);
		}

		FinalRangeTimeOutMClks -= PreRangeTimeOutMClks;
		TimeoutMicroSeconds = VL53L0X_calc_timeout_us(Dev,
						FinalRangeTimeOutMClks,
						CurrentVCSELPulsePeriodPClk);
	}

	*pTimeOutMicroSecs = TimeoutMicroSeconds;

	return Status;
}


VL53L0X_Error set_sequence_step_timeout(VL53L0X_DEV Dev,
					VL53L0X_SequenceStepId SequenceStepId,
					uint32_t TimeOutMicroSecs)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t CurrentVCSELPulsePeriodPClk;
	uint8_t MsrcEncodedTimeOut;
	uint16_t PreRangeEncodedTimeOut;
	uint16_t PreRangeTimeOutMClks;
	uint16_t MsrcRangeTimeOutMClks;
	uint16_t FinalRangeTimeOutMClks;
	uint16_t FinalRangeEncodedTimeOut;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;

	if ((SequenceStepId == VL53L0X_SEQUENCESTEP_TCC)	 ||
		(SequenceStepId == VL53L0X_SEQUENCESTEP_DSS)	 ||
		(SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)) {

		Status = VL53L0X_GetVcselPulsePeriod(Dev,
					VL53L0X_VCSEL_PERIOD_PRE_RANGE,
					&CurrentVCSELPulsePeriodPClk);

		if (Status == VL53L0X_ERROR_NONE) {
			MsrcRangeTimeOutMClks = VL53L0X_calc_timeout_mclks(Dev,
					TimeOutMicroSecs,
					(uint8_t)CurrentVCSELPulsePeriodPClk);

			if (MsrcRangeTimeOutMClks > 256)
				MsrcEncodedTimeOut = 255;
			else
				MsrcEncodedTimeOut =
					(uint8_t)MsrcRangeTimeOutMClks - 1;

			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
				LastEncodedTimeout,
				MsrcEncodedTimeOut);
		}

		if (Status == VL53L0X_ERROR_NONE) {
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_MSRC_CONFIG_TIMEOUT_MACROP,
				MsrcEncodedTimeOut);
		}
	} else {

		if (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE) {

			if (Status == VL53L0X_ERROR_NONE) {
				Status = VL53L0X_GetVcselPulsePeriod(Dev,
						VL53L0X_VCSEL_PERIOD_PRE_RANGE,
						&CurrentVCSELPulsePeriodPClk);
				PreRangeTimeOutMClks =
					VL53L0X_calc_timeout_mclks(Dev,
					TimeOutMicroSecs,
					(uint8_t)CurrentVCSELPulsePeriodPClk);
				PreRangeEncodedTimeOut = VL53L0X_encode_timeout(
					PreRangeTimeOutMClks);

				VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					LastEncodedTimeout,
					PreRangeEncodedTimeOut);
			}

			if (Status == VL53L0X_ERROR_NONE) {
				Status = VL53L0X_WrWord(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
				PreRangeEncodedTimeOut);
			}

			if (Status == VL53L0X_ERROR_NONE) {
				VL53L0X_SETDEVICESPECIFICPARAMETER(
					Dev,
					PreRangeTimeoutMicroSecs,
					TimeOutMicroSecs);
			}
		} else if (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE) {

			/* For the final range timeout, the pre-range timeout
			 * must be added. To do this both final and pre-range
			 * timeouts must be expressed in macro periods MClks
			 * because they have different vcsel periods.
			 */

			VL53L0X_GetSequenceStepEnables(Dev,
					&SchedulerSequenceSteps);
			PreRangeTimeOutMClks = 0;
			if (SchedulerSequenceSteps.PreRangeOn) {

				/* Retrieve PRE-RANGE VCSEL Period */
				Status = VL53L0X_GetVcselPulsePeriod(Dev,
					VL53L0X_VCSEL_PERIOD_PRE_RANGE,
					&CurrentVCSELPulsePeriodPClk);

				/* Retrieve PRE-RANGE Timeout in Macro periods
				 * (MCLKS) */
				if (Status == VL53L0X_ERROR_NONE) {
					Status = VL53L0X_RdWord(Dev, 0x51,
						&PreRangeEncodedTimeOut);
					PreRangeTimeOutMClks =
						VL53L0X_decode_timeout(
							PreRangeEncodedTimeOut);
				}
			}

			/* Calculate FINAL RANGE Timeout in Macro Periods
			 * (MCLKS) and add PRE-RANGE value
			 */
			if (Status == VL53L0X_ERROR_NONE) {

				Status = VL53L0X_GetVcselPulsePeriod(Dev,
						VL53L0X_VCSEL_PERIOD_FINAL_RANGE,
						&CurrentVCSELPulsePeriodPClk);
			}
			if (Status == VL53L0X_ERROR_NONE) {

				FinalRangeTimeOutMClks =
					VL53L0X_calc_timeout_mclks(Dev,
					TimeOutMicroSecs,
					(uint8_t) CurrentVCSELPulsePeriodPClk);

				FinalRangeTimeOutMClks += PreRangeTimeOutMClks;

				FinalRangeEncodedTimeOut =
				VL53L0X_encode_timeout(FinalRangeTimeOutMClks);

				if (Status == VL53L0X_ERROR_NONE) {
					Status = VL53L0X_WrWord(Dev, 0x71,
					FinalRangeEncodedTimeOut);
				}

				if (Status == VL53L0X_ERROR_NONE) {
					VL53L0X_SETDEVICESPECIFICPARAMETER(
						Dev,
						FinalRangeTimeoutMicroSecs,
						TimeOutMicroSecs);
				}
			}
		} else
			Status = VL53L0X_ERROR_INVALID_PARAMS;

	}
	return Status;
}

VL53L0X_Error VL53L0X_set_vcsel_pulse_period(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t VCSELPulsePeriodPCLK)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t vcsel_period_reg;
	uint8_t MinPreVcselPeriodPCLK = 12;
	uint8_t MaxPreVcselPeriodPCLK = 18;
	uint8_t MinFinalVcselPeriodPCLK = 8;
	uint8_t MaxFinalVcselPeriodPCLK = 14;
	uint32_t MeasurementTimingBudgetMicroSeconds;
	uint32_t FinalRangeTimeoutMicroSeconds;
	uint32_t PreRangeTimeoutMicroSeconds;
	uint32_t MsrcTimeoutMicroSeconds;
	uint8_t PhaseCalInt = 0;

	/* Check if valid clock period requested */

	if ((VCSELPulsePeriodPCLK % 2) != 0) {
		/* Value must be an even number */
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	} else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_PRE_RANGE &&
		(VCSELPulsePeriodPCLK < MinPreVcselPeriodPCLK ||
		VCSELPulsePeriodPCLK > MaxPreVcselPeriodPCLK)) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	} else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_FINAL_RANGE &&
		(VCSELPulsePeriodPCLK < MinFinalVcselPeriodPCLK ||
		 VCSELPulsePeriodPCLK > MaxFinalVcselPeriodPCLK)) {

		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}

	/* Apply specific settings for the requested clock period */

	if (Status != VL53L0X_ERROR_NONE)
		return Status;


	if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_PRE_RANGE) {

		/* Set phase check limits */
		if (VCSELPulsePeriodPCLK == 12) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x18);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);
		} else if (VCSELPulsePeriodPCLK == 14) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x30);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);
		} else if (VCSELPulsePeriodPCLK == 16) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x40);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);
		} else if (VCSELPulsePeriodPCLK == 18) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x50);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);
		}
	} else if (VcselPeriodType == VL53L0X_VCSEL_PERIOD_FINAL_RANGE) {

		if (VCSELPulsePeriodPCLK == 8) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x10);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);

			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);

			Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_LIM,
				0x30);
			Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		} else if (VCSELPulsePeriodPCLK == 10) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x28);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);

			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);

			Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_LIM,
				0x20);
			Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		} else if (VCSELPulsePeriodPCLK == 12) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x38);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);

			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);

			Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_LIM,
				0x20);
			Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		} else if (VCSELPulsePeriodPCLK == 14) {

			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH,
				0x048);
			Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW,
				0x08);

			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);

			Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
			Status |= VL53L0X_WrByte(Dev,
				VL53L0X_REG_ALGO_PHASECAL_LIM,
				0x20);
			Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		}
	}


	/* Re-calculate and apply timeouts, in macro periods */

	if (Status == VL53L0X_ERROR_NONE) {
		vcsel_period_reg = VL53L0X_encode_vcsel_period((uint8_t)
			VCSELPulsePeriodPCLK);

		/* When the VCSEL period for the pre or final range is changed,
		* the corresponding timeout must be read from the device using
		* the current VCSEL period, then the new VCSEL period can be
		* applied. The timeout then must be written back to the device
		* using the new VCSEL period.
		*
		* For the MSRC timeout, the same applies - this timeout being
		* dependant on the pre-range vcsel period.
		*/
		switch (VcselPeriodType) {
		case VL53L0X_VCSEL_PERIOD_PRE_RANGE:
			Status = get_sequence_step_timeout(Dev,
				VL53L0X_SEQUENCESTEP_PRE_RANGE,
				&PreRangeTimeoutMicroSeconds);

			if (Status == VL53L0X_ERROR_NONE)
				Status = get_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_MSRC,
					&MsrcTimeoutMicroSeconds);

			if (Status == VL53L0X_ERROR_NONE)
				Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD,
					vcsel_period_reg);


			if (Status == VL53L0X_ERROR_NONE)
				Status = set_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_PRE_RANGE,
					PreRangeTimeoutMicroSeconds);


			if (Status == VL53L0X_ERROR_NONE)
				Status = set_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_MSRC,
					MsrcTimeoutMicroSeconds);

			VL53L0X_SETDEVICESPECIFICPARAMETER(
				Dev,
				PreRangeVcselPulsePeriod,
				VCSELPulsePeriodPCLK);
			break;
		case VL53L0X_VCSEL_PERIOD_FINAL_RANGE:
			Status = get_sequence_step_timeout(Dev,
				VL53L0X_SEQUENCESTEP_FINAL_RANGE,
				&FinalRangeTimeoutMicroSeconds);

			if (Status == VL53L0X_ERROR_NONE)
				Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD,
					vcsel_period_reg);


			if (Status == VL53L0X_ERROR_NONE)
				Status = set_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_FINAL_RANGE,
					FinalRangeTimeoutMicroSeconds);

			VL53L0X_SETDEVICESPECIFICPARAMETER(
				Dev,
				FinalRangeVcselPulsePeriod,
				VCSELPulsePeriodPCLK);
			break;
		default:
			Status = VL53L0X_ERROR_INVALID_PARAMS;
		}
	}

	/* Finally, the timing budget must be re-applied */
	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_GETPARAMETERFIELD(Dev,
			MeasurementTimingBudgetMicroSeconds,
			MeasurementTimingBudgetMicroSeconds);

		Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev,
				MeasurementTimingBudgetMicroSeconds);
	}

	/* Perform the phase calibration. This is needed after changing on
	 * vcsel period.
	 * get_data_enable = 0, restore_config = 1 */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_perform_phase_calibration(
			Dev, &PhaseCalInt, 0, 1);

	return Status;
}

VL53L0X_Error VL53L0X_get_vcsel_pulse_period(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t *pVCSELPulsePeriodPCLK)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t vcsel_period_reg;

	switch (VcselPeriodType) {
	case VL53L0X_VCSEL_PERIOD_PRE_RANGE:
		Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD,
			&vcsel_period_reg);
	break;
	case VL53L0X_VCSEL_PERIOD_FINAL_RANGE:
		Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD,
			&vcsel_period_reg);
	break;
	default:
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}

	if (Status == VL53L0X_ERROR_NONE)
		*pVCSELPulsePeriodPCLK =
			VL53L0X_decode_vcsel_period(vcsel_period_reg);

	return Status;
}



VL53L0X_Error VL53L0X_set_measurement_timing_budget_micro_seconds(VL53L0X_DEV Dev,
		uint32_t MeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t FinalRangeTimingBudgetMicroSeconds;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;
	uint32_t MsrcDccTccTimeoutMicroSeconds	= 2000;
	uint32_t StartOverheadMicroSeconds		= 1320;
	uint32_t EndOverheadMicroSeconds		= 960;
	uint32_t MsrcOverheadMicroSeconds		= 660;
	uint32_t TccOverheadMicroSeconds		= 590;
	uint32_t DssOverheadMicroSeconds		= 690;
	uint32_t PreRangeOverheadMicroSeconds	= 660;
	uint32_t FinalRangeOverheadMicroSeconds = 550;
	uint32_t PreRangeTimeoutMicroSeconds	= 0;
	uint32_t cMinTimingBudgetMicroSeconds	= 20000;
	uint32_t SubTimeout = 0;

	LOG_FUNCTION_START("");

	if (MeasurementTimingBudgetMicroSeconds
			< cMinTimingBudgetMicroSeconds) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
		return Status;
	}

	FinalRangeTimingBudgetMicroSeconds =
		MeasurementTimingBudgetMicroSeconds -
		(StartOverheadMicroSeconds + EndOverheadMicroSeconds);

	Status = VL53L0X_GetSequenceStepEnables(Dev, &SchedulerSequenceSteps);

	if (Status == VL53L0X_ERROR_NONE &&
		(SchedulerSequenceSteps.TccOn  ||
		SchedulerSequenceSteps.MsrcOn ||
		SchedulerSequenceSteps.DssOn)) {

		/* TCC, MSRC and DSS all share the same timeout */
		Status = get_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_MSRC,
					&MsrcDccTccTimeoutMicroSeconds);

		/* Subtract the TCC, MSRC and DSS timeouts if they are
		 * enabled. */

		if (Status != VL53L0X_ERROR_NONE)
			return Status;

		/* TCC */
		if (SchedulerSequenceSteps.TccOn) {

			SubTimeout = MsrcDccTccTimeoutMicroSeconds
				+ TccOverheadMicroSeconds;

			if (SubTimeout <
				FinalRangeTimingBudgetMicroSeconds) {
				FinalRangeTimingBudgetMicroSeconds -=
							SubTimeout;
			} else {
				/* Requested timeout too big. */
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}
		}

		if (Status != VL53L0X_ERROR_NONE) {
			LOG_FUNCTION_END(Status);
			return Status;
		}

		/* DSS */
		if (SchedulerSequenceSteps.DssOn) {

			SubTimeout = 2 * (MsrcDccTccTimeoutMicroSeconds +
				DssOverheadMicroSeconds);

			if (SubTimeout < FinalRangeTimingBudgetMicroSeconds) {
				FinalRangeTimingBudgetMicroSeconds
							-= SubTimeout;
			} else {
				/* Requested timeout too big. */
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}
		} else if (SchedulerSequenceSteps.MsrcOn) {
			/* MSRC */
			SubTimeout = MsrcDccTccTimeoutMicroSeconds +
						MsrcOverheadMicroSeconds;

			if (SubTimeout < FinalRangeTimingBudgetMicroSeconds) {
				FinalRangeTimingBudgetMicroSeconds
							-= SubTimeout;
			} else {
				/* Requested timeout too big. */
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}
		}

	}

	if (Status != VL53L0X_ERROR_NONE) {
		LOG_FUNCTION_END(Status);
		return Status;
	}

	if (SchedulerSequenceSteps.PreRangeOn) {

		/* Subtract the Pre-range timeout if enabled. */

		Status = get_sequence_step_timeout(Dev,
				VL53L0X_SEQUENCESTEP_PRE_RANGE,
				&PreRangeTimeoutMicroSeconds);

		SubTimeout = PreRangeTimeoutMicroSeconds +
				PreRangeOverheadMicroSeconds;

		if (SubTimeout < FinalRangeTimingBudgetMicroSeconds) {
			FinalRangeTimingBudgetMicroSeconds -= SubTimeout;
		} else {
			/* Requested timeout too big. */
			Status = VL53L0X_ERROR_INVALID_PARAMS;
		}
	}


	if (Status == VL53L0X_ERROR_NONE &&
		SchedulerSequenceSteps.FinalRangeOn) {

		FinalRangeTimingBudgetMicroSeconds -=
				FinalRangeOverheadMicroSeconds;

		/* Final Range Timeout
		* Note that the final range timeout is determined by the timing
		* budget and the sum of all other timeouts within the sequence.
		* If there is no room for the final range timeout, then an error
		* will be set. Otherwise the remaining time will be applied to
		* the final range.
		*/
		Status = set_sequence_step_timeout(Dev,
			   VL53L0X_SEQUENCESTEP_FINAL_RANGE,
			   FinalRangeTimingBudgetMicroSeconds);

		VL53L0X_SETPARAMETERFIELD(Dev,
			   MeasurementTimingBudgetMicroSeconds,
			   MeasurementTimingBudgetMicroSeconds);
	}

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_get_measurement_timing_budget_micro_seconds(VL53L0X_DEV Dev,
		uint32_t *pMeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_SchedulerSequenceSteps_t SchedulerSequenceSteps;
	uint32_t FinalRangeTimeoutMicroSeconds;
	uint32_t MsrcDccTccTimeoutMicroSeconds	= 2000;
	uint32_t StartOverheadMicroSeconds		= 1910;
	uint32_t EndOverheadMicroSeconds		= 960;
	uint32_t MsrcOverheadMicroSeconds		= 660;
	uint32_t TccOverheadMicroSeconds		= 590;
	uint32_t DssOverheadMicroSeconds		= 690;
	uint32_t PreRangeOverheadMicroSeconds	= 660;
	uint32_t FinalRangeOverheadMicroSeconds = 550;
	uint32_t PreRangeTimeoutMicroSeconds	= 0;

	LOG_FUNCTION_START("");

	/* Start and end overhead times always present */
	*pMeasurementTimingBudgetMicroSeconds
		= StartOverheadMicroSeconds + EndOverheadMicroSeconds;

	Status = VL53L0X_GetSequenceStepEnables(Dev, &SchedulerSequenceSteps);

	if (Status != VL53L0X_ERROR_NONE) {
		LOG_FUNCTION_END(Status);
		return Status;
	}


	if (SchedulerSequenceSteps.TccOn  ||
		SchedulerSequenceSteps.MsrcOn ||
		SchedulerSequenceSteps.DssOn) {

		Status = get_sequence_step_timeout(Dev,
				VL53L0X_SEQUENCESTEP_MSRC,
				&MsrcDccTccTimeoutMicroSeconds);

		if (Status == VL53L0X_ERROR_NONE) {
			if (SchedulerSequenceSteps.TccOn) {
				*pMeasurementTimingBudgetMicroSeconds +=
					MsrcDccTccTimeoutMicroSeconds +
					TccOverheadMicroSeconds;
			}

			if (SchedulerSequenceSteps.DssOn) {
				*pMeasurementTimingBudgetMicroSeconds +=
				2 * (MsrcDccTccTimeoutMicroSeconds +
					DssOverheadMicroSeconds);
			} else if (SchedulerSequenceSteps.MsrcOn) {
				*pMeasurementTimingBudgetMicroSeconds +=
					MsrcDccTccTimeoutMicroSeconds +
					MsrcOverheadMicroSeconds;
			}
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (SchedulerSequenceSteps.PreRangeOn) {
			Status = get_sequence_step_timeout(Dev,
				VL53L0X_SEQUENCESTEP_PRE_RANGE,
				&PreRangeTimeoutMicroSeconds);
			*pMeasurementTimingBudgetMicroSeconds +=
				PreRangeTimeoutMicroSeconds +
				PreRangeOverheadMicroSeconds;
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (SchedulerSequenceSteps.FinalRangeOn) {
			Status = get_sequence_step_timeout(Dev,
					VL53L0X_SEQUENCESTEP_FINAL_RANGE,
					&FinalRangeTimeoutMicroSeconds);
			*pMeasurementTimingBudgetMicroSeconds +=
				(FinalRangeTimeoutMicroSeconds +
				FinalRangeOverheadMicroSeconds);
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETPARAMETERFIELD(Dev,
			MeasurementTimingBudgetMicroSeconds,
			*pMeasurementTimingBudgetMicroSeconds);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}



VL53L0X_Error VL53L0X_load_tuning_settings(VL53L0X_DEV Dev,
		uint8_t *pTuningSettingBuffer)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int i;
	int Index;
	uint8_t msb;
	uint8_t lsb;
	uint8_t SelectParam;
	uint8_t NumberOfWrites;
	uint8_t Address;
	uint8_t localBuffer[4]; /* max */
	uint16_t Temp16;

	LOG_FUNCTION_START("");

	Index = 0;

	while ((*(pTuningSettingBuffer + Index) != 0) &&
			(Status == VL53L0X_ERROR_NONE)) {
		NumberOfWrites = *(pTuningSettingBuffer + Index);
		Index++;
		if (NumberOfWrites == 0xFF) {
			/* internal parameters */
			SelectParam = *(pTuningSettingBuffer + Index);
			Index++;
			switch (SelectParam) {
			case 0: /* uint16_t SigmaEstRefArray -> 2 bytes */
				msb = *(pTuningSettingBuffer + Index);
				Index++;
				lsb = *(pTuningSettingBuffer + Index);
				Index++;
				Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
				PALDevDataSet(Dev, SigmaEstRefArray, Temp16);
				break;
			case 1: /* uint16_t SigmaEstEffPulseWidth -> 2 bytes */
				msb = *(pTuningSettingBuffer + Index);
				Index++;
				lsb = *(pTuningSettingBuffer + Index);
				Index++;
				Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
				PALDevDataSet(Dev, SigmaEstEffPulseWidth,
					Temp16);
				break;
			case 2: /* uint16_t SigmaEstEffAmbWidth -> 2 bytes */
				msb = *(pTuningSettingBuffer + Index);
				Index++;
				lsb = *(pTuningSettingBuffer + Index);
				Index++;
				Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
				PALDevDataSet(Dev, SigmaEstEffAmbWidth, Temp16);
				break;
			case 3: /* uint16_t targetRefRate -> 2 bytes */
				msb = *(pTuningSettingBuffer + Index);
				Index++;
				lsb = *(pTuningSettingBuffer + Index);
				Index++;
				Temp16 = VL53L0X_MAKEUINT16(lsb, msb);
				PALDevDataSet(Dev, targetRefRate, Temp16);
				break;
			default: /* invalid parameter */
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}

		} else if (NumberOfWrites <= 4) {
			Address = *(pTuningSettingBuffer + Index);
			Index++;

			for (i = 0; i < NumberOfWrites; i++) {
				localBuffer[i] = *(pTuningSettingBuffer +
							Index);
				Index++;
			}

			Status = VL53L0X_WriteMulti(Dev, Address, localBuffer,
					NumberOfWrites);

		} else {
			Status = VL53L0X_ERROR_INVALID_PARAMS;
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_total_xtalk_rate(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
	FixPoint1616_t *ptotal_xtalk_rate_mcps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	uint8_t xtalkCompEnable;
	FixPoint1616_t totalXtalkMegaCps;
	FixPoint1616_t xtalkPerSpadMegaCps;

	*ptotal_xtalk_rate_mcps = 0;

	Status = VL53L0X_GetXTalkCompensationEnable(Dev, &xtalkCompEnable);
	if (Status == VL53L0X_ERROR_NONE) {

		if (xtalkCompEnable) {

			VL53L0X_GETPARAMETERFIELD(
				Dev,
				XTalkCompensationRateMegaCps,
				xtalkPerSpadMegaCps);

			/* FixPoint1616 * FixPoint 8:8 = FixPoint0824 */
			totalXtalkMegaCps =
				pRangingMeasurementData->EffectiveSpadRtnCount *
				xtalkPerSpadMegaCps;

			/* FixPoint0824 >> 8 = FixPoint1616 */
			*ptotal_xtalk_rate_mcps =
				(totalXtalkMegaCps + 0x80) >> 8;
		}
	}

	return Status;
}

VL53L0X_Error VL53L0X_get_total_signal_rate(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
	FixPoint1616_t *ptotal_signal_rate_mcps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	FixPoint1616_t totalXtalkMegaCps;

	LOG_FUNCTION_START("");

	*ptotal_signal_rate_mcps =
		pRangingMeasurementData->SignalRateRtnMegaCps;

	Status = VL53L0X_get_total_xtalk_rate(
		Dev, pRangingMeasurementData, &totalXtalkMegaCps);

	if (Status == VL53L0X_ERROR_NONE)
		*ptotal_signal_rate_mcps += totalXtalkMegaCps;

	return Status;
}

VL53L0X_Error VL53L0X_calc_dmax(
	VL53L0X_DEV Dev,
	FixPoint1616_t totalSignalRate_mcps,
	FixPoint1616_t totalCorrSignalRate_mcps,
	FixPoint1616_t pwMult,
	uint32_t sigmaEstimateP1,
	FixPoint1616_t sigmaEstimateP2,
	uint32_t peakVcselDuration_us,
	uint32_t *pdmax_mm)
{
	const uint32_t cSigmaLimit		= 18;
	const FixPoint1616_t cSignalLimit	= 0x4000; /* 0.25 */
	const FixPoint1616_t cSigmaEstRef	= 0x00000042; /* 0.001 */
	const uint32_t cAmbEffWidthSigmaEst_ns = 6;
	const uint32_t cAmbEffWidthDMax_ns	   = 7;
	uint32_t dmaxCalRange_mm;
	FixPoint1616_t dmaxCalSignalRateRtn_mcps;
	FixPoint1616_t minSignalNeeded;
	FixPoint1616_t minSignalNeeded_p1;
	FixPoint1616_t minSignalNeeded_p2;
	FixPoint1616_t minSignalNeeded_p3;
	FixPoint1616_t minSignalNeeded_p4;
	FixPoint1616_t sigmaLimitTmp;
	FixPoint1616_t sigmaEstSqTmp;
	FixPoint1616_t signalLimitTmp;
	FixPoint1616_t SignalAt0mm;
	FixPoint1616_t dmaxDark;
	FixPoint1616_t dmaxAmbient;
	FixPoint1616_t dmaxDarkTmp;
	FixPoint1616_t sigmaEstP2Tmp;
	uint32_t signalRateTemp_mcps;

	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	dmaxCalRange_mm =
		PALDevDataGet(Dev, DmaxCalRangeMilliMeter);

	dmaxCalSignalRateRtn_mcps =
		PALDevDataGet(Dev, DmaxCalSignalRateRtnMegaCps);

	/* uint32 * FixPoint1616 = FixPoint1616 */
	SignalAt0mm = dmaxCalRange_mm * dmaxCalSignalRateRtn_mcps;

	/* FixPoint1616 >> 8 = FixPoint2408 */
	SignalAt0mm = (SignalAt0mm + 0x80) >> 8;
	SignalAt0mm *= dmaxCalRange_mm;

	minSignalNeeded_p1 = 0;
	if (totalCorrSignalRate_mcps > 0) {

		/* Shift by 10 bits to increase resolution prior to the
		 * division */
		signalRateTemp_mcps = totalSignalRate_mcps << 10;

		/* Add rounding value prior to division */
		minSignalNeeded_p1 = signalRateTemp_mcps +
			(totalCorrSignalRate_mcps/2);

		/* FixPoint0626/FixPoint1616 = FixPoint2210 */
		minSignalNeeded_p1 /= totalCorrSignalRate_mcps;

		/* Apply a factored version of the speed of light.
		 Correction to be applied at the end */
		minSignalNeeded_p1 *= 3;

		/* FixPoint2210 * FixPoint2210 = FixPoint1220 */
		minSignalNeeded_p1 *= minSignalNeeded_p1;

		/* FixPoint1220 >> 16 = FixPoint2804 */
		minSignalNeeded_p1 = (minSignalNeeded_p1 + 0x8000) >> 16;
	}

	minSignalNeeded_p2 = pwMult * sigmaEstimateP1;

	/* FixPoint1616 >> 16 =	 uint32 */
	minSignalNeeded_p2 = (minSignalNeeded_p2 + 0x8000) >> 16;

	/* uint32 * uint32	=  uint32 */
	minSignalNeeded_p2 *= minSignalNeeded_p2;

	/* Check sigmaEstimateP2
	 * If this value is too high there is not enough signal rate
	 * to calculate dmax value so set a suitable value to ensure
	 * a very small dmax.
	 */
	sigmaEstP2Tmp = (sigmaEstimateP2 + 0x8000) >> 16;
	sigmaEstP2Tmp = (sigmaEstP2Tmp + cAmbEffWidthSigmaEst_ns/2)/
		cAmbEffWidthSigmaEst_ns;
	sigmaEstP2Tmp *= cAmbEffWidthDMax_ns;

	if (sigmaEstP2Tmp > 0xffff) {
		minSignalNeeded_p3 = 0xfff00000;
	} else {

		/* DMAX uses a different ambient width from sigma, so apply
		 * correction.
		 * Perform division before multiplication to prevent overflow.
		 */
		sigmaEstimateP2 = (sigmaEstimateP2 + cAmbEffWidthSigmaEst_ns/2)/
			cAmbEffWidthSigmaEst_ns;
		sigmaEstimateP2 *= cAmbEffWidthDMax_ns;

		/* FixPoint1616 >> 16 = uint32 */
		minSignalNeeded_p3 = (sigmaEstimateP2 + 0x8000) >> 16;

		minSignalNeeded_p3 *= minSignalNeeded_p3;

	}

	/* FixPoint1814 / uint32 = FixPoint1814 */
	sigmaLimitTmp = ((cSigmaLimit << 14) + 500) / 1000;

	/* FixPoint1814 * FixPoint1814 = FixPoint3628 := FixPoint0428 */
	sigmaLimitTmp *= sigmaLimitTmp;

	/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
	sigmaEstSqTmp = cSigmaEstRef * cSigmaEstRef;

	/* FixPoint3232 >> 4 = FixPoint0428 */
	sigmaEstSqTmp = (sigmaEstSqTmp + 0x08) >> 4;

	/* FixPoint0428 - FixPoint0428	= FixPoint0428 */
	sigmaLimitTmp -=  sigmaEstSqTmp;

	/* uint32_t * FixPoint0428 = FixPoint0428 */
	minSignalNeeded_p4 = 4 * 12 * sigmaLimitTmp;

	/* FixPoint0428 >> 14 = FixPoint1814 */
	minSignalNeeded_p4 = (minSignalNeeded_p4 + 0x2000) >> 14;

	/* uint32 + uint32 = uint32 */
	minSignalNeeded = (minSignalNeeded_p2 + minSignalNeeded_p3);

	/* uint32 / uint32 = uint32 */
	minSignalNeeded += (peakVcselDuration_us/2);
	minSignalNeeded /= peakVcselDuration_us;

	/* uint32 << 14 = FixPoint1814 */
	minSignalNeeded <<= 14;

	/* FixPoint1814 / FixPoint1814 = uint32 */
	minSignalNeeded += (minSignalNeeded_p4/2);
	minSignalNeeded /= minSignalNeeded_p4;

	/* FixPoint3200 * FixPoint2804 := FixPoint2804*/
	minSignalNeeded *= minSignalNeeded_p1;

	/* Apply correction by dividing by 1000000.
	 * This assumes 10E16 on the numerator of the equation
	 * and 10E-22 on the denominator.
	 * We do this because 32bit fix point calculation can't
	 * handle the larger and smaller elements of this equation,
	 * i.e. speed of light and pulse widths.
	 */
	minSignalNeeded = (minSignalNeeded + 500) / 1000;
	minSignalNeeded <<= 4;

	minSignalNeeded = (minSignalNeeded + 500) / 1000;

	/* FixPoint1616 >> 8 = FixPoint2408 */
	signalLimitTmp = (cSignalLimit + 0x80) >> 8;

	/* FixPoint2408/FixPoint2408 = uint32 */
	if (signalLimitTmp != 0)
		dmaxDarkTmp = (SignalAt0mm + (signalLimitTmp / 2))
			/ signalLimitTmp;
	else
		dmaxDarkTmp = 0;

	dmaxDark = VL53L0X_isqrt(dmaxDarkTmp);

	/* FixPoint2408/FixPoint2408 = uint32 */
	if (minSignalNeeded != 0)
		dmaxAmbient = (SignalAt0mm + minSignalNeeded/2)
			/ minSignalNeeded;
	else
		dmaxAmbient = 0;

	dmaxAmbient = VL53L0X_isqrt(dmaxAmbient);

	*pdmax_mm = dmaxDark;
	if (dmaxDark > dmaxAmbient)
		*pdmax_mm = dmaxAmbient;

	LOG_FUNCTION_END(Status);

	return Status;
}


VL53L0X_Error VL53L0X_calc_sigma_estimate(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
	FixPoint1616_t *pSigmaEstimate,
	uint32_t *pDmax_mm)
{
	/* Expressed in 100ths of a ns, i.e. centi-ns */
	const uint32_t cPulseEffectiveWidth_centi_ns   = 800;
	/* Expressed in 100ths of a ns, i.e. centi-ns */
	const uint32_t cAmbientEffectiveWidth_centi_ns = 600;
	const FixPoint1616_t cSigmaEstRef	= 0x00000042; /* 0.001 */
	const uint32_t cVcselPulseWidth_ps	= 4700; /* pico secs */
	const FixPoint1616_t cSigmaEstMax	= 0x028F87AE;
	const FixPoint1616_t cSigmaEstRtnMax	= 0xF000;
	const FixPoint1616_t cAmbToSignalRatioMax = 0xF0000000/
		cAmbientEffectiveWidth_centi_ns;
	/* Time Of Flight per mm (6.6 pico secs) */
	const FixPoint1616_t cTOF_per_mm_ps		= 0x0006999A;
	const uint32_t c16BitRoundingParam		= 0x00008000;
	const FixPoint1616_t cMaxXTalk_kcps		= 0x00320000;
	const uint32_t cPllPeriod_ps			= 1655;

	uint32_t vcselTotalEventsRtn;
	uint32_t finalRangeTimeoutMicroSecs;
	uint32_t preRangeTimeoutMicroSecs;
	FixPoint1616_t sigmaEstimateP1;
	FixPoint1616_t sigmaEstimateP2;
	FixPoint1616_t sigmaEstimateP3;
	FixPoint1616_t deltaT_ps;
	FixPoint1616_t pwMult;
	FixPoint1616_t sigmaEstRtn;
	FixPoint1616_t sigmaEstimate;
	FixPoint1616_t xTalkCorrection;
	FixPoint1616_t ambientRate_kcps;
	FixPoint1616_t peakSignalRate_kcps;
	FixPoint1616_t xTalkCompRate_mcps;
	uint32_t xTalkCompRate_kcps;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	FixPoint1616_t diff1_mcps;
	FixPoint1616_t diff2_mcps;
	FixPoint1616_t sqr1;
	FixPoint1616_t sqr2;
	FixPoint1616_t sqrSum;
	FixPoint1616_t sqrtResult_centi_ns;
	FixPoint1616_t sqrtResult;
	FixPoint1616_t totalSignalRate_mcps;
	FixPoint1616_t correctedSignalRate_mcps;
	uint32_t vcselWidth;
	uint32_t finalRangeMacroPCLKS;
	uint32_t preRangeMacroPCLKS;
	uint32_t peakVcselDuration_us;
	uint8_t finalRangeVcselPCLKS;
	uint8_t preRangeVcselPCLKS;
	/*! \addtogroup calc_sigma_estimate
	 * @{
	 *
	 * Estimates the range sigma based on the
	 *
	 *	- vcsel_rate_kcps
	 *	- ambient_rate_kcps
	 *	- signal_total_events
	 *	- xtalk_rate
	 *
	 * and the following parameters
	 *
	 *	- SigmaEstRefArray
	 *	- SigmaEstEffPulseWidth
	 *	- SigmaEstEffAmbWidth
	 */

	LOG_FUNCTION_START("");

	VL53L0X_GETPARAMETERFIELD(Dev, XTalkCompensationRateMegaCps,
			xTalkCompRate_mcps);

	/*
	 * We work in kcps rather than mcps as this helps keep within the
	 * confines of the 32 Fix1616 type.
	 */

	ambientRate_kcps =
		(pRangingMeasurementData->AmbientRateRtnMegaCps * 1000) >> 16;

	correctedSignalRate_mcps =
		pRangingMeasurementData->SignalRateRtnMegaCps;


	Status = VL53L0X_get_total_signal_rate(
		Dev, pRangingMeasurementData, &totalSignalRate_mcps);
	Status = VL53L0X_get_total_xtalk_rate(
		Dev, pRangingMeasurementData, &xTalkCompRate_mcps);


	/* Signal rate measurement provided by device is the
	 * peak signal rate, not average.
	 */
	peakSignalRate_kcps = (totalSignalRate_mcps * 1000);
	peakSignalRate_kcps = (peakSignalRate_kcps + 0x8000) >> 16;

	xTalkCompRate_kcps = xTalkCompRate_mcps * 1000;

	if (xTalkCompRate_kcps > cMaxXTalk_kcps)
		xTalkCompRate_kcps = cMaxXTalk_kcps;

	if (Status == VL53L0X_ERROR_NONE) {

		/* Calculate final range macro periods */
		finalRangeTimeoutMicroSecs = VL53L0X_GETDEVICESPECIFICPARAMETER(
			Dev, FinalRangeTimeoutMicroSecs);

		finalRangeVcselPCLKS = VL53L0X_GETDEVICESPECIFICPARAMETER(
			Dev, FinalRangeVcselPulsePeriod);

		finalRangeMacroPCLKS = VL53L0X_calc_timeout_mclks(
			Dev, finalRangeTimeoutMicroSecs, finalRangeVcselPCLKS);

		/* Calculate pre-range macro periods */
		preRangeTimeoutMicroSecs = VL53L0X_GETDEVICESPECIFICPARAMETER(
			Dev, PreRangeTimeoutMicroSecs);

		preRangeVcselPCLKS = VL53L0X_GETDEVICESPECIFICPARAMETER(
			Dev, PreRangeVcselPulsePeriod);

		preRangeMacroPCLKS = VL53L0X_calc_timeout_mclks(
			Dev, preRangeTimeoutMicroSecs, preRangeVcselPCLKS);

		vcselWidth = 3;
		if (finalRangeVcselPCLKS == 8)
			vcselWidth = 2;


		peakVcselDuration_us = vcselWidth * 2048 *
			(preRangeMacroPCLKS + finalRangeMacroPCLKS);
		peakVcselDuration_us = (peakVcselDuration_us + 500)/1000;
		peakVcselDuration_us *= cPllPeriod_ps;
		peakVcselDuration_us = (peakVcselDuration_us + 500)/1000;

		/* Fix1616 >> 8 = Fix2408 */
		totalSignalRate_mcps = (totalSignalRate_mcps + 0x80) >> 8;

		/* Fix2408 * uint32 = Fix2408 */
		vcselTotalEventsRtn = totalSignalRate_mcps *
			peakVcselDuration_us;

		/* Fix2408 >> 8 = uint32 */
		vcselTotalEventsRtn = (vcselTotalEventsRtn + 0x80) >> 8;

		/* Fix2408 << 8 = Fix1616 = */
		totalSignalRate_mcps <<= 8;
	}

	if (Status != VL53L0X_ERROR_NONE) {
		LOG_FUNCTION_END(Status);
		return Status;
	}

	if (peakSignalRate_kcps == 0) {
		*pSigmaEstimate = cSigmaEstMax;
		PALDevDataSet(Dev, SigmaEstimate, cSigmaEstMax);
		*pDmax_mm = 0;
	} else {
		if (vcselTotalEventsRtn < 1)
			vcselTotalEventsRtn = 1;

		/*
		 * Calculate individual components of the main equation -
		 * replicating the equation implemented in the script
		 * OpenAll_Ewok_ranging_data.jsl.
		 *
		 * sigmaEstimateP1 represents the effective pulse width, which
		 * is a tuning parameter, rather than a real value.
		 *
		 * sigmaEstimateP2 represents the ambient/signal rate ratio
		 * expressed as a multiple of the effective ambient width
		 * (tuning parameter).
		 *
		 * sigmaEstimateP3 provides the signal event component, with the
		 * knowledge that
		 *	- Noise of a square pulse is 1/sqrt(12) of the pulse
		 *	 width.
		 *	- at 0Lux, sigma is proportional to
		 *	  effectiveVcselPulseWidth/sqrt(12 * signalTotalEvents)
		 *
		 * deltaT_ps represents the time of flight in pico secs for the
		 * current range measurement, using the "TOF per mm" constant
		 * (in ps).
		 */

		sigmaEstimateP1 = cPulseEffectiveWidth_centi_ns;

		/* ((FixPoint1616 << 16)* uint32)/uint32 = FixPoint1616 */
		sigmaEstimateP2 = (ambientRate_kcps << 16)/peakSignalRate_kcps;
		if (sigmaEstimateP2 > cAmbToSignalRatioMax) {
			/* Clip to prevent overflow. Will ensure safe
			 * max result. */
			sigmaEstimateP2 = cAmbToSignalRatioMax;
		}
		sigmaEstimateP2 *= cAmbientEffectiveWidth_centi_ns;

		sigmaEstimateP3 = 2 * VL53L0X_isqrt(vcselTotalEventsRtn * 12);

		/* uint32 * FixPoint1616 = FixPoint1616 */
		deltaT_ps = pRangingMeasurementData->RangeMilliMeter *
					cTOF_per_mm_ps;

		/*
		 * vcselRate - xtalkCompRate
		 * (uint32 << 16) - FixPoint1616 = FixPoint1616.
		 * Divide result by 1000 to convert to mcps.
		 * 500 is added to ensure rounding when integer division
		 * truncates.
		 */
		diff1_mcps = (((peakSignalRate_kcps << 16) -
			xTalkCompRate_kcps) + 500)/1000;

		/* vcselRate + xtalkCompRate */
		diff2_mcps = (((peakSignalRate_kcps << 16) +
			xTalkCompRate_kcps) + 500)/1000;

		/* Shift by 8 bits to increase resolution prior to the
		 * division */
		diff1_mcps <<= 8;

		/* FixPoint0824/FixPoint1616 = FixPoint2408 */
		xTalkCorrection	 = diff1_mcps/diff2_mcps;
		if(xTalkCorrection < 0){
			xTalkCorrection = -xTalkCorrection;
		}

		/* FixPoint2408 << 8 = FixPoint1616 */
		xTalkCorrection <<= 8;

		/* FixPoint1616/uint32 = FixPoint1616 */
		pwMult = deltaT_ps/cVcselPulseWidth_ps; /* smaller than 1.0f */

		/*
		 * FixPoint1616 * FixPoint1616 = FixPoint3232, however both
		 * values are small enough such that32 bits will not be
		 * exceeded.
		 */
		pwMult *= ((1 << 16) - xTalkCorrection);

		/* (FixPoint3232 >> 16) = FixPoint1616 */
		pwMult =  (pwMult + c16BitRoundingParam) >> 16;

		/* FixPoint1616 + FixPoint1616 = FixPoint1616 */
		pwMult += (1 << 16);

		/*
		 * At this point the value will be 1.xx, therefore if we square
		 * the value this will exceed 32 bits. To address this perform
		 * a single shift to the right before the multiplication.
		 */
		pwMult >>= 1;
		/* FixPoint1715 * FixPoint1715 = FixPoint3430 */
		pwMult = pwMult * pwMult;

		/* (FixPoint3430 >> 14) = Fix1616 */
		pwMult >>= 14;

		/* FixPoint1616 * uint32 = FixPoint1616 */
		sqr1 = pwMult * sigmaEstimateP1;

		/* (FixPoint1616 >> 16) = FixPoint3200 */
		sqr1 = (sqr1 + 0x8000) >> 16;

		/* FixPoint3200 * FixPoint3200 = FixPoint6400 */
		sqr1 *= sqr1;

		sqr2 = sigmaEstimateP2;

		/* (FixPoint1616 >> 16) = FixPoint3200 */
		sqr2 = (sqr2 + 0x8000) >> 16;

		/* FixPoint3200 * FixPoint3200 = FixPoint6400 */
		sqr2 *= sqr2;

		/* FixPoint64000 + FixPoint6400 = FixPoint6400 */
		sqrSum = sqr1 + sqr2;

		/* SQRT(FixPoin6400) = FixPoint3200 */
		sqrtResult_centi_ns = VL53L0X_isqrt(sqrSum);

		/* (FixPoint3200 << 16) = FixPoint1616 */
		sqrtResult_centi_ns <<= 16;

		/*
		 * Note that the Speed Of Light is expressed in um per 1E-10
		 * seconds (2997) Therefore to get mm/ns we have to divide by
		 * 10000
		 */
		sigmaEstRtn = (((sqrtResult_centi_ns+50)/100) /
				sigmaEstimateP3);
		sigmaEstRtn		 *= VL53L0X_SPEED_OF_LIGHT_IN_AIR;

		/* Add 5000 before dividing by 10000 to ensure rounding. */
		sigmaEstRtn		 += 5000;
		sigmaEstRtn		 /= 10000;

		if (sigmaEstRtn > cSigmaEstRtnMax) {
			/* Clip to prevent overflow. Will ensure safe
			 * max result. */
			sigmaEstRtn = cSigmaEstRtnMax;
		}

		/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
		sqr1 = sigmaEstRtn * sigmaEstRtn;
		/* FixPoint1616 * FixPoint1616 = FixPoint3232 */
		sqr2 = cSigmaEstRef * cSigmaEstRef;

		/* sqrt(FixPoint3232) = FixPoint1616 */
		sqrtResult = VL53L0X_isqrt((sqr1 + sqr2));
		/*
		 * Note that the Shift by 4 bits increases resolution prior to
		 * the sqrt, therefore the result must be shifted by 2 bits to
		 * the right to revert back to the FixPoint1616 format.
		 */

		sigmaEstimate	 = 1000 * sqrtResult;

		if ((peakSignalRate_kcps < 1) || (vcselTotalEventsRtn < 1) ||
				(sigmaEstimate > cSigmaEstMax)) {
				sigmaEstimate = cSigmaEstMax;
		}

		*pSigmaEstimate = (uint32_t)(sigmaEstimate);
		PALDevDataSet(Dev, SigmaEstimate, *pSigmaEstimate);
		Status = VL53L0X_calc_dmax(
			Dev,
			totalSignalRate_mcps,
			correctedSignalRate_mcps,
			pwMult,
			sigmaEstimateP1,
			sigmaEstimateP2,
			peakVcselDuration_us,
			pDmax_mm);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_pal_range_status(VL53L0X_DEV Dev,
		uint8_t DeviceRangeStatus,
		FixPoint1616_t SignalRate,
		uint16_t EffectiveSpadRtnCount,
		VL53L0X_RangingMeasurementData_t *pRangingMeasurementData,
		uint8_t *pPalRangeStatus)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t NoneFlag;
	uint8_t SigmaLimitflag = 0;
	uint8_t SignalRefClipflag = 0;
	uint8_t RangeIgnoreThresholdflag = 0;
	uint8_t SigmaLimitCheckEnable = 0;
	uint8_t SignalRateFinalRangeLimitCheckEnable = 0;
	uint8_t SignalRefClipLimitCheckEnable = 0;
	uint8_t RangeIgnoreThresholdLimitCheckEnable = 0;
	FixPoint1616_t SigmaEstimate;
	FixPoint1616_t SigmaLimitValue;
	FixPoint1616_t SignalRefClipValue;
	FixPoint1616_t RangeIgnoreThresholdValue;
	FixPoint1616_t SignalRatePerSpad;
	uint8_t DeviceRangeStatusInternal = 0;
	uint16_t tmpWord = 0;
	uint8_t Temp8;
	uint32_t Dmax_mm = 0;
	FixPoint1616_t LastSignalRefMcps;

	LOG_FUNCTION_START("");


	/*
	 * VL53L0X has a good ranging when the value of the
	 * DeviceRangeStatus = 11. This function will replace the value 0 with
	 * the value 11 in the DeviceRangeStatus.
	 * In addition, the SigmaEstimator is not included in the VL53L0X
	 * DeviceRangeStatus, this will be added in the PalRangeStatus.
	 */

	DeviceRangeStatusInternal = ((DeviceRangeStatus & 0x78) >> 3);

	if (DeviceRangeStatusInternal == 0 ||
		DeviceRangeStatusInternal == 5 ||
		DeviceRangeStatusInternal == 7 ||
		DeviceRangeStatusInternal == 12 ||
		DeviceRangeStatusInternal == 13 ||
		DeviceRangeStatusInternal == 14 ||
		DeviceRangeStatusInternal == 15
			) {
		NoneFlag = 1;
	} else {
		NoneFlag = 0;
	}

	/* LastSignalRefMcps */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_RdWord(Dev,
			VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_REF,
			&tmpWord);

	LastSignalRefMcps = VL53L0X_FIXPOINT97TOFIXPOINT1616(tmpWord);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);

	PALDevDataSet(Dev, LastSignalRefMcps, LastSignalRefMcps);

	/*
	 * Check if Sigma limit is enabled, if yes then do comparison with limit
	 * value and put the result back into pPalRangeStatus.
	 */
	if (Status == VL53L0X_ERROR_NONE)
		Status =  VL53L0X_GetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
			&SigmaLimitCheckEnable);

	if ((SigmaLimitCheckEnable != 0) && (Status == VL53L0X_ERROR_NONE)) {
		/*
		* compute the Sigma and check with limit
		*/
		Status = VL53L0X_calc_sigma_estimate(
			Dev,
			pRangingMeasurementData,
			&SigmaEstimate,
			&Dmax_mm);
		if (Status == VL53L0X_ERROR_NONE)
			pRangingMeasurementData->RangeDMaxMilliMeter = Dmax_mm;

		if (Status == VL53L0X_ERROR_NONE) {
			Status = VL53L0X_GetLimitCheckValue(Dev,
				VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
				&SigmaLimitValue);

			if ((SigmaLimitValue > 0) &&
				(SigmaEstimate > SigmaLimitValue))
					/* Limit Fail */
					SigmaLimitflag = 1;
		}
	}

	/*
	 * Check if Signal ref clip limit is enabled, if yes then do comparison
	 * with limit value and put the result back into pPalRangeStatus.
	 */
	if (Status == VL53L0X_ERROR_NONE)
		Status =  VL53L0X_GetLimitCheckEnable(Dev,
				VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
				&SignalRefClipLimitCheckEnable);

	if ((SignalRefClipLimitCheckEnable != 0) &&
			(Status == VL53L0X_ERROR_NONE)) {

		Status = VL53L0X_GetLimitCheckValue(Dev,
				VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
				&SignalRefClipValue);

		if ((SignalRefClipValue > 0) &&
				(LastSignalRefMcps > SignalRefClipValue)) {
			/* Limit Fail */
			SignalRefClipflag = 1;
		}
	}

	/*
	 * Check if Signal ref clip limit is enabled, if yes then do comparison
	 * with limit value and put the result back into pPalRangeStatus.
	 * EffectiveSpadRtnCount has a format 8.8
	 * If (Return signal rate < (1.5 x Xtalk x number of Spads)) : FAIL
	 */
	if (Status == VL53L0X_ERROR_NONE)
		Status =  VL53L0X_GetLimitCheckEnable(Dev,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				&RangeIgnoreThresholdLimitCheckEnable);

	if ((RangeIgnoreThresholdLimitCheckEnable != 0) &&
			(Status == VL53L0X_ERROR_NONE)) {

		/* Compute the signal rate per spad */
		if (EffectiveSpadRtnCount == 0) {
			SignalRatePerSpad = 0;
		} else {
			SignalRatePerSpad = (FixPoint1616_t)((256 * SignalRate)
				/ EffectiveSpadRtnCount);
		}

		Status = VL53L0X_GetLimitCheckValue(Dev,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				&RangeIgnoreThresholdValue);

		if ((RangeIgnoreThresholdValue > 0) &&
			(SignalRatePerSpad < RangeIgnoreThresholdValue)) {
			/* Limit Fail add 2^6 to range status */
			RangeIgnoreThresholdflag = 1;
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (NoneFlag == 1) {
			*pPalRangeStatus = 255;	 /* NONE */
		} else if (DeviceRangeStatusInternal == 1 ||
					DeviceRangeStatusInternal == 2 ||
					DeviceRangeStatusInternal == 3) {
			*pPalRangeStatus = 5; /* HW fail */
		} else if (DeviceRangeStatusInternal == 6 ||
					DeviceRangeStatusInternal == 9) {
			*pPalRangeStatus = 4;  /* Phase fail */
		} else if (DeviceRangeStatusInternal == 8 ||
					DeviceRangeStatusInternal == 10 ||
					SignalRefClipflag == 1) {
			*pPalRangeStatus = 3;  /* Min range */
		} else if (DeviceRangeStatusInternal == 4 ||
					RangeIgnoreThresholdflag == 1) {
			*pPalRangeStatus = 2;  /* Signal Fail */
		} else if (SigmaLimitflag == 1) {
			*pPalRangeStatus = 1;  /* Sigma	 Fail */
		} else {
			*pPalRangeStatus = 0; /* Range Valid */
		}
	}

	/* DMAX only relevant during range error */
	if (*pPalRangeStatus == 0)
		pRangingMeasurementData->RangeDMaxMilliMeter = 0;

	/* fill the Limit Check Status */

	Status =  VL53L0X_GetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
			&SignalRateFinalRangeLimitCheckEnable);

	if (Status == VL53L0X_ERROR_NONE) {
		if ((SigmaLimitCheckEnable == 0) || (SigmaLimitflag == 1))
			Temp8 = 1;
		else
			Temp8 = 0;
		VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksStatus,
				VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, Temp8);

		if ((DeviceRangeStatusInternal == 4) ||
				(SignalRateFinalRangeLimitCheckEnable == 0))
			Temp8 = 1;
		else
			Temp8 = 0;
		VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksStatus,
				VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
				Temp8);

		if ((SignalRefClipLimitCheckEnable == 0) ||
					(SignalRefClipflag == 1))
			Temp8 = 1;
		else
			Temp8 = 0;

		VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksStatus,
				VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, Temp8);

		if ((RangeIgnoreThresholdLimitCheckEnable == 0) ||
				(RangeIgnoreThresholdflag == 1))
			Temp8 = 1;
		else
			Temp8 = 0;

		VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksStatus,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				Temp8);
	}

	LOG_FUNCTION_END(Status);
	return Status;

}

/**************************************************api*************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/


VL53L0X_Error VL53L0X_GetVersion(VL53L0X_Version_t *pVersion)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	pVersion->major = VL53L0X_IMPLEMENTATION_VER_MAJOR;
	pVersion->minor = VL53L0X_IMPLEMENTATION_VER_MINOR;
	pVersion->build = VL53L0X_IMPLEMENTATION_VER_SUB;

	pVersion->revision = VL53L0X_IMPLEMENTATION_VER_REVISION;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetPalSpecVersion(VL53L0X_Version_t *pPalSpecVersion)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	pPalSpecVersion->major = VL53L0X_SPECIFICATION_VER_MAJOR;
	pPalSpecVersion->minor = VL53L0X_SPECIFICATION_VER_MINOR;
	pPalSpecVersion->build = VL53L0X_SPECIFICATION_VER_SUB;

	pPalSpecVersion->revision = VL53L0X_SPECIFICATION_VER_REVISION;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetProductRevision(VL53L0X_DEV Dev,
	uint8_t *pProductRevisionMajor, uint8_t *pProductRevisionMinor)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t revision_id;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_IDENTIFICATION_REVISION_ID,
		&revision_id);
	*pProductRevisionMajor = 1;
	*pProductRevisionMinor = (revision_id & 0xF0) >> 4;

	LOG_FUNCTION_END(Status);
	return Status;

}

VL53L0X_Error VL53L0X_GetDeviceInfo(VL53L0X_DEV Dev,
	VL53L0X_DeviceInfo_t *pVL53L0X_DeviceInfo)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_device_info(Dev, pVL53L0X_DeviceInfo);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetDeviceErrorStatus(VL53L0X_DEV Dev,
	VL53L0X_DeviceError *pDeviceErrorStatus)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t RangeStatus;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_RESULT_RANGE_STATUS,
		&RangeStatus);

	*pDeviceErrorStatus = (VL53L0X_DeviceError)((RangeStatus & 0x78) >> 3);

	LOG_FUNCTION_END(Status);
	return Status;
}


VL53L0X_Error VL53L0X_GetDeviceErrorString(VL53L0X_DeviceError ErrorCode,
	char *pDeviceErrorString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	Status = VL53L0X_get_device_error_string(ErrorCode, pDeviceErrorString);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetRangeStatusString(uint8_t RangeStatus,
	char *pRangeStatusString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_range_status_string(RangeStatus,
		pRangeStatusString);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetPalErrorString(VL53L0X_Error PalErrorCode,
	char *pPalErrorString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_pal_error_string(PalErrorCode, pPalErrorString);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetPalStateString(VL53L0X_State PalStateCode,
	char *pPalStateString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_pal_state_string(PalStateCode, pPalStateString);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetPalState(VL53L0X_DEV Dev, VL53L0X_State *pPalState)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	*pPalState = PALDevDataGet(Dev, PalState);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetPowerMode(VL53L0X_DEV Dev, VL53L0X_PowerModes PowerMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	/* Only level1 of Power mode exists */
	if ((PowerMode != VL53L0X_POWERMODE_STANDBY_LEVEL1)
		&& (PowerMode != VL53L0X_POWERMODE_IDLE_LEVEL1)) {
		Status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
	} else if (PowerMode == VL53L0X_POWERMODE_STANDBY_LEVEL1) {
		/* set the standby level1 of power mode */
		Status = VL53L0X_WrByte(Dev, 0x80, 0x00);
		if (Status == VL53L0X_ERROR_NONE) {
			/* Set PAL State to standby */
			PALDevDataSet(Dev, PalState, VL53L0X_STATE_STANDBY);
			PALDevDataSet(Dev, PowerMode,
				VL53L0X_POWERMODE_STANDBY_LEVEL1);
		}

	} else {
		/* VL53L0X_POWERMODE_IDLE_LEVEL1 */
		Status = VL53L0X_WrByte(Dev, 0x80, 0x00);
		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_StaticInit(Dev);

		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, PowerMode,
				VL53L0X_POWERMODE_IDLE_LEVEL1);

	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetPowerMode(VL53L0X_DEV Dev, VL53L0X_PowerModes *pPowerMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	/* Only level1 of Power mode exists */
	Status = VL53L0X_RdByte(Dev, 0x80, &Byte);

	if (Status == VL53L0X_ERROR_NONE) {
		if (Byte == 1) {
			PALDevDataSet(Dev, PowerMode,
				VL53L0X_POWERMODE_IDLE_LEVEL1);
		} else {
			PALDevDataSet(Dev, PowerMode,
				VL53L0X_POWERMODE_STANDBY_LEVEL1);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetOffsetCalibrationDataMicroMeter(VL53L0X_DEV Dev,
	int32_t OffsetCalibrationDataMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_set_offset_calibration_data_micro_meter(Dev,
		OffsetCalibrationDataMicroMeter);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetOffsetCalibrationDataMicroMeter(VL53L0X_DEV Dev,
	int32_t *pOffsetCalibrationDataMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_offset_calibration_data_micro_meter(Dev,
		pOffsetCalibrationDataMicroMeter);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetLinearityCorrectiveGain(VL53L0X_DEV Dev,
	int16_t LinearityCorrectiveGain)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	if ((LinearityCorrectiveGain < 0) || (LinearityCorrectiveGain > 1000))
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	else {
		PALDevDataSet(Dev, LinearityCorrectiveGain,
			LinearityCorrectiveGain);

		if (LinearityCorrectiveGain != 1000) {
			/* Disable FW Xtalk */
			Status = VL53L0X_WrWord(Dev,
			VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS, 0);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetLinearityCorrectiveGain(VL53L0X_DEV Dev,
	uint16_t *pLinearityCorrectiveGain)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	*pLinearityCorrectiveGain = PALDevDataGet(Dev, LinearityCorrectiveGain);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetGroupParamHold(VL53L0X_DEV Dev, uint8_t GroupParamHold)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetUpperLimitMilliMeter(VL53L0X_DEV Dev,
	uint16_t *pUpperLimitMilliMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetTotalSignalRate(VL53L0X_DEV Dev,
	FixPoint1616_t *pTotalSignalRate)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_RangingMeasurementData_t LastRangeDataBuffer;

	LOG_FUNCTION_START("");

	LastRangeDataBuffer = PALDevDataGet(Dev, LastRangeMeasure);

	Status = VL53L0X_get_total_signal_rate(
		Dev, &LastRangeDataBuffer, pTotalSignalRate);

	LOG_FUNCTION_END(Status);
	return Status;
}

/* End Group PAL General Functions */

/* Group PAL Init Functions */
VL53L0X_Error VL53L0X_SetDeviceAddress(VL53L0X_DEV Dev, uint8_t DeviceAddress)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS,
		DeviceAddress / 2);

	LOG_FUNCTION_END(Status);
	return Status;
}

#define USE_I2C_2V8

VL53L0X_Error VL53L0X_DataInit(VL53L0X_DEV Dev)
{
	uint8_t b = 0;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_DeviceParameters_t CurrentParameters;
	int i;
	uint8_t StopVariable;

	LOG_FUNCTION_START("");

	/* by default the I2C is running at 1V8 if you want to change it you
	 * need to include this define at compilation level. */
#ifdef USE_I2C_2V8
	Status = VL53L0X_UpdateByte(Dev,
		VL53L0X_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
		0xFE,
		0x01);
#endif

	/* Set I2C standard mode */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0x88, 0x00);

	/* read WHO_AM_I */
	
	Status = VL53L0X_RdByte(Dev, 0xC0, &b);
	   
	/* read WHO_AM_I */

	VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, ReadDataFromDeviceDone, 0);

#ifdef USE_IQC_STATION
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_apply_offset_adjustment(Dev);
#endif

	/* Default value is 1000 for Linearity Corrective Gain */
	PALDevDataSet(Dev, LinearityCorrectiveGain, 1000);

	/* Dmax default Parameter */
	PALDevDataSet(Dev, DmaxCalRangeMilliMeter, 400);
	PALDevDataSet(Dev, DmaxCalSignalRateRtnMegaCps,
		(FixPoint1616_t)((0x00016B85))); /* 1.42 No Cover Glass*/

	/* Set Default static parameters
	 *set first temporary values 9.44MHz * 65536 = 618660 */
	VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, OscFrequencyMHz, 618660);

	/* Set Default XTalkCompensationRateMegaCps to 0  */
	VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationRateMegaCps, 0);

	/* Get default parameters */
	Status = VL53L0X_GetDeviceParameters(Dev, &CurrentParameters);

	if (Status == VL53L0X_ERROR_NONE) {
		/* initialize PAL values */
		CurrentParameters.DeviceMode = VL53L0X_DEVICEMODE_SINGLE_RANGING;
		CurrentParameters.HistogramMode = VL53L0X_HISTOGRAMMODE_DISABLED;
		PALDevDataSet(Dev, CurrentParameters, CurrentParameters);
	}

	/* Sigma estimator variable */
	PALDevDataSet(Dev, SigmaEstRefArray, 100);
	PALDevDataSet(Dev, SigmaEstEffPulseWidth, 900);
	PALDevDataSet(Dev, SigmaEstEffAmbWidth, 500);
	PALDevDataSet(Dev, targetRefRate, 0x0A00); /* 20 MCPS in 9:7 format */

	/* Use internal default settings */
	PALDevDataSet(Dev, UseInternalTuningSettings, 1);

	Status |= VL53L0X_WrByte(Dev, 0x80, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0x00, 0x00);
	Status |= VL53L0X_RdByte(Dev, 0x91, &StopVariable);
	PALDevDataSet(Dev, StopVariable, StopVariable);
	Status |= VL53L0X_WrByte(Dev, 0x00, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);
	Status |= VL53L0X_WrByte(Dev, 0x80, 0x00);

	/* Enable all check */
	for (i = 0; i < VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS; i++) {
		if (Status == VL53L0X_ERROR_NONE)
			Status |= VL53L0X_SetLimitCheckEnable(Dev, i, 1);
		else
			break;

	}

	/* Disable the following checks */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, 0);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC, 0);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetLimitCheckEnable(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE, 0);

	/* Limit default values */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckValue(Dev,
			VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
				(FixPoint1616_t)(18 * 65536));
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckValue(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
				(FixPoint1616_t)(25 * 65536 / 100));
				/* 0.25 * 65536 */
	}

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckValue(Dev,
			VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
				(FixPoint1616_t)(35 * 65536));
	}

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckValue(Dev,
			VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				(FixPoint1616_t)(0 * 65536));
	}

	if (Status == VL53L0X_ERROR_NONE) {

		PALDevDataSet(Dev, SequenceConfig, 0xFF);
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
			0xFF);

		/* Set PAL state to tell that we are waiting for call to
		 * VL53L0X_StaticInit */
		PALDevDataSet(Dev, PalState, VL53L0X_STATE_WAIT_STATICINIT);
	}

	if (Status == VL53L0X_ERROR_NONE)
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, RefSpadsInitialised, 0);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetTuningSettingBuffer(VL53L0X_DEV Dev,
	uint8_t *pTuningSettingBuffer, uint8_t UseInternalTuningSettings)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	if (UseInternalTuningSettings == 1) {
		/* Force use internal settings */
		PALDevDataSet(Dev, UseInternalTuningSettings, 1);
	} else {

		/* check that the first byte is not 0 */
		if (*pTuningSettingBuffer != 0) {
			PALDevDataSet(Dev, pTuningSettingsPointer,
				pTuningSettingBuffer);
			PALDevDataSet(Dev, UseInternalTuningSettings, 0);

		} else {
			Status = VL53L0X_ERROR_INVALID_PARAMS;
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetTuningSettingBuffer(VL53L0X_DEV Dev,
	uint8_t **ppTuningSettingBuffer, uint8_t *pUseInternalTuningSettings)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	*ppTuningSettingBuffer = PALDevDataGet(Dev, pTuningSettingsPointer);
	*pUseInternalTuningSettings = PALDevDataGet(Dev,
		UseInternalTuningSettings);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_StaticInit(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_DeviceParameters_t CurrentParameters = {0};
	uint8_t *pTuningSettingBuffer;
	uint16_t tempword = 0;
	uint8_t tempbyte = 0;
	uint8_t UseInternalTuningSettings = 0;
	uint32_t count = 0;
	uint8_t isApertureSpads = 0;
	uint32_t refSpadCount = 0;
	uint8_t ApertureSpads = 0;
	uint8_t vcselPulsePeriodPCLK;
	FixPoint1616_t seqTimeoutMilliSecs;

	LOG_FUNCTION_START("");

	Status = VL53L0X_get_info_from_device(Dev, 1);

	/* set the ref spad from NVM */
	count	= (uint32_t)VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
		ReferenceSpadCount);
	ApertureSpads = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
		ReferenceSpadType);

	/* NVM value invalid */
	if ((ApertureSpads > 1) ||
		((ApertureSpads == 1) && (count > 32)) ||
		((ApertureSpads == 0) && (count > 12)))
		{
			Status = VL53L0X_perform_ref_spad_management(Dev, &refSpadCount,
			&isApertureSpads);
		}
		
	else
	{
		Status = VL53L0X_set_reference_spads(Dev, count, ApertureSpads);
	}
		
	/* Initialize tuning settings buffer to prevent compiler warning. */
	pTuningSettingBuffer = DefaultTuningSettings;

	if (Status == VL53L0X_ERROR_NONE) {
		UseInternalTuningSettings = PALDevDataGet(Dev,
			UseInternalTuningSettings);

		if (UseInternalTuningSettings == 0)
			pTuningSettingBuffer = PALDevDataGet(Dev,
				pTuningSettingsPointer);
		else
			pTuningSettingBuffer = DefaultTuningSettings;

	}

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_load_tuning_settings(Dev, pTuningSettingBuffer);

	/* Set interrupt config to new sample ready */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetGpioConfig(Dev, 0, 0,
		VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY,
		VL53L0X_INTERRUPTPOLARITY_LOW);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
		Status |= VL53L0X_RdWord(Dev, 0x84, &tempword);
		Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, OscFrequencyMHz,
			VL53L0X_FIXPOINT412TOFIXPOINT1616(tempword));
	}
	/* After static init, some device parameters may be changed,
	 * so update them */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetDeviceParameters(Dev, &CurrentParameters);


	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetFractionEnable(Dev, &tempbyte);
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, RangeFractionalEnable, tempbyte);

	}

	if (Status == VL53L0X_ERROR_NONE)
		PALDevDataSet(Dev, CurrentParameters, CurrentParameters);

	/* read the sequence config and save it */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_RdByte(Dev,
		VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &tempbyte);
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, tempbyte);

	}

	/* Disable MSRC and TCC by default */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetSequenceStepEnable(Dev,
					VL53L0X_SEQUENCESTEP_TCC, 0);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetSequenceStepEnable(Dev,
		VL53L0X_SEQUENCESTEP_MSRC, 0);


	/* Set PAL State to standby */
	if (Status == VL53L0X_ERROR_NONE)
		PALDevDataSet(Dev, PalState, VL53L0X_STATE_IDLE);



	/* Store pre-range vcsel period */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetVcselPulsePeriod(
			Dev,
			VL53L0X_VCSEL_PERIOD_PRE_RANGE,
			&vcselPulsePeriodPCLK);
	}

	if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETDEVICESPECIFICPARAMETER(
				Dev,
				PreRangeVcselPulsePeriod,
				vcselPulsePeriodPCLK);
	}
	/* Store final-range vcsel period */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetVcselPulsePeriod(
			Dev,
			VL53L0X_VCSEL_PERIOD_FINAL_RANGE,
			&vcselPulsePeriodPCLK);
	}

	if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETDEVICESPECIFICPARAMETER(
				Dev,
				FinalRangeVcselPulsePeriod,
				vcselPulsePeriodPCLK);
	}

	/* Store pre-range timeout */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetSequenceStepTimeout(
			Dev,
			VL53L0X_SEQUENCESTEP_PRE_RANGE,
			&seqTimeoutMilliSecs);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETDEVICESPECIFICPARAMETER(
			Dev,
			PreRangeTimeoutMicroSecs,
			seqTimeoutMilliSecs);
	}

	/* Store final-range timeout */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetSequenceStepTimeout(
			Dev,
			VL53L0X_SEQUENCESTEP_FINAL_RANGE,
			&seqTimeoutMilliSecs);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETDEVICESPECIFICPARAMETER(
			Dev,
			FinalRangeTimeoutMicroSecs,
			seqTimeoutMilliSecs);
	}
	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_WaitDeviceBooted(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_ResetDevice(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	/* Set reset bit */
	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N,
		0x00);

	/* Wait for some time */
	if (Status == VL53L0X_ERROR_NONE) {
		do {
			Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Byte);
		} while (Byte != 0x00);
	}

	/* Release reset */
	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N,
		0x01);

	/* Wait until correct boot-up of the device */
	if (Status == VL53L0X_ERROR_NONE) {
		do {
			Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Byte);
		} while (Byte == 0x00);
	}

	/* Set PAL State to VL53L0X_STATE_POWERDOWN */
	if (Status == VL53L0X_ERROR_NONE)
		PALDevDataSet(Dev, PalState, VL53L0X_STATE_POWERDOWN);


	LOG_FUNCTION_END(Status);
	return Status;
}
/* End Group PAL Init Functions */

/* Group PAL Parameters Functions */
VL53L0X_Error VL53L0X_SetDeviceParameters(VL53L0X_DEV Dev,
	const VL53L0X_DeviceParameters_t *pDeviceParameters)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int i;
	LOG_FUNCTION_START("");
	Status = VL53L0X_SetDeviceMode(Dev, pDeviceParameters->DeviceMode);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetInterMeasurementPeriodMilliSeconds(Dev,
			pDeviceParameters->InterMeasurementPeriodMilliSeconds);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetXTalkCompensationRateMegaCps(Dev,
			pDeviceParameters->XTalkCompensationRateMegaCps);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetOffsetCalibrationDataMicroMeter(Dev,
			pDeviceParameters->RangeOffsetMicroMeters);


	for (i = 0; i < VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS; i++) {
		if (Status == VL53L0X_ERROR_NONE)
			Status |= VL53L0X_SetLimitCheckEnable(Dev, i,
				pDeviceParameters->LimitChecksEnable[i]);
		else
			break;

		if (Status == VL53L0X_ERROR_NONE)
			Status |= VL53L0X_SetLimitCheckValue(Dev, i,
				pDeviceParameters->LimitChecksValue[i]);
		else
			break;

	}

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetWrapAroundCheckEnable(Dev,
			pDeviceParameters->WrapAroundCheckEnable);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev,
			pDeviceParameters->MeasurementTimingBudgetMicroSeconds);


	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetDeviceParameters(VL53L0X_DEV Dev,
	VL53L0X_DeviceParameters_t *pDeviceParameters)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int i;

	LOG_FUNCTION_START("");

	Status = VL53L0X_GetDeviceMode(Dev, &(pDeviceParameters->DeviceMode));

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetInterMeasurementPeriodMilliSeconds(Dev,
		&(pDeviceParameters->InterMeasurementPeriodMilliSeconds));


	if (Status == VL53L0X_ERROR_NONE)
		pDeviceParameters->XTalkCompensationEnable = 0;

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetXTalkCompensationRateMegaCps(Dev,
			&(pDeviceParameters->XTalkCompensationRateMegaCps));


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetOffsetCalibrationDataMicroMeter(Dev,
			&(pDeviceParameters->RangeOffsetMicroMeters));


	if (Status == VL53L0X_ERROR_NONE) {
		for (i = 0; i < VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS; i++) {
			/* get first the values, then the enables.
			 * VL53L0X_GetLimitCheckValue will modify the enable
			 * flags
			 */
			if (Status == VL53L0X_ERROR_NONE) {
				Status |= VL53L0X_GetLimitCheckValue(Dev, i,
				&(pDeviceParameters->LimitChecksValue[i]));
			} else {
				break;
			}
			if (Status == VL53L0X_ERROR_NONE) {
				Status |= VL53L0X_GetLimitCheckEnable(Dev, i,
				&(pDeviceParameters->LimitChecksEnable[i]));
			} else {
				break;
			}
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetWrapAroundCheckEnable(Dev,
			&(pDeviceParameters->WrapAroundCheckEnable));
	}

	/* Need to be done at the end as it uses VCSELPulsePeriod */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetMeasurementTimingBudgetMicroSeconds(Dev,
		&(pDeviceParameters->MeasurementTimingBudgetMicroSeconds));
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes DeviceMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("%d", (int)DeviceMode);

	switch (DeviceMode) {
	case VL53L0X_DEVICEMODE_SINGLE_RANGING:
	case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
	case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
	case VL53L0X_DEVICEMODE_GPIO_DRIVE:
	case VL53L0X_DEVICEMODE_GPIO_OSC:
		/* Supported modes */
		VL53L0X_SETPARAMETERFIELD(Dev, DeviceMode, DeviceMode);
		break;
	default:
		/* Unsupported mode */
		Status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetDeviceMode(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes *pDeviceMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	VL53L0X_GETPARAMETERFIELD(Dev, DeviceMode, *pDeviceMode);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetRangeFractionEnable(VL53L0X_DEV Dev,	uint8_t Enable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("%d", (int)Enable);

	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_RANGE_CONFIG, Enable);

	if (Status == VL53L0X_ERROR_NONE)
		PALDevDataSet(Dev, RangeFractionalEnable, Enable);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetFractionEnable(VL53L0X_DEV Dev, uint8_t *pEnabled)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_RANGE_CONFIG, pEnabled);

	if (Status == VL53L0X_ERROR_NONE)
		*pEnabled = (*pEnabled & 1);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetHistogramMode(VL53L0X_DEV Dev,
	VL53L0X_HistogramModes HistogramMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetHistogramMode(VL53L0X_DEV Dev,
	VL53L0X_HistogramModes *pHistogramMode)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev,
	uint32_t MeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_set_measurement_timing_budget_micro_seconds(Dev,
		MeasurementTimingBudgetMicroSeconds);

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_GetMeasurementTimingBudgetMicroSeconds(VL53L0X_DEV Dev,
	uint32_t *pMeasurementTimingBudgetMicroSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_measurement_timing_budget_micro_seconds(Dev,
		pMeasurementTimingBudgetMicroSeconds);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetVcselPulsePeriod(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t VCSELPulsePeriodPCLK)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_set_vcsel_pulse_period(Dev, VcselPeriodType,
		VCSELPulsePeriodPCLK);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetVcselPulsePeriod(VL53L0X_DEV Dev,
	VL53L0X_VcselPeriod VcselPeriodType, uint8_t *pVCSELPulsePeriodPCLK)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_vcsel_pulse_period(Dev, VcselPeriodType,
		pVCSELPulsePeriodPCLK);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetSequenceStepEnable(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, uint8_t SequenceStepEnabled)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;
	uint8_t SequenceConfigNew = 0;
	uint32_t MeasurementTimingBudgetMicroSeconds;
	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
		&SequenceConfig);

	SequenceConfigNew = SequenceConfig;

	if (Status == VL53L0X_ERROR_NONE) {
		if (SequenceStepEnabled == 1) {

			/* Enable requested sequence step
			 */
			switch (SequenceStepId) {
			case VL53L0X_SEQUENCESTEP_TCC:
				SequenceConfigNew |= 0x10;
				break;
			case VL53L0X_SEQUENCESTEP_DSS:
				SequenceConfigNew |= 0x28;
				break;
			case VL53L0X_SEQUENCESTEP_MSRC:
				SequenceConfigNew |= 0x04;
				break;
			case VL53L0X_SEQUENCESTEP_PRE_RANGE:
				SequenceConfigNew |= 0x40;
				break;
			case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
				SequenceConfigNew |= 0x80;
				break;
			default:
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}
		} else {
			/* Disable requested sequence step
			 */
			switch (SequenceStepId) {
			case VL53L0X_SEQUENCESTEP_TCC:
				SequenceConfigNew &= 0xef;
				break;
			case VL53L0X_SEQUENCESTEP_DSS:
				SequenceConfigNew &= 0xd7;
				break;
			case VL53L0X_SEQUENCESTEP_MSRC:
				SequenceConfigNew &= 0xfb;
				break;
			case VL53L0X_SEQUENCESTEP_PRE_RANGE:
				SequenceConfigNew &= 0xbf;
				break;
			case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
				SequenceConfigNew &= 0x7f;
				break;
			default:
				Status = VL53L0X_ERROR_INVALID_PARAMS;
			}
		}
	}

	if (SequenceConfigNew != SequenceConfig) {
		/* Apply New Setting */
		if (Status == VL53L0X_ERROR_NONE) {
			Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, SequenceConfigNew);
		}
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, SequenceConfigNew);


		/* Recalculate timing budget */
		if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_GETPARAMETERFIELD(Dev,
				MeasurementTimingBudgetMicroSeconds,
				MeasurementTimingBudgetMicroSeconds);

			VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev,
				MeasurementTimingBudgetMicroSeconds);
		}
	}

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error sequence_step_enabled(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, uint8_t SequenceConfig,
	uint8_t *pSequenceStepEnabled)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	*pSequenceStepEnabled = 0;
	LOG_FUNCTION_START("");

	switch (SequenceStepId) {
	case VL53L0X_SEQUENCESTEP_TCC:
		*pSequenceStepEnabled = (SequenceConfig & 0x10) >> 4;
		break;
	case VL53L0X_SEQUENCESTEP_DSS:
		*pSequenceStepEnabled = (SequenceConfig & 0x08) >> 3;
		break;
	case VL53L0X_SEQUENCESTEP_MSRC:
		*pSequenceStepEnabled = (SequenceConfig & 0x04) >> 2;
		break;
	case VL53L0X_SEQUENCESTEP_PRE_RANGE:
		*pSequenceStepEnabled = (SequenceConfig & 0x40) >> 6;
		break;
	case VL53L0X_SEQUENCESTEP_FINAL_RANGE:
		*pSequenceStepEnabled = (SequenceConfig & 0x80) >> 7;
		break;
	default:
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetSequenceStepEnable(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, uint8_t *pSequenceStepEnabled)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;
	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
		&SequenceConfig);

	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev, SequenceStepId,
			SequenceConfig, pSequenceStepEnabled);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetSequenceStepEnables(VL53L0X_DEV Dev,
	VL53L0X_SchedulerSequenceSteps_t *pSchedulerSequenceSteps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;
	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
		&SequenceConfig);

	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev,
		VL53L0X_SEQUENCESTEP_TCC, SequenceConfig,
			&pSchedulerSequenceSteps->TccOn);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev,
		VL53L0X_SEQUENCESTEP_DSS, SequenceConfig,
			&pSchedulerSequenceSteps->DssOn);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev,
		VL53L0X_SEQUENCESTEP_MSRC, SequenceConfig,
			&pSchedulerSequenceSteps->MsrcOn);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev,
		VL53L0X_SEQUENCESTEP_PRE_RANGE, SequenceConfig,
			&pSchedulerSequenceSteps->PreRangeOn);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		Status = sequence_step_enabled(Dev,
		VL53L0X_SEQUENCESTEP_FINAL_RANGE, SequenceConfig,
			&pSchedulerSequenceSteps->FinalRangeOn);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetNumberOfSequenceSteps(VL53L0X_DEV Dev,
	uint8_t *pNumberOfSequenceSteps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	*pNumberOfSequenceSteps = VL53L0X_SEQUENCESTEP_NUMBER_OF_CHECKS;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetSequenceStepsInfo(VL53L0X_SequenceStepId SequenceStepId,
	char *pSequenceStepsString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_sequence_steps_info(
			SequenceStepId,
			pSequenceStepsString);

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_SetSequenceStepTimeout(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, FixPoint1616_t TimeOutMilliSecs)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Error Status1 = VL53L0X_ERROR_NONE;
	uint32_t TimeoutMicroSeconds = ((TimeOutMilliSecs * 1000) + 0x8000)
		>> 16;
	uint32_t MeasurementTimingBudgetMicroSeconds;
	FixPoint1616_t OldTimeOutMicroSeconds;

	LOG_FUNCTION_START("");

	/* Read back the current value in case we need to revert back to this.
	 */
	Status = get_sequence_step_timeout(Dev, SequenceStepId,
		&OldTimeOutMicroSeconds);

	if (Status == VL53L0X_ERROR_NONE) {
		Status = set_sequence_step_timeout(Dev, SequenceStepId,
			TimeoutMicroSeconds);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_GETPARAMETERFIELD(Dev,
			MeasurementTimingBudgetMicroSeconds,
			MeasurementTimingBudgetMicroSeconds);

		/* At this point we don't know if the requested value is valid,
		 therefore proceed to update the entire timing budget and
		 if this fails, revert back to the previous value.
		 */
		Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(Dev,
			MeasurementTimingBudgetMicroSeconds);

		if (Status != VL53L0X_ERROR_NONE) {
			Status1 = set_sequence_step_timeout(Dev, SequenceStepId,
				OldTimeOutMicroSeconds);

			if (Status1 == VL53L0X_ERROR_NONE) {
				Status1 =
				VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
					Dev,
					MeasurementTimingBudgetMicroSeconds);
			}

			Status = Status1;
		}
	}

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_GetSequenceStepTimeout(VL53L0X_DEV Dev,
	VL53L0X_SequenceStepId SequenceStepId, FixPoint1616_t *pTimeOutMilliSecs)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t TimeoutMicroSeconds;
	uint32_t WholeNumber_ms = 0;
	uint32_t Fraction_ms = 0;
	LOG_FUNCTION_START("");

	Status = get_sequence_step_timeout(Dev, SequenceStepId,
		&TimeoutMicroSeconds);
	if (Status == VL53L0X_ERROR_NONE) {
		WholeNumber_ms = TimeoutMicroSeconds / 1000;
		Fraction_ms = TimeoutMicroSeconds - (WholeNumber_ms * 1000);
		*pTimeOutMilliSecs = (WholeNumber_ms << 16)
			+ (((Fraction_ms * 0xffff) + 500) / 1000);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev,
	uint32_t InterMeasurementPeriodMilliSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t osc_calibrate_val;
	uint32_t IMPeriodMilliSeconds;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdWord(Dev, VL53L0X_REG_OSC_CALIBRATE_VAL,
		&osc_calibrate_val);

	if (Status == VL53L0X_ERROR_NONE) {
		if (osc_calibrate_val != 0) {
			IMPeriodMilliSeconds =
				InterMeasurementPeriodMilliSeconds
					* osc_calibrate_val;
		} else {
			IMPeriodMilliSeconds =
				InterMeasurementPeriodMilliSeconds;
		}
		Status = VL53L0X_WrDWord(Dev,
		VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD,
			IMPeriodMilliSeconds);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETPARAMETERFIELD(Dev,
			InterMeasurementPeriodMilliSeconds,
			InterMeasurementPeriodMilliSeconds);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetInterMeasurementPeriodMilliSeconds(VL53L0X_DEV Dev,
	uint32_t *pInterMeasurementPeriodMilliSeconds)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t osc_calibrate_val;
	uint32_t IMPeriodMilliSeconds;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdWord(Dev, VL53L0X_REG_OSC_CALIBRATE_VAL,
		&osc_calibrate_val);

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_RdDWord(Dev,
		VL53L0X_REG_SYSTEM_INTERMEASUREMENT_PERIOD,
			&IMPeriodMilliSeconds);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (osc_calibrate_val != 0) {
			*pInterMeasurementPeriodMilliSeconds =
				IMPeriodMilliSeconds / osc_calibrate_val;
		}
		VL53L0X_SETPARAMETERFIELD(Dev,
			InterMeasurementPeriodMilliSeconds,
			*pInterMeasurementPeriodMilliSeconds);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetXTalkCompensationEnable(VL53L0X_DEV Dev,
	uint8_t XTalkCompensationEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	FixPoint1616_t TempFix1616;
	uint16_t LinearityCorrectiveGain;

	LOG_FUNCTION_START("");

	LinearityCorrectiveGain = PALDevDataGet(Dev, LinearityCorrectiveGain);

	if ((XTalkCompensationEnable == 0)
		|| (LinearityCorrectiveGain != 1000)) {
		TempFix1616 = 0;
	} else {
		VL53L0X_GETPARAMETERFIELD(Dev, XTalkCompensationRateMegaCps,
			TempFix1616);
	}

	/* the following register has a format 3.13 */
	Status = VL53L0X_WrWord(Dev,
	VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS,
		VL53L0X_FIXPOINT1616TOFIXPOINT313(TempFix1616));

	if (Status == VL53L0X_ERROR_NONE) {
		if (XTalkCompensationEnable == 0) {
			VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationEnable,
				0);
		} else {
			VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationEnable,
				1);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetXTalkCompensationEnable(VL53L0X_DEV Dev,
	uint8_t *pXTalkCompensationEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Temp8;
	LOG_FUNCTION_START("");

	VL53L0X_GETPARAMETERFIELD(Dev, XTalkCompensationEnable, Temp8);
	*pXTalkCompensationEnable = Temp8;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetXTalkCompensationRateMegaCps(VL53L0X_DEV Dev,
	FixPoint1616_t XTalkCompensationRateMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Temp8;
	uint16_t LinearityCorrectiveGain;
	uint16_t data;
	LOG_FUNCTION_START("");

	VL53L0X_GETPARAMETERFIELD(Dev, XTalkCompensationEnable, Temp8);
	LinearityCorrectiveGain = PALDevDataGet(Dev, LinearityCorrectiveGain);

	if (Temp8 == 0) { /* disabled write only internal value */
		VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationRateMegaCps,
			XTalkCompensationRateMegaCps);
	} else {
		/* the following register has a format 3.13 */
		if (LinearityCorrectiveGain == 1000) {
			data = VL53L0X_FIXPOINT1616TOFIXPOINT313(
				XTalkCompensationRateMegaCps);
		} else {
			data = 0;
		}

		Status = VL53L0X_WrWord(Dev,
		VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS, data);

		if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETPARAMETERFIELD(Dev,
				XTalkCompensationRateMegaCps,
				XTalkCompensationRateMegaCps);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetXTalkCompensationRateMegaCps(VL53L0X_DEV Dev,
	FixPoint1616_t *pXTalkCompensationRateMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t Value;
	FixPoint1616_t TempFix1616;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdWord(Dev,
	VL53L0X_REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS, (uint16_t *)&Value);
	if (Status == VL53L0X_ERROR_NONE) {
		if (Value == 0) {
			/* the Xtalk is disabled return value from memory */
			VL53L0X_GETPARAMETERFIELD(Dev,
				XTalkCompensationRateMegaCps, TempFix1616);
			*pXTalkCompensationRateMegaCps = TempFix1616;
			VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationEnable,
				0);
		} else {
			TempFix1616 = VL53L0X_FIXPOINT313TOFIXPOINT1616(Value);
			*pXTalkCompensationRateMegaCps = TempFix1616;
			VL53L0X_SETPARAMETERFIELD(Dev,
				XTalkCompensationRateMegaCps, TempFix1616);
			VL53L0X_SETPARAMETERFIELD(Dev, XTalkCompensationEnable,
				1);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetRefCalibration(VL53L0X_DEV Dev, uint8_t VhvSettings,
	uint8_t PhaseCal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_set_ref_calibration(Dev, VhvSettings, PhaseCal);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetRefCalibration(VL53L0X_DEV Dev, uint8_t *pVhvSettings,
	uint8_t *pPhaseCal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_ref_calibration(Dev, pVhvSettings, pPhaseCal);

	LOG_FUNCTION_END(Status);
	return Status;
}

/*
 * CHECK LIMIT FUNCTIONS
 */

VL53L0X_Error VL53L0X_GetNumberOfLimitCheck(uint16_t *pNumberOfLimitCheck)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	*pNumberOfLimitCheck = VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetLimitCheckInfo(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	char *pLimitCheckString)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	Status = VL53L0X_get_limit_check_info(Dev, LimitCheckId,
		pLimitCheckString);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetLimitCheckStatus(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	uint8_t *pLimitCheckStatus)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Temp8;

	LOG_FUNCTION_START("");

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	} else {

		VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksStatus,
			LimitCheckId, Temp8);

		*pLimitCheckStatus = Temp8;

	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	uint8_t LimitCheckEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	FixPoint1616_t TempFix1616 = 0;
	uint8_t LimitCheckEnableInt = 0;
	uint8_t LimitCheckDisable = 0;
	uint8_t Temp8;

	LOG_FUNCTION_START("");

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	} else {
		if (LimitCheckEnable == 0) {
			TempFix1616 = 0;
			LimitCheckEnableInt = 0;
			LimitCheckDisable = 1;

		} else {
			VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
				LimitCheckId, TempFix1616);
			LimitCheckDisable = 0;
			/* this to be sure to have either 0 or 1 */
			LimitCheckEnableInt = 1;
		}

		switch (LimitCheckId) {

		case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
				VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
				LimitCheckEnableInt);

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:

			Status = VL53L0X_WrWord(Dev,
			VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
				VL53L0X_FIXPOINT1616TOFIXPOINT97(TempFix1616));

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:

			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
				VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
				LimitCheckEnableInt);

			break;

		case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:

			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				LimitCheckEnableInt);

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:

			Temp8 = (uint8_t)(LimitCheckDisable << 1);
			Status = VL53L0X_UpdateByte(Dev,
				VL53L0X_REG_MSRC_CONFIG_CONTROL,
				0xFE, Temp8);

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:

			Temp8 = (uint8_t)(LimitCheckDisable << 4);
			Status = VL53L0X_UpdateByte(Dev,
				VL53L0X_REG_MSRC_CONFIG_CONTROL,
				0xEF, Temp8);

			break;


		default:
			Status = VL53L0X_ERROR_INVALID_PARAMS;

		}

	}

	if (Status == VL53L0X_ERROR_NONE) {
		if (LimitCheckEnable == 0) {
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
				LimitCheckId, 0);
		} else {
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
				LimitCheckId, 1);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetLimitCheckEnable(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	uint8_t *pLimitCheckEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Temp8;

	LOG_FUNCTION_START("");

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
		*pLimitCheckEnable = 0;
	} else {
		VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksEnable,
			LimitCheckId, Temp8);
		*pLimitCheckEnable = Temp8;
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetLimitCheckValue(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	FixPoint1616_t LimitCheckValue)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Temp8;

	LOG_FUNCTION_START("");

	VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksEnable, LimitCheckId,
		Temp8);

	if (Temp8 == 0) { /* disabled write only internal value */
		VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
			LimitCheckId, LimitCheckValue);
	} else {

		switch (LimitCheckId) {

		case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
				VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
				LimitCheckValue);
			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:

			Status = VL53L0X_WrWord(Dev,
			VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
				VL53L0X_FIXPOINT1616TOFIXPOINT97(
					LimitCheckValue));

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:

			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
				VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
				LimitCheckValue);

			break;

		case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:

			/* internal computation: */
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
				LimitCheckValue);

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
		case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:

			Status = VL53L0X_WrWord(Dev,
				VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT,
				VL53L0X_FIXPOINT1616TOFIXPOINT97(
					LimitCheckValue));

			break;

		default:
			Status = VL53L0X_ERROR_INVALID_PARAMS;

		}

		if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
				LimitCheckId, LimitCheckValue);
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetLimitCheckValue(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	FixPoint1616_t *pLimitCheckValue)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t EnableZeroValue = 0;
	uint16_t Temp16;
	FixPoint1616_t TempFix1616;

	LOG_FUNCTION_START("");

	switch (LimitCheckId) {

	case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
		/* internal computation: */
		VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
			VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, TempFix1616);
		EnableZeroValue = 0;
		break;

	case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
		Status = VL53L0X_RdWord(Dev,
		VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
			&Temp16);
		if (Status == VL53L0X_ERROR_NONE)
			TempFix1616 = VL53L0X_FIXPOINT97TOFIXPOINT1616(Temp16);


		EnableZeroValue = 1;
		break;

	case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
		/* internal computation: */
		VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
			VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP, TempFix1616);
		EnableZeroValue = 0;
		break;

	case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
		/* internal computation: */
		VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksValue,
			VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, TempFix1616);
		EnableZeroValue = 0;
		break;

	case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
	case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
		Status = VL53L0X_RdWord(Dev,
			VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT,
			&Temp16);
		if (Status == VL53L0X_ERROR_NONE)
			TempFix1616 = VL53L0X_FIXPOINT97TOFIXPOINT1616(Temp16);


		EnableZeroValue = 0;
		break;

	default:
		Status = VL53L0X_ERROR_INVALID_PARAMS;

	}

	if (Status == VL53L0X_ERROR_NONE) {

		if (EnableZeroValue == 1) {

			if (TempFix1616 == 0) {
				/* disabled: return value from memory */
				VL53L0X_GETARRAYPARAMETERFIELD(Dev,
					LimitChecksValue, LimitCheckId,
					TempFix1616);
				*pLimitCheckValue = TempFix1616;
				VL53L0X_SETARRAYPARAMETERFIELD(Dev,
					LimitChecksEnable, LimitCheckId, 0);
			} else {
				*pLimitCheckValue = TempFix1616;
				VL53L0X_SETARRAYPARAMETERFIELD(Dev,
					LimitChecksValue, LimitCheckId,
					TempFix1616);
				VL53L0X_SETARRAYPARAMETERFIELD(Dev,
					LimitChecksEnable, LimitCheckId, 1);
			}
		} else {
			*pLimitCheckValue = TempFix1616;
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;

}

VL53L0X_Error VL53L0X_GetLimitCheckCurrent(VL53L0X_DEV Dev, uint16_t LimitCheckId,
	FixPoint1616_t *pLimitCheckCurrent)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_RangingMeasurementData_t LastRangeDataBuffer;

	LOG_FUNCTION_START("");

	if (LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS) {
		Status = VL53L0X_ERROR_INVALID_PARAMS;
	} else {
		switch (LimitCheckId) {
		case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
			/* Need to run a ranging to have the latest values */
			*pLimitCheckCurrent = PALDevDataGet(Dev, SigmaEstimate);

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
			/* Need to run a ranging to have the latest values */
			LastRangeDataBuffer = PALDevDataGet(Dev,
				LastRangeMeasure);
			*pLimitCheckCurrent =
				LastRangeDataBuffer.SignalRateRtnMegaCps;

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
			/* Need to run a ranging to have the latest values */
			*pLimitCheckCurrent = PALDevDataGet(Dev,
				LastSignalRefMcps);

			break;

		case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
			/* Need to run a ranging to have the latest values */
			LastRangeDataBuffer = PALDevDataGet(Dev,
				LastRangeMeasure);
			*pLimitCheckCurrent =
				LastRangeDataBuffer.SignalRateRtnMegaCps;

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
			/* Need to run a ranging to have the latest values */
			LastRangeDataBuffer = PALDevDataGet(Dev,
				LastRangeMeasure);
			*pLimitCheckCurrent =
				LastRangeDataBuffer.SignalRateRtnMegaCps;

			break;

		case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
			/* Need to run a ranging to have the latest values */
			LastRangeDataBuffer = PALDevDataGet(Dev,
				LastRangeMeasure);
			*pLimitCheckCurrent =
				LastRangeDataBuffer.SignalRateRtnMegaCps;

			break;

		default:
			Status = VL53L0X_ERROR_INVALID_PARAMS;
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;

}

/*
 * WRAPAROUND Check
 */
VL53L0X_Error VL53L0X_SetWrapAroundCheckEnable(VL53L0X_DEV Dev,
	uint8_t WrapAroundCheckEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	uint8_t WrapAroundCheckEnableInt;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &Byte);
	if (WrapAroundCheckEnable == 0) {
		/* Disable wraparound */
		Byte = Byte & 0x7F;
		WrapAroundCheckEnableInt = 0;
	} else {
		/*Enable wraparound */
		Byte = Byte | 0x80;
		WrapAroundCheckEnableInt = 1;
	}

	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, Byte);

	if (Status == VL53L0X_ERROR_NONE) {
		PALDevDataSet(Dev, SequenceConfig, Byte);
		VL53L0X_SETPARAMETERFIELD(Dev, WrapAroundCheckEnable,
			WrapAroundCheckEnableInt);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetWrapAroundCheckEnable(VL53L0X_DEV Dev,
	uint8_t *pWrapAroundCheckEnable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t data;

	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, &data);
	if (Status == VL53L0X_ERROR_NONE) {
		PALDevDataSet(Dev, SequenceConfig, data);
		if (data & (0x01 << 7))
			*pWrapAroundCheckEnable = 0x01;
		else
			*pWrapAroundCheckEnable = 0x00;
	}
	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETPARAMETERFIELD(Dev, WrapAroundCheckEnable,
			*pWrapAroundCheckEnable);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetDmaxCalParameters(VL53L0X_DEV Dev,
	uint16_t RangeMilliMeter, FixPoint1616_t SignalRateRtnMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	FixPoint1616_t SignalRateRtnMegaCpsTemp = 0;

	LOG_FUNCTION_START("");

	/* Check if one of input parameter is zero, in that case the
	 * value are get from NVM */
	if ((RangeMilliMeter == 0) || (SignalRateRtnMegaCps == 0)) {
		/* NVM parameters */
		/* Run VL53L0X_get_info_from_device wit option 4 to get
		 * signal rate at 400 mm if the value have been already
		 * get this function will return with no access to device */
		VL53L0X_get_info_from_device(Dev, 4);

		SignalRateRtnMegaCpsTemp = VL53L0X_GETDEVICESPECIFICPARAMETER(
			Dev, SignalRateMeasFixed400mm);

		PALDevDataSet(Dev, DmaxCalRangeMilliMeter, 400);
		PALDevDataSet(Dev, DmaxCalSignalRateRtnMegaCps,
			SignalRateRtnMegaCpsTemp);
	} else {
		/* User parameters */
		PALDevDataSet(Dev, DmaxCalRangeMilliMeter, RangeMilliMeter);
		PALDevDataSet(Dev, DmaxCalSignalRateRtnMegaCps,
			SignalRateRtnMegaCps);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetDmaxCalParameters(VL53L0X_DEV Dev,
	uint16_t *pRangeMilliMeter, FixPoint1616_t *pSignalRateRtnMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	*pRangeMilliMeter = PALDevDataGet(Dev, DmaxCalRangeMilliMeter);
	*pSignalRateRtnMegaCps = PALDevDataGet(Dev,
		DmaxCalSignalRateRtnMegaCps);

	LOG_FUNCTION_END(Status);
	return Status;
}

/* End Group PAL Parameters Functions */

/* Group PAL Measurement Functions */
VL53L0X_Error VL53L0X_PerformSingleMeasurement(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_DeviceModes DeviceMode;

	LOG_FUNCTION_START("");

	/* Get Current DeviceMode */
	Status = VL53L0X_GetDeviceMode(Dev, &DeviceMode);

	/* Start immediately to run a single ranging measurement in case of
	 * single ranging or single histogram */
	if (Status == VL53L0X_ERROR_NONE
		&& DeviceMode == VL53L0X_DEVICEMODE_SINGLE_RANGING)
		Status = VL53L0X_StartMeasurement(Dev);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_measurement_poll_for_completion(Dev);


	/* Change PAL State in case of single ranging or single histogram */
	if (Status == VL53L0X_ERROR_NONE
		&& DeviceMode == VL53L0X_DEVICEMODE_SINGLE_RANGING)
		PALDevDataSet(Dev, PalState, VL53L0X_STATE_IDLE);


	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformSingleHistogramMeasurement(VL53L0X_DEV Dev,
	VL53L0X_HistogramMeasurementData_t *pHistogramMeasurementData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformRefCalibration(VL53L0X_DEV Dev, uint8_t *pVhvSettings,
	uint8_t *pPhaseCal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_perform_ref_calibration(Dev, pVhvSettings,
		pPhaseCal, 1);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformXTalkMeasurement(VL53L0X_DEV Dev,
	uint32_t TimeoutMs, FixPoint1616_t *pXtalkPerSpad,
	uint8_t *pAmbientTooHigh)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented on VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformXTalkCalibration(VL53L0X_DEV Dev,
	FixPoint1616_t XTalkCalDistance,
	FixPoint1616_t *pXTalkCompensationRateMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_perform_xtalk_calibration(Dev, XTalkCalDistance,
		pXTalkCompensationRateMegaCps);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformOffsetCalibration(VL53L0X_DEV Dev,
	FixPoint1616_t CalDistanceMilliMeter, int32_t *pOffsetMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_perform_offset_calibration(Dev, CalDistanceMilliMeter,
		pOffsetMicroMeter);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_CheckAndLoadInterruptSettings(VL53L0X_DEV Dev,
	uint8_t StartNotStopFlag)
{
	uint8_t InterruptConfig;
	FixPoint1616_t ThresholdLow;
	FixPoint1616_t ThresholdHigh;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	InterruptConfig = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
		Pin0GpioFunctionality);

	if ((InterruptConfig ==
		VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW) ||
		(InterruptConfig ==
		VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH) ||
		(InterruptConfig ==
		VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT)) {

		Status = VL53L0X_GetInterruptThresholds(Dev,
			VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
			&ThresholdLow, &ThresholdHigh);

		if (((ThresholdLow > 255*65536) ||
			(ThresholdHigh > 255*65536)) &&
			(Status == VL53L0X_ERROR_NONE)) {

			if (StartNotStopFlag != 0) {
				Status = VL53L0X_load_tuning_settings(Dev,
					InterruptThresholdSettings);
			} else {
				Status |= VL53L0X_WrByte(Dev, 0xFF, 0x04);
				Status |= VL53L0X_WrByte(Dev, 0x70, 0x00);
				Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);
				Status |= VL53L0X_WrByte(Dev, 0x80, 0x00);
			}

		}


	}

	return Status;

}


VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_DeviceModes DeviceMode;
	uint8_t Byte;
	uint8_t StartStopByte = VL53L0X_REG_SYSRANGE_MODE_START_STOP;
	uint32_t LoopNb;
	LOG_FUNCTION_START("");

	/* Get Current DeviceMode */
	VL53L0X_GetDeviceMode(Dev, &DeviceMode);

	Status = VL53L0X_WrByte(Dev, 0x80, 0x01);
	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status = VL53L0X_WrByte(Dev, 0x00, 0x00);
	Status = VL53L0X_WrByte(Dev, 0x91, PALDevDataGet(Dev, StopVariable));
	Status = VL53L0X_WrByte(Dev, 0x00, 0x01);
	Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);
	Status = VL53L0X_WrByte(Dev, 0x80, 0x00);

	switch (DeviceMode) {
	case VL53L0X_DEVICEMODE_SINGLE_RANGING:
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01);

		Byte = StartStopByte;
		if (Status == VL53L0X_ERROR_NONE) {
			/* Wait until start bit has been cleared */
			LoopNb = 0;
			do {
				if (LoopNb > 0)
					Status = VL53L0X_RdByte(Dev,
					VL53L0X_REG_SYSRANGE_START, &Byte);
				LoopNb = LoopNb + 1;
			} while (((Byte & StartStopByte) == StartStopByte)
				&& (Status == VL53L0X_ERROR_NONE)
				&& (LoopNb < VL53L0X_DEFAULT_MAX_LOOP));

			if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
				Status = VL53L0X_ERROR_TIME_OUT;

		}

		break;
	case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
		/* Back-to-back mode */

		/* Check if need to apply interrupt settings */
		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

		Status = VL53L0X_WrByte(Dev,
		VL53L0X_REG_SYSRANGE_START,
		VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
		if (Status == VL53L0X_ERROR_NONE) {
			/* Set PAL State to Running */
			PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);
		}
		break;
	case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
		/* Continuous mode */
		/* Check if need to apply interrupt settings */
		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

		Status = VL53L0X_WrByte(Dev,
		VL53L0X_REG_SYSRANGE_START,
		VL53L0X_REG_SYSRANGE_MODE_TIMED);

		if (Status == VL53L0X_ERROR_NONE) {
			/* Set PAL State to Running */
			PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);
		}
		break;
	default:
		/* Selected mode not supported */
		Status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
	}


	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_StopMeasurement(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSRANGE_START,
	VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT);

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status = VL53L0X_WrByte(Dev, 0x00, 0x00);
	Status = VL53L0X_WrByte(Dev, 0x91, 0x00);
	Status = VL53L0X_WrByte(Dev, 0x00, 0x01);
	Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);

	if (Status == VL53L0X_ERROR_NONE) {
		/* Set PAL State to Idle */
		PALDevDataSet(Dev, PalState, VL53L0X_STATE_IDLE);
	}

	/* Check if need to apply interrupt settings */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_CheckAndLoadInterruptSettings(Dev, 0);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetMeasurementDataReady(VL53L0X_DEV Dev,
	uint8_t *pMeasurementDataReady)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SysRangeStatusRegister;
	uint8_t InterruptConfig;
	uint32_t InterruptMask;
	LOG_FUNCTION_START("");

	InterruptConfig = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
		Pin0GpioFunctionality);

	if (InterruptConfig ==
		VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY) {
		Status = VL53L0X_GetInterruptMaskStatus(Dev, &InterruptMask);
		if (InterruptMask ==
		VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY)
			*pMeasurementDataReady = 1;
		else
			*pMeasurementDataReady = 0;
	} else {
		Status = VL53L0X_RdByte(Dev, VL53L0X_REG_RESULT_RANGE_STATUS,
			&SysRangeStatusRegister);
		if (Status == VL53L0X_ERROR_NONE) {
			if (SysRangeStatusRegister & 0x01)
				*pMeasurementDataReady = 1;
			else
				*pMeasurementDataReady = 0;
		}
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_WaitDeviceReadyForNewMeasurement(VL53L0X_DEV Dev,
	uint32_t MaxLoop)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented for VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}


VL53L0X_Error VL53L0X_GetRangingMeasurementData(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t DeviceRangeStatus;
	uint8_t RangeFractionalEnable;
	uint8_t PalRangeStatus;
	uint8_t XTalkCompensationEnable;
	uint16_t AmbientRate;
	FixPoint1616_t SignalRate;
	uint16_t XTalkCompensationRateMegaCps;
	uint16_t EffectiveSpadRtnCount;
	uint16_t tmpuint16;
	uint16_t XtalkRangeMilliMeter;
	uint16_t LinearityCorrectiveGain;
	uint8_t localBuffer[12];
	VL53L0X_RangingMeasurementData_t LastRangeDataBuffer;

	LOG_FUNCTION_START("");

	/*
	 * use multi read even if some registers are not useful, result will
	 * be more efficient
	 * start reading at 0x14 dec20
	 * end reading at 0x21 dec33 total 14 bytes to read
	 */
	Status = VL53L0X_ReadMulti(Dev, 0x14, localBuffer, 12);

	if (Status == VL53L0X_ERROR_NONE) {

		pRangingMeasurementData->ZoneId = 0; /* Only one zone */
		pRangingMeasurementData->TimeStamp = 0; /* Not Implemented */

		tmpuint16 = VL53L0X_MAKEUINT16(localBuffer[11], localBuffer[10]);
		/* cut1.1 if SYSTEM__RANGE_CONFIG if 1 range is 2bits fractional
		 *(format 11.2) else no fractional
		 */

		pRangingMeasurementData->MeasurementTimeUsec = 0;

		SignalRate = VL53L0X_FIXPOINT97TOFIXPOINT1616(
			VL53L0X_MAKEUINT16(localBuffer[7], localBuffer[6]));
		/* peak_signal_count_rate_rtn_mcps */
		pRangingMeasurementData->SignalRateRtnMegaCps = SignalRate;

		AmbientRate = VL53L0X_MAKEUINT16(localBuffer[9], localBuffer[8]);
		pRangingMeasurementData->AmbientRateRtnMegaCps =
			VL53L0X_FIXPOINT97TOFIXPOINT1616(AmbientRate);

		EffectiveSpadRtnCount = VL53L0X_MAKEUINT16(localBuffer[3],
			localBuffer[2]);
		/* EffectiveSpadRtnCount is 8.8 format */
		pRangingMeasurementData->EffectiveSpadRtnCount =
			EffectiveSpadRtnCount;

		DeviceRangeStatus = localBuffer[0];

		/* Get Linearity Corrective Gain */
		LinearityCorrectiveGain = PALDevDataGet(Dev,
			LinearityCorrectiveGain);

		/* Get ranging configuration */
		RangeFractionalEnable = PALDevDataGet(Dev,
			RangeFractionalEnable);

		if (LinearityCorrectiveGain != 1000) {

			tmpuint16 = (uint16_t)((LinearityCorrectiveGain
				* tmpuint16 + 500) / 1000);

			/* Implement Xtalk */
			VL53L0X_GETPARAMETERFIELD(Dev,
				XTalkCompensationRateMegaCps,
				XTalkCompensationRateMegaCps);
			VL53L0X_GETPARAMETERFIELD(Dev, XTalkCompensationEnable,
				XTalkCompensationEnable);

			if (XTalkCompensationEnable) {

				if ((SignalRate
					- ((XTalkCompensationRateMegaCps
					* EffectiveSpadRtnCount) >> 8))
					<= 0) {
					if (RangeFractionalEnable)
						XtalkRangeMilliMeter = 8888;
					else
						XtalkRangeMilliMeter = 8888
							<< 2;
				} else {
					XtalkRangeMilliMeter =
					(tmpuint16 * SignalRate)
						/ (SignalRate
						- ((XTalkCompensationRateMegaCps
						* EffectiveSpadRtnCount)
						>> 8));
				}

				tmpuint16 = XtalkRangeMilliMeter;
			}

		}

		if (RangeFractionalEnable) {
			pRangingMeasurementData->RangeMilliMeter =
				(uint16_t)((tmpuint16) >> 2);
			pRangingMeasurementData->RangeFractionalPart =
				(uint8_t)((tmpuint16 & 0x03) << 6);
		} else {
			pRangingMeasurementData->RangeMilliMeter = tmpuint16;
			pRangingMeasurementData->RangeFractionalPart = 0;
		}

		/*
		 * For a standard definition of RangeStatus, this should
		 * return 0 in case of good result after a ranging
		 * The range status depends on the device so call a device
		 * specific function to obtain the right Status.
		 */
		Status |= VL53L0X_get_pal_range_status(Dev, DeviceRangeStatus,
			SignalRate, EffectiveSpadRtnCount,
			pRangingMeasurementData, &PalRangeStatus);

		if (Status == VL53L0X_ERROR_NONE)
			pRangingMeasurementData->RangeStatus = PalRangeStatus;

	}

	if (Status == VL53L0X_ERROR_NONE) {
		/* Copy last read data into Dev buffer */
		LastRangeDataBuffer = PALDevDataGet(Dev, LastRangeMeasure);

		LastRangeDataBuffer.RangeMilliMeter =
			pRangingMeasurementData->RangeMilliMeter;
		LastRangeDataBuffer.RangeFractionalPart =
			pRangingMeasurementData->RangeFractionalPart;
		LastRangeDataBuffer.RangeDMaxMilliMeter =
			pRangingMeasurementData->RangeDMaxMilliMeter;
		LastRangeDataBuffer.MeasurementTimeUsec =
			pRangingMeasurementData->MeasurementTimeUsec;
		LastRangeDataBuffer.SignalRateRtnMegaCps =
			pRangingMeasurementData->SignalRateRtnMegaCps;
		LastRangeDataBuffer.AmbientRateRtnMegaCps =
			pRangingMeasurementData->AmbientRateRtnMegaCps;
		LastRangeDataBuffer.EffectiveSpadRtnCount =
			pRangingMeasurementData->EffectiveSpadRtnCount;
		LastRangeDataBuffer.RangeStatus =
			pRangingMeasurementData->RangeStatus;

		PALDevDataSet(Dev, LastRangeMeasure, LastRangeDataBuffer);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetMeasurementRefSignal(VL53L0X_DEV Dev,
	FixPoint1616_t *pMeasurementRefSignal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	*pMeasurementRefSignal = PALDevDataGet(Dev, LastSignalRefMcps);

	LOG_FUNCTION_END(Status);
	return Status;

}

VL53L0X_Error VL53L0X_GetHistogramMeasurementData(VL53L0X_DEV Dev,
	VL53L0X_HistogramMeasurementData_t *pHistogramMeasurementData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_PerformSingleRangingMeasurement(VL53L0X_DEV Dev,
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	/* This function will do a complete single ranging
	 * Here we fix the mode! */
	Status = VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_PerformSingleMeasurement(Dev);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetRangingMeasurementData(Dev,
			pRangingMeasurementData);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_ClearInterruptMask(Dev, 0);


	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t NumberOfROIZones)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	if (NumberOfROIZones != 1)
		Status = VL53L0X_ERROR_INVALID_PARAMS;


	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t *pNumberOfROIZones)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	*pNumberOfROIZones = 1;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetMaxNumberOfROIZones(VL53L0X_DEV Dev,
	uint8_t *pMaxNumberOfROIZones)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	LOG_FUNCTION_START("");

	*pMaxNumberOfROIZones = 1;

	LOG_FUNCTION_END(Status);
	return Status;
}

/* End Group PAL Measurement Functions */

VL53L0X_Error VL53L0X_SetGpioConfig(VL53L0X_DEV Dev, uint8_t Pin,
	VL53L0X_DeviceModes DeviceMode, VL53L0X_GpioFunctionality Functionality,
	VL53L0X_InterruptPolarity Polarity)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t data;

	LOG_FUNCTION_START("");

	if (Pin != 0) {
		Status = VL53L0X_ERROR_GPIO_NOT_EXISTING;
	} else if (DeviceMode == VL53L0X_DEVICEMODE_GPIO_DRIVE) {
		if (Polarity == VL53L0X_INTERRUPTPOLARITY_LOW)
			data = 0x10;
		else
			data = 1;

		Status = VL53L0X_WrByte(Dev,
		VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, data);

	} else if (DeviceMode == VL53L0X_DEVICEMODE_GPIO_OSC) {

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0x00, 0x00);

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		Status |= VL53L0X_WrByte(Dev, 0x80, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0x85, 0x02);

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x04);
		Status |= VL53L0X_WrByte(Dev, 0xcd, 0x00);
		Status |= VL53L0X_WrByte(Dev, 0xcc, 0x11);

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x07);
		Status |= VL53L0X_WrByte(Dev, 0xbe, 0x00);

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x06);
		Status |= VL53L0X_WrByte(Dev, 0xcc, 0x09);

		Status |= VL53L0X_WrByte(Dev, 0xff, 0x00);
		Status |= VL53L0X_WrByte(Dev, 0xff, 0x01);
		Status |= VL53L0X_WrByte(Dev, 0x00, 0x00);

	} else {

		if (Status == VL53L0X_ERROR_NONE) {
			switch (Functionality) {
			case VL53L0X_GPIOFUNCTIONALITY_OFF:
				data = 0x00;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW:
				data = 0x01;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH:
				data = 0x02;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT:
				data = 0x03;
				break;
			case VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY:
				data = 0x04;
				break;
			default:
				Status =
				VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED;
			}
		}

		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, data);

		if (Status == VL53L0X_ERROR_NONE) {
			if (Polarity == VL53L0X_INTERRUPTPOLARITY_LOW)
				data = 0;
			else
				data = (uint8_t)(1 << 4);

			Status = VL53L0X_UpdateByte(Dev,
			VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH, 0xEF, data);
		}

		if (Status == VL53L0X_ERROR_NONE)
			VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
				Pin0GpioFunctionality, Functionality);

		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_ClearInterruptMask(Dev, 0);

	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetGpioConfig(VL53L0X_DEV Dev, uint8_t Pin,
	VL53L0X_DeviceModes *pDeviceMode,
	VL53L0X_GpioFunctionality *pFunctionality,
	VL53L0X_InterruptPolarity *pPolarity)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_GpioFunctionality GpioFunctionality;
	uint8_t data;

	LOG_FUNCTION_START("");

	/* pDeviceMode not managed by Ewok it return the current mode */

	Status = VL53L0X_GetDeviceMode(Dev, pDeviceMode);

	if (Status == VL53L0X_ERROR_NONE) {
		if (Pin != 0) {
			Status = VL53L0X_ERROR_GPIO_NOT_EXISTING;
		} else {
			Status = VL53L0X_RdByte(Dev,
			VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO, &data);
		}
	}

	if (Status == VL53L0X_ERROR_NONE) {
		switch (data & 0x07) {
		case 0x00:
			GpioFunctionality = VL53L0X_GPIOFUNCTIONALITY_OFF;
			break;
		case 0x01:
			GpioFunctionality =
			VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW;
			break;
		case 0x02:
			GpioFunctionality =
			VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH;
			break;
		case 0x03:
			GpioFunctionality =
			VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT;
			break;
		case 0x04:
			GpioFunctionality =
			VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY;
			break;
		default:
			Status = VL53L0X_ERROR_GPIO_FUNCTIONALITY_NOT_SUPPORTED;
		}
	}

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_RdByte(Dev, VL53L0X_REG_GPIO_HV_MUX_ACTIVE_HIGH,
			&data);

	if (Status == VL53L0X_ERROR_NONE) {
		if ((data & (uint8_t)(1 << 4)) == 0)
			*pPolarity = VL53L0X_INTERRUPTPOLARITY_LOW;
		else
			*pPolarity = VL53L0X_INTERRUPTPOLARITY_HIGH;
	}

	if (Status == VL53L0X_ERROR_NONE) {
		*pFunctionality = GpioFunctionality;
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, Pin0GpioFunctionality,
			GpioFunctionality);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetInterruptThresholds(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes DeviceMode, FixPoint1616_t ThresholdLow,
	FixPoint1616_t ThresholdHigh)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t Threshold16;
	LOG_FUNCTION_START("");

	/* no dependency on DeviceMode for Ewok */
	/* Need to divide by 2 because the FW will apply a x2 */
	Threshold16 = (uint16_t)((ThresholdLow >> 17) & 0x00fff);
	Status = VL53L0X_WrWord(Dev, VL53L0X_REG_SYSTEM_THRESH_LOW, Threshold16);

	if (Status == VL53L0X_ERROR_NONE) {
		/* Need to divide by 2 because the FW will apply a x2 */
		Threshold16 = (uint16_t)((ThresholdHigh >> 17) & 0x00fff);
		Status = VL53L0X_WrWord(Dev, VL53L0X_REG_SYSTEM_THRESH_HIGH,
			Threshold16);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetInterruptThresholds(VL53L0X_DEV Dev,
	VL53L0X_DeviceModes DeviceMode, FixPoint1616_t *pThresholdLow,
	FixPoint1616_t *pThresholdHigh)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t Threshold16;
	LOG_FUNCTION_START("");

	/* no dependency on DeviceMode for Ewok */

	Status = VL53L0X_RdWord(Dev, VL53L0X_REG_SYSTEM_THRESH_LOW, &Threshold16);
	/* Need to multiply by 2 because the FW will apply a x2 */
	*pThresholdLow = (FixPoint1616_t)((0x00fff & Threshold16) << 17);

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_RdWord(Dev, VL53L0X_REG_SYSTEM_THRESH_HIGH,
			&Threshold16);
		/* Need to multiply by 2 because the FW will apply a x2 */
		*pThresholdHigh =
			(FixPoint1616_t)((0x00fff & Threshold16) << 17);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetStopCompletedStatus(VL53L0X_DEV Dev,
	uint32_t *pStopStatus)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte = 0;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_RdByte(Dev, 0x04, &Byte);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x0);

	*pStopStatus = Byte;
	
	if (Byte == 0) {
		Status = VL53L0X_WrByte(Dev, 0x80, 0x01);
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
		Status = VL53L0X_WrByte(Dev, 0x00, 0x00);
		Status = VL53L0X_WrByte(Dev, 0x91,
			PALDevDataGet(Dev, StopVariable));
		Status = VL53L0X_WrByte(Dev, 0x00, 0x01);
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);
		Status = VL53L0X_WrByte(Dev, 0x80, 0x00);
	}

	LOG_FUNCTION_END(Status);
	return Status;
}

/* Group PAL Interrupt Functions */
VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t InterruptMask)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t LoopCount;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	/* clear bit 0 range interrupt, bit 1 error interrupt */
	LoopCount = 0;
	do {
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
		Status |= VL53L0X_WrByte(Dev,
			VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x00);
		Status |= VL53L0X_RdByte(Dev,
			VL53L0X_REG_RESULT_INTERRUPT_STATUS, &Byte);
		LoopCount++;
	} while (((Byte & 0x07) != 0x00)
			&& (LoopCount < 3)
			&& (Status == VL53L0X_ERROR_NONE));


	if (LoopCount >= 3)
		Status = VL53L0X_ERROR_INTERRUPT_NOT_CLEARED;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetInterruptMaskStatus(VL53L0X_DEV Dev,
	uint32_t *pInterruptMaskStatus)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	Status = VL53L0X_RdByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &Byte);
	*pInterruptMaskStatus = Byte & 0x07;

	if (Byte & 0x18)
		Status = VL53L0X_ERROR_RANGE_ERROR;

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_EnableInterruptMask(VL53L0X_DEV Dev, uint32_t InterruptMask)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NOT_IMPLEMENTED;
	LOG_FUNCTION_START("");

	/* not implemented for VL53L0X */

	LOG_FUNCTION_END(Status);
	return Status;
}

/* End Group PAL Interrupt Functions */

/* Group SPAD functions */

VL53L0X_Error VL53L0X_SetSpadAmbientDamperThreshold(VL53L0X_DEV Dev,
	uint16_t SpadAmbientDamperThreshold)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_WrWord(Dev, 0x40, SpadAmbientDamperThreshold);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetSpadAmbientDamperThreshold(VL53L0X_DEV Dev,
	uint16_t *pSpadAmbientDamperThreshold)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_RdWord(Dev, 0x40, pSpadAmbientDamperThreshold);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_SetSpadAmbientDamperFactor(VL53L0X_DEV Dev,
	uint16_t SpadAmbientDamperFactor)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	Byte = (uint8_t)(SpadAmbientDamperFactor & 0x00FF);

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0x42, Byte);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_GetSpadAmbientDamperFactor(VL53L0X_DEV Dev,
	uint16_t *pSpadAmbientDamperFactor)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t Byte;
	LOG_FUNCTION_START("");

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_RdByte(Dev, 0x42, &Byte);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);
	*pSpadAmbientDamperFactor = (uint16_t)Byte;

	LOG_FUNCTION_END(Status);
	return Status;
}

/* END Group SPAD functions */

/*****************************************************************************
 * Internal functions
 *****************************************************************************/

VL53L0X_Error VL53L0X_SetReferenceSpads(VL53L0X_DEV Dev, uint32_t count,
	uint8_t isApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_set_reference_spads(Dev, count, isApertureSpads);

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_GetReferenceSpads(VL53L0X_DEV Dev, uint32_t *pSpadCount,
	uint8_t *pIsApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_get_reference_spads(Dev, pSpadCount, pIsApertureSpads);

	LOG_FUNCTION_END(Status);

	return Status;
}

VL53L0X_Error VL53L0X_PerformRefSpadManagement(VL53L0X_DEV Dev,
	uint32_t *refSpadCount, uint8_t *isApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	LOG_FUNCTION_START("");

	Status = VL53L0X_perform_ref_spad_management(Dev, refSpadCount,
		isApertureSpads);

	LOG_FUNCTION_END(Status);

	return Status;
}

/******************************************************calibration*******************************************************/
/******************************************************calibration*******************************************************/
/******************************************************calibration*******************************************************/




VL53L0X_Error VL53L0X_perform_xtalk_calibration(VL53L0X_DEV Dev,
			FixPoint1616_t XTalkCalDistance,
			FixPoint1616_t *pXTalkCompensationRateMegaCps)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t sum_ranging = 0;
	uint16_t sum_spads = 0;
	FixPoint1616_t sum_signalRate = 0;
	FixPoint1616_t total_count = 0;
	uint8_t xtalk_meas = 0;
	VL53L0X_RangingMeasurementData_t RangingMeasurementData;
	FixPoint1616_t xTalkStoredMeanSignalRate;
	FixPoint1616_t xTalkStoredMeanRange;
	FixPoint1616_t xTalkStoredMeanRtnSpads;
	uint32_t signalXTalkTotalPerSpad;
	uint32_t xTalkStoredMeanRtnSpadsAsInt;
	uint32_t xTalkCalDistanceAsInt;
	FixPoint1616_t XTalkCompensationRateMegaCps;

	if (XTalkCalDistance <= 0)
		Status = VL53L0X_ERROR_INVALID_PARAMS;

	/* Disable the XTalk compensation */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetXTalkCompensationEnable(Dev, 0);

	/* Disable the RIT */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_SetLimitCheckEnable(Dev,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
	}

	/* Perform 50 measurements and compute the averages */
	if (Status == VL53L0X_ERROR_NONE) {
		sum_ranging = 0;
		sum_spads = 0;
		sum_signalRate = 0;
		total_count = 0;
		for (xtalk_meas = 0; xtalk_meas < 50; xtalk_meas++) {
			Status = VL53L0X_PerformSingleRangingMeasurement(Dev,
				&RangingMeasurementData);

			if (Status != VL53L0X_ERROR_NONE)
				break;

			/* The range is valid when RangeStatus = 0 */
			if (RangingMeasurementData.RangeStatus == 0) {
				sum_ranging = sum_ranging +
					RangingMeasurementData.RangeMilliMeter;
				sum_signalRate = sum_signalRate +
				RangingMeasurementData.SignalRateRtnMegaCps;
				sum_spads = sum_spads +
				RangingMeasurementData.EffectiveSpadRtnCount
					/ 256;
				total_count = total_count + 1;
			}
		}

		/* no valid values found */
		if (total_count == 0)
			Status = VL53L0X_ERROR_RANGE_ERROR;

	}


	if (Status == VL53L0X_ERROR_NONE) {
		/* FixPoint1616_t / uint16_t = FixPoint1616_t */
		xTalkStoredMeanSignalRate = sum_signalRate / total_count;
		xTalkStoredMeanRange = (FixPoint1616_t)((uint32_t)(
			sum_ranging << 16) / total_count);
		xTalkStoredMeanRtnSpads = (FixPoint1616_t)((uint32_t)(
			sum_spads << 16) / total_count);

		/* Round Mean Spads to Whole Number.
		 * Typically the calculated mean SPAD count is a whole number
		 * or very close to a whole
		 * number, therefore any truncation will not result in a
		 * significant loss in accuracy.
		 * Also, for a grey target at a typical distance of around
		 * 400mm, around 220 SPADs will
		 * be enabled, therefore, any truncation will result in a loss
		 * of accuracy of less than
		 * 0.5%.
		 */
		xTalkStoredMeanRtnSpadsAsInt = (xTalkStoredMeanRtnSpads +
			0x8000) >> 16;

		/* Round Cal Distance to Whole Number.
		 * Note that the cal distance is in mm, therefore no resolution
		 * is lost.*/
		 xTalkCalDistanceAsInt = (XTalkCalDistance + 0x8000) >> 16;

		if (xTalkStoredMeanRtnSpadsAsInt == 0 ||
		   xTalkCalDistanceAsInt == 0 ||
		   xTalkStoredMeanRange >= XTalkCalDistance) {
			XTalkCompensationRateMegaCps = 0;
		} else {
			/* Round Cal Distance to Whole Number.
			   Note that the cal distance is in mm, therefore no
			   resolution is lost.*/
			xTalkCalDistanceAsInt = (XTalkCalDistance +
				0x8000) >> 16;

			/* Apply division by mean spad count early in the
			 * calculation to keep the numbers small.
			 * This ensures we can maintain a 32bit calculation.
			 * Fixed1616 / int := Fixed1616 */
			signalXTalkTotalPerSpad = (xTalkStoredMeanSignalRate) /
				xTalkStoredMeanRtnSpadsAsInt;

			/* Complete the calculation for total Signal XTalk per
			 * SPAD
			 * Fixed1616 * (Fixed1616 - Fixed1616/int) :=
			 * (2^16 * Fixed1616)
			 */
			signalXTalkTotalPerSpad *= ((1 << 16) -
				(xTalkStoredMeanRange / xTalkCalDistanceAsInt));

			/* Round from 2^16 * Fixed1616, to Fixed1616. */
			XTalkCompensationRateMegaCps = (signalXTalkTotalPerSpad
				+ 0x8000) >> 16;
		}

		*pXTalkCompensationRateMegaCps = XTalkCompensationRateMegaCps;

		/* Enable the XTalk compensation */
		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_SetXTalkCompensationEnable(Dev, 1);

		/* Enable the XTalk compensation */
		if (Status == VL53L0X_ERROR_NONE)
			Status = VL53L0X_SetXTalkCompensationRateMegaCps(Dev,
					XTalkCompensationRateMegaCps);

	}

	return Status;
}

VL53L0X_Error VL53L0X_perform_offset_calibration(VL53L0X_DEV Dev,
			FixPoint1616_t CalDistanceMilliMeter,
			int32_t *pOffsetMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t sum_ranging = 0;
	FixPoint1616_t total_count = 0;
	VL53L0X_RangingMeasurementData_t RangingMeasurementData;
	FixPoint1616_t StoredMeanRange;
	uint32_t StoredMeanRangeAsInt;
	uint32_t CalDistanceAsInt_mm;
	uint8_t SequenceStepEnabled;
	int meas = 0;

	if (CalDistanceMilliMeter <= 0)
		Status = VL53L0X_ERROR_INVALID_PARAMS;

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetOffsetCalibrationDataMicroMeter(Dev, 0);


	/* Get the value of the TCC */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_GetSequenceStepEnable(Dev,
				VL53L0X_SEQUENCESTEP_TCC, &SequenceStepEnabled);


	/* Disable the TCC */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetSequenceStepEnable(Dev,
				VL53L0X_SEQUENCESTEP_TCC, 0);


	/* Disable the RIT */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_SetLimitCheckEnable(Dev,
				VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);

	/* Perform 50 measurements and compute the averages */
	if (Status == VL53L0X_ERROR_NONE) {
		sum_ranging = 0;
		total_count = 0;
		for (meas = 0; meas < 50; meas++) {
			Status = VL53L0X_PerformSingleRangingMeasurement(Dev,
					&RangingMeasurementData);

			if (Status != VL53L0X_ERROR_NONE)
				break;

			/* The range is valid when RangeStatus = 0 */
			if (RangingMeasurementData.RangeStatus == 0) {
				sum_ranging = sum_ranging +
					RangingMeasurementData.RangeMilliMeter;
				total_count = total_count + 1;
			}
		}

		/* no valid values found */
		if (total_count == 0)
			Status = VL53L0X_ERROR_RANGE_ERROR;
	}


	if (Status == VL53L0X_ERROR_NONE) {
		/* FixPoint1616_t / uint16_t = FixPoint1616_t */
		StoredMeanRange = (FixPoint1616_t)((uint32_t)(sum_ranging << 16)
			/ total_count);

		StoredMeanRangeAsInt = (StoredMeanRange + 0x8000) >> 16;

		/* Round Cal Distance to Whole Number.
		 * Note that the cal distance is in mm, therefore no resolution
		 * is lost.*/
		 CalDistanceAsInt_mm = (CalDistanceMilliMeter + 0x8000) >> 16;

		 *pOffsetMicroMeter = (CalDistanceAsInt_mm -
				 StoredMeanRangeAsInt) * 1000;

		/* Apply the calculated offset */
		if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETPARAMETERFIELD(Dev, RangeOffsetMicroMeters,
					*pOffsetMicroMeter);
			Status = VL53L0X_SetOffsetCalibrationDataMicroMeter(Dev,
					*pOffsetMicroMeter);
		}

	}

	/* Restore the TCC */
	if (Status == VL53L0X_ERROR_NONE) {
		if (SequenceStepEnabled != 0)
			Status = VL53L0X_SetSequenceStepEnable(Dev,
					VL53L0X_SEQUENCESTEP_TCC, 1);
	}

	return Status;
}


VL53L0X_Error VL53L0X_set_offset_calibration_data_micro_meter(VL53L0X_DEV Dev,
		int32_t OffsetCalibrationDataMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t cMaxOffsetMicroMeter = 511000;
	int32_t cMinOffsetMicroMeter = -512000;
	int16_t cOffsetRange = 4096;
	uint32_t encodedOffsetVal;

	LOG_FUNCTION_START("");

	if (OffsetCalibrationDataMicroMeter > cMaxOffsetMicroMeter)
		OffsetCalibrationDataMicroMeter = cMaxOffsetMicroMeter;
	else if (OffsetCalibrationDataMicroMeter < cMinOffsetMicroMeter)
		OffsetCalibrationDataMicroMeter = cMinOffsetMicroMeter;

	/* The offset register is 10.2 format and units are mm
	 * therefore conversion is applied by a division of
	 * 250.
	 */
	if (OffsetCalibrationDataMicroMeter >= 0) {
		encodedOffsetVal =
			OffsetCalibrationDataMicroMeter/250;
	} else {
		encodedOffsetVal =
			cOffsetRange +
			OffsetCalibrationDataMicroMeter/250;
	}

	Status = VL53L0X_WrWord(Dev,
		VL53L0X_REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM,
		encodedOffsetVal);

	LOG_FUNCTION_END(Status);
	return Status;
}

VL53L0X_Error VL53L0X_get_offset_calibration_data_micro_meter(VL53L0X_DEV Dev,
		int32_t *pOffsetCalibrationDataMicroMeter)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint16_t RangeOffsetRegister;
	int16_t cMaxOffset = 2047;
	int16_t cOffsetRange = 4096;

	/* Note that offset has 10.2 format */

	Status = VL53L0X_RdWord(Dev,
				VL53L0X_REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM,
				&RangeOffsetRegister);

	if (Status == VL53L0X_ERROR_NONE) {
		RangeOffsetRegister = (RangeOffsetRegister & 0x0fff);

		/* Apply 12 bit 2's compliment conversion */
		if (RangeOffsetRegister > cMaxOffset)
			*pOffsetCalibrationDataMicroMeter =
				(int16_t)(RangeOffsetRegister - cOffsetRange)
					* 250;
		else
			*pOffsetCalibrationDataMicroMeter =
				(int16_t)RangeOffsetRegister * 250;

	}

	return Status;
}


VL53L0X_Error VL53L0X_apply_offset_adjustment(VL53L0X_DEV Dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t CorrectedOffsetMicroMeters;
	int32_t CurrentOffsetMicroMeters;

	/* if we run on this function we can read all the NVM info
	 * used by the API */
	Status = VL53L0X_get_info_from_device(Dev, 7);

	/* Read back current device offset */
	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetOffsetCalibrationDataMicroMeter(Dev,
					&CurrentOffsetMicroMeters);
	}

	/* Apply Offset Adjustment derived from 400mm measurements */
	if (Status == VL53L0X_ERROR_NONE) {

		/* Store initial device offset */
		PALDevDataSet(Dev, Part2PartOffsetNVMMicroMeter,
			CurrentOffsetMicroMeters);

		CorrectedOffsetMicroMeters = CurrentOffsetMicroMeters +
			(int32_t)PALDevDataGet(Dev,
				Part2PartOffsetAdjustmentNVMMicroMeter);

		Status = VL53L0X_SetOffsetCalibrationDataMicroMeter(Dev,
					CorrectedOffsetMicroMeters);

		/* store current, adjusted offset */
		if (Status == VL53L0X_ERROR_NONE) {
			VL53L0X_SETPARAMETERFIELD(Dev, RangeOffsetMicroMeters,
					CorrectedOffsetMicroMeters);
		}
	}

	return Status;
}

void get_next_good_spad(uint8_t goodSpadArray[], uint32_t size,
			uint32_t curr, int32_t *next)
{
	uint32_t startIndex;
	uint32_t fineOffset;
	uint32_t cSpadsPerByte = 8;
	uint32_t coarseIndex;
	uint32_t fineIndex;
	uint8_t dataByte;
	uint8_t success = 0;

	/*
	 * Starting with the current good spad, loop through the array to find
	 * the next. i.e. the next bit set in the sequence.
	 *
	 * The coarse index is the byte index of the array and the fine index is
	 * the index of the bit within each byte.
	 */

	*next = -1;

	startIndex = curr / cSpadsPerByte;
	fineOffset = curr % cSpadsPerByte;

	for (coarseIndex = startIndex; ((coarseIndex < size) && !success);
				coarseIndex++) {
		fineIndex = 0;
		dataByte = goodSpadArray[coarseIndex];

		if (coarseIndex == startIndex) {
			/* locate the bit position of the provided current
			 * spad bit before iterating */
			dataByte >>= fineOffset;
			fineIndex = fineOffset;
		}

		while (fineIndex < cSpadsPerByte) {
			if ((dataByte & 0x1) == 1) {
				success = 1;
				*next = coarseIndex * cSpadsPerByte + fineIndex;
				break;
			}
			dataByte >>= 1;
			fineIndex++;
		}
	}
}


uint8_t is_aperture(uint32_t spadIndex)
{
	/*
	 * This function reports if a given spad index is an aperture SPAD by
	 * deriving the quadrant.
	 */
	uint32_t quadrant;
	uint8_t isAperture = 1;
	quadrant = spadIndex >> 6;
	if (refArrayQuadrants[quadrant] == REF_ARRAY_SPAD_0)
		isAperture = 0;

	return isAperture;
}


VL53L0X_Error enable_spad_bit(uint8_t spadArray[], uint32_t size,
	uint32_t spadIndex)
{
	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	uint32_t cSpadsPerByte = 8;
	uint32_t coarseIndex;
	uint32_t fineIndex;

	coarseIndex = spadIndex / cSpadsPerByte;
	fineIndex = spadIndex % cSpadsPerByte;
	if (coarseIndex >= size)
		status = VL53L0X_ERROR_REF_SPAD_INIT;
	else
		spadArray[coarseIndex] |= (1 << fineIndex);

	return status;
}

VL53L0X_Error count_enabled_spads(uint8_t spadArray[],
		uint32_t byteCount, uint32_t maxSpads,
		uint32_t *pTotalSpadsEnabled, uint8_t *pIsAperture)
{
	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	uint32_t cSpadsPerByte = 8;
	uint32_t lastByte;
	uint32_t lastBit;
	uint32_t byteIndex = 0;
	uint32_t bitIndex = 0;
	uint8_t tempByte;
	uint8_t spadTypeIdentified = 0;

	/* The entire array will not be used for spads, therefore the last
	 * byte and last bit is determined from the max spads value.
	 */

	lastByte = maxSpads / cSpadsPerByte;
	lastBit = maxSpads % cSpadsPerByte;

	/* Check that the max spads value does not exceed the array bounds. */
	if (lastByte >= byteCount)
		status = VL53L0X_ERROR_REF_SPAD_INIT;

	*pTotalSpadsEnabled = 0;

	/* Count the bits enabled in the whole bytes */
	for (byteIndex = 0; byteIndex <= (lastByte - 1); byteIndex++) {
		tempByte = spadArray[byteIndex];

		for (bitIndex = 0; bitIndex <= cSpadsPerByte; bitIndex++) {
			if ((tempByte & 0x01) == 1) {
				(*pTotalSpadsEnabled)++;

				if (!spadTypeIdentified) {
					*pIsAperture = 1;
					if ((byteIndex < 2) && (bitIndex < 4))
							*pIsAperture = 0;
					spadTypeIdentified = 1;
				}
			}
			tempByte >>= 1;
		}
	}

	/* Count the number of bits enabled in the last byte accounting
	 * for the fact that not all bits in the byte may be used.
	 */
	tempByte = spadArray[lastByte];

	for (bitIndex = 0; bitIndex <= lastBit; bitIndex++) {
		if ((tempByte & 0x01) == 1)
			(*pTotalSpadsEnabled)++;
	}

	return status;
}

VL53L0X_Error set_ref_spad_map(VL53L0X_DEV Dev, uint8_t *refSpadArray)
{
	VL53L0X_Error status = VL53L0X_WriteMulti(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0,
				refSpadArray, 6);
	return status;
}

VL53L0X_Error get_ref_spad_map(VL53L0X_DEV Dev, uint8_t *refSpadArray)
{
	VL53L0X_Error status = VL53L0X_ReadMulti(Dev,
				VL53L0X_REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0,
				refSpadArray,
				6);
	return status;
}

VL53L0X_Error enable_ref_spads(VL53L0X_DEV Dev,
				uint8_t apertureSpads,
				uint8_t goodSpadArray[],
				uint8_t spadArray[],
				uint32_t size,
				uint32_t start,
				uint32_t offset,
				uint32_t spadCount,
				uint32_t *lastSpad)
{
	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	uint32_t index;
	uint32_t i;
	int32_t nextGoodSpad = offset;
	uint32_t currentSpad;
	uint8_t checkSpadArray[6];
	/*
	 * This function takes in a spad array which may or may not have SPADS
	 * already enabled and appends from a given offset a requested number
	 * of new SPAD enables. The 'good spad map' is applied to
	 * determine the next SPADs to enable.
	 *
	 * This function applies to only aperture or only non-aperture spads.
	 * Checks are performed to ensure this.
	 */

	currentSpad = offset;
	for (index = 0; index < spadCount; index++) {
		get_next_good_spad(goodSpadArray, size, currentSpad,
			&nextGoodSpad);

		if (nextGoodSpad == -1) {
			status = VL53L0X_ERROR_REF_SPAD_INIT;
			break;
		}

		/* Confirm that the next good SPAD is non-aperture */
		if (is_aperture(start + nextGoodSpad) != apertureSpads) {
			/* if we can't get the required number of good aperture
			 * spads from the current quadrant then this is an error
			 */
			status = VL53L0X_ERROR_REF_SPAD_INIT;
			break;
		}
		currentSpad = (uint32_t)nextGoodSpad;
		enable_spad_bit(spadArray, size, currentSpad);
		currentSpad++;
	}
	*lastSpad = currentSpad;

	if (status == VL53L0X_ERROR_NONE)
		status = set_ref_spad_map(Dev, spadArray);

	if (status == VL53L0X_ERROR_NONE) {
		status = get_ref_spad_map(Dev, checkSpadArray);

		i = 0;

		/* Compare spad maps. If not equal report error. */
		while (i < size) {
			if (spadArray[i] != checkSpadArray[i]) {
				status = VL53L0X_ERROR_REF_SPAD_INIT;
				break;
			}
			i++;
		}
	}
	// status = VL53L0X_ERROR_NONE;
	return status;
}


VL53L0X_Error perform_ref_signal_measurement(VL53L0X_DEV Dev,
		uint16_t *refSignalRate)
{
	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	VL53L0X_RangingMeasurementData_t rangingMeasurementData;

	uint8_t SequenceConfig = 0;

	/* store the value of the sequence config,
	 * this will be reset before the end of the function
	 */

	SequenceConfig = PALDevDataGet(Dev, SequenceConfig);

	/*
	 * This function performs a reference signal rate measurement.
	 */
	if (status == VL53L0X_ERROR_NONE)
		status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0xC0);

	if (status == VL53L0X_ERROR_NONE)
		status = VL53L0X_PerformSingleRangingMeasurement(Dev,
				&rangingMeasurementData);

	if (status == VL53L0X_ERROR_NONE)
		status = VL53L0X_WrByte(Dev, 0xFF, 0x01);

	if (status == VL53L0X_ERROR_NONE)
		status = VL53L0X_RdWord(Dev,
			VL53L0X_REG_RESULT_PEAK_SIGNAL_RATE_REF,
			refSignalRate);

	if (status == VL53L0X_ERROR_NONE)
		status = VL53L0X_WrByte(Dev, 0xFF, 0x00);

	if (status == VL53L0X_ERROR_NONE) {
		/* restore the previous Sequence Config */
		status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
				SequenceConfig);
		if (status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, SequenceConfig);
	}

	return status;
}

VL53L0X_Error VL53L0X_perform_ref_spad_management(VL53L0X_DEV Dev,
				uint32_t *refSpadCount,
				uint8_t *isApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t lastSpadArray[6];
	uint8_t startSelect = 0xB4;
	uint32_t minimumSpadCount = 3;
	uint32_t maxSpadCount = 44;
	uint32_t currentSpadIndex = 0;
	uint32_t lastSpadIndex = 0;
	int32_t nextGoodSpad = 0;
	uint16_t targetRefRate = 0x0A00; /* 20 MCPS in 9:7 format */
	uint16_t peakSignalRateRef;
	uint32_t needAptSpads = 0;
	uint32_t index = 0;
	uint32_t spadArraySize = 6;
	uint32_t signalRateDiff = 0;
	uint32_t lastSignalRateDiff = 0;
	uint8_t complete = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint32_t refSpadCount_int = 0;
	uint8_t	 isApertureSpads_int = 0;

	/*
	 * The reference SPAD initialization procedure determines the minimum
	 * amount of reference spads to be enables to achieve a target reference
	 * signal rate and should be performed once during initialization.
	 *
	 * Either aperture or non-aperture spads are applied but never both.
	 * Firstly non-aperture spads are set, begining with 5 spads, and
	 * increased one spad at a time until the closest measurement to the
	 * target rate is achieved.
	 *
	 * If the target rate is exceeded when 5 non-aperture spads are enabled,
	 * initialization is performed instead with aperture spads.
	 *
	 * When setting spads, a 'Good Spad Map' is applied.
	 *
	 * This procedure operates within a SPAD window of interest of a maximum
	 * 44 spads.
	 * The start point is currently fixed to 180, which lies towards the end
	 * of the non-aperture quadrant and runs in to the adjacent aperture
	 * quadrant.
	 */


	targetRefRate = PALDevDataGet(Dev, targetRefRate);

	/*
	 * Initialize Spad arrays.
	 * Currently the good spad map is initialised to 'All good'.
	 * This is a short term implementation. The good spad map will be
	 * provided as an input.
	 * Note that there are 6 bytes. Only the first 44 bits will be used to
	 * represent spads.
	 */
	for (index = 0; index < spadArraySize; index++)
		Dev->Data.SpadData.RefSpadEnables[index] = 0;


	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START_SELECT,
			startSelect);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
				VL53L0X_REG_POWER_MANAGEMENT_GO1_POWER_FORCE, 0);

	/* Perform ref calibration */
	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_perform_ref_calibration(Dev, &VhvSettings,
			&PhaseCal, 0);

	if (Status == VL53L0X_ERROR_NONE) {
		/* Enable Minimum NON-APERTURE Spads */
		currentSpadIndex = 0;
		lastSpadIndex = currentSpadIndex;
		needAptSpads = 0;
		Status = enable_ref_spads(Dev,
					needAptSpads,
					Dev->Data.SpadData.RefGoodSpadMap,
					Dev->Data.SpadData.RefSpadEnables,
					spadArraySize,
					startSelect,
					currentSpadIndex,
					minimumSpadCount,
					&lastSpadIndex);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		currentSpadIndex = lastSpadIndex;

		Status = perform_ref_signal_measurement(Dev,
			&peakSignalRateRef);
		if ((Status == VL53L0X_ERROR_NONE) &&
			(peakSignalRateRef > targetRefRate)) {
			/* Signal rate measurement too high,
			 * switch to APERTURE SPADs */

			for (index = 0; index < spadArraySize; index++)
				Dev->Data.SpadData.RefSpadEnables[index] = 0;


			/* Increment to the first APERTURE spad */
			while ((is_aperture(startSelect + currentSpadIndex)
				== 0) && (currentSpadIndex < maxSpadCount)) {
				currentSpadIndex++;
			}

			needAptSpads = 1;

			Status = enable_ref_spads(Dev,
					needAptSpads,
					Dev->Data.SpadData.RefGoodSpadMap,
					Dev->Data.SpadData.RefSpadEnables,
					spadArraySize,
					startSelect,
					currentSpadIndex,
					minimumSpadCount,
					&lastSpadIndex);

			if (Status == VL53L0X_ERROR_NONE) {
				currentSpadIndex = lastSpadIndex;
				Status = perform_ref_signal_measurement(Dev,
						&peakSignalRateRef);

				if ((Status == VL53L0X_ERROR_NONE) &&
					(peakSignalRateRef > targetRefRate)) {
					/* Signal rate still too high after
					 * setting the minimum number of
					 * APERTURE spads. Can do no more
					 * therefore set the min number of
					 * aperture spads as the result.
					 */
					isApertureSpads_int = 1;
					refSpadCount_int = minimumSpadCount;
				}
			}
		} else {
			needAptSpads = 0;
		}
	}

	if ((Status == VL53L0X_ERROR_NONE) &&
		(peakSignalRateRef < targetRefRate)) {
		/* At this point, the minimum number of either aperture
		 * or non-aperture spads have been set. Proceed to add
		 * spads and perform measurements until the target
		 * reference is reached.
		 */
		isApertureSpads_int = needAptSpads;
		refSpadCount_int	= minimumSpadCount;

		memcpy(lastSpadArray, Dev->Data.SpadData.RefSpadEnables,
				spadArraySize);
		if(peakSignalRateRef > targetRefRate)
		{
			signalRateDiff = peakSignalRateRef - targetRefRate;
		}
		else
		{
			signalRateDiff = targetRefRate - peakSignalRateRef ;
		}
		//lastSignalRateDiff = abs(peakSignalRateRef -targetRefRate);
		complete = 0;

		while (!complete) {
			get_next_good_spad(
				Dev->Data.SpadData.RefGoodSpadMap,
				spadArraySize, currentSpadIndex,
				&nextGoodSpad);

			if (nextGoodSpad == -1) {
				Status = VL53L0X_ERROR_REF_SPAD_INIT;
				break;
			}

			(refSpadCount_int)++;

			/* Cannot combine Aperture and Non-Aperture spads, so
			 * ensure the current spad is of the correct type.
			 */
			if (is_aperture((uint32_t)startSelect + nextGoodSpad) !=
					needAptSpads) {
				Status = VL53L0X_ERROR_REF_SPAD_INIT;
				break;
			}

			currentSpadIndex = nextGoodSpad;
			Status = enable_spad_bit(
					Dev->Data.SpadData.RefSpadEnables,
					spadArraySize, currentSpadIndex);

			if (Status == VL53L0X_ERROR_NONE) {
				currentSpadIndex++;
				/* Proceed to apply the additional spad and
				 * perform measurement. */
				Status = set_ref_spad_map(Dev,
					Dev->Data.SpadData.RefSpadEnables);
			}

			if (Status != VL53L0X_ERROR_NONE)
				break;

			Status = perform_ref_signal_measurement(Dev,
					&peakSignalRateRef);

			if (Status != VL53L0X_ERROR_NONE)
				break;
			if(peakSignalRateRef > targetRefRate)
			{
				signalRateDiff = peakSignalRateRef - targetRefRate;
			}
			else
			{
				signalRateDiff = targetRefRate - peakSignalRateRef ;
			}
			

			if (peakSignalRateRef > targetRefRate) {
				/* Select the spad map that provides the
				 * measurement closest to the target rate,
				 * either above or below it.
				 */
				if (signalRateDiff > lastSignalRateDiff) {
					/* Previous spad map produced a closer
					 * measurement, so choose this. */
					Status = set_ref_spad_map(Dev,
							lastSpadArray);
					memcpy(
					Dev->Data.SpadData.RefSpadEnables,
					lastSpadArray, spadArraySize);

					(refSpadCount_int)--;
				}
				complete = 1;
			} else {
				/* Continue to add spads */
				lastSignalRateDiff = signalRateDiff;
				memcpy(lastSpadArray,
					Dev->Data.SpadData.RefSpadEnables,
					spadArraySize);
			}

		} /* while */
	}

	if (Status == VL53L0X_ERROR_NONE) {
		*refSpadCount = refSpadCount_int;
		*isApertureSpads = isApertureSpads_int;

		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, RefSpadsInitialised, 1);
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadCount, (uint8_t)(*refSpadCount));
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadType, *isApertureSpads);
	}

	return Status;
}

VL53L0X_Error VL53L0X_set_reference_spads(VL53L0X_DEV Dev,
				 uint32_t count, uint8_t isApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t currentSpadIndex = 0;
	uint8_t startSelect = 0xB4;
	uint32_t spadArraySize = 6;
	uint32_t maxSpadCount = 44;
	uint32_t lastSpadIndex;
	uint32_t index;

	/*
	 * This function applies a requested number of reference spads, either
	 * aperture or
	 * non-aperture, as requested.
	 * The good spad map will be applied.
	 */

	Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev,
			VL53L0X_REG_GLOBAL_CONFIG_REF_EN_START_SELECT,
			startSelect);

	for (index = 0; index < spadArraySize; index++)
		Dev->Data.SpadData.RefSpadEnables[index] = 0;

	if (isApertureSpads) {
		/* Increment to the first APERTURE spad */
		while ((is_aperture(startSelect + currentSpadIndex) == 0) &&
			  (currentSpadIndex < maxSpadCount)) {
			currentSpadIndex++;
		}
	}
	Status = enable_ref_spads(Dev,
				isApertureSpads,
				Dev->Data.SpadData.RefGoodSpadMap,
				Dev->Data.SpadData.RefSpadEnables,
				spadArraySize,
				startSelect,
				currentSpadIndex,
				count,
				&lastSpadIndex);
	if (Status == VL53L0X_ERROR_NONE) {
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev, RefSpadsInitialised, 1);
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadCount, (uint8_t)(count));
		VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadType, isApertureSpads);
	}

	return Status;
}

VL53L0X_Error VL53L0X_get_reference_spads(VL53L0X_DEV Dev,
			uint32_t *pSpadCount, uint8_t *pIsApertureSpads)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t refSpadsInitialised;
	uint8_t refSpadArray[6];
	uint32_t cMaxSpadCount = 44;
	uint32_t cSpadArraySize = 6;
	uint32_t spadsEnabled;
	uint8_t isApertureSpads = 0;

	refSpadsInitialised = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
					RefSpadsInitialised);

	if (refSpadsInitialised == 1) {

		*pSpadCount = (uint32_t)VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadCount);
		*pIsApertureSpads = VL53L0X_GETDEVICESPECIFICPARAMETER(Dev,
			ReferenceSpadType);
	} else {

		/* obtain spad info from device.*/
		Status = get_ref_spad_map(Dev, refSpadArray);

		if (Status == VL53L0X_ERROR_NONE) {
			/* count enabled spads within spad map array and
			 * determine if Aperture or Non-Aperture.
			 */
			Status = count_enabled_spads(refSpadArray,
							cSpadArraySize,
							cMaxSpadCount,
							&spadsEnabled,
							&isApertureSpads);

			if (Status == VL53L0X_ERROR_NONE) {

				*pSpadCount = spadsEnabled;
				*pIsApertureSpads = isApertureSpads;

				VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					RefSpadsInitialised, 1);
				VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					ReferenceSpadCount,
					(uint8_t)spadsEnabled);
				VL53L0X_SETDEVICESPECIFICPARAMETER(Dev,
					ReferenceSpadType, isApertureSpads);
			}
		}
	}

	return Status;
}


VL53L0X_Error VL53L0X_perform_single_ref_calibration(VL53L0X_DEV Dev,
		uint8_t vhv_init_byte)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSRANGE_START,
				VL53L0X_REG_SYSRANGE_MODE_START_STOP |
				vhv_init_byte);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_measurement_poll_for_completion(Dev);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_ClearInterruptMask(Dev, 0);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x00);

	return Status;
}


VL53L0X_Error VL53L0X_ref_calibration_io(VL53L0X_DEV Dev, uint8_t read_not_write,
	uint8_t VhvSettings, uint8_t PhaseCal,
	uint8_t *pVhvSettings, uint8_t *pPhaseCal,
	const uint8_t vhv_enable, const uint8_t phase_enable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t PhaseCalint = 0;

	/* Read VHV from device */
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0x00, 0x00);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);

	if (read_not_write) {
		if (vhv_enable)
			Status |= VL53L0X_RdByte(Dev, 0xCB, pVhvSettings);
		if (phase_enable)
			Status |= VL53L0X_RdByte(Dev, 0xEE, &PhaseCalint);
	} else {
		if (vhv_enable)
			Status |= VL53L0X_WrByte(Dev, 0xCB, VhvSettings);
		if (phase_enable)
			Status |= VL53L0X_UpdateByte(Dev, 0xEE, 0x80, PhaseCal);
	}

	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0x00, 0x01);
	Status |= VL53L0X_WrByte(Dev, 0xFF, 0x00);

	*pPhaseCal = (uint8_t)(PhaseCalint&0xEF);

	return Status;
}


VL53L0X_Error VL53L0X_perform_vhv_calibration(VL53L0X_DEV Dev,
	uint8_t *pVhvSettings, const uint8_t get_data_enable,
	const uint8_t restore_config)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint8_t PhaseCalInt = 0;

	/* store the value of the sequence config,
	 * this will be reset before the end of the function
	 */

	if (restore_config)
		SequenceConfig = PALDevDataGet(Dev, SequenceConfig);

	/* Run VHV */
	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x01);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_perform_single_ref_calibration(Dev, 0x40);

	/* Read VHV from device */
	if ((Status == VL53L0X_ERROR_NONE) && (get_data_enable == 1)) {
		Status = VL53L0X_ref_calibration_io(Dev, 1,
			VhvSettings, PhaseCal, /* Not used here */
			pVhvSettings, &PhaseCalInt,
			1, 0);
	} else
		*pVhvSettings = 0;


	if ((Status == VL53L0X_ERROR_NONE) && restore_config) {
		/* restore the previous Sequence Config */
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
				SequenceConfig);
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, SequenceConfig);

	}

	return Status;
}

VL53L0X_Error VL53L0X_perform_phase_calibration(VL53L0X_DEV Dev,
	uint8_t *pPhaseCal, const uint8_t get_data_enable,
	const uint8_t restore_config)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;
	uint8_t VhvSettingsint;

	/* store the value of the sequence config,
	 * this will be reset before the end of the function
	 */

	if (restore_config)
		SequenceConfig = PALDevDataGet(Dev, SequenceConfig);

	/* Run PhaseCal */
	Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG, 0x02);

	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_perform_single_ref_calibration(Dev, 0x0);

	/* Read PhaseCal from device */
	if ((Status == VL53L0X_ERROR_NONE) && (get_data_enable == 1)) {
		Status = VL53L0X_ref_calibration_io(Dev, 1,
			VhvSettings, PhaseCal, /* Not used here */
			&VhvSettingsint, pPhaseCal,
			0, 1);
	} else
		*pPhaseCal = 0;


	if ((Status == VL53L0X_ERROR_NONE) && restore_config) {
		/* restore the previous Sequence Config */
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
				SequenceConfig);
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, SequenceConfig);

	}

	return Status;
}

VL53L0X_Error VL53L0X_perform_ref_calibration(VL53L0X_DEV Dev,
	uint8_t *pVhvSettings, uint8_t *pPhaseCal, uint8_t get_data_enable)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t SequenceConfig = 0;

	/* store the value of the sequence config,
	 * this will be reset before the end of the function
	 */

	SequenceConfig = PALDevDataGet(Dev, SequenceConfig);

	/* In the following function we don't save the config to optimize
	 * writes on device. Config is saved and restored only once. */
	Status = VL53L0X_perform_vhv_calibration(
			Dev, pVhvSettings, get_data_enable, 0);


	if (Status == VL53L0X_ERROR_NONE)
		Status = VL53L0X_perform_phase_calibration(
			Dev, pPhaseCal, get_data_enable, 0);


	if (Status == VL53L0X_ERROR_NONE) {
		/* restore the previous Sequence Config */
		Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_SEQUENCE_CONFIG,
				SequenceConfig);
		if (Status == VL53L0X_ERROR_NONE)
			PALDevDataSet(Dev, SequenceConfig, SequenceConfig);

	}

	return Status;
}

VL53L0X_Error VL53L0X_set_ref_calibration(VL53L0X_DEV Dev,
		uint8_t VhvSettings, uint8_t PhaseCal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t pVhvSettings;
	uint8_t pPhaseCal;

	Status = VL53L0X_ref_calibration_io(Dev, 0,
		VhvSettings, PhaseCal,
		&pVhvSettings, &pPhaseCal,
		1, 1);

	return Status;
}

VL53L0X_Error VL53L0X_get_ref_calibration(VL53L0X_DEV Dev,
		uint8_t *pVhvSettings, uint8_t *pPhaseCal)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t VhvSettings = 0;
	uint8_t PhaseCal = 0;

	Status = VL53L0X_ref_calibration_io(Dev, 1,
		VhvSettings, PhaseCal,
		pVhvSettings, pPhaseCal,
		1, 1);

	return Status;
}




/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/
static VL53L0X_Dev_t MyDevice = {
	.I2cDevAddr = 0x29,
	.comms_type = 1,
	.comms_speed_khz = 400,
};

static VL53L0X_Dev_t *pMyDevice = &MyDevice;
static VL53L0X_Version_t version;
static VL53L0X_Version_t* pVersion = &version;

static VL53L0X_Error check_version(void)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	int32_t status_int;

	status_int = VL53L0X_GetVersion(pVersion);
	if (status_int != 0){
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;
		return Status;
	}
    
	if( pVersion->major != VERSION_REQUIRED_MAJOR ||
		pVersion->minor != VERSION_REQUIRED_MINOR ||	
		pVersion->build != VERSION_REQUIRED_BUILD ){
		}
	return Status;
}

static VL53L0X_Error VL53L0X_calibration_oprt(void)
{
	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	uint8_t VhvSettings;
	uint8_t PhaseCal;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	Status = VL53L0X_PerformRefSpadManagement(pMyDevice,
		&refSpadCount, &isApertureSpads); 
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	Status = VL53L0X_PerformRefCalibration(pMyDevice,
		&VhvSettings, &PhaseCal); 
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	return Status;
}

static VL53L0X_Error VL53L0X_set_limit_param(void)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;        

	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_SetLimitCheckValue(pMyDevice,
		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
		(FixPoint1616_t)(2261));
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	return Status;
}


static int32_t common_init(void)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	Status=check_version();
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	Status = VL53L0X_DataInit(&MyDevice); 
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_StaticInit(pMyDevice); 
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status=VL53L0X_calibration_oprt();
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	Status = VL53L0X_set_limit_param();
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
	if(VL53L0X_ERROR_NONE!=Status) return Status;
	return Status;
}


VL53L0X_Error VL53L0X_single_ranging_init(void)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;        

	Status = VL53L0X_SetLimitCheckEnable(pMyDevice,
		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	Status = VL53L0X_SetLimitCheckValue(pMyDevice,
		VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
		(FixPoint1616_t)2261);
	if(VL53L0X_ERROR_NONE!=Status) return Status;

	return Status;
}


static VL53L0X_Error PerformSingleRangingMeasurement(VL53L0X_RangingMeasurementData_t* RangingMeasurementData)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	if(Status == VL53L0X_ERROR_NONE){
	Status = VL53L0X_PerformSingleRangingMeasurement(pMyDevice,
			RangingMeasurementData);
	return Status;
    }
	return Status;
}

static uint32_t read_distance(void)
{
	VL53L0X_RangingMeasurementData_t RangingMeasurementData;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	memset(&RangingMeasurementData,0,sizeof(VL53L0X_RangingMeasurementData_t));
	Status = PerformSingleRangingMeasurement(&RangingMeasurementData);
	if(VL53L0X_ERROR_NONE == Status){
	return RangingMeasurementData.RangeMilliMeter;
	}
	else{
		return -1;
	}
}

/**************************************************************************************************************/

static ssize_t vl53l0x_distance_show(struct kobject* kobjs,struct kobj_attribute *attr,char *buf)
{
	uint32_t distance = 0;
	distance = read_distance();
	return sprintf(buf,"The distance = %d\n",distance);
}
static struct kobj_attribute status_attr = __ATTR_RO(vl53l0x_distance);


/**************************************************************************************************************/
/**************************************************************************************************************/
/**************************************************************************************************************/


static const struct i2c_device_id vl53l0x_drv_id_table[] = {
	{"vl53l0x",0},
	{},
};


static int major;
static struct class *vl53l0x_i2c_cls;
static struct device *vl53l0x_i2c_dev;
static const char* CLASS_NAME = "vl53l0x_cls";
static const char* DEVICE_NAME = "vl53l0x_dev";



static int vl53l0x_i2c_open(struct inode *node, struct file *file)
{
	return 0;
}

static ssize_t vl53l0x_i2c_read(struct file *file,char *buf, size_t len,loff_t *offset)
{
	int cnt = 0;
	uint32_t buf_len = 0;
	uint32_t distance = 0;
	uint8_t dis_buf[50] = {0};
	distance = read_distance();
	if(distance < 0){
		sprintf(dis_buf,"Read sensor failed\n");
	}
	else if(distance >= 2000){
		sprintf(dis_buf,"Object Distance out of range\n");
	}
	else{
		sprintf(dis_buf,"Object Distance = %d mm\n",distance);
	}
	buf_len = strlen(dis_buf);
	cnt = copy_to_user(buf,dis_buf,strlen(dis_buf));
	if(cnt){
		printk(KERN_INFO "copy to user failed\n");
		return -ENOMEM;
	}
	return buf_len;
}


static struct file_operations file_oprts = {
	.open = vl53l0x_i2c_open,
	.read = vl53l0x_i2c_read,
};


static int vl53l0x_drv_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	vl53l0x_client = client;

	Status = common_init();
	if(VL53L0X_ERROR_NONE != Status){
		printk(KERN_ALERT "Init sensor failed!\n");
		return -ENODEV;
	}
	Status = VL53L0X_single_ranging_init();
	if(VL53L0X_ERROR_NONE != Status){
		printk(KERN_ALERT "single ranging init failed!\n");
		return -ENODEV;
    	}

	if(sysfs_create_file(&vl53l0x_client->dev.kobj, &status_attr.attr) < 0){
		printk(KERN_ALERT "Fail to create sys file\n");
		return -ENOMEM;
    	}

	major = register_chrdev(0,DEVICE_NAME,&file_oprts);
	if(major < 0 ){
		sysfs_remove_file(&vl53l0x_client->dev.kobj, &status_attr.attr);
		printk(KERN_ALERT "Register failed!!\r\n");
		return major;
	}
	printk(KERN_ALERT "Registe success,major number = %d\r\n",major);

	vl53l0x_i2c_cls = class_create(THIS_MODULE,CLASS_NAME);
	if(IS_ERR(vl53l0x_i2c_cls)){
		sysfs_remove_file(&vl53l0x_client->dev.kobj, &status_attr.attr);
		unregister_chrdev(major,DEVICE_NAME);
		return PTR_ERR(vl53l0x_i2c_cls);
	}

	vl53l0x_i2c_dev = device_create(vl53l0x_i2c_cls,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
	if(IS_ERR(vl53l0x_i2c_dev)){
		sysfs_remove_file(&vl53l0x_client->dev.kobj, &status_attr.attr);
		class_destroy(vl53l0x_i2c_cls);
		unregister_chrdev(major,DEVICE_NAME);
		return PTR_ERR(vl53l0x_i2c_dev);
	}
    	printk(KERN_ALERT "vl53l0x_i2c device init success!!\r\n");
    

    	return 0;
}

static int vl53l0x_drv_remove(struct i2c_client *client)
{
	sysfs_remove_file(&vl53l0x_client->dev.kobj, &status_attr.attr);
	device_destroy(vl53l0x_i2c_cls,MKDEV(major,0));
	class_unregister(vl53l0x_i2c_cls);
	class_destroy(vl53l0x_i2c_cls);
	unregister_chrdev(major,DEVICE_NAME);

    	return 0;
}



static const struct of_device_id vl53l0x_match_table[] = {
	{.compatible = "st,vl53l0x"},
	{},
};

static struct i2c_driver vl53l0x_drv = {
	.driver = {
		.name = "vl53l0x",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(vl53l0x_match_table),
	},
	.probe = vl53l0x_drv_probe,
	.remove = vl53l0x_drv_remove,
	.id_table = vl53l0x_drv_id_table,
};

MODULE_DEVICE_TABLE(of,vl53l0x_match_table);


int drv_init(void)
{
	int ret = 0;
	ret  = i2c_add_driver(&vl53l0x_drv);
	if(ret){
		printk(KERN_ALERT "add driver failed!!!\n");
		return -ENODEV;
	}
	return 0;
}


void drv_exit(void)
{
	i2c_del_driver(&vl53l0x_drv);
	return ;
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is for Seeed vl53l0x ranging sensor.");
MODULE_AUTHOR("Downey");

module_init(drv_init);
module_exit(drv_exit);
