/* ThorLabsDFM.cpp
 *
 * Driver for ThorLabs Deformable Mirror
 *
 * Mark Rivers
 * University of Chicago
 *
 */

/* Standard includes... */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* EPICS includes */
#include <epicsString.h>
#include <iocsh.h>


#include <epicsExport.h>
#include <ThorLabsDFM.h>

#define DRIVER_VERSION      "1.0.0"

/** Only used for debugging/error messages to identify where the message comes from*/
static const char *driverName = "ThorLabsDFM";

extern "C" int ThorLabsDFMConfig(const char *portName, const char *resourceIn)
{
    new ThorLabsDFM(portName, resourceIn);
    return 0;
}

void error_exit(ViSession instrHdl, ViStatus err)
{
	ViChar buf[TLDFM_ERR_DESCR_BUFER_SIZE];

	// Get error description and print out error
	TLDFM_error_message (instrHdl, err, buf);
	fprintf(stderr, "\nERROR: %s\n", buf);

	// close session to instrument if open
	if(VI_NULL != instrHdl)
	{
		TLDFM_close(instrHdl);
	}

	// exit program
	printf("\nThe program is shutting down.\n");

	exit (EXIT_FAILURE);
}

ThorLabsDFM::ThorLabsDFM(const char *portName, const char *resourceIn)
    : asynPortDriver(portName, THORLABS_MAX_ADDR,
            asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask,
            asynInt32Mask | asynFloat64Mask | asynOctetMask,
            ASYN_CANBLOCK, 1, 0, 0)

{
    const char *functionName = "ThorLabsDFM";
  	ViStatus err;
  	unsigned int i;
  	ViUInt32 deviceCount = 0;
  
  	printf("Scanning for instruments...\n");
  	err = TLDFM_get_device_count(VI_NULL, &deviceCount);
  	if(TL_ERROR_RSRC_NFOUND == err)
  	{
  		error_exit(0, err);
  	}
  	else if(0 == deviceCount)
  	{
  		printf("No matching instruments found\n\n");
  		error_exit(0, err);
  	}
  
  	printf("Found %d matching instrument(s):\n\n", deviceCount);
  
  	if (deviceCount > 0) {
  	  for(ViUInt32 i = 0; i < deviceCount;)
    	{
    		err = TLDFM_get_device_information(VI_NULL,
    										   i,
    										   manufacturer_,
    										   instrumentName_,
    										   serialNumber_,
    										   &deviceAvailable_,
    										   resourceName_);
    
    		printf("%d:\t%s\t%s\tS/N:%s\t%s\t%s\n",
    			   ++i,
    			   manufacturer_,
    			   instrumentName_,
    			   serialNumber_,
    			   deviceAvailable_ ? "available" : "locked",
    			   resourceName_);
    	}
   }
  
  	err = TLDFM_get_device_information(VI_NULL,
  									   0,
  									   manufacturer_,
  									   instrumentName_,
  									   serialNumber_,
  									   &deviceAvailable_,
  									   resourceName_);
  
  	if(VI_SUCCESS != err)
  	{
  	   error_exit(0, err);
  	}

    if (resourceIn && strlen(resourceIn) > 0)
    strcpy(resourceName_, resourceIn);
    
	// Open session to instrument
	printf("Opening session to '%s' ...\n", resourceName_);
	err = TLDFMX_init(resourceName_, VI_TRUE, VI_FALSE, &instrHdl_);
	if(err) error_exit(instrHdl_, err);  // can not open session to instrument

  err = TLDFM_get_device_configuration (instrHdl_,
                                        &segmentCnt_,
                                        &minSegmentVoltage_,
                                        &maxSegmentVoltage_,
                                        &segmentCommonVoltageMax_,
                                        &tiltElementCnt_,
                                        &minTiltVoltage_,
                                        &maxTiltVoltage_,
                                        &tiltCommonVoltageMax_);

    /* Internal asyn driver parameters */
    createParam(TLDFM_ReadStatusString,             asynParamInt32,   &TLDFM_ReadStatus);
    createParam(TLDFM_MeasuredSegmentVoltsString,   asynParamFloat64, &TLDFM_MeasuredSegmentVolts);
    createParam(TLDFM_MeasuredTiltVoltsString,      asynParamFloat64, &TLDFM_MeasuredTiltVolts);
    createParam(TLDFM_SetSegmentVoltsString,        asynParamFloat64, &TLDFM_SetSegmentVolts);
    createParam(TLDFM_ResetSegmentVoltsString,      asynParamInt32,   &TLDFM_ResetSegmentVolts);
    createParam(TLDFM_SetTiltVoltsString,           asynParamFloat64, &TLDFM_SetTiltVolts);
    createParam(TLDFM_SetTiltAmplitudeString,       asynParamFloat64, &TLDFM_SetTiltAmplitude);
    createParam(TLDFM_SetTiltAngleString,           asynParamFloat64, &TLDFM_SetTiltAngle);
    createParam(TLDFM_SetZernikeValueString,        asynParamFloat64, &TLDFM_SetZernikeValue);
    createParam(TLDFM_ResetZernikeValuesString,     asynParamInt32,   &TLDFM_ResetZernikeValues);
    createParam(TLDFM_TemperatureString,            asynParamFloat64, &TLDFM_Temperature);
    
    // This allows the initial values of the ao records for the voltages to be read from the device
    // If they are in autosave these will be overwritten
    this->readVoltages();
    for (i=0; i<segmentCnt_; i++) {
        setDoubleParam(i, TLDFM_SetSegmentVolts, segmentVoltages_[i]);
    }
    for (i=0; i<tiltElementCnt_; i++) {
        setDoubleParam(i, TLDFM_SetTiltVolts, tiltVoltages_[i]);
    }
    this->readTemperatures();
}

