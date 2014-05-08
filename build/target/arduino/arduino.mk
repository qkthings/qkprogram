###############################################################################
# qkthings
###############################################################################

include $(TARGET_GLOBAL)

###############################################################################
# SOURCE
###############################################################################
INCLUDE_DIR += \
$(QKPROGRAM_DIR)/lib/hal/arduino

C_SRC_DIR += \
$(QKPROGRAM_DIR)/lib/hal/arduino

FEATURES = \
_QK_FEAT_POWER_MANAGEMENT \
_QK_FEAT_FRAGMENTATION \
_QK_FEAT_EEPROM

