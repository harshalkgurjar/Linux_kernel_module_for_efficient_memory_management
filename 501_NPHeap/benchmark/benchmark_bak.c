#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <npheap.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int i=0,number_of_processes = 1, number_of_objects=1024, max_size_of_objects = 8192 ,j; 
    int a;
    int pid;
    int size;
    char data[8192];
    char filename[256];
    char *mapped_data;
    int devfd;
    unsigned long long msec_time;
    FILE *fp;
    struct timeval current_time;
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s number_of_objects max_size_of_objects number_of_processes\n",argv[0]);
        exit(1);
    }
    number_of_objects = atoi(argv[1]);
    max_size_of_objects = atoi(argv[2]);
    number_of_processes = atoi(argv[3]);
    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0)
    {
        fprintf(stderr, "Device open failed");
        exit(1);
    }
    srand((int)time(NULL)+(int)getpid());
    // Writing to objects
    //uncomment this
     pid=getpid();
    for(i=0;i<(number_of_processes-1) && pid != 0;i++)
    {
      pid=fork();
    }
    sprintf(filename,"npheap.%d.log",(int)getpid());
    fp = fopen(filename,"w");
    //testing
   /* mapped_data = (char *)npheap_alloc(devfd,0,12);
    mapped_data = (char *)npheap_alloc(devfd,1,12);
    mapped_data = (char *)npheap_alloc(devfd,2,12);
    mapped_data = (char *)npheap_alloc(devfd,3,12);
    mapped_data = (char *)npheap_alloc(devfd,2,12);*/
    for(i = 0; i < number_of_objects; i++)
    {
        npheap_lock(devfd,i);
        do 
        {
            size = max_size_of_objects;//rand() % max_size_of_objects;
        }
        while(size ==0);
        mapped_data = (char *)npheap_alloc(devfd,i,size);
//	printf("Size requested is %d \n",size);
//	printf("%p this is mapped data\n",mapped_data);
	if(!mapped_data)
        {
	  fprintf(stderr,"Failed in npheap_alloc()\n");
	  exit(1);
        }
//        memset(mapped_data, 0, 4096);
        a = rand()+1;
        gettimeofday(&current_time, NULL);
        for(j=strlen(mapped_data); j < size-10; j=strlen(mapped_data))
        {
            sprintf(mapped_data,"%s%d",mapped_data,a);
        }
        fprintf(fp,"S\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
	
	npheap_unlock(devfd,i);
    }
/*    
    // try delete something
    i = rand()%256;
    npheap_lock(devfd,i);
    npheap_delete(devfd,i);
    fprintf(fp,"D\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
    npheap_unlock(devfd,i);*/
    npheap_alloc(devfd,0,size);
    npheap_getsize(devfd,1);
    npheap_alloc(devfd,3,20);
    printf("value returned is %d \n", npheap_getsize(devfd,3));
    fclose(fp);
    close(devfd);
    if(pid != 0)
        wait(NULL);
    return 0;
}

