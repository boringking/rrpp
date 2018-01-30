#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#include "ip1829.h"
#include "ip1829fdat.h"

static struct cdev ip1829_cdev;
struct class *ip1829_class;
static DEFINE_MUTEX(ip1829_mutex);
u8 CPU_IF_SPEED_NORMAL;

#define	IP1829_MAJOR	248
#define IP1829_NAME	"ip1829_cdev"

//#define IP1829DEBUG

MODULE_LICENSE ("GPL");

static int ip1829_open(struct inode *inode, struct file *fs)
{
#ifdef IP1829DEBUG
	printk("ip1829: open...\n");
#endif
	try_module_get(THIS_MODULE);

	return 0;
}

static int ip1829_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
#ifdef IP1829DEBUG
	printk("ip1829: release!\n");
#endif
	return 0;
}

static ssize_t ip1829_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	//  	return simple_read_from_buffer(buffer, length, offset, msg, 200);
	return 0;
}

static ssize_t ip1829_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	/*	if (len > 199)
		return -EINVAL;
		copy_from_user(msg, buff, len);
		msg[len] = '\0';
		return len;*/
	return 0;
}

static int ip1829_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	unsigned long len, cmdid, fno, fsubg, fgrp, fusg;
	void *cptr;
	char *cdata;
	int ret;

#ifdef IP1829DEBUG
	printk(KERN_ALERT "ip1829: +ioctl...\n");
#endif
	len = (int)(_IOC_SIZE(cmd));
	cptr = (void *)arg;

	do {
		cdata = kmalloc(len, GFP_KERNEL);
		if (!cdata)
		{
			ret = -ENOMEM;
			goto out_ip1829_ioctl;
		}

		if(access_ok(VERIFY_READ, cptr, len)){
			copy_from_user(cdata, cptr, len);
		}
		else{
			printk(KERN_ERR "[switchDriver] copy_from_user access fail by %s:%d CMD:%x\n",__FUNCTION__,__LINE__,cmd);
			ret = -EFAULT;
			goto out_ip1829_ioctl;
		}
		cmdid = *((unsigned long *)(cdata+8));
		fno = (cmdid >> _CMDID_NRSHIFT) & _CMDID_NRMASK;
		fsubg=(cmdid >> _CMDID_SUBGSHIFT) & _CMDID_SUBGMASK;
		fgrp= (cmdid >> _CMDID_GRPSHIFT) & _CMDID_GRPMASK;
		fusg= (cmdid >> _CMDID_USGSHIFT) & _CMDID_USGMASK;
#ifdef IP1829DEBUG
		printk(KERN_ALERT "cmdid=0x%08x\n", (unsigned int)cmdid);
#endif

		if (fusg == _CMDID_USG_COMMON)
		{
			switch (fgrp)
			{
				case _CMDID_GRP_BASIC:
					switch (fsubg)
					{
						case _CMDID_SUBG_SMI:
							ret = func_of_common_smi[fno](cdata, len);	break;

						case _CMDID_SUBG_CAP:
							ret = func_of_common_cap[fno](cdata, len);	break;

						case _CMDID_SUBG_LUT:
							ret = func_of_common_lut[fno](cdata, len);	break;

						case _CMDID_SUBG_SNIFFER:
							ret = func_of_common_sniffer[fno](cdata, len);	break;

						case _CMDID_SUBG_STORM:
							ret = func_of_common_storm[fno](cdata, len);	break;

						case _CMDID_SUBG_EOC:
							ret = func_of_common_eoc[fno](cdata, len);	break;

						case _CMDID_SUBG_LD:
							ret = func_of_common_ld[fno](cdata, len);	break;

						case _CMDID_SUBG_WOL:
							ret = func_of_common_wol[fno](cdata, len);	break;

						case _CMDID_SUBG_STAG:
							ret = func_of_common_stag[fno](cdata, len);	break;

						case _CMDID_SUBG_PTP:
							ret = func_of_common_ptp[fno](cdata, len);	break;

						case _CMDID_SUB_BANDWIDTH:
							ret = func_of_common_bandwidth[fno](cdata, len);	break;

						case _CMDID_SUBG_MISC:
							ret = func_of_common_misc[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_VLAN:
					switch (fsubg)
					{
						case _CMDID_SUBG_VLAN:
							ret = func_of_common_vlan[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_QOS:
					switch (fsubg)
					{
						case _CMDID_SUB_BANDWIDTH:
							ret = func_of_common_bandwidth[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_SEC:
					switch (fsubg)
					{
						case _CMDID_SUBG_IMP:
							ret = func_of_common_imp[fno](cdata, len);	break;

						case _CMDID_SUBG_COS:
							ret = func_of_common_cos[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_ADV:
					switch (fsubg)
					{
						case _CMDID_SUBG_STP:
							ret = func_of_common_stp[fno](cdata, len);	break;

						case _CMDID_SUBG_LACP:
							ret = func_of_common_lacp[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				default:
					ret = -EINVAL;
			}
		}
		else if (fusg == _CMDID_USG_IP1829)
		{
			switch (fgrp)
			{
				case _CMDID_GRP_BASIC:
					switch (fsubg)
					{
						case _CMDID_SUBG_CAP:
							ret = func_of_common_cap[fno](cdata, len);	break;

						case _CMDID_SUBG_LUT:
							ret = func_of_ip1829_lut[fno](cdata, len);	break;

						case _CMDID_SUBG_SNIFFER:
							ret = func_of_ip1829_sniffer[fno](cdata, len);	break;

						case _CMDID_SUBG_STORM:
							ret = func_of_ip1829_storm[fno](cdata, len);	break;

						case _CMDID_SUBG_EOC:
							ret = func_of_ip1829_eoc[fno](cdata, len);	break;

						case _CMDID_SUBG_LD:
							ret = func_of_ip1829_ld[fno](cdata, len);	break;

						case _CMDID_SUBG_WOL:
							ret = func_of_ip1829_wol[fno](cdata, len);	break;

						case _CMDID_SUBG_IGMP:
							ret = func_of_ip1829_igmp[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_VLAN:
					switch (fsubg)
					{
						case _CMDID_SUBG_VLAN:
							ret = func_of_ip1829_vlan[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;
				case _CMDID_GRP_QOS:
					switch (fsubg)
					{
						case _CMDID_SUB_QOS:
							ret = func_of_ip1829_qos[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;
				case _CMDID_GRP_ACL:
					switch (fsubg)
					{
						case _CMDID_SUBG_ACL:
							ret = func_of_ip1829_acl[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_SEC:
					switch (fsubg)
					{
						case _CMDID_SUBG_IMP:
							ret = func_of_ip1829_imp[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				case _CMDID_GRP_ADV:
					switch (fsubg)
					{
						case _CMDID_SUBG_STP:
							ret = func_of_ip1829_stp[fno](cdata, len);	break;

						case _CMDID_SUBG_LACP:
							ret = func_of_ip1829_lacp[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;
				case _CMDID_GRP_MON:
					switch (fsubg)
					{
						case _CMDID_SUBG_MIB_COUNTER:
							ret = func_of_ip1829_mib_counter[fno](cdata, len);	break;

						default:
							ret = -EINVAL;
					}
					break;

				default:
					ret = -EINVAL;
			}
		}
		else
			ret = -EINVAL;

		if (ret < 0)	goto out_ip1829_ioctl;
		if (copy_to_user(cptr, cdata, len))
		{
			ret = -EFAULT;
			goto out_ip1829_ioctl;
		}
		cptr= (void *)*((unsigned long *)cdata);
		len = *((unsigned long *)(cdata+4));

		kfree(cdata);
		cdata = NULL;
	} while (cptr);
out_ip1829_ioctl:
	if(cdata)
	{
		//memset(cdata, 0x0, len);
		kfree(cdata);
	}
#ifdef IP1829DEBUG
	printk(KERN_ALERT "ip1829: -ioctl...\n");
#endif
	return (ret < 0) ? ret : 0;
}

static long ip1829_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret;

	mutex_lock(&ip1829_mutex);
	ret = ip1829_ioctl(filep, cmd, arg);
	mutex_unlock(&ip1829_mutex);

	return ret;
}

static struct file_operations ip1829_fops = {
	.owner			= THIS_MODULE,
	.read			= ip1829_read,
	.write			= ip1829_write,
	.unlocked_ioctl	= ip1829_unlocked_ioctl,
	.open			= ip1829_open,
	.release		= ip1829_release
};

extern void IP2Page(u8 page);
extern u16 Read_Reg_0_With_1s(void);
static int __init ip1829_init(void)
{
	int result;

	result = register_chrdev_region(MKDEV(IP1829_MAJOR, 0), 1, IP1829_NAME);
	if (result < 0)
	{
		printk(KERN_WARNING "ip1829: can't get major %d\n", IP1829_MAJOR);
		return result;
	}

	cdev_init(&ip1829_cdev, &ip1829_fops);
	ip1829_cdev.owner = THIS_MODULE;
	result = cdev_add(&ip1829_cdev, MKDEV(IP1829_MAJOR, 0), 1);
	if (result)
	{
		printk(KERN_WARNING "ip1829: error %d adding driver\n", result);
		return result;
	}

	// set ip218 mdc divisor
	/*
	 * 0: Divide by 4. read LUT will be wrong
	 * 1: Divide by 8.
	 * 2: Divide by 16.
	 * 3: Divide by 32.
	 * 4: Divide by 64.
	 * 5: Divide by 128.
	 * 6: Divide by 256.
	 * 7: Divide by 512.
	 * */
	ic_mdio_set_divisor(0);
	CPU_IF_SPEED_NORMAL = 0; //high speed

	IP2Page(0);
	if(0x8290 == Read_Reg_0_With_1s())
	{
		printk("ip1829: CPU I/F High speed.");
	}
	else
	{
		ic_mdio_set_divisor(3);
		CPU_IF_SPEED_NORMAL = 1;
		printk("ip1829: CPU I/F Normal speed.");
	}

	// for acl_man init.
	acl_init();

	printk(" Driver loaded!\n");
	return 0;
	/*
fail_return:
	if(ip1829_cdev){
		cdev_del(ip1829_cdev);
		kfree(ip1829_cdev);
		ip1829_cdev=NULL;
	}
	if(devno)
		unregister_chrdev_region(devno, 1);
	if(ip1829_class)
		class_destroy(ip1829_class);
	return -1;
	*/
}

static void __exit ip1829_exit(void)
{
	/*
	if(ip1829_cdev){
		cdev_del(ip1829_cdev);
		kfree(ip1829_cdev);
		ip1829_cdev=NULL;
	}
	if(devno)
		unregister_chrdev_region(devno, 1);
	if(ip1829_class)
		class_destroy(ip1829_class);
	*/
	cdev_del(&ip1829_cdev);
	unregister_chrdev_region(MKDEV(IP1829_MAJOR, 0), 1);
	printk("ip1829: Driver unloaded!\n");
}

module_init(ip1829_init);
module_exit(ip1829_exit);
