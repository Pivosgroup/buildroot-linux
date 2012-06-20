
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(BOARD_VOUT_USES_FREESCALE),false)
LOCAL_CFLAGS += -DENABLE_FREE_SCALE
endif

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) 												

LOCAL_SRC_FILES +=system/android.c system/systemsetting.c

$(shell cd $(LOCAL_PATH);touch version.c)
LIBPLAYER_GIT_VERSION="$(shell cd $(LOCAL_PATH);git log | grep commit -m 1 | cut -d' ' -f 2)"
LIBPLAYER_GIT_UNCOMMIT_FILE_NUM=$(shell cd $(LOCAL_PATH);git diff | grep +++ -c)
LIBPLAYER_LAST_CHANGED="$(shell cd $(LOCAL_PATH);git log | grep Date -m 1)"
LIBPLAYER_BUILD_TIME=" $(shell date)"
LIBPLAYER_BUILD_NAME=" $(shell echo ${LOGNAME})"

LOCAL_CFLAGS+=-DHAVE_VERSION_INFO
LOCAL_CFLAGS+=-DLIBPLAYER_GIT_VERSION=\"${LIBPLAYER_GIT_VERSION}${LIBPLAYER_GIT_DIRTY}\"
LOCAL_CFLAGS+=-DLIBPLAYER_LAST_CHANGED=\"${LIBPLAYER_LAST_CHANGED}\"
LOCAL_CFLAGS+=-DLIBPLAYER_BUILD_TIME=\"${LIBPLAYER_BUILD_TIME}\"
LOCAL_CFLAGS+=-DLIBPLAYER_BUILD_NAME=\"${LIBPLAYER_BUILD_NAME}\"
LOCAL_CFLAGS+=-DLIBPLAYER_GIT_UNCOMMIT_FILE_NUM=${LIBPLAYER_GIT_UNCOMMIT_FILE_NUM}

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../../amcodec/include \
	$(LOCAL_PATH)/../../amadec/include \
	$(LOCAL_PATH)/../../amffmpeg

LOCAL_MODULE := libamplayer

LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(BOARD_VOUT_USES_FREESCALE),false)
LOCAL_CFLAGS += -DENABLE_FREE_SCALE
endif

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) 									

LOCAL_SRC_FILES +=system/android.c system/systemsetting.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/../../amcodec/include \
        $(LOCAL_PATH)/../../amadec/include \
        $(LOCAL_PATH)/../../amffmpeg

LOCAL_CFLAGS+=-DHAVE_VERSION_INFO
LOCAL_CFLAGS+=-DLIBPLAYER_GIT_VERSION=\"${LIBPLAYER_GIT_VERSION}${LIBPLAYER_GIT_DIRTY}\"
LOCAL_CFLAGS+=-DLIBPLAYER_LAST_CHANGED=\"${LIBPLAYER_LAST_CHANGED}\"
LOCAL_CFLAGS+=-DLIBPLAYER_BUILD_TIME=\"${LIBPLAYER_BUILD_TIME}\"
LOCAL_CFLAGS+=-DLIBPLAYER_BUILD_NAME=\"${LIBPLAYER_BUILD_NAME}\"
LOCAL_CFLAGS+=-DLIBPLAYER_GIT_UNCOMMIT_FILE_NUM=${LIBPLAYER_GIT_UNCOMMIT_FILE_NUM}

LOCAL_STATIC_LIBRARIES := libamcodec libavformat libswscale libavcodec libavutil libamadec
LOCAL_SHARED_LIBRARIES += libutils libmedia libz libbinder libdl libcutils libc libamavutils

LOCAL_MODULE := libamplayer
LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

##$(shell cd $(LOCAL_PATH);rm ${VERSION_FILE_NAME})

