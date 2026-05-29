TARGET = firmware

SRC_DIR = src
BUILD_DIR = build

CC = sdcc
CFLAGS = -mmcs51 --model-medium --std=c99 --debug --xram-loc 0x0000 --xram-size 0x2000 --code-loc 0x0000
AS = sdas8051
ASFLAGS = -l

C_SRCS = $(wildcard $(SRC_DIR)/*.c)
AS_SRCS = $(wildcard $(SRC_DIR)/*.a51)


RELFILES  = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.rel,$(C_SRCS))
RELFILES += $(patsubst $(SRC_DIR)/%.a51,$(BUILD_DIR)/%.rel,$(AS_SRCS))

IHX = $(BUILD_DIR)/$(TARGET).ihx
HEX = $(BUILD_DIR)/$(TARGET).hex

all: $(HEX) report

# Create build dir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C -> .rel
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.a51 | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# Link
$(IHX): $(RELFILES)
	$(CC) $(CFLAGS) -o $@ $(RELFILES)

# Convert IHX -> HEX
$(HEX): $(IHX)
	packihx $< > $@

report: $(HEX)
	cat $(BUILD_DIR)/$(TARGET).mem

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean report