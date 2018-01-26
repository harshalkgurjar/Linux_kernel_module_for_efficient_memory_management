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
#include <asm/page.h>

extern struct miscdevice npheap_dev;


// Define head pointer of list as NULL
// Define a struct for objects in a linked list
struct npheap_node{
    int object_no;
    unsigned long pfn;
    int size;
    struct npheap_node *next;
};

struct npheap_node* HEAD = NULL;  // Head pointer of linked list





int npheap_mmap(struct file *filep, struct vm_area_struct *vma)
{
    int flag=0;  // If object found, make flag = 1
    unsigned long pfn_of_same_object;
    
    // We search object_no in linked List and if found map the already associated pfn to user space vma
    struct npheap_node *temp=(struct npheap_node*)HEAD;
    
	
	while(temp!=NULL)
    {
        if(temp->object_no==vma->vm_pgoff)
	{
		flag=1;
		pfn_of_same_object = temp->pfn;
	}
	
	temp=temp->next;
    }


    // If object is Found
    if(flag==1)
    {
	//printk("Object%d Found, Now Mapping with PFN=%lu!\n",vma->vm_pgoff,pfn_of_same_object);
	if(remap_pfn_range(vma, vma->vm_start, pfn_of_same_object, vma->vm_end - vma->vm_start, vma->vm_page_prot))
            return -EAGAIN;
        else
        {
	  //unsigned long ppfn=pfn_of_same_object<<PAGE_SHIFT;
	  //void* va_p=__va(ppfn);
	  //printk("%s \n",(char* )va_p);
	    //printk("Successfully Mapped\n\n\n");
	    return 0;
	}
        
    }


    // If object not found
    else {
	 //printk("Object%d not found!\n",vma->vm_pgoff);
	 void* ptr = kmalloc(vma->vm_end - vma->vm_start, GFP_KERNEL);
         unsigned long phys_ptr=__pa(ptr);
         unsigned long PFN= phys_ptr>>PAGE_SHIFT;
         //printk(" I got a pfn of %lu \n",PFN);
         //printk(" %04x vma_start \n",vma->vm_start);
       
	//  After mapping object, we need to add to lInked list
	    struct npheap_node* head;
	    
	    

	    // Now if Linked List is empty that is if HEAD == NULL, add node as HEAD
	    if(HEAD==NULL)
	    {
	        HEAD=(struct npheap_node*)kmalloc(sizeof(struct npheap_node),GFP_KERNEL);  // will cast a kernel virtual void pointer given by kmalloc to a pointer to struct data.
		head=(struct npheap_node*)HEAD;
		head->object_no=vma->vm_pgoff;
		head->pfn=PFN;
		head->size=vma->vm_end-vma->vm_start;
		head->next=NULL;
	    }
	    // Else linked list is not empty and we need to append to linked list
	    else 
	    {
		temp=HEAD; // As we do not want to alter our HEAD pointer in the following loop
		while(temp->next!=NULL)
		{
			temp=temp->next;
		}
		temp->next=(struct npheap_node *)kmalloc(sizeof(struct npheap_node),GFP_KERNEL);
		temp->next->object_no=vma->vm_pgoff;
		temp->next->pfn=PFN;
		temp->next->size=vma->vm_end-vma->vm_start;
		temp->next->next=NULL;
	    }		 

	    //Now do the mapping
	    if(remap_pfn_range(vma, vma->vm_start, PFN, vma->vm_end - vma->vm_start, vma->vm_page_prot))
            	return -EAGAIN;
            else
            {
	    	//printk("Successfully mapped \n\n\n");
	    	return 0;
	    }
	   
	}
 }

int npheap_init(void)
{
    int ret;
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return ret;
}

void npheap_exit(void)
{	
	struct npheap_node *temp=(struct npheap_node*)HEAD;
	struct npheap_node *prev=(struct npheap_node*)HEAD;
	while(temp->next!=NULL){
	prev=temp;
	temp=temp->next;
	kfree(prev);	
	}
	//kfree(HEAD);    
	misc_deregister(&npheap_dev);
}
 
