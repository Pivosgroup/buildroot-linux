LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

$(shell cd $(LOCAL_PATH) && { \
for f in *.bin; do \
  md5sum "$$f" > "$$f".checksum; \
done;})

audio_firmware_files := $(wildcard $(LOCAL_PATH)/*.bin)

audio_firmware_files += $(wildcard $(LOCAL_PATH)/*.checksum)


LOCAL_MODULE := audio_firmware
LOCAL_MODULE_TAGS := optional

LOCAL_REQUIRED_MODULES := $(audio_firmware_files)

audio_firmware: $(audio_firmware_files) | $(ACP)
	$(hide) mkdir -p $(TARGET_OUT_ETC)/firmware/
	$(hide) $(ACP) -fp $(audio_firmware_files) $(TARGET_OUT_ETC)/firmware/

$(error build from this directory disabled)
include $(BUILD_PHONY_PACKAGE)

