LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := controler.c bc_control.c shell_control.c
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../player/include \
	$(LOCAL_PATH)/../../amffmpeg \
	$(LOCAL_PATH)/../../amcodec/include \
	$(LOCAL_PATH)/../../amadec/include
	
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libamcontroler

include $(BUILD_STATIC_LIBRARY)

