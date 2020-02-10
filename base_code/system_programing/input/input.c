
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEVICES_NUM 10
#define MAX_LENTH 256
#define INPUT_FILE "/proc/bus/input/devices"
#define NAME_LEN 40
#define HANDLER_LEN 7

typedef struct usb_dev {
    char name[NAME_LEN];
    char handler[HANDLER_LEN];
} usb_dev_t;

typedef struct usb_dev_handler {
    char handler[HANDLER_LEN];
} usb_dev_handler_t;

int usb_dev_search(char *path);
int usb_dev_parsing(char *str, usb_dev_handler_t *handler);

usb_dev_t usb_devices[DEVICES_NUM];
usb_dev_handler_t handler[2];

int main(void)
{
    int line;
    printf("%ld\n",sizeof(usb_devices));
    memset(usb_devices,0,sizeof(usb_devices));
    memset(handler,0,sizeof(handler));
    line = usb_dev_search(INPUT_FILE);
    printf("line : %d\n", line);
    usb_dev_parsing("HDA", handler);
    // printf("%s\n", usb_dev_parsing("HDA Intel Front Headphone"));
    exit(0);
}
 
int usb_dev_search(char *path)
{
    int num = 0;
    int usb_dev_num = 0;
    char *start;
    char buffer[MAX_LENTH];
    
    memset(usb_devices,0,sizeof(usb_devices));

    FILE *pf = fopen(path,"r");
    if (pf == NULL) {
        printf("%s file open file\n", path);
    }

    while ((fgets(buffer, MAX_LENTH, pf) != NULL)) {
        if ((start = strstr(buffer, "Name=")) != NULL) {
            if(strlen(usb_devices[usb_dev_num].name) == 0) {
                memcpy(usb_devices[usb_dev_num].name, start+5, NAME_LEN-1);
                usb_devices[usb_dev_num].handler[NAME_LEN-1] = '\0';
                printf("usb_devices[%d].name = %s", usb_dev_num, usb_devices[usb_dev_num].name);
            }
        }

        if ((start = strstr(buffer, "Handlers=")) != NULL) {
            if ((start = strstr(buffer, "event")) != NULL) {
                if(strlen(usb_devices[usb_dev_num].handler) == 0) {
                    memcpy(usb_devices[usb_dev_num].handler, start, HANDLER_LEN-1);
                    usb_devices[usb_dev_num].handler[HANDLER_LEN-1] = '\0';
                    printf("usb_devices[%d].handler = %s\n", usb_dev_num, usb_devices[usb_dev_num].handler);
                }
            }
        }
        if((strlen(usb_devices[usb_dev_num].name) != 0) && (strlen(usb_devices[usb_dev_num].handler) != 0)) {
            usb_dev_num++;
            if (usb_dev_num > DEVICES_NUM) {
                printf("usb_dev_num > DEVICES_NUM");
                goto ret;
            }
        }
        num++;
    }

ret:
    printf("close %s\n", INPUT_FILE);
    fclose(pf);
    return num;
}

int usb_dev_parsing(char *str, usb_dev_handler_t *handler)
{
    int i = 0;
    int h = 0;
    for (i = 0; i < DEVICES_NUM; i++) {
        printf("%s", usb_devices[i].name);
        if (strstr(usb_devices[i].name, str) != NULL) {
            printf("%s\n", usb_devices[i].handler);
            // return usb_devices[i].handler;
            memcpy(handler[h].handler, usb_devices[i].handler,sizeof(usb_devices[i].handler));
            printf("handler[%d].handler = %s, len: %ld\n", h, handler[h].handler, sizeof(usb_devices[i].handler));
            h++;
            if(h >= 2)
                return h;
        }
    }
    return h;
}

