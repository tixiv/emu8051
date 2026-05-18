
BUILD_DIR = build

C_FIRMWARE_HEX=../cfirmware/build/firmware.hex

ORIG_HEX=$(BUILD_DIR)/orig.hex
PATCHED_HEX=$(BUILD_DIR)/patched.hex
BOTH_HEX=$(BUILD_DIR)/both.hex

PATCH=$(BUILD_DIR)/patch.a51.hex
STARTUP_PATCH=$(BUILD_DIR)/startup_patch.a51.hex

all: $(ORIG_HEX) $(PATCHED_HEX) $(BOTH_HEX)

clean:
	rm $(BUILD_DIR)/*

$(BUILD_DIR)/%.a51.hex: %.a51
	as31 -O$@ $<

$(BOTH_HEX): $(PATCH) $(STARTUP_PATCH) $(C_FIRMWARE_HEX)
	srec_cat CAL_05.bin -binary -offset 0x0000 \
	         $(STARTUP_PATCH) -Intel -multiple \
	         $(PATCH) -Intel -multiple \
			 $(C_FIRMWARE_HEX) -Intel -exclude 0xF000 0xF057 -multiple \
	         -o $(BOTH_HEX) -Intel -address-length=2

$(PATCHED_HEX): $(PATCH)
	srec_cat CAL_05.bin -binary -offset 0x0000 \
	         $(PATCH) -Intel -multiple \
	         -o $@ -Intel -address-length=2

$(ORIG_HEX):
	srec_cat CAL_05.bin -binary -offset 0x0000 \
	         -o $@ -Intel -address-length=2
