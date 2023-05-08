################################################################################
#
# uvc-gadget package
#
################################################################################

UVC_GADGET_VERSION = 1.0
UVC_GADGET_SITE = $(BR2_EXTERNAL_RASPI4_MINM_ATTACK_PATH)/package/uvc-gadget/src
UVC_GADGET_SITE_METHOD = local
UVC_GADGET_DEPENDENCIES = opencv4 jpeg

define UVC_GADGET_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_LD)" -C $(@D)
endef

define UVC_GADGET_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/uvc-gadget  $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

