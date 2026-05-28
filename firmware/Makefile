
BUILD_DIR = build

C_FIRMWARE_HEX=../cfirmware/build/firmware.hex

PATCHED_BIN=$(BUILD_DIR)/patched.bin
BOTH_BIN=$(BUILD_DIR)/both.bin

PATCH=$(BUILD_DIR)/patch.a51.hex
STARTUP_PATCH=$(BUILD_DIR)/startup_patch.a51.hex

all: $(PATCHED_BIN) $(BOTH_BIN)

clean:
	rm $(BUILD_DIR)/*

$(BUILD_DIR)/%.a51.hex: %.a51
	as31 -O$@ $<

$(BOTH_BIN): $(C_FIRMWARE_HEX) $(STARTUP_PATCH)
	srec_cat CAL_05.bin -binary -offset 0x0000 \
	         $(STARTUP_PATCH) -Intel -multiple \
	         CAL_05.bin -binary -offset 0x10000 \
			 $(C_FIRMWARE_HEX) -Intel -offset 0x10000 -multiple \
			 $(STARTUP_PATCH) -Intel -offset 0x10000 -multiple \
	         -o $@ -binary

$(PATCHED_BIN): $(PATCH)
	srec_cat CAL_05.bin -binary -offset 0x0000 \
	         $(PATCH) -Intel -multiple \
	         -o $@ -binary
