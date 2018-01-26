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
////////////////////////////////////////////////////////////////////////
//Project 1: Darshit Pandit, dupandit; Saikrishna Vasudevan, svasude4; Harshal Gurjar, hkgurjar; 
///////////////////////////////////////////////////////////////////////

#include "npheap.h"

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

// Define a struct for objects in a linked list
extern struct npheap_node{
    int object_no;
    unsigned long pfn;
    int size;
    struct npheap_node *next;
};

//Head pointer of Linked list
extern struct npheap_node* HEAD ;





// DEFINE LOCK mutex
DEFINE_MUTEX(my_mutex);
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
  mutex_lock(&my_mutex);
  return 0;
}     

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
    mutex_unlock(&my_mutex);
    return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
  struct npheap_node* temp;
  temp=HEAD; 
  if (temp!=NULL)
    {
	// Traverse through linked list to find the object_id and return size if found
      while(temp!=NULL)
	{
	  if (temp->object_no==(user_cmd->offset)/PAGE_SIZE)
	    {
	      printk("offset is %d, and the size is %d \n",(user_cmd->offset)/PAGE_SIZE,temp->size) ;
	      
	      return temp->size; 
	    }
	  temp=temp->next;
	}
    }
  return 0;
}
long npheap_delete(struct npheap_cmd __user *user_cmd)
{
  
  struct npheap_node* temp;
  temp=HEAD;
  struct npheap_node* save;
  if (temp!=NULL)
    {
      //if the first node is the object
      if (temp->object_no==(user_cmd->offset)/PAGE_SIZE)
	{
	  HEAD=temp->next;
	  kfree(temp);
	  return 0;
	}

      // Find object, if found delete the object 
      while(temp->next!=NULL)
	{
	  if (temp->next->object_no==(user_cmd->offset)/PAGE_SIZE)
	    {
	      save=temp->next;
	      temp->next=temp->next->next;
	      kfree(save);
	      return 0;
	    }
	  temp=temp->next;
	}
    }

  return 0;
}

long npheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case NPHEAP_IOCTL_LOCK:
        return npheap_lock((void __user *) arg);
    case NPHEAP_IOCTL_UNLOCK:
        return npheap_unlock((void __user *) arg);
    case NPHEAP_IOCTL_GETSIZE:
        return npheap_getsize((void __user *) arg);
    case NPHEAP_IOCTL_DELETE:
        return npheap_delete((void __user *) arg);
    default:
        return -ENOTTY;
    }
}