ViStatus ThorLabsDFM::readVoltages()
{
  	ViStatus err;
  	unsigned int i;

    err = TLDFM_get_measured_voltages(instrHdl_, segmentVoltages_, tiltVoltages_);
    if (err) {
        return err;
    }
    for (i=0; i<segmentCnt_; i++) {
        setDoubleParam(i, TLDFM_MeasuredSegmentVolts, segmentVoltages_[i]);
    }
    for (i=0; i<tiltElementCnt_; i++) {
        setDoubleParam(i, TLDFM_MeasuredTiltVolts, tiltVoltages_[i]);
    }
    /* Call the callbacks */
    for (i=0; i<segmentCnt_; i++) {
        callParamCallbacks(i);
    }
    return err;
}

ViStatus ThorLabsDFM::readTemperatures()
{
  	ViStatus err;
  	unsigned int i;

    err = TLDFM_get_temperatures(instrHdl_, &(temperatures_[0]), &(temperatures_[1]), &(temperatures_[2]), &(temperatures_[3]));
    if (err) {
        return err;
    }
    for (i=0; i<THORLABS_MAX_TEMPERATURES; i++) {
        setDoubleParam(i, TLDFM_Temperature, temperatures_[i]);
    }
    /* Call the callbacks */
    for (i=0; i<THORLABS_MAX_TEMPERATURES; i++) {
        callParamCallbacks(i);
    }
    return err;
}

/* virtual methods to override from asynNDArrayDriver */
asynStatus ThorLabsDFM::writeInt32( asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
  	ViStatus err;
    int address;
    double zernikeValues[THORLABS_MAX_ZERNIKE_VALUES];
    double voltages[THORLABS_MAX_SEGMENT_VOLTAGES];
    const char* functionName = "writeInt32";

    this->getAddress(pasynUser, &address);
    /* Set the parameter and readback in the parameter library.  This may be overwritten later but that's OK */
    setIntegerParam(address, function, value);
    if (function == TLDFM_ReadStatus) {
        err = this->readVoltages();
        if (err) {
            goto done;
        }
        err = this->readTemperatures();
        if (err) {
            goto done;
        }
    }
    else if (function == TLDFM_ResetSegmentVolts) {
        for (int i=0; i<THORLABS_MAX_SEGMENT_VOLTAGES; i++) {
            setDoubleParam(i, TLDFM_SetSegmentVolts, THORLABS_RESET_VOLTAGE_VALUE);
            voltages[i] = THORLABS_RESET_VOLTAGE_VALUE;
        }
        err = TLDFM_set_segment_voltages(instrHdl_, voltages);
    }
    else if (function == TLDFM_ResetZernikeValues) {
        for (int i=0; i<THORLABS_MAX_ZERNIKE_VALUES; i++) {
            setDoubleParam(i, TLDFM_SetZernikeValue, THORLABS_RESET_ZERNIKE_VALUE);
            zernikeValues[i] = THORLABS_RESET_ZERNIKE_VALUE;
        }
        err = TLDFMX_calculate_zernike_pattern(instrHdl_, Z_All_Flag, zernikeValues, voltages);
        if (err) goto done;
        for (int i=0; i<THORLABS_MAX_SEGMENT_VOLTAGES; i++) {
            setDoubleParam(i, TLDFM_SetSegmentVolts, voltages[i]);
        }
        err = TLDFM_set_segment_voltages(instrHdl_, voltages);
    }
    done:
    if (err) {
        const char *paramName;
        this->getParamName(function, &paramName);
        TLDFM_error_message(instrHdl_, err, errorMessage_);
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s::%s param=%s, error calling TLDFM library, err=%d, message=%s\n",
            driverName, functionName, paramName, err, errorMessage_);
        return asynError;
    }
    
    return asynSuccess;
}

