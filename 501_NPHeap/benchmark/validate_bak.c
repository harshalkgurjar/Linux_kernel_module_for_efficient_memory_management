#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <npheap.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int i=0,number_of_threads = 1, number_of_objects=1024; 
    int tid;
    __u64 size;
    __u64 object_id;
    __u64 current_time;
    char data[8192],op,*mapped_data;
    char **obj;
    int devfd;
    int error = 0;
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s number_of_objects\n",argv[0]);
        exit(1);
    }
    number_of_objects = atoi(argv[1]);
    obj = (char **)malloc(number_of_objects*sizeof(char *));
    for(i = 0; i < number_of_objects; i++)
    {
        obj[i] = (char *)calloc(8192, sizeof(char));
    }
    // Replay the log
    // Validate
    while(scanf("%c %d %llu %llu %llu %s",&op, &tid, &current_time, &object_id, &size, &data[0])!=EOF)
    {
        if(op == 'S')
        {
            strcpy(obj[(int)object_id],data);
            memset(data,0,8192);
        } else if (op == 'G') {
            if (strcmp(obj[(int)object_id], data)) {   
                fprintf(stderr, "%d: Key %d has a wrong value %s v.s. %s\n",tid,(int)object_id,data,obj[(int)object_id]);
                error++; 
            }
        }
        else if (op == 'D') {
            memset(obj[(int)object_id],0,8192);
        }
        if (error > 5) {
            break;
        }
    }
    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0)
    {
        fprintf(stderr, "Device open failed");
        exit(1);
    }
    for(i = 0; i < number_of_objects; i++)
    {
        mapped_data = (char *)npheap_alloc(devfd,i,8192);
        if(strcmp(mapped_data,obj[i])!=0)
        {
            fprintf(stderr, "Object %d has a wrong value %s v.s. %s\n",i,mapped_data,obj[i]);
            error++;
        }
	else
	  {
	    fprintf(stderr,"Passed\n");
	  }
    }
    if(error == 0)
        fprintf(stderr,"Pass\n");
    close(devfd);
    return 0;
}

