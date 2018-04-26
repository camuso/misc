
#ifndef PRDBG_H
#define PRDBG_H

#define __PRDBG

#ifdef __PRDBG

#define DBGPFX "* "__FILE__"::"

#define prdbg(fmt, ...) \
do { \
       printk(KERN_INFO "** %s():%d " fmt, __func__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define prcaller() \
do { \
       printk(KERN_INFO DBGPFX "Caller: %pS\n", __builtin_return_address(0)); \
} while (0)

#define prentry() \
do { \
	printk(KERN_INFO "\n"); \
	printk(KERN_INFO DBGPFX "%s():%d Caller %pS\n", \
			__func__, __LINE__, __builtin_return_address(0)); \
} while (0)

#else	// ifndef __PRDBG

#define prdbg(fmt, ...)
#define prcaller()
#define prentry()

#endif	// ifdef __PRDBG

#endif	// ifdef PRDBG_H
