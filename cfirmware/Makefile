TARGET = firmware

SRC_DIR = src
BUILD_DIR = build

CC = sdcc

CFLAGS = -mmcs51 --model-medium --std=c99 --debug --xram-loc 0x0000 --xram-size 0x8000

SRCS = $(wildcard $(SRC_DIR)/*.c)

RELFILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.rel,$(SRCS))

IHX = $(BUILD_DIR)/$(TARGET).ihx
HEX = $(BUILD_DIR)/$(TARGET).hex

all: $(HEX)

# Create build dir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C -> .rel
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(IHX): $(RELFILES)
	$(CC) $(CFLAGS) -o $@ $(RELFILES)

# Convert IHX -> HEX
$(HEX): $(IHX)
	packihx $< > $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean