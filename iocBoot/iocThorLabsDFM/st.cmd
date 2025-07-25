< envPaths

dbLoadDatabase("$(THORLABSDFM)/dbd/ThorLabsDFMApp.dbd")
ThorLabsDFMApp_registerRecordDeviceDriver(pdbbase)

# Prefix for all records
epicsEnvSet("PREFIX", "TLDFM1:")
# The port name for the detector
epicsEnvSet("PORT", "TLDFM11")
epicsEnvSet("RESOURCE_NAME", "")

ThorLabsDFMConfig("$(PORT)", "")

dbLoadTemplate("./ThorLabsDFM.substitutions", "P=$(PREFIX),R=DFM1:,PORT=$(PORT)")
save_restoreSet_IncompleteSetsOk(1)
save_restoreSet_DatedBackupFiles(1)
save_restoreSet_NumSeqFiles(3)
save_restoreSet_SeqPeriodInSeconds(300)
set_savefile_path(".", "autosave")
set_pass0_restoreFile("auto_settings.sav")
set_pass1_restoreFile("auto_settings.sav")
set_requestfile_path(".")
set_requestfile_path("$(THORLABSDFM)/db")

asynSetTraceIOMask($(PORT), 0, ESCAPE)
#asynSetTraceMask($(PORT), 0, ERROR|DRIVER)


iocInit

### Start up the autosave task and tell it what to do.
# Save settings every thirty seconds
create_monitor_set("auto_settings.req", 30, "P=$(PREFIX), R=DFM1:")

# Wait 1 seconds for all records with PINI=YES to process
epicsThreadSleep(1)
