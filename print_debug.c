bool print_debug(char *fmt, ...)
{
	va_list args;

	static int count = 0;
	static unsigned long jifftime;
	static bool throt_msg_printed = false;
	static bool firstpass = true;

	if (unlikely(firstpass)) {
		jifftime = jiffies;
		firstpass = false;
	}

	if (count < kcs_dbg_throt_thold) {
		va_start(args, fmt);
		printk(KERN_DEBUG fmt, args);
		va_end(args);
	} else {
		if(!throt_msg_printed) {
			printk("\nKCS_DBG: Throttling debug messages...\n");
			throt_msg_printed = true;
		}
		if (jiffies_to_msecs(jiffies - jifftime) >
		    (kcs_dbg_throt_wait * 1000)) {
			count = 0;
			jifftime = jiffies;
			throt_msg_printed = false;
		}
	}

	return throt_msg_printed;
}


