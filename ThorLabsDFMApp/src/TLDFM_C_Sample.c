/******************************************************************************

   Thorlabs TLDFM Instrument Driver Sample Application

   Source file    TLDFM_C_Sample.c

   Date:          Jun-07-2017
   Software-Nr:   091103
   Version:       3.0
   Copyright:     Copyright(c) 2017, Thorlabs GmbH (www.thorlabs.com)
   Author(s):     Christian Westphal

   Disclaimer:

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


   NOTE: You may need to set your compiler include search path to the
         VXIPNP include directory.
         This is typically 'C:\VXIPNP\WinNT\include' or
         'C:\Program Files\IVI Foundation\VISA\WinNT\include'

******************************************************************************/

/*=============================================================================
 Include Files
=============================================================================*/
#include "TLDFM.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*=============================================================================
 Type Definitions
=============================================================================*/
typedef struct
{
	ViUInt8 firstHighByte;
	ViUInt8 firstLowByte;
	ViUInt8 secondHighByte;
	ViUInt8 secondLowByte;
} SAMPLE_dword_converter_t;

typedef union
{
	SAMPLE_dword_converter_t byteRepresentation;
	ViUInt32                 dwordRepresentation;
} SAMPLE_dword_t;


/*=============================================================================
 Macros
=============================================================================*/
#define  MAX_SEGMENTS            (40)

/*=============================================================================
 Callbacks
=============================================================================*/
ViStatus _VI_FUNCH StatusCallback(ViChar[]);


/*=============================================================================
 Prototypes
=============================================================================*/
void error_exit(ViSession handle, ViStatus err);
void waitKeypress(void);
ViStatus select_instrument(ViChar** resource);
ViStatus get_device_id(ViSession handle);


/*=============================================================================
 Functions
=============================================================================*/
int main(int argc, char** argv)
{
	ViStatus  err;
	ViChar    *rscPtr;
	ViSession instrHdl = VI_NULL;

	ViUInt32  i, Segments, TiltElements;
	ViReal64  VoltageMirrorMin, VoltageMirrorMax, VoltageMirrorCommon;
	ViReal64  VoltageTiltMin, VoltageTiltMax, VoltageTiltCommon;
	ViReal64  SegmentVoltages[MAX_SEGMENTS];
	ViReal64  TiltAmplitude, TiltAngle;
	ViReal64  FeedbackVoltage, FeedbackCurrent, MonitorVoltage;
	ViReal64  SegmentVoltage;

	
	

	printf("-----------------------------------------------------------\n");
	printf(" Thorlabs TLDFM instrument driver sample \n");
	printf("-----------------------------------------------------------\n\n");

	
	// Parameter checking / resource scanning
	if(argc < 2)
	{
		// Find resources
		err = select_instrument(&rscPtr);
		if(VI_SUCCESS != err)
		{
			error_exit(instrHdl, err);  // something went wrong
		}
		if(!rscPtr)
		{
			exit(EXIT_SUCCESS);     // none found
		}
	}
	else
	{
		// Got resource in command line
		rscPtr = argv[1];
	}
	
	
	

	// Open session to instrument
	printf("Opening session to '%s' ...\n", rscPtr);
	waitKeypress();
	err = TLDFM_init(rscPtr, VI_TRUE, VI_TRUE, &instrHdl);
	if(err) error_exit(instrHdl, err);  // can not open session to instrument
	
	
	

	// Manual Status Update
	fprintf(stderr, "\nRequest Status\n");
	ViStatus stat = TL_ERROR_SYSTEM_ERROR;
	ViChar   errMsg[TLDFM_ERR_DESCR_BUFER_SIZE];
	err = TLDFM_error_query(instrHdl, &stat, errMsg);
	if(err) error_exit(instrHdl, err);
	fprintf(stderr, "STATUS: %s\n\n", errMsg);
	
	// Automatic Status Update
	// Install Push Notifications Handler
	//printf("\nInstall push notifications handler.\n\n");
	//waitKeypress();
	//err = TLDFM_add_status_delegate(instrHdl, StatusCallback);
	//if(err) error_exit(instrHdl, err);

	// Enable Device Push Notifications
	//printf("\nEnable device push notifications.\n");
	//waitKeypress();
	//err = TLDFM_enable_event(instrHdl);
	//if(err) error_exit(instrHdl, err);




	// Get instrument info
	err = get_device_id(instrHdl);
	if(err) error_exit(instrHdl, err);




	// Get device configuration and voltage limits
	err = TLDFM_get_device_configuration(instrHdl,
										 &Segments,
										 &VoltageMirrorMin,
										 &VoltageMirrorMax,
										 &VoltageMirrorCommon,
										 &TiltElements,
										 &VoltageTiltMin,
										 &VoltageTiltMax,
										 &VoltageTiltCommon);
	if(err) error_exit(instrHdl, err);
	printf("\nNumber of Segments        :  %2d\n", Segments);
	printf("Min. Voltage              :  %8.3f\n", VoltageMirrorMin);
	printf("Max. Voltage              :  %8.3f\n", VoltageMirrorMax);
	printf("\n");
	printf("No. of Tip/Tilt Elements  :  %2d\n", TiltElements);
	printf("Min. Voltage              :  %8.3f\n", VoltageTiltMin);
	printf("Max. Voltage              :  %8.3f\n", VoltageMirrorMax);




	// Switch off hysteresis compensation for both mirror and tip/tilt
	err = TLDFM_enable_hysteresis_compensation(instrHdl, T_MIRROR, VI_FALSE);
	if(err) error_exit(instrHdl, err);
	if(0 != TiltElements)
	{
		err = TLDFM_enable_hysteresis_compensation(instrHdl, T_TILT, VI_FALSE);
		if(err) error_exit(instrHdl, err);
	}




	// Single Segment Control
	printf("\nNow a single segment will be set to an arbitrary voltage:\n");
	ViUInt32 segmentID = 3;
	ViReal64 voltageSetPoint = 70.0;
	printf("Segment %2d\nSet-Point: %8.3fV\n", segmentID, voltageSetPoint);
	waitKeypress();

	err = TLDFM_set_segment_voltage(instrHdl, segmentID, voltageSetPoint);
	if(err) error_exit(instrHdl, err);
	err = TLDFM_get_segment_voltage(instrHdl, segmentID, &SegmentVoltage);
	if(err) error_exit(instrHdl, err);
	printf("Actual: %8.3fV\n", SegmentVoltage);




	// Reset Device: All Voltages = 0.0V; Hysteresis = On
	printf("\n\nNow the device is reset to the user save state.\n");
	waitKeypress();
	err = TLDFM_reset(instrHdl);
	if(err) error_exit(instrHdl, err);




	// Mirror Control
	printf("\nNow all segments will be set to different voltages:\n");
	for(i = 0; i < Segments; i++)
	{
		SegmentVoltages[i] = VoltageMirrorMin +
							 (ViReal64)i *
							 (VoltageMirrorMax - VoltageMirrorMin) / (ViReal64)(Segments - 1);
	}
	waitKeypress();

	err = TLDFM_set_segment_voltages(instrHdl, SegmentVoltages);
	if(err) error_exit(instrHdl, err);

	printf("Voltages Set-Points:\n");
	for(i = 0; i < Segments; i++)
	{
		printf("Segment[%2d] = %8.3fV\n", i, SegmentVoltages[i]);
	}




	if(0 != TiltElements)
	{
		// Adjust Mirror Tip/Tilt
		printf("\nNow tip/tilt will be controlled:\n");
		TiltAmplitude = 1.0; // max.
		printf("TiltAmplitude = %8.3f\n", TiltAmplitude);
		waitKeypress();

		for(TiltAngle = -180.0; TiltAngle <= 180.0; TiltAngle += 30.0)
		{
			err = TLDFM_set_tilt_amplitude_angle (instrHdl, TiltAmplitude, TiltAngle);
			if(err) error_exit(instrHdl, err);

			printf("TiltAngle     = %8.3fdeg\n", TiltAngle);
		}
	}




	// Read Feedback Input
	printf("\nNow the analogue feedback signals are read:\n");
	waitKeypress();

	err = TLDFM_get_feedback (instrHdl, &FeedbackVoltage, &FeedbackCurrent);
	if(err) error_exit(instrHdl, err);
	printf("Feedback Voltage: %8.3f V\n", FeedbackVoltage);
	printf("Feedback Current: %8.3f uA\n", FeedbackCurrent);

	err = TLDFM_get_monitor_voltage(instrHdl, &MonitorVoltage);
	if(err) error_exit(instrHdl, err);
	printf("Monitor Voltage: %8.3f V\n", MonitorVoltage);




	// Reset Device Again
	printf("\nNow the device is reset again.\n");
	waitKeypress();
	err = TLDFM_reset(instrHdl);
	if(err) error_exit(instrHdl, err);




	// Disable Device Push Notifications
	//printf("\nDisable device push notifications.\n");
	//waitKeypress();
	//err = TLDFM_disable_event(instrHdl);
	//if(err) error_exit(instrHdl, err);

	// Close Status Push Notifications
	//printf("\nRemove push notifications handler.\n");
	//waitKeypress();
	//err = TLDFM_add_status_delegate(instrHdl, VI_NULL);
	//if(err) error_exit(instrHdl, err);




	// Close Session and Terminate Program
	printf("\nThis demo session will be closed now.\n");
	waitKeypress();
	err = TLDFM_close (instrHdl);

	return (EXIT_SUCCESS);
}


/*-----------------------------------------------------------------------------
 Read out device ID and print it to screen
-----------------------------------------------------------------------------*/
ViStatus get_device_id(ViSession instrHdl)
{
	ViStatus err;
	ViChar   manufNameBuf[TLDFM_BUFFER_SIZE];
	ViChar   instrNameBuf[TLDFM_MAX_INSTR_NAME_LENGTH];
	ViChar   snBuf[TLDFM_MAX_SN_LENGTH];
	ViChar   drvRevBuf[TLDFM_MAX_STRING_LENGTH];
	ViChar   fwRevBuf[TLDFM_MAX_STRING_LENGTH];


	err = TLDFM_get_manufacturer_name (instrHdl, manufNameBuf);
	if(err) return(err);
	printf("Manufacturer:  %s\n", manufNameBuf);

	err = TLDFM_get_instrument_name (instrHdl, instrNameBuf);
	if(err) return(err);
	printf("Instrument:    %s\n", instrNameBuf);

	err = TLDFM_get_serial_Number (instrHdl, snBuf);
	if(err) return(err);
	printf("Serial number: %s\n", snBuf);

	err = TLDFM_revision_query (instrHdl, drvRevBuf, fwRevBuf);
	if(err) return(err);
	printf("Driver:        %s\n", drvRevBuf);
	printf("Firmware:      %s\n\n", fwRevBuf);

	return(VI_SUCCESS);
}


/*-----------------------------------------------------------------------------
 Read out device ID and print it to screen
-----------------------------------------------------------------------------*/
ViStatus select_instrument(ViChar **resource)
{
	ViStatus err;
	ViUInt32 deviceCount = 0;
	int      choice = 0;

	ViChar    manufacturer[TLDFM_BUFFER_SIZE];
	ViChar    instrumentName[TLDFM_MAX_INSTR_NAME_LENGTH];
	ViChar    serialNumber[TLDFM_MAX_SN_LENGTH];
	ViBoolean deviceAvailable;
	ViChar    resourceName[TLDFM_BUFFER_SIZE];

	printf("Scanning for instruments...\n");
	err = TLDFM_get_device_count(VI_NULL, &deviceCount);
	if(TL_ERROR_RSRC_NFOUND == err)
	{
		return err;
	}
	else if(0 == deviceCount)
	{
		printf("No matching instruments found\n\n");
		return err;
	}

	printf("Found %d matching instrument(s):\n\n", deviceCount);

	for(ViUInt32 i = 0; i < deviceCount;)
	{
		err = TLDFM_get_device_information(VI_NULL,
										   i,
										   manufacturer,
										   instrumentName,
										   serialNumber,
										   &deviceAvailable,
										   resourceName);

		printf("%d:\t%s\t%s\tS/N:%s\t%s\n",
			   ++i,
			   manufacturer,
			   instrumentName,
			   serialNumber,
			   deviceAvailable ? "available" : "locked");
	}

	if(2 <= deviceCount)
	{
		ViBoolean deviceSelected = VI_FALSE;
		printf("\nPlease choose: ");
		do
		{
			do
			{
				choice = getchar();
			}
			while(EOF == choice);

			if((0 >= choice) || (deviceCount < choice))
			{
				printf("Invalid choice\n\n");
				choice = EOF;
			}
			else
			{
				deviceSelected = VI_TRUE;
			}
		}
		while(!deviceSelected);
	}
	else
	{
		choice = 1;
		printf("\nPress any key to continue");
		waitKeypress();
	}

	err = TLDFM_get_device_information(VI_NULL,
									   (ViUInt32)(choice - 1),
									   manufacturer,
									   instrumentName,
									   serialNumber,
									   &deviceAvailable,
									   resourceName);

	if(VI_SUCCESS == err)
	{
		*resource = malloc(TLDFM_BUFFER_SIZE);
		strncpy(*resource, resourceName, TLDFM_BUFFER_SIZE);
	}
	return err;
}

/*-----------------------------------------------------------------------------
  Exit with error message
-----------------------------------------------------------------------------*/
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
	waitKeypress();

	exit (EXIT_FAILURE);
}


/*-----------------------------------------------------------------------------
  Print keypress message and wait
-----------------------------------------------------------------------------*/
void waitKeypress(void)
{
	printf("Press <ENTER> to continue\n");
	while(EOF == getchar());
}


/*-----------------------------------------------------------------------------
  Receive device push notifications
-----------------------------------------------------------------------------*/
ViStatus _VI_FUNCH StatusCallback(ViChar statusMessage[])
{
	fprintf(stderr, "\nSTATUS: %s\n", statusMessage);
	return VI_SUCCESS;
}


/******************************************************************************

  End of Source file

******************************************************************************/

