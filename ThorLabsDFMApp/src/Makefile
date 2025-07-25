TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================

DBD += ThorLabsDFMSupport.dbd
PROD_NAME = ThorLabsDFMApp

LIBRARY_IOC += ThorLabsDFM
PROD_IOC  += $(PROD_NAME)

LIB_SRCS += ThorLabsDFM.cpp

LIB_LIBS           += asyn 
LIB_LIBS           += $(EPICS_BASE_IOC_LIBS)

DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += base.dbd
$(PROD_NAME)_DBD += asyn.dbd
$(PROD_NAME)_DBD += asSupport.dbd
$(PROD_NAME)_DBD += ThorLabsDFMSupport.dbd

## <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
$(PROD_NAME)_SRCS   += $(PROD_NAME)_registerRecordDeviceDriver.cpp
$(PROD_NAME)_SRCS   += $(PROD_NAME)Main.cpp
$(PROD_NAME)_LIBS   += ThorLabsDFM
$(PROD_NAME)_LIBS   += TLDFMX_64
$(PROD_NAME)_LIBS   += TLDFM_64
$(PROD_NAME)_LIBS   += asyn 
$(PROD_NAME)_LIBS   += autosave
$(PROD_NAME)_LIBS   += $(EPICS_BASE_IOC_LIBS)


PROD_IOC            += TLDFM_C_Sample
TLDFM_C_Sample_SRCS += TLDFM_C_Sample.c
TLDFM_C_Sample_LIBS += TLDFM_64

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
