
all: output.hex

clean:
	rm patch.a51.hex output.hex

patch.a51.hex: patch.a51
	as31 patch.a51

output.hex: patch.a51.hex
	srec_cat CAL_05.bin -binary -offset 0x0000 patch.a51.hex -Intel -multiple -o output.hex -Intel -address-length=2
