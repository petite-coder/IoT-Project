#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>              // POSIX 운영체제 API에 대한 액세스 제공
#include<fcntl.h>               // 타겟시스템 입출력 장치 관련
#include<sys/types.h>           // 시스템에서 사용하는 자료형 정보
#include<sys/ioctl.h>           // 하드웨어의 제어와 상태 정보
#include<sys/stat.h>            // 파일의 상태에 대한 정보
#include <linux/input.h>
#include <time.h>
#include <stdbool.h>

// 디바이스 드라이버 경로
#define dip "/dev/dipsw"                // Dip Switch
#define lcd "/dev/clcd"                 // Character LCD
#define tact "/dev/tactsw"              // Tact Switch
#define fnd "/dev/fnd"                  // FND
#define dot "/dev/dot"                  // Dip Switch
#define led "/dev/led"

// Tact Switch 정의
#define TACT_4 4
#define TACT_5 5
#define TACT_6 6

// FND 숫자 정의
#define ONE 0xF9
#define TWO 0xA4
#define THREE 0xB0
#define FOUR 0x99
#define FIVE 0x92
#define SIX 0x82
#define SEVEN 0xF8
#define EIGHT 0x80
#define NINE 0x90
#define ZERO 0xC0
#define DOT 0b01111111

//  1 	  2 	3     4	    5	  6     7      8 	9     0     .
// 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xC0, 0x80

// DOT 알파벳 정의
unsigned char alphabet[26][8] = {
    {0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42}, // A
    {0x3c, 0x22, 0x22, 0x3c, 0x22, 0x22, 0x22, 0x3c}, // B
    {0x1C, 0x22, 0x20, 0x20, 0x20, 0x20, 0x22, 0x1C}, // C
    {0x38, 0x44, 0x42, 0x42, 0x42, 0x42, 0x44, 0x38}, // D
    {0x3E, 0x20, 0x20, 0x3E, 0x20, 0x20, 0x20, 0x3E}, // E
    {0x3E, 0x20, 0x20, 0x3E, 0x20, 0x20, 0x20, 0x20}, // F
    {0x1C, 0x22, 0x42, 0x40, 0x40, 0x47, 0x42, 0x3C}, // G
    {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x42}, // H
    {0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C}, // I
    {0x1C, 0x08, 0x08, 0x08, 0x08, 0x48, 0x48, 0x30}, // J
    {0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x44}, // K
    {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E}, // L
    {0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81}, // M
    {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x42}, // N
    {0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C}, // O
    {0x7C, 0x42, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40}, // P
    {0x38, 0x44, 0x82, 0x82, 0x82, 0x8A, 0x44, 0x3A}, // Q
    {0x7C, 0x42, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42}, // R
    {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x02, 0x42, 0x3C}, // S
    {0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // T
    {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, // U
    {0x81, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x18}, // V
    {0x81, 0x99, 0x99, 0x99, 0x99, 0x99, 0x5A, 0x24}, // W
    {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}, // X
    {0x81, 0x81, 0x42, 0x42, 0x3C, 0x18, 0x18, 0x18}, // Y
    {0xFF, 0x03, 0x02, 0x04, 0x08, 0x10, 0x60, 0xFF}  // Z
};

#define ALPHABET_COUNT 26

int dipSw, clcd, tactSw, fnds, dotMat, leds;
unsigned char fnd_data[11]; // 최대 10개(0~9)의 암호 저장
char openedAlpha[11];
int opened = 0;
char *openStat;     // 암호 공개 모드로 공개된 알파벳이 반영된 암호화 문장
int counter = 0;  // 카운터 변수 추가
int score;
time_t start_time, end_time;
double elapsed_time;

unsigned char TT[8] = {                 
    // 오답 DOT 패턴(T_T)
    0b00000000, 0b11100111, 0b01000010, 0b01000010,
    0b00000000, 0b00000000, 0b00111100, 0b00000000
};
unsigned char reveal_pattern[8] = {     
    // 암호 공개 모드 DOT 패턴(열쇠)
    0b00111000, 0b01001000, 0b10001000, 0b10010001,
    0b11101010, 0b00000101, 0b00000010, 0b00000001
};
unsigned char restore_pattern[8] = {    
    // 수정 모드 DOT 패턴(뒤로가기)
    0b00011100, 0b10100010, 0b11000001, 0b11100001,
    0b00000001, 0b01000001, 0b00100010, 0b00011100
};

typedef struct cryptogram {
    char sent[100];         // 문제
    char topic[50];         // 주제
    char hint[100];         // 힌트
    char encrypted[100];    // 첫 암호화 상태
} cryptogram;

char* encrypt(const char *message);
void handle_dip_switch(unsigned char dipNum, cryptogram select, char *newStat);
void open_mode(char *newStat, cryptogram select);
void edit_mode(char *newStat, cryptogram select);
void stop_timer();
void game_completed();
void handle_correct_answer();
void handle_incorrect_answer(const char *newStat, unsigned char predip, char *mode);
unsigned char* toFnd(char *chNum);
char *replace_char(char *str, char find, char replace);
char* whichN(char *numb);
char* reveal(char *numb, char num, cryptogram q);
unsigned char** getAlphabet(char* current);
char dotChar(unsigned char *newAlphabet);
void compare_strings(const char *str2, char *result, cryptogram q);
unsigned char** convert(const char* input);
void restore_characters(char *newStat, char selectedAlpha, cryptogram select);
void updateState(char **newStat, char *getArr, int fndIdx, int dotIdx);
bool check_dip(unsigned char predip);
int tactFnd(char* rest, char* newStat, unsigned char preDip, int state, cryptogram s);   // state > 0: FND 조작, 1: Dot Matrix 조작
void init_dev(const char *dev);
void close_devices();
void clearFnd();
void display_fnd(int index);
void get_tact(unsigned char *tact_num);
int printLcd(char *str);
void displayOnDotMatrix(int fd, unsigned char* pattern);    // 도트 매트릭스에 문자 표시
void clearDotMatrix();
void free_dot_patterns(unsigned char** result, int size);
void displayElapsedTime();
int led_write(unsigned char led_data);

/* ----------------- Main Start ----------------- */

int main(){
    /* 랜덤 문제 선택 */
    char* source[30][3] = {
        {"TOPIC : SONG", "QUEEN", "BOHEMIAN RHAPSODY"},
        {"TOPIC : SONG", "QUEEN", "WE ARE THE CHAMPIONS"},
        {"TOPIC : SONG", "QUEEN", "WE WILL ROCK YOU"},
        {"TOPIC : SONG", "TWICE", "DANCE THE NIGHT AWAY"},
        {"TOPIC : SONG", "TWICE", "I CANT STOP ME"},
        {"TOPIC : SONG", "TWICE", "WHAT IS LOVE"},
        {"TOPIC : SONG", "FROZEN", "DO YOU WANT TO BUILD A SNOWMAN"},
        {"TOPIC : SONG", "FROZEN", "LOVE IS AN OPEN DOOR"},
        {"TOPIC : SONG", "FROZEN", "FOR THE FIRST TIME IN FOREVER"},
        {"TOPIC : SONG", "COCO", "REMEMBER ME"},
        {"TOPIC : MOVIE", "ROBOT", "TRANSFOMERS"},
        {"TOPIC : MOVIE", "MARVEL", "AVENGERS"},
        {"TOPIC : MOVIE", "FANTASY", "THE LORD OF THE RINGS"},
        {"TOPIC : MOVIE", "CAR", "THE FAST AND THE FURIOUS"},
        {"TOPIC : MOVIE", "BARDEM", "NO COUNTRY FOR OLD MEN"},
        {"TOPIC : MOVIE", "JIM CARREY", "ETERNAL SUNSHINE OF THE SPOTLESS MIND"},
        {"TOPIC : MOVIE", "JOHNNY DEPP", "PIRATES OF THE CARIBBEAN"},
        {"TOPIC : MOVIE", "TOM HARDY", "MAD MAX FURY ROAD"},
        {"TOPIC : MOVIE", "BATMAN", "DARK KNIGHT"},
        {"TOPIC : MOVIE", "SUPERMAN", "MAN OF STEAL"},
    };

    srand(time(NULL));
    int num = sizeof(source) / sizeof(source[0]);
    int idx = rand() % num;

    cryptogram select;
    strcpy(select.topic, source[idx][0]);
    strcpy(select.hint, source[idx][1]);
    strcpy(select.sent, source[idx][2]);

    printf("Selected text: %s\n", select.sent);
    printf("Selected topic: %s\n", select.topic);

    char* stat = encrypt(select.sent);        // 암호화
    strcpy(select.encrypted, stat);
    openStat = select.encrypted;
            
    printf("Encrypted message: %s\n", select.encrypted);   // 디버깅용
    char* newStat = stat;

    /* --- GAME START --- */

    unsigned char dipNum;
    unsigned char preDip = 0xFF;
    unsigned char led_data;
    char msg[33] = "";
    int state;
    int time_check = 0;

    init_dev("dip");

    // LCD > 3초간 주제 출력
    strncpy(msg, select.topic, sizeof(msg) - 1);
    printLcd(msg);
    sleep(3);

    while(1){
        read(dipSw, &dipNum, sizeof(dipNum));
        printf("%d\n", dipNum);  // dip 작동값 출력
        
        if (dipNum == 0){
            /* 문제 풀이 */
            if (preDip != 0) {
                printf("Dip switch is off\n");
                preDip = 0; // 이전 DIP 스위치 상태를 갱신
            }

            // LCD > 문제 출력
            printf("Decrypt Mode\n");
            strncpy(msg, newStat, sizeof(msg) - 1); // 문제 출력
            printLcd(msg);

            if(time_check ==  0){
                start_time = time(NULL);
                printf("Timer Start\n");
                time_check++;
            }

            // 모든 답이 맞았는지 확인
            if (strcmp(newStat, select.sent) == 0) {
                stop_timer();
                score = 100 - counter;
                game_completed();
                close_devices();
                free(newStat);
                return 0;
            }
            
            while(1){   // 문제 풀이 모드 진입
                char* getArr = whichN(newStat);   // 남은 암호(중복, 공백X) for FND + fnd_data setting
                printf("Decrypt getArr: %s\n", getArr);
                
                // TACT & FND > TACT로 조종, FND에 해석할 암호 출력 및 선택
                state = 0; 
                int fndIdx = tactFnd(getArr, newStat, dipNum, state, select);  
                printf("fndIdx: %d\n", fndIdx);   // 디버깅용

                if(fndIdx == -1) {  // 문제 풀이 모드 종료 > 힌트 모드로 진입
                    printf("End Decrypt Mode\n");
                    free(getArr);
                    read(dipSw, &dipNum, sizeof(dipNum));
                    break;  
                }

                if(fndIdx == -2){   // 남은 암호가 없을 때 답이 맞았는지 확인
                    if (strcmp(newStat, select.sent) == 0) {    // 정답
                        handle_correct_answer();
                        
                        close_devices();
                        free(getArr); 
                        free(newStat); 
                        return 0;
                    } else {    // 오답
                        handle_incorrect_answer(newStat, dipNum, "Decrypt");
                        free(getArr);
                        break;
                    }
                }else{
                    usleep(300000);

                    // TACT & DOT > TACT로 조종, DOT에 변환할 알파벳 출력 및 선택
                    state = 1;
                    int dotIdx = tactFnd(getArr, newStat, dipNum, state, select); 
                    printf("dotIdx: %d\n", dotIdx);   // 디버깅용

                    if(dotIdx == -1) {  // 문제 풀이 모드 종료 > 힌트 모드로 진입
                        printf("End Decrypt Mode\n");
                        free(getArr);
                        read(dipSw, &dipNum, sizeof(dipNum));
                        break;  
                    }
                    updateState(&newStat, getArr, fndIdx, dotIdx);
                    free(getArr);
                }

                if(check_dip(dipNum))   break;

                usleep(500000);
            }
        } else if(dipNum != preDip){
            /* Dip > 힌트 선택 */
            handle_dip_switch(dipNum, select, newStat);
            preDip = dipNum; 
        }   
        usleep(500000);
    }

    free(newStat);
    close_devices();

    return 0;
}

/* ----------------- Main End ----------------- */

/*  -------- Operate function --------  */

char* encrypt(const char *message){ 
    /* 암호화 */
    int len = strlen(message);
    char unique_alphabets[ALPHABET_COUNT] = {0};
    int unique_count = 0;
    int mapping[ALPHABET_COUNT] = {0};
    int used_numbers[10] = {0};
    int i;

    // 문장 안의 고유 알파벳 찾기
    for (i = 0; i < len; i++) {
        char ch = tolower(message[i]);
        if (isalpha(ch) && strchr(unique_alphabets, ch) == NULL) {
            unique_alphabets[unique_count++] = ch;
        }
    }

    int encrypt_count;
    if (unique_count > 11) {
        encrypt_count = 10;
    } else {
        encrypt_count = unique_count - 2;
    }

    for (i = 0; i < ALPHABET_COUNT; i++) {
        mapping[i] = -1;
    }

    int encrypted_count = 0;

    while (encrypted_count < encrypt_count) {
        int index = rand() % unique_count;
        char ch = unique_alphabets[index];
        if (mapping[ch - 'a'] == -1) {
            int random_number;
            do {
                random_number = rand() % 10;
            } while (used_numbers[random_number]);
            mapping[ch - 'a'] = random_number;
            used_numbers[random_number] = 1;
            encrypted_count++;
        }
    }

    // 암호화
    char *encrypted_message = (char *)malloc(len + 1);
    if (!encrypted_message) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < len; i++) {
        char ch = tolower(message[i]);
        if (isalpha(ch) && mapping[ch - 'a'] != -1) {
            encrypted_message[i] = mapping[ch - 'a'] + '0';
        } else {
            encrypted_message[i] = message[i];
        }
    }

    encrypted_message[len] = '\0';
    return encrypted_message;
}

void handle_dip_switch(unsigned char dipNum, cryptogram select, char *newStat) {
    char msg[33] = "";

    switch (dipNum) {
        case 1: /* LCD > 주제 다시 보기 */
            printf("Review Topic\n");
            strncpy(msg, select.topic, sizeof(msg) - 1);
            printLcd(msg);
            break;
        case 2: /* LCD > 힌트 보기 */
            printf("View Hint\n");
            strncpy(msg, select.hint, sizeof(msg) - 1);
            printLcd(msg);
            counter += 5;
            break;
        case 4: /* 암호 공개 모드 */
            printf("PW Open Mode\n");
            open_mode(newStat, select);
            break;
        case 16:/* 수정 모드 */
            printf("Edit Mode\n");
            edit_mode(newStat, select);
            break;
    }
}

void open_mode(char *newStat, cryptogram select) {  // 암호 공개 모드
    init_dev("dot");
    displayOnDotMatrix(dotMat, reveal_pattern);
    sleep(1);
    close(dotMat);
    unsigned char dipNum;
    char msg[33] = "";

    read(dipSw, &dipNum, sizeof(dipNum));

    while (1) { // 암호 공개 모드 진입
        char* getArr = whichN(newStat); // 남은 암호
        printf("PW Open getArr: %s\n", getArr);

        if (getArr[0] == '\0' && strcmp(newStat, select.sent) == 0) {   // 모든 암호가 공개됨 + 정답
            stop_timer();
            score = 100 - counter;  
            printf("All codes have been revealed\n");

            init_dev("dot");
            clearDotMatrix();
            close(dotMat);   

            game_completed(elapsed_time, score);

            free(getArr);
            free(newStat);
            close_devices();
            return;
        } else if (getArr[0]=='\0') { // 암호는 모두 공개됐으나, 답이 아님
            handle_incorrect_answer(newStat, dipNum, "PW Open");
            free(getArr);
        } else {
            int state = 0;
            int rNum = tactFnd(getArr, newStat, dipNum, state, select);   // 인덱스 받음
            printf("rNum: %d\n", rNum); 

            if(rNum == -1) { // Dip 변경됨 
                printf("End PW Open Mode\n");
                init_dev("dot");
                clearDotMatrix();
                close(dotMat);
                free(getArr);
                break;  // 암호 공개 모드 종료
            } else {
                printf("%c Open\n", getArr[rNum]);
                char* updatedStat = reveal(newStat, getArr[rNum], select);
                strncpy(newStat, updatedStat, strlen(updatedStat) + 1); 
                printf("newStat: %s\n", newStat);   // 디버깅용

                strncpy(msg, newStat, sizeof(msg) - 1);
                printLcd(msg);

                free(getArr);
                free(updatedStat);
            }
        }

        if(check_dip(dipNum))   break;

        usleep(500000);
    }
}

void edit_mode(char *newStat, cryptogram select) {  // 수정 모드
    unsigned char dipNum;
    char msg[33] = "";

    read(dipSw, &dipNum, sizeof(dipNum));

    init_dev("dot");
    displayOnDotMatrix(dotMat, restore_pattern);
    sleep(1);
    close(dotMat);

    while (1) { // 수정 모드 진입
        char* getArr = whichN(newStat); // 남은 암호(중복, 공백X)
        printf("Edit getArr: %s\n", getArr);

        if (strcmp(newStat, openStat) == 0) {   // 바꾼 암호가 없을 때
            init_dev("dot");
            clearDotMatrix();
            close(dotMat);
            printf("There are no characters to recover.\n");

            // LCD > 안내
            strncpy(msg, "NO CHARACTERS TO RECOVER", sizeof(msg) - 1);
            printLcd(msg);

            break;
        } else {
            int state = 2;  // Dot Matrix 조작 + 수정 모드
            int dotIdx = tactFnd(getArr, newStat, dipNum, state, select);
            printf("dotIdx: %d\n", dotIdx);

            if(dotIdx == -2){
                init_dev("dot");
                clearDotMatrix();
                close(dotMat);
                continue;
            }

            if (dotIdx == -1) {
                printf("End Edit Mode\n");
                init_dev("dot");
                clearDotMatrix();
                close(dotMat);
                free(getArr);
                break;  // 수정 모드 종료
            } else {
                char result[100];
                compare_strings(newStat, result, select);
                printf("Restore %c\n", result[dotIdx]); // 복구할 알파벳

                restore_characters(newStat, result[dotIdx], select);
                printf("Edited newStat: %s\n", newStat);

                strncpy(msg, newStat, sizeof(msg) - 1);
                printLcd(msg);
            }
        }

        if(check_dip(dipNum)){
            free(getArr);
            break;
        }
        usleep(500000);
    }
}

void stop_timer() {
    end_time = time(NULL);
    elapsed_time = difftime(end_time, start_time);
}

void game_completed() {
    printf("Game completed in %.2f seconds.\n", elapsed_time);
    char msg[33];
    sprintf(msg, "SCORE: %d\nTHX FOR PLAYING", score);
    printLcd(msg);  // 점수 출력
    sleep(5);

    displayElapsedTime(elapsed_time); // 경과 시간 출력
    sleep(5); 
}

void handle_correct_answer() {
    stop_timer();
    score = 100 - counter; 
    game_completed();
    close_devices();
}

void handle_incorrect_answer(const char *newStat, unsigned char predip, char *mode) {
    char msg[33];
    strncpy(msg, "EDIT YOUR CODE WITH DIP5", sizeof(msg) - 1);
    printLcd(msg);
    close(clcd);

    init_dev("dot");
    write(dotMat, &TT, sizeof(TT));
    sleep(1);
    clearDotMatrix();
    close(dotMat);

    unsigned char led_data = 0xef;
    led_write(led_data);
    close(leds);

    strncpy(msg, newStat, sizeof(msg) - 1);
    printLcd(msg);

    while(1) {
        if(check_dip(predip))   break;
    }
}

unsigned char* toFnd(char *chNum){
    // 입력: 남아있는 암호 문자열 e.g. '325671'
    int len = strlen(chNum);
    int i;
    unsigned char *trans = (unsigned char*)malloc(len * sizeof(unsigned char));

    for(i=0; i<len; i++){
        switch (chNum[i])
        {
            case '0': trans[i] = ZERO; break;
            case '1': trans[i] = ONE; break;
            case '2': trans[i] = TWO; break;
            case '3': trans[i] = THREE; break;
            case '4': trans[i] = FOUR; break;
            case '5': trans[i] = FIVE; break;
            case '6': trans[i] = SIX; break;
            case '7': trans[i] = SEVEN; break;
            case '8': trans[i] = EIGHT; break;
            case '9': trans[i] = NINE; break;
        }
    }

    return trans;
}

char *replace_char(char *str, char find, char replace) {     
    // 선택한 암호를 답으로 replace
    // str에서 모든 find를 찾아 replace로 변환
    int i;
    int len = strlen(str);
    char *new_str = (char*)malloc(len + 1);
    if (!new_str) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < len; i++) {
        if (str[i] == find) {
            new_str[i] = replace;
        } else {
            new_str[i] = str[i];
        }
    }

    new_str[len] = '\0';
    return new_str;
}

