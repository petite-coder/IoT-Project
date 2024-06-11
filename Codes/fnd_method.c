#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<string.h>
#include<time.h>

#define fnd "/dev/fnd"

int fnds;
unsigned char fnd_num[4] = {0,};
unsigned char Time_Table[]={~0x3f, ~0x06, ~0x5b, ~0x4f, ~0x66, ~0x6d, ~0x7d, ~0x07, ~0x7f, ~0x67, ~0x00};
int timer_active;
int minutes;
int seconds;
void update_timer();
void start_timer();
void stop_timer();
void show_final_time();

int main(){
    fnds = open(fnd, O_RDWR);
    if(fnds < 0){printf("Can't open FND.\n"); exit(0);}

    start_timer();

    while (minutes < 100) {
        update_timer();        
    }

    close(fnds);
    return 0;
}

void start_timer() {
    timer_active = 1;
    minutes = 0;
    seconds = 0;
}

void update_timer() {
    if (!timer_active) return;

    sleep(1);

    seconds++;

    if (seconds == 60) {
        seconds = 0;
        minutes++;
    }

    if (minutes >= 100) {
        timer_active = 0;
    }
}

void stop_timer() {
    timer_active = 0;
}

void show_final_time() {
    fnds = open(fnd, O_RDWR);
    if(fnds < 0) {
        perror("FND open error");
        exit(1);
    }

    fnd_num[0] = Time_Table[minutes / 10];
    fnd_num[1] = Time_Table[minutes % 10];
    fnd_num[2] = Time_Table[seconds / 10];
    fnd_num[3] = Time_Table[seconds % 10];

    write(fnds, fnd_num, sizeof(fnd_num));
    close(fnds);
}