asynStatus ThorLabsDFM::writeFloat64( asynUser *pasynUser, epicsFloat64 value)
{
    asynStatus status = asynSuccess;
    int function = pasynUser->reason;
  	ViStatus err = 0;
    int address;
    double zernikeValues[THORLABS_MAX_ZERNIKE_VALUES];
    double voltages[THORLABS_MAX_SEGMENT_VOLTAGES];
    const char *functionName = "writeFloat64";

    this->getAddress(pasynUser, &address);
    /* Set the parameter and readback in the parameter library.  This may be overwritten later but that's OK */
    status = setDoubleParam(address, function, value);

    if (function == TLDFM_SetSegmentVolts) {
        err = TLDFM_set_segment_voltage(instrHdl_, address, value);
    }
    else if (function == TLDFM_SetTiltVolts) {
        err = TLDFM_set_tilt_voltage(instrHdl_, address, value);
    }
    else if ((function == TLDFM_SetTiltAmplitude) ||
             (function == TLDFM_SetTiltAngle)) {
        double amplitude, angle;
        getDoubleParam(TLDFM_SetTiltAmplitude, &amplitude);
        getDoubleParam(TLDFM_SetTiltAngle, &angle);
        // We use % for amplitude, SDK wants 0-1.
        err = TLDFM_set_tilt_amplitude_angle(instrHdl_, amplitude/100., angle);
        if (err) goto done;
        err = TLDFM_get_tilt_voltages(instrHdl_, voltages);
        for (int i=0; i<THORLABS_MAX_TILT_VOLTAGES; i++) {
            setDoubleParam(i, TLDFM_SetTiltVolts, voltages[i]);
        }       
    }
    else if (function == TLDFM_SetZernikeValue) {
        for (int i=0; i<THORLABS_MAX_ZERNIKE_VALUES; i++) {
           getDoubleParam(i, TLDFM_SetZernikeValue, &zernikeValues[i]);
        }
        err = TLDFMX_calculate_zernike_pattern(instrHdl_, Z_All_Flag, zernikeValues, voltages);
        if (err) goto done;
        for (int i=0; i<THORLABS_MAX_SEGMENT_VOLTAGES; i++) {
            setDoubleParam(i, TLDFM_SetSegmentVolts, voltages[i]);
        }
        err = TLDFM_set_segment_voltages(instrHdl_, voltages);
    }

    done:
    if (err) {
        const char *paramName;
        this->getParamName(function, &paramName);
        TLDFM_error_message(instrHdl_, err, errorMessage_);
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s::%s param=%s, error calling TLDFM library, err=%d, message=%s\n",
            driverName, functionName, paramName, err, errorMessage_);
        return asynError;
    }
    /* Call the callback */
    for (int i=0; i<THORLABS_MAX_ADDR; i++) {
        callParamCallbacks(i);
    }
    return asynSuccess;
}

void ThorLabsDFM::report(FILE *fp, int details)
{
    if (details > 0) {
        fprintf(fp, "  Device information:\n");                                           
        fprintf(fp, "    Manufacturer=%s\n", manufacturer_);                                           
        fprintf(fp, "    Instrument name=%s\n", instrumentName_);                                           
        fprintf(fp, "    Serial number=%s\n", serialNumber_);                                           
        fprintf(fp, "    Device available=%s\n", deviceAvailable_ ? "available" : "locked");                                           
        fprintf(fp, "    Resource name=%s\n", resourceName_);                                           
        fprintf(fp, "    Segment count=%d\n", segmentCnt_);                                           
        fprintf(fp, "    Min. segment voltage=%f\n", minSegmentVoltage_);                                           
        fprintf(fp, "    Max. segment voltage=%f\n", maxSegmentVoltage_);                                           
        fprintf(fp, "    Max. segment common voltage=%f\n", segmentCommonVoltageMax_);                                           
        fprintf(fp, "    Tilt element count=%d\n", tiltElementCnt_);                                           
        fprintf(fp, "    Min. tilt voltage=%f\n", minTiltVoltage_);                                           
        fprintf(fp, "    Max. tilt voltage=%f\n", maxTiltVoltage_);                                           
        fprintf(fp, "    Max. tilt common voltage=%f\n", tiltCommonVoltageMax_);                                           
    }
    asynPortDriver::report(fp, details);
}

static const iocshArg ThorLabsDFMConfigArg0 = {"Asyn port name", iocshArgString};
static const iocshArg ThorLabsDFMConfigArg1 = {"Resource name", iocshArgString};
static const iocshArg * const ThorLabsDFMConfigArgs[] =  {&ThorLabsDFMConfigArg0,
                                                    &ThorLabsDFMConfigArg1};
static const iocshFuncDef configThorLabsDFM = {"ThorLabsDFMConfig", 2, ThorLabsDFMConfigArgs};
static void configThorLabsDFMCallFunc(const iocshArgBuf *args)
{
    ThorLabsDFMConfig(args[0].sval, args[1].sval);
}

static void ThorLabsDFMRegister(void)
{
    iocshRegister(&configThorLabsDFM, configThorLabsDFMCallFunc);
}

extern "C" {
epicsExportRegistrar(ThorLabsDFMRegister);
}
	ViChar buf[TLDFM_ERR_DESCR_BUFER_SIZE];
