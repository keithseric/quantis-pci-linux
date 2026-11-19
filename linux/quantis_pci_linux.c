/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Linux modernization modifications, 2026.
 * Licensing revision: 2026-11-17.
 * Maintainer: Seric <seric@seric.com>
 *
 * This file has been modified from the original
 * ID Quantique Quantis v18.03.08 source.
 *
 * Original copyright notices and upstream licensing
 * grants are preserved below.
 *
 * Project modifications are distributed under the
 * GNU General Public License version 2 only.
 */

/*
 * Quantis PCI driver
 *
 * Copyright (C) 2004-2014 ID Quantique SA, Carouge/Geneva, Switzerland
 * All rights reserved.
 *
 * ----------------------------------------------------------------------------
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY.
 *
 * ----------------------------------------------------------------------------
 *
 * Alternatively, this software may be distributed under the terms of the
 * terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * ----------------------------------------------------------------------------
 *
 * For history of changes, ChangeLog.txt
 */


#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/sched/signal.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/version.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>




#include "../common/quantis_pci.h"
#include "../common/quantis_pci_common.h"




/****************************** Module parameters *****************************
 *
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 * see http://tldp.org/LDP/lkmpg/2.6/html/x323.html
 */

// static int subdir = 0;

// module_param (subdir, int, 0);
// MODULE_PARM_DESC(subdir, "Force device(s) to be created on a /dev subdirectory (0-1)");

// #if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14))
// #define pm_message_t u32
// #endif


/* Allotted device number */
dev_t quantis_dev_number;

/* Tie with the device model */
struct class* quantis_class;


/** Logging routines **/
#define QUANTIS_INFO(fmt, args...) \
  printk(KERN_INFO "%s: " fmt "\n" , QUANTIS_PCI_DRIVER_SHORTNAME , ## args)

#define QUANTIS_WARNING(fmt, args...) \
  printk(KERN_WARNING "%s: WARNING " fmt "\n" , QUANTIS_PCI_DRIVER_SHORTNAME , ## args)

#define QUANTIS_ERROR(fmt, args...) \
  printk(KERN_ERR "%s: ERROR " fmt "\n" , QUANTIS_PCI_DRIVER_SHORTNAME , ## args)






#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
#define DEV_INIT __devinit
#define DEV_EXIT __devexit
#define DEV_EXIT_P __devexit_p
#else
#define DEV_INIT 
#define DEV_EXIT 
#define DEV_EXIT_P 
#endif


#define PCI_BASE_ADDRESS_REGISTER0        0
#define PCI_BASE_ADDRESS_REGISTER1        1

/**
 * Quantis PCI per-device structure
 */
struct quantis_pci_device
{
  unsigned char pci_dev_irq;                         /* irq of this device */
  void __iomem* regs;                                   /* BUS address in CPU space */
  struct semaphore mutex;                            /* device's mutex */
  unsigned char buffer[QUANTIS_DEVICE_BUFFER_SIZE];  /* random data's buffer */
  u_int32_t device_number;                           /* internal number of this device */
  struct cdev cdev;                                  /* char device structure */
  struct pci_dev *pdev;                              /* associated PCI device */
  struct device chardev;                             /* device lifetime */
  bool disconnected;                                 /* PCI device removed */
  char pciBusName[256];
  char ais31StartupTestsRequestFlag;
};

/** Holds all device's information */
quantis_pci_device* quantis_devices[QUANTIS_PCI_MAX_CARDS];

/*
 * Slots assigned during this module's lifetime.
 * Do not reuse a minor number after PCI removal, because an
 * old character-device instance may still have references.
 */
static bool quantis_slots_reserved[QUANTIS_PCI_MAX_CARDS];

/** Number of Quantis PCI devices */
int quantis_pci_cards_count = 0;

/** Global mutex */
struct semaphore quantis_mutex;


/*
 * Release the per-device allocation when its final reference is dropped.
 * PCI resources must already have been released by the remove path.
 *
 * Do not acquire quantis_mutex here: the final put_device() may
 * occur while the caller already holds that semaphore.
 *
 * Assigned minor numbers remain reserved until module unload.
 */
static void quantis_device_release(struct device *dev)
{
  quantis_pci_device *device =
      container_of(dev, quantis_pci_device, chardev);

  kfree(device);
}


/*
 * Find an unused device slot.
 * Caller must hold quantis_mutex.
 */
static int quantis_find_free_slot(void)
{
  int i;

  for (i = 0; i < QUANTIS_PCI_MAX_CARDS; i++)
  {
    if (!quantis_devices[i] && !quantis_slots_reserved[i])
      return i;
  }

  return -ENOSPC;
}


/****************************** Shared functions *****************************
 *
 * Drivers are not allowed to directly access the hardware, but have to use 
 * the shared quantis functions (defined in quantis_pci_common.h).
 * 
 * quantis_reg_get and quantis_reg_set must be defined here since are
 * os-specific.
 */

quantis_register_value quantis_reg_get(quantis_pci_device* qdev, quantis_register reg)
{
  return readl((u8 __iomem *)qdev->regs + reg);
}

void quantis_reg_set(quantis_pci_device* qdev, quantis_register reg, quantis_register_value value)
{
  writel(value, (u8 __iomem *)qdev->regs + reg);
}


//Get Pci bus device Id
u_int32_t quantis_get_pci_bus_device_id(quantis_pci_device* qdev)
{
  int pciBusDeviceId;
  char pciDevice[64];
  char pciBusDeviceFunctionName[64];
  
  memset(pciBusDeviceFunctionName, 0, sizeof(pciBusDeviceFunctionName));
  memset(pciDevice, 0, sizeof(pciDevice));
  
  sprintf(pciBusDeviceFunctionName, "%s", qdev->pciBusName);
  
  memcpy(pciDevice, &pciBusDeviceFunctionName[5], 2);    
  sscanf(pciDevice, "%x", &pciBusDeviceId);  
  
  return pciBusDeviceId;
}


/********************* Declarations of internal functions ********************
 *
 */

static int quantis_open(struct inode* inode, struct file* file);

static int quantis_close(struct inode* inode, struct file* file);

static ssize_t quantis_read(struct file* file,
                            char __user* buffer,
                            size_t length,
                            loff_t* ppos);



static int quantis_ioctl(struct inode* inode,
                         struct file* file,
                         unsigned int cmd,
                         unsigned long arg);


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
long quantis_unlocked_ioctl(struct file* file, 
                    unsigned int cmd,
                    unsigned long arg);
#endif



static int DEV_INIT quantis_pci_probe(struct pci_dev* pdev,
                                       const struct pci_device_id* ent);

static void DEV_EXIT quantis_pci_remove(struct pci_dev* pdev);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int quantis_pci_proc_read(char* buf,
                                 char** start,
                                 off_t offset,
                                 int count,
                                 int* eof,
                                 void* data);
#endif


#ifdef CONFIG_PM /* Power-management related functions */
static int quantis_pci_suspend(struct pci_dev *pdev,
                               pm_message_t state);

static int quantis_pci_resume(struct pci_dev *pdev);
#endif /* CONFIG_PM */

static int __init quantis_module_init(void);

static void __exit quantis_module_exit(void);


/**
 * The set of PCI cards that this driver supports.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
struct pci_device_id quantis_pci_ids_table[] __devinitdata =
#else
struct pci_device_id quantis_pci_ids_table[] =
#endif
{
  { PCI_DEVICE(VENDOR_ID_HESSO, DEVICE_ID_QUANTIS_PCI) },
  /*{ PCI_DEVICE(VENDOR_ID_???, DEVICE_ID_????) },*/
  {0},
};


/************************* Kernel's modules properties ************************
 *
 */

module_init(quantis_module_init);
module_exit(quantis_module_exit);

MODULE_DEVICE_TABLE(pci, quantis_pci_ids_table);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(QUANTIS_PCI_DRIVER_AUTHOR);
MODULE_DESCRIPTION(QUANTIS_PCI_DRIVER_NAME);



/***************************** Internal structures ****************************
 *
 */


/**
 * File operations structure.
 */
static struct file_operations quantis_fops =
{
  .owner    =   THIS_MODULE,      /* Owner */
  .open     =   quantis_open,     /* Open method */
  .release  =   quantis_close,    /* Release method */
  .read     =   quantis_read,     /* Read method */
  /*.write    =   quantis_write,*/    /* Write method */
  /*.llseek   =   quantis_llseek,*/   /* Seek method */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
	.unlocked_ioctl =	quantis_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	/*
	 * All IOCTL arguments are pointers to
	 * 32-bit integers or unused.
	 */
	.compat_ioctl = compat_ptr_ioctl,
#endif
#else
 .ioctl		=		quantis_ioctl,    /* Ioctl method */
#endif

};



/**
 *
 */
static struct pci_driver quantis_pci_driver =
{
  .name      = QUANTIS_PCI_DRIVER_SHORTNAME,
  .id_table  = quantis_pci_ids_table,
  .probe     = quantis_pci_probe,
  .remove    = DEV_EXIT_P(quantis_pci_remove),
#ifdef CONFIG_PM
  .suspend   = quantis_pci_suspend,
  .resume    = quantis_pci_resume,
#endif
};







/**
 *
 */
int quantis_open(struct inode* inode, struct file* file)
{
  quantis_pci_device *device =
      container_of(inode->i_cdev, quantis_pci_device, cdev);

  if (down_interruptible(&quantis_mutex))
    return -ERESTARTSYS;

  /*
   * The PCI remove path clears the published slot under this lock.
   * Reject opens against devices that are no longer published.
   */
  if (READ_ONCE(device->disconnected) ||
      device->device_number >= QUANTIS_PCI_MAX_CARDS ||
      quantis_devices[device->device_number] != device)
  {
    up(&quantis_mutex);
    return -ENODEV;
  }

  get_device(&device->chardev);
  up(&quantis_mutex);

  file->private_data = device;
  return 0;
}

/**
 *
 */
int quantis_close(struct inode* inode, struct file* file)
{
  quantis_pci_device *device = file->private_data;

  file->private_data = NULL;

  if (device)
    put_device(&device->chardev);

  return 0;
}


/**
 *
 */
ssize_t quantis_read(struct file* file,
                     char __user* buffer,
                     size_t length,
                     loff_t* ppos)
{
  int bytes_read;
  unsigned long uncopied;
  bool nonblock = !!(READ_ONCE(file->f_flags) & O_NONBLOCK);
  quantis_pci_device* device = file->private_data;

  if (!length)
    return 0;

  if (!device)
    return -ENODEV;

  if (length > QUANTIS_DEVICE_BUFFER_SIZE)
    length = QUANTIS_DEVICE_BUFFER_SIZE;

  if (!access_ok(buffer, length))
    return -EFAULT;

  if (nonblock)
  {
    if (down_trylock(&device->mutex))
      return -EAGAIN;
  }
  else
  {
    if (down_interruptible(&device->mutex))
      return -ERESTARTSYS;
  }

  if (READ_ONCE(device->disconnected))
  {
    up(&device->mutex);
    return -ENODEV;
  }

  bytes_read = quantis_rng_read(device,
                                device->buffer,
                                length,
                                nonblock);

  if (bytes_read > 0)
  {
    uncopied = copy_to_user(buffer,
                            device->buffer,
                            bytes_read);

    if (uncopied == bytes_read)
      bytes_read = -EFAULT;
    else
      bytes_read -= uncopied;
  }

  /*
   * Do not retain acquired random data in the reusable
   * device buffer after a read completes or fails.
   */
  memzero_explicit(device->buffer, length);

  up(&device->mutex);

  return bytes_read;
}


/**
 *
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))

long quantis_unlocked_ioctl(struct file* f, 
                            unsigned int command,
                            unsigned long arg)
{
  struct inode *inode = file_inode(f);

  return quantis_ioctl(inode, f, command, arg);
}
#endif





/**
 *
 */
int quantis_ioctl(struct inode* inode,
                  struct file* file,
                  unsigned int cmd,
                  unsigned long arg)
{
  int status = 0;
  quantis_pci_device* device = file->private_data;

  if (!device)
    return -ENODEV;

  /*
   * Read access permits diagnostics, not hardware administration.
   * Require both a writable descriptor and CAP_SYS_RAWIO for
   * operations that change hardware or startup-test state.
   */
  switch (cmd)
  {
    case QUANTIS_IOCTL_RESET_BOARD:
    case QUANTIS_IOCTL_ENABLE_MODULE:
    case QUANTIS_IOCTL_DISABLE_MODULE:
    case QUANTIS_IOCTL_CLEAR_AIS31_STARTUP_TESTS_REQUEST_FLAG:
      if (!(file->f_mode & FMODE_WRITE))
        return -EBADF;

      if (!capable(CAP_SYS_RAWIO))
        return -EPERM;

      break;

    case QUANTIS_IOCTL_GET_DRIVER_VERSION:
    case QUANTIS_IOCTL_GET_CARD_COUNT:
    case QUANTIS_IOCTL_GET_BOARD_VERSION:
    case QUANTIS_IOCTL_GET_MODULES_MASK:
    case QUANTIS_IOCTL_GET_MODULES_STATUS:
    case QUANTIS_IOCTL_GET_PCI_BUS_DEVICE_ID:
    case QUANTIS_IOCTL_GET_AIS31_STARTUP_TESTS_REQUEST_FLAG:
      if (!(file->f_mode & FMODE_READ))
        return -EBADF;

      break;

    default:
      return -ENOTTY;
  }

  if (down_interruptible(&device->mutex))
    return -ERESTARTSYS;

  /* Serialize hardware access with PCI removal. */
  if (READ_ONCE(device->disconnected))
  {
    up(&device->mutex);
    return -ENODEV;
  }

  switch (cmd)
  {
    case QUANTIS_IOCTL_GET_DRIVER_VERSION:
    {
      status = put_user((u_int32_t)QUANTIS_PCI_DRIVER_VERSION, (u_int32_t __user *)arg);
      break;
    }

    case QUANTIS_IOCTL_GET_CARD_COUNT:
    {
      status = put_user((u_int32_t)READ_ONCE(quantis_pci_cards_count),
                        (u_int32_t __user *)arg);
      break;
    }

    case QUANTIS_IOCTL_GET_BOARD_VERSION:
    {
      u_int32_t version = quantis_rng_version(device);
      status = put_user(version, (u_int32_t __user *)arg);
      break;
    }

    case QUANTIS_IOCTL_RESET_BOARD:
    {
      quantis_rng_reset(device);
      break;
    }

    case QUANTIS_IOCTL_GET_MODULES_MASK:
    {
      u_int32_t mask = quantis_rng_modules_mask(device);
      status = put_user(mask, (u_int32_t __user *)arg);
      break;
    }

    case QUANTIS_IOCTL_ENABLE_MODULE:
    {
      u_int32_t modules;
      status = get_user(modules, (u_int32_t __user *)arg);
      if (status)
        break;
      if (!modules ||
          (modules & ~quantis_rng_modules_mask(device)))
      {
        status = -EINVAL;
        break;
      }

      quantis_rng_enable_modules(device, modules);
      break;
    }

    case QUANTIS_IOCTL_DISABLE_MODULE:
    {
      u_int32_t modules;
      status = get_user(modules, (u_int32_t __user *)arg);
      if (status)
        break;
      if (!modules ||
          (modules & ~quantis_rng_modules_mask(device)))
      {
        status = -EINVAL;
        break;
      }

      quantis_rng_disable_modules(device, modules);
      break;
    }

    case QUANTIS_IOCTL_GET_MODULES_STATUS:
    {
      u_int32_t modules_status = quantis_rng_modules_status(device);
      status = put_user(modules_status, (u_int32_t __user *)arg);
      break;
    }

    case QUANTIS_IOCTL_GET_PCI_BUS_DEVICE_ID:
    {
      u_int32_t deviceId = quantis_get_pci_bus_device_id(device);
      status = put_user((u_int32_t)deviceId, (u_int32_t __user *)arg);
      break;
    }    
    
    case QUANTIS_IOCTL_GET_AIS31_STARTUP_TESTS_REQUEST_FLAG:
    {      
      status = put_user((u_int32_t)device->ais31StartupTestsRequestFlag, (u_int32_t __user *)arg);
      break;
    }    
    
    case QUANTIS_IOCTL_CLEAR_AIS31_STARTUP_TESTS_REQUEST_FLAG:
    {      
      device->ais31StartupTestsRequestFlag = 0;
      break;
    }    
    
    default:
    {
      status = -ENOTTY;
    }
  } // switch (cmd)

  up(&device->mutex);

  return status;
}


#ifdef CONFIG_PM
int quantis_pci_suspend(struct pci_dev* pdev, pm_message_t state)
{
  return 0;
}

int quantis_pci_resume(struct pci_dev* pdev)
{
  return 0;
}
#endif /* CONFIG_PM */


int DEV_INIT quantis_pci_probe(struct pci_dev* pdev,
                                const struct pci_device_id* ent)
{
  quantis_pci_device *device;
  int status, slot;

  down(&quantis_mutex);
  slot = quantis_find_free_slot();
  if (slot < 0) {
    up(&quantis_mutex);
    return slot;
  }
  quantis_slots_reserved[slot] = true;
  up(&quantis_mutex);

  device = kzalloc(sizeof(*device), GFP_KERNEL);
  if (!device) {
    status = -ENOMEM;
    goto err_slot;
  }

  device_initialize(&device->chardev);
  device->chardev.release = quantis_device_release;
  device->chardev.class = quantis_class;
  device->chardev.parent = &pdev->dev;
  device->chardev.devt = MKDEV(MAJOR(quantis_dev_number), slot);

  device->pdev = pdev;
  device->device_number = slot;
  device->ais31StartupTestsRequestFlag = 1;
  sema_init(&device->mutex, 1);

  status = dev_set_name(&device->chardev, "%s%d",
                        QUANTIS_PCI_DEVICE_NAME, slot);
  if (status)
    goto err_device;

  status = pci_enable_device(pdev);
  if (status)
    goto err_device;

  status = pci_request_regions(pdev, QUANTIS_PCI_DEVICE_NAME);
  if (status)
    goto err_disable;

  if (!(pci_resource_flags(pdev, 1) & IORESOURCE_MEM) ||
      pci_resource_len(pdev, 1) < QUANTIS_REG_LENGTH) {
    status = -ENODEV;
    goto err_regions;
  }

  device->regs = ioremap(pci_resource_start(pdev, 1),
                         QUANTIS_REG_LENGTH);
  if (!device->regs) {
    status = -ENOMEM;
    goto err_regions;
  }

  device->pci_dev_irq = pdev->irq;
  strscpy(device->pciBusName, pci_name(pdev),
          sizeof(device->pciBusName));

  quantis_rng_reset(device);

  cdev_init(&device->cdev, &quantis_fops);
  device->cdev.owner = THIS_MODULE;

  /*
   * Publish the initialized device before cdev registration,
   * because cdev_device_add() may permit immediate opens.
   */
  down(&quantis_mutex);
  quantis_devices[slot] = device;
  quantis_pci_cards_count++;
  up(&quantis_mutex);

  status = cdev_device_add(&device->cdev, &device->chardev);
  if (status)
    goto err_publish;

  pci_set_drvdata(pdev, device);

  QUANTIS_INFO("Found card #%d at %s", slot, pci_name(pdev));
  QUANTIS_INFO("Core version 0x%08x",
               quantis_rng_version(device));
  QUANTIS_INFO("Device registered at /dev/%s%d",
               QUANTIS_PCI_DEVICE_NAME, slot);
  return 0;

err_publish:
  down(&quantis_mutex);
  down(&device->mutex);
  WRITE_ONCE(device->disconnected, true);
  quantis_devices[slot] = NULL;
  quantis_pci_cards_count--;
  up(&device->mutex);
  up(&quantis_mutex);

  /* Existing file operations must finish before MMIO teardown. */
  down(&device->mutex);
  iounmap(device->regs);
  device->regs = NULL;
  up(&device->mutex);

  /*
   * Keep the slot reserved: registration may have briefly
   * exposed the cdev to userspace.
   */
  pci_release_regions(pdev);
  pci_disable_device(pdev);
  put_device(&device->chardev);
  return status;

err_regions:
  pci_release_regions(pdev);
err_disable:
  pci_disable_device(pdev);
err_device:
  put_device(&device->chardev);
err_slot:
  down(&quantis_mutex);
  quantis_slots_reserved[slot] = false;
  up(&quantis_mutex);
  return status;
}


void DEV_EXIT quantis_pci_remove(struct pci_dev* pdev)
{
  quantis_pci_device *device = pci_get_drvdata(pdev);
  unsigned int slot;

  if (!device) {
    QUANTIS_ERROR("Unable to remove unknown PCI device");
    return;
  }

  slot = device->device_number;

  /* Prevent new opens while retiring this device. */
  down(&quantis_mutex);
  down(&device->mutex);
  WRITE_ONCE(device->disconnected, true);
  quantis_devices[slot] = NULL;
  quantis_pci_cards_count--;
  up(&device->mutex);
  up(&quantis_mutex);

  cdev_device_del(&device->cdev, &device->chardev);

  /* Wait for active hardware operations before unmapping. */
  down(&device->mutex);
  iounmap(device->regs);
  device->regs = NULL;
  up(&device->mutex);

  pci_release_regions(pdev);
  pci_disable_device(pdev);
  pci_set_drvdata(pdev, NULL);

  /* Existing open descriptors retain their device references. */
  put_device(&device->chardev);

  QUANTIS_INFO("Removed card #%u", slot);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
/**
 *
 */
int quantis_pci_proc_read(char* buf,
                          char** start,
                          off_t offset,
                          int count,
                          int* eof,
                          void* data)
{
  int len = 0;
  int cards_count = 0;
  int i;
  int j;

  down(&quantis_mutex);
  cards_count = quantis_pci_cards_count;
  up(&quantis_mutex);

  /* Driver info */
  len += snprintf(buf + len, count - len,
                  QUANTIS_PCI_DRIVER_NAME"\n");

  len += snprintf(buf + len, count - len,
                  "driver name         : %s\n",
                  QUANTIS_PCI_DRIVER_SHORTNAME);

  len += snprintf(buf + len, count - len,
                  "driver version      : %d.%d\n",
                  QUANTIS_PCI_DRIVER_VERSION / 10,
                  QUANTIS_PCI_DRIVER_VERSION % 10);

  len += snprintf(buf + len, count - len,
                  "device major number : %d\n",
                  MAJOR(quantis_dev_number));

  len += snprintf(buf + len, count - len,
                  "max cards supported : %d\n",
                  QUANTIS_PCI_MAX_CARDS);

  len += snprintf(buf + len, count - len,
                  "card(s) found       : %d\n",
                  cards_count);

  /* Display info about each card */
  for(i = 0; i < cards_count; i++)
  {
    quantis_pci_device* device = quantis_devices[i];;

    len += snprintf(buf + len, count - len,
                    "\ncard #%d details:\n", i);

    if (!device)
    {
      len += snprintf(buf + len, count - len,
                      "   No information available for this card\n");
      continue;
    }

    len += snprintf(buf + len, count - len,
                    "   core version: 0x%08x\n",
                    quantis_rng_version(device));

    // TODO number of modules and which has been enabled/disabled
    for (j = 0; j < QUANTIS_PCI_MAX_MODULES; j++)
    {
      char* str_mask = NULL;
      char* str_status = NULL;
      if (quantis_rng_modules_mask(device) & (1 << j))
      {
        str_mask = "found";
        if (quantis_rng_modules_status(device) & (1 << j))
        {
          str_status = "(enabled)";
        }
        else
        {
          str_status = "(disabled)";
        }
      }
      else
      {
        str_mask = "not found";
        str_status = "";
      }
      len += snprintf(buf + len, count - len,
                      "   module %d: %s %s\n",
                      j, str_mask, str_status);
    }
  }

  *eof = 1;

  return len;
}
#endif


/**
 *
 */
static int __init quantis_module_init(void)
{
  int status = 0;
  QUANTIS_INFO("Initializing %s version %d.%d",
               QUANTIS_PCI_DRIVER_NAME,
               QUANTIS_PCI_DRIVER_VERSION / 10,
               QUANTIS_PCI_DRIVER_VERSION % 10);
  QUANTIS_INFO("  support enabled up to %d PCI/PCIe card(s)", QUANTIS_PCI_MAX_CARDS);

  sema_init(&quantis_mutex, 1);

  /* Request dynamic allocation of a device major number */
  // TODO: use register_chrdev_region if quantis_dev_number is set!!!!
  status = alloc_chrdev_region(&quantis_dev_number,
                               0,
                               QUANTIS_PCI_MAX_CARDS,
                               QUANTIS_PCI_DEVICE_NAME);
  if (status < 0)
  {
    QUANTIS_ERROR("Can't register quantis device");
    return status;
  }

  /* Populate sysfs entries */
  quantis_class = class_create(THIS_MODULE, QUANTIS_PCI_DEVICE_NAME);
  if (IS_ERR(quantis_class))
  {
    status = PTR_ERR(quantis_class);
    goto cleanup_chrdev;
  }

  /* Register the PCI driver */
  status = pci_register_driver(&quantis_pci_driver);
  if (status < 0)
  {
    QUANTIS_ERROR("PCI init module failed");
    goto cleanup_1;
  }


  /*
   * Legacy /proc registration disabled for Linux 6.1.
   * The character device remains available for hardware RNG access.
   */

  /* Info */
  QUANTIS_INFO("Driver loaded. Found %d card(s)", quantis_pci_cards_count);

  return 0;

  /* Cleanup functions */
cleanup_1:
  class_destroy(quantis_class);

cleanup_chrdev:
  unregister_chrdev_region(quantis_dev_number, QUANTIS_PCI_MAX_CARDS);
  return status;
}


/**
 *
 */
static void __exit quantis_module_exit(void)
{
  QUANTIS_INFO("Unloading driver...");

  /* Unregister the pci driver */
  pci_unregister_driver(&quantis_pci_driver);

  /* Destroys the struct quantis_class structure */
  class_destroy(quantis_class);

  /* Releases the magior number */
  unregister_chrdev_region(quantis_dev_number, QUANTIS_PCI_MAX_CARDS);

  QUANTIS_INFO("Driver unloaded");
}
