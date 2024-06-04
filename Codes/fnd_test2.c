#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define fnd "/dev/fnd"

unsigned char fnd_num[4] = {0,};
int fnds;
unsigned char Time_Table[] = {
    ~0x3f, ~0x06, ~0x5b, ~0x4f, ~0x66, 
    ~0x6d, ~0x7d, ~0x07, ~0x7f, ~0x67, 
    ~0x00
};  // 7-segment 숫자 코드 테이블 (0-9 및 빈칸)
unsigned char Dot = 0x80;  // 도트 비트 (마지막 비트)

int main() {
    fnds = open(fnd, O_RDWR);
    if (fnds < 0) {
        printf("Can't open FND.\n");
        exit(0);
    }

    int minutes = 0;
    int seconds = 0;

    while (minutes < 100) {
        fnd_num[0] = Time_Table[minutes / 10];
        fnd_num[1] = Time_Table[minutes % 10] | Dot;  // 도트 추가
        fnd_num[2] = Time_Table[seconds / 10];
        fnd_num[3] = Time_Table[seconds % 10];

        write(fnds, fnd_num, sizeof(fnd_num));

        sleep(1);

        seconds++;
        if (seconds == 60) {
            seconds = 0;
            minutes++;
        }
    }

    close(fnds);
    return 0;
}
