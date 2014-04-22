struct bootparams {
	unsigned char gpio_ddr[16];
	unsigned char gpio_use[16];
	unsigned int clock_freq;
	unsigned int ram_end;
	unsigned char *command_line;
};

