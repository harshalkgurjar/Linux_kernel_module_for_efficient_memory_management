
///////////////////////////////////////////////////////////////////////

// Project 2 :  Darshit Pandit, dupandit; Harshal Gurjar, hkgurjar; Saikrishna Vasudevan, svasude4

//////////////////////////////////////////////////////////////////////



#include <npheap/tnpheap_ioctl.h>
#include <npheap/npheap.h>
#include <npheap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>

struct my_tnpheap
{
    void *mapping;
    void *buffer;
    __u64 version;
    __u64 offset;
    __u64 size;
    struct my_tnpheap *next
};


struct tnpheap_cmd cmd;
struct my_tnpheap *start = NULL;

__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
    __u64 ver;
    struct tnpheap_cmd vercmd;
    vercmd.offset = offset;
    ver = ioctl(tnpheap_dev,TNPHEAP_IOCTL_GET_VERSION,&vercmd);
    return ver;
}



int tnpheap_handler(int sig, siginfo_t *si)
{
    return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
    __u64 ver;
    struct my_tnpheap *temp = start;
    while(temp!=NULL)
    
{
  //printf("searching for offset: current : %d \n",temp->offset);
        if (offset == temp->offset)
	{

            ver = tnpheap_get_version(npheap_dev,tnpheap_dev,offset);
	   // printf("Object %lld already there & Current version of the object is %lld\n\n",offset,ver);
	    temp->version=ver;
	    return temp->buffer;
	}
	else 
	{
	temp=temp->next;
	}
    }

    //printf("Object %ld NOT there, Adding...........\n",offset);
    struct my_tnpheap *temp2 = start;
    struct my_tnpheap *new_node = (struct my_tnpheap*)malloc(sizeof(struct my_tnpheap));
    new_node->mapping = (void *)npheap_alloc(npheap_dev,offset,size);
    new_node->buffer = (void *)malloc(size);
    new_node->version = tnpheap_get_version(npheap_dev,tnpheap_dev,offset);
    new_node->offset = offset;
    new_node->size = size;
    new_node->next = NULL;

    if(temp2==NULL)
    {
        //printf("first object: %ld!!!!!\n",offset);
	start = new_node;
	temp2 = start;
    }
    
    else
    {
        while(temp2->next!=NULL)
        {
            
	    temp2=temp2->next;
        }
	//	printf("Appending to list offset %d\n",offset);
        temp2->next = new_node;
    }

   // printf("returning buffer\n",offset);
    return new_node->buffer;
}

__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
    return ioctl(tnpheap_dev,TNPHEAP_IOCTL_START_TX,&cmd);
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
    // Consider my_tnpheap LL, traverse it one by one to check if the version matches for each object :
    // If NO : return 1
    // IF YES : copy from buffer space to kernel space and return 0
    // While copying , make sure that is atomic
  /*      __u64 version;
    __u64 offset;
    __u64 size;
    void *data;
  */
   // fprintf(stderr,"trying for lock 1\n");
   // fprintf(stderr,"trying for lock 1.2\n");
    struct tnpheap_cmd*  for_lock_commit=     (struct tnpheap_cmd*) malloc(sizeof(struct tnpheap_cmd));
   
   // fprintf(stderr,"trying for lock 1.3\n");
    for_lock_commit->size = 0;  // 0 for lock
    for_lock_commit->version = 0;
    for_lock_commit->offset = 0;
    for_lock_commit->data = NULL;
    
   // fprintf(stderr,"trying for lock 2\n");
    ioctl(tnpheap_dev,TNPHEAP_IOCTL_COMMIT,for_lock_commit);
    
    struct my_tnpheap *temp3 = start;
    //npheap_lock(npheap_dev,0);
    //Check version
    
    char *buf;
    char *map;
   // fprintf(stderr,"trying for lock 3\n");

     
    while(temp3!=NULL)
    {
      if(temp3->version != tnpheap_get_version(npheap_dev,tnpheap_dev,temp3->offset))
      {
//	fprintf(stderr,"aborting since versions don't match \n");
	for_lock_commit->size = 1;
//	fprintf(stderr,"trying for unlock\n");
	ioctl(tnpheap_dev,TNPHEAP_IOCTL_COMMIT,for_lock_commit);

	return 1;
      }
        temp3=temp3->next;
    }

    // Now we are confirmed that version matches for all objects
  //  printf("Versions for all objects are matches, now we commit\n");
    struct my_tnpheap *temp4 = start;

    while(temp4!=NULL)
    {
        buf = temp4->buffer;
	while(temp4->size != npheap_getsize(npheap_dev,temp4->offset))
        {
          //npheap_lock(npheap_dev,temp4->offset);
          npheap_delete(npheap_dev,temp4->offset);
          temp4->mapping = (void *)npheap_alloc(npheap_dev,temp4->offset,temp4->size);
          temp4->size = npheap_getsize(npheap_dev,temp4->offset);
          //npheap_unlock(npheap_dev,temp4->offset);
        }
    //    printf("Size has matched\n");
//	printf("get size result: %d , size is %d \n ",npheap_getsize(npheap_dev,temp4->offset),temp4->size);
        map = temp4->mapping;
	
	  {
	    memset(map, 0, temp4->size);
	    memcpy(map, buf, temp4->size);
	  }
        //free(buf);
        for_lock_commit->offset = temp4->offset;
        for_lock_commit->size = 2;
        ioctl(tnpheap_dev,TNPHEAP_IOCTL_COMMIT,for_lock_commit);
        temp4=temp4->next;
    }
    //free(new_node);*/
    for_lock_commit->size = 1;
  //  fprintf(stderr,"trying for unlock\n");
    ioctl(tnpheap_dev,TNPHEAP_IOCTL_COMMIT,for_lock_commit);
    return 0;

//return 0;
}
