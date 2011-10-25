LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

$(shell cd $(LOCAL_PATH) && { \
for f in *.bin; do \
  md5sum "$$f" > "$$f".checksum; \
done;})

copy_from := $(wildcard $(LOCAL_PATH)/*.bin)

copy_from += $(wildcard $(LOCAL_PATH)/*.checksum)

install_pairs := $(foreach f,$(copy_from),$(f):system/etc/firmware/$(notdir $(f)))

$(error $(install_pairs))

PRODUCT_COPY_FILES += $(install_pairs)


