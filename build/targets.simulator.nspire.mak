$(BUILD_DIR)/%.tns: $(BUILD_DIR)/%.elf
# comment one of these lines. For B&W old nspire, do not use the compress option
	genzehn --compress --input  $(BUILD_DIR)/epsilon.elf --output gamma.tns --name "Gamma"  --uses-lcd-blit true
#	genzehn --input  $(BUILD_DIR)/epsilon.elf --output gamma.tns --name "Gamma"  --uses-lcd-blit true
	genzehn --info --input gamma.tns
	firebird-send gamma.tns /ndless