char* whichN(char *numb){  
    // 남아있는 암호를 공백, 중복 제거해 fnd로 변환
    // numb: 진행 상태
    int i, j = 0;
    
    int len = strlen(numb);
    char *arr = (char*)malloc(32 * sizeof(char));
    if (!arr) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    memset(arr, 0, 32);  // 배열을 초기화
    
    for (i = 0, j = 0; i < len; i++) {
        if (isdigit(numb[i]) && strchr(arr, numb[i]) == NULL) {
            arr[j++] = numb[i];
            arr[j] = '\0';
        }
    }
    
    printf("whichN arr: %s\n", arr);

    unsigned char *fndD = toFnd(arr);

    strcpy(fnd_data,fndD);
    free(fndD);

    return arr;
}

char* reveal(char *numb, char num, cryptogram q) {
    // numb: 진행 상태, num: 변환할 암호, q: 문제
    int t;
    int len = strlen(numb);
    char* newNumb = strdup(numb); // 원본 문자열을 복사하여 수정할 수 있는 메모리 영역에 저장

    for (t = 0; t < len; t++) {
        if (numb[t] == num) {
            newNumb[t] = q.sent[t];
            if(strchr(openedAlpha, newNumb[t]) == NULL){
                openedAlpha[opened] = newNumb[t];
                opened++;
            }
            openStat[t] = q.sent[t];
        }
    }

    openedAlpha[opened] = '\0';
    counter += 5;
    printf("openStat: %s\n", openStat);
    printf("reveal> newNumb: %s\n", newNumb);
    return newNumb;
}

