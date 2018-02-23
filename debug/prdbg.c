
#define DBGPFX "* "PFX

#define prdbg(fmt, ...) \
do { \
       printk(KERN_INFO DBGPFX "%s(): " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define prcaller() \
do { \
       printk(KERN_INFO DBGPFX "Caller: %pS\n", __builtin_return_address(0)); \
} while (0)

#define prentry() \
do { \
	printk(KERN_INFO "\n"); \
	printk(KERN_INFO DBGPFX "%s(): Caller %pS\n", \
			__func__, __builtin_return_address(0)); \
} while (0)

