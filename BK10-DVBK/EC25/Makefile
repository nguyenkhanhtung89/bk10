#--------------------------------------------------------------
#               Quectel OpenLinux
#--------------------------------------------------------------
QL_SDK_PATH   ?= $(shell pwd)/../..

QL_EXP_TARGETS = bkgps
QL_EXP_LDLIBS  = -lpthread -lql_lib_audio -lcurl -lcrypto -lssl

#--------------------------------------------------------
# TOOLS BASIC CONFIG
# Note: No Need to change them
#--------------------------------------------------------
CPPFLAGS += -I./                     \
            -I./inc                  \
            -I../../include          \
            -I$(QL_SDK_PATH)/lib/interface/inc        \
            -I$(SDKTARGETSYSROOT)/usr/include               \
            -I$(SDKTARGETSYSROOT)/usr/include               \
            -I$(SDKTARGETSYSROOT)/usr/include/data          \
            -I$(SDKTARGETSYSROOT)/usr/include/dsutils       \
            -I$(SDKTARGETSYSROOT)/usr/include/qmi           \
            -I$(SDKTARGETSYSROOT)/usr/include/qmi-framework \
            -I$(SDKTARGETSYSROOT)/usr/include/ql-manager    \
			-I$(SDKTARGETSYSROOT)/usr/include/libxml2       \

LDFLAGS += -L./                          \
           ${QL_EXP_LDLIBS}              \
           -L$(SDKTARGETSYSROOT)/usr/lib \
           -L$(QL_SDK_PATH)/lib          \
           -L$(QL_SDK_PATH)/lib -lrt ${QL_EXP_LDLIBS}         \
           -lpthread                                          \
           -lxml2

STD_LIB= $(SDKTARGETSYSROOT)/usr/lib/libdsi_netctrl.so     \
         $(SDKTARGETSYSROOT)/usr/lib/libdsutils.so         \
         $(SDKTARGETSYSROOT)/usr/lib/libqmiservices.so     \
         $(SDKTARGETSYSROOT)/usr/lib/libqmi_cci.so         \
         $(SDKTARGETSYSROOT)/usr/lib/libqmi_common_so.so   \
         $(SDKTARGETSYSROOT)/usr/lib/libqmi.so             \
         $(SDKTARGETSYSROOT)/usr/lib/libmcm.so             \
         $(SDKTARGETSYSROOT)/usr/lib/libql_sys_log.so      \
         $(SDKTARGETSYSROOT)/usr/lib/libsqlite3.so      	\
         $(SDKTARGETSYSROOT)/usr/lib/libql_utils.so        \
         $(SDKTARGETSYSROOT)/usr/lib/libql_mgmt_client.so

DUAL_LIB=$(QL_SDK_PATH)/lib/libql_common_api.a
SINGLE_LIB=-lql_mgmt_client -lql_utils

USR_LIB=$(QL_SDK_PATH)/lib/libql_common_api.a		\
		$(QL_SDK_PATH)/lib/libql_peripheral.a		\

#---------------------
# Source code files
#---------------------
LOCAL_SRC_FILES = main.c network.c gps.c mcu.c log.c queue.c database.c setting.c utils.c vibrate.c lis3dsh.c lis3dh_reg.c rfid.c driver.c serial.c atc.c audio.c fw.c store.c camera.c devconfig.c aes.c encrypt.c filter.c kalman.c

bkgps: 
#	$(CC) $(CPPFLAGS) $(CFLAGS) $(LOCAL_SRC_FILES)
	$(COMPILE.c) $(CPPFLAGS) $(LDFLAGS) $(LOCAL_SRC_FILES)
	$(LINK.o) *.o -lm  $(LDFLAGS) $(USR_LIB) $(STD_LIB) -o $@


all: $(QL_EXP_TARGETS)
.PHPNY: all

clean:
	rm -rf $(QL_EXP_TARGETS) *.o

.PHONY:checkmake
checkmake:  
	@echo -e "CURDIR =		\n	${CURDIR}"  
	@echo -e "\nMAKE_VERSION =	\n	${MAKE_VERSION}"  
	@echo -e "\nMAKEFILE_LIST =	\n	${MAKEFILE_LIST}"  
	@echo -e "\nCOMPILE.c =		\n	${COMPILE.c}"
	@echo -e "\nCOMPILE.cc =	\n	${COMPILE.cc}"
	@echo -e "\nCOMPILE.cpp =	\n	${COMPILE.cpp}"
	@echo -e "\nLINK.cc =		\n	${LINK.cc}"
	@echo -e "\nLINK.o =		\n	${LINK.o}"
	@echo -e "\nCPPFLAGS =		\n	${CPPFLAGS}"
	@echo -e "\nCFLAGS =		\n	${CFLAGS}"
	@echo -e "\nCXXFLAGS =		\n	${CXXFLAGS}"
	@echo -e "\nLDFLAGS =		\n	${LDFLAGS}"
	@echo -e "\nLDLIBS =		\n	${LDLIBS}"
