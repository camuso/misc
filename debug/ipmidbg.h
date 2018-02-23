#ifndef IPMIDBG_H
#define IPMIDBG_H

#include <linux/platform_device.h>

#ifndef to_platform_driver
#define to_platform_driver(drv)	(container_of((drv), struct platform_driver, \
				 driver))
#endif

// I know it's bad form to statically define objects that displace memory
// in a header file, but this is debug code, and each compilation unit
// that needs this stuff needs its own copy. This is a lot simpler than
// cutting and pasting all this cruft into the source files. It's bad
// enough that there will be debug code in the sources. This just helps
// to minimize it.

static struct _debug_mailbox {
	bool ipmi;
	const char *name;
	const struct device_driver *drv;
	struct platform_driver *pdrv;
}
dbgmbx = {
	.ipmi = false,
	.name = NULL,
	.drv = NULL,
	.pdrv = NULL
};

#ifndef DBGPFX
#define DBGPFX "* "__FILE__"::"
#endif

#define dbgmsg(fmt, ...) \
do { \
	if (dbgmbx.ipmi) \
		printk(KERN_INFO DBGPFX "%s(): " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define dbgentry() \
do { \
	if (dbgmbx.ipmi) { \
		printk(KERN_INFO "\n"); \
		printk(KERN_INFO DBGPFX "%s(): Caller %pS\n", \
				__func__, __builtin_return_address(0)); \
	} \
} while (0)

static bool isipmi(const struct device_driver *drv)
{
	dbgmbx.drv = drv;
	dbgmbx.pdrv = dbgmbx.drv  ? to_platform_driver(drv) : NULL;
	dbgmbx.name = dbgmbx.pdrv ? dbgmbx.pdrv->driver.name : "";
	dbgmbx.ipmi = dbgmbx.pdrv ? !!(strstr(dbgmbx.name, "ipmi")) : false;
	return (dbgmbx.ipmi);
}

#define clr_dbgmbx() \
do { \
	if (dbgmbx.ipmi) \
		isipmi(NULL); \
} while (0)

#endif // ifndef IPMIDBG_H