unsigned char** getAlphabet(char* current){
    // 암호화된 문장에 이미 공개되어있는 알파벳 제외 dot화
    // current: 진행 상태
    int i, j = 0;

    unsigned char** newAlphabet = (unsigned char**)malloc(26 * sizeof(unsigned char*));
    if (!newAlphabet) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    printf("getAlphabet:");
    for(i=0; i<26; i++){
        if(strchr(current, 'A'+i) == NULL && strchr(current, 'a'+i) == NULL){  // 대소문자 모두 고려
            printf("%c", 'A'+i);    // 디버깅용

            newAlphabet[j] = (unsigned char*)malloc(8 * sizeof(unsigned char));
            if (!newAlphabet[j]) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }
            memcpy(newAlphabet[j], alphabet[i], 8);
            j++;
        }
    }
    printf(" > dot\n");
    for (; j < 26; j++) {  // 남은 공간을 NULL로 채웁니다.
        newAlphabet[j] = NULL;
    }
    return newAlphabet;
}

char dotChar(unsigned char* newAlphabet){
    // 입력받은 dot 패턴을 char 문자로 변환
    char alpha;
    int i;

    for (i = 0; i < 26; i++) {
        if (memcmp(alphabet[i], newAlphabet, 8) == 0) {
            alpha = 'A' + i;
            break;
        }
    }

    return alpha;
}

