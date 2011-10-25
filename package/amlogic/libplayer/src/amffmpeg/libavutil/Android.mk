LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include $(LOCAL_PATH)/../common.mk
LOCAL_SRC_FILES := $(FFFILES)
LOCAL_C_INCLUDES :=		\
	$(LOCAL_PATH)		\
	$(LOCAL_PATH)/..
LOCAL_CFLAGS += $(FFCFLAGS)
LOCAL_MODULE := $(FFNAME)
LOCAL_MODULE_TAGS := user eng debug
# Reset CC as it's overwritten by common.mk
CC := $(HOST_CC)
include $(BUILD_STATIC_LIBRARY)
