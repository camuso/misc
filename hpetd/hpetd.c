#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Tony Camuso <tony.camuso@hp.com>");
MODULE_DESCRIPTION("Init hpetd to throw NMIs");

#define HPETD_FWREV	0x08
#define HPETD_CONTROL 	0x60
#define HPETD_CTRL_ENA  (1<<2)
#define HPETD_ERRBIT  	(1<<30)
#define HPETD_SETREG	0x8004
#define HPETD_MAXDEV	16

static void *bp;

struct dev_data {
	unsigned long	pa;	// physical address
	unsigned long	size;	// size of address space

	union {			// union of various pointer types
		u8 *b;		// : for the virtual address
		u16 *w;
		u32 *l;
		void *v;
	} va;

	struct pci_dev *pdev;	// pointer to the device
};

static struct dev_data dev_array[HPETD_MAXDEV];
static int devcount;

static int hpetd_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void hpetd_remove(struct pci_dev *dev);

static struct __devinitconst pci_device_id hpetd_ids[] = {
	{PCI_DEVICE(0x103C , 0x7007)},
	{0},
};

MODULE_DEVICE_TABLE(pci, hpetd_ids);

static struct pci_driver hpetd_driver = {
	.name = "hpetd",
	.id_table = hpetd_ids,
	.probe = hpetd_probe,
	.remove = hpetd_remove,
};

static int __init hpetd_init(void)
{
	int status = 0;

	printk(KERN_INFO "hpetd: loading.\n");

	if((status = pci_register_driver(&hpetd_driver)) != 0) {
		printk(KERN_ALERT "hpetd: kmod could not register.\n");
		goto init_exit;
	}
 
init_exit:
	return status;
}

static void __exit hpetd_exit(void)
{
	int index;

	for(index = 0; index < devcount; index++) {
		printk(KERN_INFO "hpetd[%d]: unloading.\n", index);
		iounmap(dev_array[index].va.v);
	}

	pci_unregister_driver(&hpetd_driver);
}


static int hpetd_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int rc = 0;
	u8  reg8;
	unsigned long base_pa;
	unsigned long end_pa;
	unsigned long size;
	unsigned long reg32;

	printk(KERN_INFO "hpetd[%d]: probing.\n", devcount);
	if((rc = pci_enable_device(pdev)) != 0) {
		printk(KERN_INFO "hpetd[%d]: Cannot enable device\n.", devcount);
		goto probe_exit;
	}

	pci_read_config_byte(pdev, HPETD_FWREV, &reg8);
	printk("hpetd[%d]: fw rev cfg reg %02x = %02x\n",
		devcount, HPETD_FWREV, reg8);

	// make sure bit 2 of the control register is set
	//
	pci_read_config_byte(pdev, HPETD_CONTROL, &reg8);
	printk("hpetd[%d]: cfg register %02x = %02x\n", 
		devcount, HPETD_CONTROL, reg8);

	if(!(reg8 & HPETD_CTRL_ENA)) {
		printk("hpetd[%d]: Write %02x to cfg reg %02x\n",
			devcount, HPETD_CTRL_ENA, HPETD_CONTROL);
		reg8 |= HPETD_CTRL_ENA;
		pci_write_config_byte(pdev, HPETD_CONTROL, reg8);
		pci_read_config_byte(pdev, HPETD_CONTROL, &reg8);
		printk("hpetd[%d]: cfg  register %02x = %02x\n", 
			devcount, HPETD_CONTROL, reg8);
	}

	base_pa	= pci_resource_start(pdev, 1);
	end_pa	= pci_resource_end(pdev, 1);
	size 	= end_pa - base_pa;
	bp	=  ioremap_nocache(base_pa, size);

	printk("hpetd[%d]: mmio base pa: %08lx\n", devcount, base_pa);
	printk("hpetd[%d]: mmio  end pa: %08lx\n", devcount, end_pa);
	printk("hpetd[%d]: mmio pa size: %08lx\n", devcount, size);
	printk("hpetd[%d]: mmio base va: %p\n", devcount, bp);

	// Get all the device information togegther.
	//
	dev_array[devcount].pa	 = base_pa;
	dev_array[devcount].size = size;
	dev_array[devcount].va.v = bp;
	dev_array[devcount].pdev = pdev;

	// Read the register first, just to see what's in there.
	// Then write a 1 to the error bit
	// Then read it back to see if it got there.
	//
	reg32 = readl(bp + HPETD_SETREG);
	mb();
	printk("hpetd[%d]: current SETREG value: %08lx\n", devcount, reg32);

	if(reg32 != 0) {
		printk("hpetd[%d]: writing %08x to MMIO offset %08x\n",
			devcount, 0, HPETD_SETREG);
		writel(0, (bp + HPETD_SETREG));
		mb();
		reg32 = readl(bp + HPETD_SETREG);
		printk("hpetd[%d]: SETREG zero value: %08lx\n", devcount, reg32);
	}

	printk("hpetd[%d]: writing %08x to MMIO offset %08x\n",
		devcount, HPETD_ERRBIT, HPETD_SETREG);
	mb();
	writel(HPETD_ERRBIT, (bp + HPETD_SETREG));
	mb();
	reg32 = readl(bp + HPETD_SETREG);
	printk("hpetd[%d]: SETREG new value: %08lx\n", devcount, reg32);

	// Increment the device count.
	//
	devcount++;

probe_exit:
	return rc;
}

static void hpetd_remove(struct pci_dev *pdev)
{
	printk("hpetd: being removed\n");
}

module_init(hpetd_init);
module_exit(hpetd_exit);

