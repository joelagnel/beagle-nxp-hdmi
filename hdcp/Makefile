

############### select your chip + platform here ###################

# TDA_TX := TDA19989
TDA_TX := TDA19988
# TDA_TX := TDA9984

####################################################################

EXTRA_CFLAGS += -DFUNC_PTR=" " -DCONST_DAT="const " -DRAM_DAT=" "
EXTRA_CFLAGS += -DTDA_NAME=$(TDA_TX)
EXTRA_CFLAGS += -DTMFL_LINUX_OS_KERNEL_DRIVER
EXTRA_CFLAGS += -DTMFL_NO_RTOS
#EXTRA_CFLAGS += -DIRQ
EXTRA_CFLAGS += -DUSE_SHARED_HDMI_INSTANCE

ifeq ($(TDA_TX),TDA19988)
EXTRA_CFLAGS += -DTMFL_TDA19988
TDA_TX       := TDA19989
endif

ifeq ($(TDA_TX),TDA19989)
EXTRA_CFLAGS += -DTMFL_TDA19989 -DTMFL_TDA9989 -DTMFL_HDCP_SUPPORT
endif

########## board service layer #####################################

ifeq ($(TDA_TX),TDA19989)
BSL := comps/tmbslTDA9989/src
endif

ifeq ($(TDA_TX),TDA9984)
BSL := comps/tmbslTDA9984/src
endif

####################################################################

obj-m += nwolc.o

# Linux module
nwolc-objs := tda998x_hdcp.o

# NXP comps
ifeq ($(TDA_TX),TDA19989)
nwolc-objs += $(BSL)/tmbslTDA9989_HDCP.o
endif

ifeq ($(TDA_TX),TDA9984)
nwolc-objs += $(BSL)/tmbslTDA9984_HDCP.o
endif

####################################################################

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(obj)/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslHdmiTx/inc/

ifeq ($(TDA_TX),TDA19989)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9989/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9989/src
endif

ifeq ($(TDA_TX),TDA9984)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9984/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9984/src
endif



