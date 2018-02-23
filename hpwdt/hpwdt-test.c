#include <stdio.h>
#include <fcntl.h>
#include <linux/types.h>
#include "linux/ioctl.h"

#define WATCHDOG_IOCTL_BASE     'W'

struct watchdog_info {
        __u32 options;          /* Options the card/driver supports */
        __u32 firmware_version; /* Firmware version of the card */
        __u8  identity[32];     /* Identity of the board */
};

#define WDIOC_GETSUPPORT        _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_GETSTATUS         _IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define WDIOC_GETBOOTSTATUS     _IOR(WATCHDOG_IOCTL_BASE, 2, int)
#define WDIOC_GETTEMP           _IOR(WATCHDOG_IOCTL_BASE, 3, int)
#define WDIOC_SETOPTIONS        _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE         _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT        _IOR(WATCHDOG_IOCTL_BASE, 7, int)

#define WDIOF_UNKNOWN           -1      /* Unknown flag error */
#define WDIOS_UNKNOWN           -1      /* Unknown status error */

int main()
{


	int fd, refresh_count;
	static int timer;
	static unsigned long arg;
	int count = 0;
	int choice;

	fd = open("/dev/watchdog", O_WRONLY);
	if (fd == -1) {
		perror("watchdog");
		return 1;
	}

	do {
		printf("\n\nPlease choose from the menu below:\n");
		printf("1. Get the hpwdt timer value.\n");
		printf("2. Set the hpwdt timer value.\n");
		printf("3. Force the timer expiration.\n");
		printf("4. Run the watchdog timer (constant refresh).\n");
		printf("5. Quit\n");

		scanf("%d", &choice);

		switch(choice) {
			case 1: 
				if (ioctl(fd, WDIOC_GETTIMEOUT, &arg) == -1) {
					printf("unable to make ioctl call\n");
					return -1;
				}
				printf("ioctl: The current hp-wdt timer value is %d seconds.\n", arg);
				break;
			case 2: 
				printf("Enter the timeout value you desire (in seconds).\n");
				scanf("%d", &timer);
				if (ioctl(fd, WDIOC_SETTIMEOUT, &timer) == -1) {
					printf("unable to make ioctl call to keep timer alive.\n");
					return -1;
				}
				break;
			case 3:
				if (ioctl(fd, WDIOC_GETTIMEOUT, &arg) == -1) {
					printf("unable to make ioctl call\n");
					return -1;
				}

				printf("NOTE: The timer will expire %d seconds.\n", (arg));
				sleep(arg+5);
 				break;
			case 4: 
				if (ioctl(fd, WDIOC_GETTIMEOUT, &arg) == -1) {
					printf("unable to make ioctl call\n");
					return -1;
				}

				for (;;) {
					if (ioctl(fd, WDIOC_KEEPALIVE, &arg) == -1) {
						printf("unable to make ioctl call to keep timer alive.\n");
						return -1;
					}
					sleep(arg-10);
					printf("Timer reloaded to stay alive.\n");
				}
				break;
			case 5: 
				write(fd, "V", 1);
				break;
			default:
				printf("No such option: %d\n", choice);
				break;
		}

	} while (choice != 5);

	fsync(fd);
	close(fd);	
	return 0;

}   