void compare_strings(const char *str2, char *result, cryptogram q) {
    // str1: 첫 암호화 문장  |  str2: 풀이 중인 문장
    // 두 문장을 비교해 이미 변환된 문자를 찾아냄
    // 암호 모드에서 공개된 알파벳 제외
    const char *str1 = q.encrypted;
    int max_len = strlen(str1);
    int i, j = 0;

    memset(result, 0, sizeof(result));
 
    printf("compare_strings > openedAlpha: %s\n", openedAlpha);
    printf("compare_strings > result1: %s\n", result);

    for (i = 0; i < max_len; i++) {
        if(str1[i] != str2[i] && isalpha(str2[i]) &&  (strchr(result, str2[i]) == NULL)){
            if (strchr(openedAlpha, str2[i]) == NULL || str2[i] != q.sent[i]) {
                result[j++] = str2[i];
            }
        }
    }
    result[j] = '\0';
    printf("compare > result2: %s\n", result);
}


unsigned char** convert(const char* input) {
    // 문자열을 dot 패턴으로 변환
    int len = strlen(input);
    unsigned char** result = (unsigned char**)malloc((len + 1) * sizeof(unsigned char*));
    int i;

    for (i = 0; i < len; i++) {
        unsigned char* pattern = NULL;
        if (input[i] >= 'A' && input[i] <= 'Z') {
            pattern = alphabet[input[i] - 'A'];
        } else if (input[i] >= 'a' && input[i] <= 'z') {
            pattern = alphabet[input[i] - 'a'];
        }

        if (pattern != NULL) {
            result[i] = (unsigned char*)malloc(8 * sizeof(unsigned char));
            memcpy(result[i], pattern, 8);
        } else {
            result[i] = NULL; // 알파벳이 아닌 경우 NULL로 설정
        }
    }
    result[len] = NULL; // 배열의 끝을 표시하기 위해 NULL 추가
    return result;
}

