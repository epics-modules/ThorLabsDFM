#ifndef THORLABSDFM_H
#define THORLABSDFM_H

#include <asynPortDriver.h>
#include <TLDFM.h>

#define THORLABS_MAX_SEGMENT_VOLTAGES  40
#define THORLABS_MAX_TILT_VOLTAGES      3
#define THORLABS_MAX_TEMPERATURES       4

/* Internal asyn driver parameters */
#define TLDFM_ReadStatusString               "TLDFM_ReadStatus"
#define TLDFM_MeasuredSegmentVoltsString     "TLDFM_MeasuredSegmentVolts"
#define TLDFM_MeasuredTiltVoltsString        "TLDFM_MeasuredTiltVolts"
#define TLDFM_SetSegmentVoltsString          "TLDFM_SetSegmentVolts"
#define TLDFM_SetTiltVoltsString             "TLDFM_SetTiltVolts"
#define TLDFM_SetTiltAmplitudeString         "TLDFM_SetTiltAmplitude"
#define TLDFM_SetTiltAngleString             "TLDFM_SetTiltAngle"
#define TLDFM_TemperatureString              "TLDFM_Temperature"


class ThorLabsDFM : public asynPortDriver
{
public:
    ThorLabsDFM(const char *portName, const char *resourceName);

    /* virtual methods to override from asynNDArrayDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    void report(FILE *fp, int details);

protected:

    /* General parameters */
    #define FIRST_TLDFM_PARAM TLDFM_ReadStatus
    /* Internal asyn driver parameters */
    int TLDFM_ReadStatus;
    int TLDFM_MeasuredSegmentVolts;
    int TLDFM_MeasuredTiltVolts;
    int TLDFM_SetSegmentVolts;
    int TLDFM_SetTiltVolts;
    int TLDFM_SetTiltAmplitude;
    int TLDFM_SetTiltAngle;
    int TLDFM_Temperature;

private:
    ViStatus ThorLabsDFM::readVoltages();
    ViStatus ThorLabsDFM::readTemperatures();
  	ViSession instrHdl_;
  	ViChar    errorMessage_[TLDFM_ERR_DESCR_BUFER_SIZE];
  	ViChar    manufacturer_[TLDFM_BUFFER_SIZE];
  	ViChar    instrumentName_[TLDFM_MAX_INSTR_NAME_LENGTH];
  	ViChar    serialNumber_[TLDFM_MAX_SN_LENGTH];
  	ViBoolean deviceAvailable_;
  	ViChar    resourceName_[TLDFM_BUFFER_SIZE];
    ViUInt32  segmentCnt_;
    ViReal64  minSegmentVoltage_;
    ViReal64  maxSegmentVoltage_;
    ViReal64  segmentCommonVoltageMax_;
    ViUInt32  tiltElementCnt_;
    ViReal64  minTiltVoltage_;
    ViReal64  maxTiltVoltage_;
    ViReal64  tiltCommonVoltageMax_;

    ViReal64  segmentVoltages_[THORLABS_MAX_SEGMENT_VOLTAGES];
    ViReal64  tiltVoltages_[THORLABS_MAX_TILT_VOLTAGES];
    ViReal64  temperatures_[THORLABS_MAX_TEMPERATURES];


};
#ifdef __cplusplus
extern "C"
{
#endif

int ThorLabDFMConfig(const char *portName, const char *resourceName);

#ifdef __cplusplus
}
#endif

#endif /* THORLABSDFM_H */
