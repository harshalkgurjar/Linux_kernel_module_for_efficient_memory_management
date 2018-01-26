//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "tnpheap_ioctl.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/time.h>

    DEFINE_MUTEX(my_mutex);
struct miscdevice tnpheap_dev;

struct ver_off
{
    __u64 version;
    __u64 offset;
    struct ver_off *next;
};

struct ver_off *head = NULL;

int trans_id;

__u64 tnpheap_get_version(struct tnpheap_cmd __user *user_cmd)
{
    printk("Entering kernel get version\n");
    struct tnpheap_cmd cmd;
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
      printk("Copy in version failed\n");
    }
    else
    {
      printk("Object %ld requesting version\n",cmd.offset);
      struct ver_off *traverse = head;

      // If offset is there in ver_off
      while(traverse!=NULL)
      {
        if (cmd.offset == traverse->offset)
        {
          printk("Object %ld already there\n",cmd.offset);
          printk("Object %ld : version %ld\n",cmd.offset,traverse->version);
          return traverse->version;
        }
        else
        {
          traverse=traverse->next;
        }
      }


      // Else make a node and :
        //- Make it a first object if LL is empty
        // - Append at the end of linked list
      printk("Object %ld not in veroff\n",cmd.offset);
      struct ver_off *traverse2 = head;

      struct ver_off *new_node =(struct ver_off*)kmalloc(sizeof(struct ver_off),GFP_KERNEL);
      new_node->offset = cmd.offset;
      new_node->version = 1;

      // First object
      if(traverse2==NULL)
      {
         head = new_node;
	traverse2=head;
      }

      else
      {
        while(traverse2->next!=NULL)
        {
          traverse2=traverse2->next;
        }
      traverse2->next=new_node;
       }

      printk("Object %ld : version %ld\n",cmd.offset,traverse2->version);
      return traverse2->version ;
    }
}

__u64 tnpheap_start_tx(struct tnpheap_cmd __user *user_cmd)
{   printk("Entering kernel transaction\n");
    struct tnpheap_cmd cmd;
    __u64 ret=0;
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
	     printk("Copy in transaction failded\n");
    }
    trans_id +=1;
    printk("Starting Trasaction : %d\n",trans_id);
    return trans_id ;
    }

__u64 tnpheap_commit(struct tnpheap_cmd __user *user_cmd)
{
    struct tnpheap_cmd cmd;
    __u64 ret=0;
    printk("inside kernel\n");	
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        printk("Copy failed in tnpheap_commit\n");
    }

    // If size = 0, then perform lock
    // If size = 1, then perform unlocked
    // If size = 2, increment version number for that object
    else if(cmd.size == 0)
    {
	printk("locking \n");
	mutex_lock(&my_mutex);
    }

    else if(cmd.size == 1)
    {
	printk("unlocking \n");
	mutex_unlock(&my_mutex);
    }

    else if(cmd.size == 2)
    {
      struct ver_off *my_temp = head;
      printk("incrementing version for offset %d \n",cmd.offset);
      while (my_temp!=NULL)
      {
          if(my_temp->offset == cmd.offset)
          {
            printk("Incremnting version for %ld\n",my_temp->offset);
            my_temp->version += 1;
            break;
	  }
	else
           my_temp = my_temp->next;
      }
    }
    return ret;
}



__u64 tnpheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case TNPHEAP_IOCTL_START_TX:
        return tnpheap_start_tx((void __user *) arg);
    case TNPHEAP_IOCTL_GET_VERSION:
        return tnpheap_get_version((void __user *) arg);
    case TNPHEAP_IOCTL_COMMIT:
        return tnpheap_commit((void __user *) arg);
    default:
        return -ENOTTY;
    }
}

static const struct file_operations tnpheap_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = tnpheap_ioctl,
};

struct miscdevice tnpheap_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tnpheap",
    .fops = &tnpheap_fops,
};

static int __init tnpheap_module_init(void)
{
    int ret = 0;
    if ((ret = misc_register(&tnpheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return 1;
}

static void __exit tnpheap_module_exit(void)
{
    misc_deregister(&tnpheap_dev);
    return;
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(tnpheap_module_init);
module_exit(tnpheap_module_exit);