void restore_characters(char *newStat, char selectedAlpha, cryptogram select) {
    int i;

    for (i = 0; i < strlen(newStat); i++) {
        if (newStat[i] == selectedAlpha) {
            newStat[i] = select.encrypted[i];   // 원래의 숫자 암호로 복구
        }
    }
}

void updateState(char **newStat, char *getArr, int fndIdx, int dotIdx) {
    // 진행 상태 갱신
    unsigned char* newAlphabet = getAlphabet(*newStat)[dotIdx];
    char charDot = dotChar(newAlphabet);

    printf("Decrypt: %c > %c\n", getArr[fndIdx], charDot);
    char* updatedStat = replace_char(*newStat, getArr[fndIdx], charDot);
    free(*newStat);
    *newStat = updatedStat;
    printf("newStat: %s\n", *newStat);

    char msg[33];
    strncpy(msg, *newStat, sizeof(msg) - 1);
    printLcd(msg);

    clearFnd();
}

bool check_dip(unsigned char predip){
    unsigned char dipNum;
    read(dipSw, &dipNum, sizeof(dipNum));

    if(predip != dipNum){
        if (predip == 0){
            printf("End Descrypt Mode\n");
            clearFnd();
            return true;
        } else if (predip == 4 || predip == 16){
            init_dev("dot");
            clearDotMatrix();
            close(dotMat);
            clearFnd();
            printf("End %s Mode\n", (predip == 4) ? "PW Open" : "Edit");
            return true;
        }
    }
    return false;
}

