struct bootparams {
	short size;
	unsigned char gpio_ddr[24];
	unsigned char gpio_use[24];
	unsigned int clock_freq;
	unsigned int ram_end;
	unsigned char *command_line;
} __attribute__((aligned(2), packed));

