LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include $(LOCAL_PATH)/../common.mk
LOCAL_SRC_FILES := $(FFFILES)
LOCAL_C_INCLUDES :=		\
	$(LOCAL_PATH)		\
	$(LOCAL_PATH)/..	\
	external/zlib
LOCAL_CFLAGS += $(FFCFLAGS)
LOCAL_MODULE := $(FFNAME)
LOCAL_MODULE_TAGS := user eng debug
include $(BUILD_STATIC_LIBRARY)
# Reset CC as it's overwritten by common.mk
CC := $(HOST_CC)