/*  -------- IO function --------  */ 
int tactFnd(char* rest, char* newStat, unsigned char preDip, int state, cryptogram s){ 
    // rest: 남은 암호   |   newStat: 진행 중인 문제   |   preDip: 실행 전 dip 상태   |    state: 모드 결정   |   s: 문제 구조체
    // <state> 0: FND+Tact   |   1: Dot + Tact   |   2: 수정모드 Dot + Tact
    unsigned char tact_num;
    unsigned char dipNum;
    int current_index = 0;
    int len = strlen(rest);
    unsigned char** newAlphabet = NULL;  // 초기화;
    char result[100];
    int check = 0;
    
    printf("tactFnd > rest: %s, predip: %02x, state: %d\n", rest, preDip, state);

    if(len == 0 && state != 2){
        printf("Error: 'rest' has zero length\n");
        return -2;
    }

    if(state == 1 || state == 2){
        // Dot Matrix 초기 설정
        init_dev("dot");
        usleep(150000);
    }

    // Tact Switch 초기 설정
    init_dev("tact");
    
    if(state == 0){
        // FND 초기 설정
        init_dev("fnd");
        display_fnd(current_index);
        check = len - 1;
    } else if(state == 1){
        newAlphabet = getAlphabet(newStat); // 남은 암호에 없는 알파벳의 dot 패턴
        for (check = 0; check < 26; check++) {
            if (newAlphabet[check] == NULL) {
                break;
            }
        }
        check--; // 마지막 인덱스: 유효한 알파벳의 수 - 1
        displayOnDotMatrix(dotMat, newAlphabet[current_index]);  // 첫 알파벳을 즉시 표시
    } else if(state == 2){
        memset(result, 0, sizeof(result));
        compare_strings(newStat, result, s);
        printf("tactFnd > result: %s\n", result);
        if(result[0]=='\0'){
            printf("Error: 'result' has zero length\n");
            clearDotMatrix();
            close(dotMat);
            return -2;
        }
        newAlphabet = convert(result);      // 이미 변환된 알파벳의 dot 패턴

        check = strlen(result)-1;
        displayOnDotMatrix(dotMat, newAlphabet[current_index]);  // 첫 알파벳을 즉시 표시
    }

    printf("tactFnd check: %d\n", check);

    // dot matrix 상태 출력
    if ((state == 1 || state == 2) && newAlphabet != NULL) {
        displayOnDotMatrix(dotMat, newAlphabet[current_index]);  // 첫 알파벳을 즉시 표시
    }

    while(1) {
        get_tact(&tact_num);
        read(dipSw, &dipNum, sizeof(dipNum)); // DIP 상태 확인

        if (dipNum != preDip) {   // DIP 스위치 상태가 변경되었을 때
            close(tactSw);
            if (state == 0) {
                close(fnds);
            } else if (dotMat >= 0) {
                close(dotMat);
            }
            if (newAlphabet) free_dot_patterns(newAlphabet, check + 1);

            printf("tactFnd > changed dipNum - dipNum: %02x, preDip: %02x\n", dipNum, preDip);
            return -1;
        }

        switch(tact_num) {
            // tact를 조종해 선택
            case TACT_4: // 이전 숫자 or 알파벳
                current_index = (current_index == 0) ? check : current_index - 1;
                
                if(state == 0){
                    display_fnd(current_index);
                } else if (state == 1 || state == 2) {
                    displayOnDotMatrix(dotMat, newAlphabet[current_index]);
                }
                
                break;

            case TACT_5: // 현재 숫자 or 알파벳 선택
                if(state == 0){
                    printf("Selected FND Index: %d\n", current_index);
                    clearFnd();
                    close(fnds);
                } else if (state == 1 || state == 2){
                    printf("Selected DOT Index: %d\n", current_index);
                    clearDotMatrix();
                    close(dotMat);
                }
                
                close(tactSw);
                if (newAlphabet) free_dot_patterns(newAlphabet, check + 1);

                return current_index;

            case TACT_6: // 다음 숫자 or 알파벳
                current_index = (current_index == check) ? 0 : current_index + 1;
                if(state == 0){
                    display_fnd(current_index);
                } else if (state == 1 || state == 2)
                {
                    displayOnDotMatrix(dotMat, newAlphabet[current_index]);
                }
                break;
            
            default:
                break;
        }

        usleep(300000); // 0.3초 대기
    }
    close(tactSw);
    if (newAlphabet) free_dot_patterns(newAlphabet, check + 1);
}

void init_dev(const char *dev) {
    if (strcmp(dev, "dip") == 0) {
        dipSw = open(dip, O_RDWR);
        if (dipSw < 0) {
            perror("dip open error");
            exit(1);
        }
    } else if (strcmp(dev, "fnd") == 0) {
        fnds = open(fnd, O_RDWR);
        if (fnds < 0) {
            perror("fnd open error");
            exit(1);
        }
    } else if (strcmp(dev, "lcd") == 0) {
        clcd = open(lcd, O_RDWR);
        if (clcd < 0) {
            perror("lcd open error");
            exit(1);
        }
    } else if (strcmp(dev, "led") == 0) {
        leds = open(led, O_RDWR);
        if (leds < 0) {
            perror("led open error");
            exit(1);
        }
    } else if (strcmp(dev, "dot") == 0) {
        dotMat = open(dot, O_RDWR);
        if (dotMat < 0) {
            perror("dot open error");
            exit(1);
        }
    } else if (strcmp(dev, "tact") == 0) {
        tactSw = open(tact, O_RDWR);
        if (tactSw < 0) {
            perror("tact open error");
            exit(1);
        }
    } else {
        fprintf(stderr, "Unknown device: %s\n", dev);
        exit(1);
    }
}

