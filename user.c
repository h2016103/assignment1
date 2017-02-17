
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>          /* ioctl */
#define MAJOR_NUM 'r'
#define DEVICE_FILE_NAME "/dev/trng_dev"
#define DEVICE_NAME "trng_dev"
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)

/*
 * Functions for the ioctl calls
 */



int ioctl_get_nth_byte(int file_desc, int *number)
{

  
    char c;

    c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, number);

     
    return 0;
}

/*
 * Main - Call the ioctl functions
 */
int main()
{
    int file_desc;
    int number[3];
     number[2] = 0;
   
   
    file_desc = open(DEVICE_FILE_NAME, 0);
    if (file_desc < 0) {
        printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        exit(-1);
    }
    printf("please enter lower range : ");
    scanf("%d",&number[0]);
    printf("please enter upper range : ");
    scanf("%d",&number[1]);
   
    
    ioctl_get_nth_byte(file_desc , number);
    printf("\nrandom no generated :  %d",number[2]); 
    printf("\n");
    return 0;
}
