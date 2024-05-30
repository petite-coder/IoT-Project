#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>              // POSIX 운영체제 API에 대한 액세스 제공
#include<fcntl.h>               // 타겟시스템 입출력 장치 관련
#include<sys/types.h>           // 시스템에서 사용하는 자료형 정보
#include<sys/ioctl.h>           // 하드웨어의 제어와 상태 정보
#include<sys/stat.h>            // 파일의 상태에 대한 정보

// 디바이스 드라이버 경로
#define dip "/dev/dipsw"        // Dip Switch

// int getDipSW(unsigned char *input);

// int dipSw;

int main()
{
    unsigned char c;
    int dip_d;
    
    dip_d = open(dip, O_RDWR);

    if(dip_d < 0) {
        perror("dip open error");
        exit(1);
    }

    while(1){
        while(1){
            read(dip_d, &c, sizeof(c));
            if(c)
                break;
        }

        printf("%d\n", c);  // dip 작동값 출력
        return 0;
    }

}
/*
Dip Switch 작동값
1번: 1
2번: 2
3번: 4
4번: 8
5번: 16
6번: 32
7번: 64
8번: 128
*/
    
    