void close_devices() {
    if (fnds >= 0) {
        close(fnds);
        fnds = -1;  // 파일 디스크립터를 닫은 후 유효하지 않은 값으로 설정
    }
    if (clcd >= 0) {
        close(clcd);
        clcd = -1;
    }
    if (dipSw >= 0) {
        close(dipSw);
        dipSw = -1;
    }
    if (leds >= 0) {
        close(leds);
        leds = -1;
    }
    if (dotMat >= 0) {
        close(dotMat);
        dotMat = -1;
    }
}

void clearFnd(){
    init_dev("fnd");
    unsigned char buf[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // FND 데이터 준비
    write(fnds, buf, sizeof(buf)); // FND에 데이터 쓰기
    usleep(150000);
    close(fnds);
}

void display_fnd(int index) {
    unsigned char buf[4] = {fnd_data[index], 0xFF, 0xFF, 0xFF}; // FND 데이터 준비
    write(fnds, buf, sizeof(buf)); // FND에 데이터 쓰기
}

void get_tact(unsigned char *tact_num) {
    read(tactSw, tact_num, sizeof(*tact_num));
    usleep(100000);
}

int printLcd(char *str){
    char buffer[33]; // 최대 32문자 (16자 x 2줄) + NULL
    char *s;
    int i = 0;

    memset(buffer, ' ', sizeof(buffer));
    buffer[32] = '\0'; 

    // handling '\n'
    for (s = str; *s; s++) {
        if (*s == '\n') {
            // 첫줄 남은 부분 공백으로 채움
            while (i < 16) {
                buffer[i++] = ' ';
            }
            i = 16;
            continue;
        }
        if (i >= 32) {
            break; // buffer overflow 방지
        }
        buffer[i++] = *s;
    }

    buffer[i] = '\0';

    init_dev("lcd");
    // Clear Lcd
    char clear_command = 0x01;
    if (write(clcd, &clear_command, 1) < 0) {
        perror("Failed to clear CLCD");
        close(clcd);
        exit(1);
    }

    usleep(150000); // Lcd에 clear가 적용될 시간을 줌
    
    // Lcd에 입력
    if (write(clcd, buffer, sizeof(buffer)) < 0) {
        perror("Failed to write message to CLCD");
        close(clcd);
        exit(1);
    }
    
    usleep(150000);
    return 0;
}

void displayOnDotMatrix(int fd, unsigned char* pattern) {
    static unsigned char current_pattern[8] = {0};  // 현재 표시 중인 패턴을 저장
    unsigned char clear_pattern[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // 클리어 패턴
    
    if (memcmp(current_pattern, pattern, 8) != 0) { // 패턴이 변경되었을 때만 업데이트
        // 클리어 패턴 쓰기
        if (write(fd, clear_pattern, sizeof(clear_pattern)) < 0) {
            perror("Dot matrix clear error");
            exit(1);
        }
        usleep(10000);  // 잠깐 대기

        // 새로운 패턴을 쓰기
        if (write(fd, pattern, sizeof(alphabet[0])) < 0) {
            perror("Dot matrix write error");
            exit(1);
        }
        memcpy(current_pattern, pattern, 8); // 현재 패턴을 저장
    }
}

void clearDotMatrix() {
    if (dotMat >= 0) {
        unsigned char clear_pattern[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        if (write(dotMat, clear_pattern, sizeof(clear_pattern)) < 0) {
            perror("Dot matrix clear error");
            close(dotMat);
            exit(1);
        }
        usleep(10000);
    }
}

void free_dot_patterns(unsigned char** result, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (result[i] != NULL) {
            free(result[i]);
        }
    }
    free(result);
}

void displayElapsedTime() {
    unsigned int total_seconds = (unsigned int)elapsed_time;
    unsigned int minutes = total_seconds / 60;
    unsigned int seconds = total_seconds % 60;
    
    unsigned char buf[33];

    init_dev("lcd");
    snprintf(buf, sizeof(buf), "PLAY TIME: \n%02uMIN %02uSEC", minutes, seconds);
    printLcd(buf);
    close(clcd);

}

int led_write(unsigned char led_data){
    init_dev("led");

    // on LED
    if (write(leds, &led_data, sizeof(unsigned char)) < 0) {
        perror("Failed to turn on the LED");
        close(leds);
        exit(1);
    }
    
    sleep(2);
    return 0;
}