#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>              // POSIX 운영체제 API에 대한 액세스 제공
#include<fcntl.h>               // 타겟시스템 입출력 장치 관련
#include<sys/types.h>           // 시스템에서 사용하는 자료형 정보
#include<sys/ioctl.h>           // 하드웨어의 제어와 상태 정보
#include<sys/stat.h>            // 파일의 상태에 대한 정보

// 디바이스 드라이버 경로
#define dip "/dev/dipsw"        // Dip Switch
#define lcd "/dev/clcd"         // Character LCD
#define tact "/dev/tactsw"      // Tact Switch
#define fnd "/dev/fnd"          // FND

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

//  1 	  2 	3     4	    5	  6     7      8 	9     0     .
// 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xC0, 0x80

// 암호화
#define ALPHABET_COUNT 26
#define MAX_TITLE_LENGTH 32

int dipSw;
int clcd;
int tactSw;
int fnds;
unsigned char fnd_data[11]; // 최대 10개(0~9)의 암호 저장

typedef struct cryptogram
{
    char *sent;      // 문제
    char *subj;      // 주제
    char *hint;      // 힌트
} cryptogram;

void get_tact(unsigned char *tact_num);
unsigned char* toFnd(char *chNum);
int clearLcd();
int printLcd(char str[]);
void display_fnd(int index);
char* whichN(char *numb);
int tactFnd(char* rest);
char* reveal(char *numb, char num, cryptogram q);
void clearFnd();
void generate_random_mapping(char *unique_alphabets, int unique_count, int *mapping, const char *message);
char* encrypt_message(const char *message, int *mapping);

/* -- Main Start -- */

int main(){
    char *proverbs[] = {
        "A journey of a thousand miles begins with a single step",
        "Actions speak louder than words",
        "A picture is worth a thousand words",
        "Better late than never",
        "Birds of a feather flock together"
    };

    char *song_titles[] = {
        "Here Comes the Sun",
        "Let It Be",
        "Bohemian Rhapsody",
        "Hotel California",
        "Stairway to Heaven"
    };

    char *topics[] = {"Topic: Proverbs", "Topic: Song Titles"};

    int num_proverbs = sizeof(proverbs) / sizeof(proverbs[0]);
    int num_song_titles = sizeof(song_titles) / sizeof(song_titles[0]);
    int num_topics = sizeof(topics) / sizeof(topics[0]);

    srand(time(NULL));
    int selected_topic_idx = rand() % num_topics;
    char *selected_topic = topics[selected_topic_idx];

    char selected_text[100];

    if (selected_topic_idx == 0) {
        strcpy(selected_text, proverbs[rand() % num_proverbs]);
    } else {
        strcpy(selected_text, song_titles[rand() % num_song_titles]);
    }

    printf("Selected text: %s\n", selected_text);
    printf("Selected topic: %s\n", selected_topic);
    
    cryptogram c1 = {selected_text, selected_topic, "Hint"};
    cryptogram select = c1;

    // 암호화 시작

    char unique_alphabets[ALPHABET_COUNT] = {0};
    int unique_count = 0;
    int len = strlen(selected_text);
    int i;

    // Find unique alphabets in the title
    for (i = 0; i < len; i++) {
        char ch = tolower(selected_text[i]);
        if (isalpha(ch) && strchr(unique_alphabets, ch) == NULL) {
            unique_alphabets[unique_count++] = ch;
        }
    }

    int mapping[ALPHABET_COUNT];
    for (i = 0; i < ALPHABET_COUNT; i++) {
        mapping[i] = -1;
    }

    int encrypt_count = (int)((unique_count * 0.7) + 0.5);  // 70% rounding up

    // Generate random mapping for the unique alphabets
    generate_random_mapping(unique_alphabets, encrypt_count, mapping, selected_text);

    // Encrypt the message
    char *encrypted_message = encrypt_message(selected_text, mapping);

    printf("Encrypted message: %s\n", encrypted_message);

    unsigned char dipNum;
    unsigned char preDip = 0xFF;
    char msg[33] = "";
    char *stat = encrypted_message; // dip 작동 전까지의 진행 상태
    char *newStat = stat;
    char *getArr;

    dipSw = open(dip, O_RDWR);
    strncpy(msg, selected_topic, sizeof(msg) - 1);
    printLcd(msg);
    sleep(3);

    if(dipSw < 0) {
        perror("dip open error");
        exit(1);
    }

    while(1){
        
        read(dipSw, &dipNum, sizeof(dipNum));
        printf("%d\n", dipNum);  // dip 작동값 출력
        
        if (dipNum == 0){
            if (preDip != 0) {
                printf("Dip switch is off\n");
                clearLcd();
                strncpy(msg, newStat, sizeof(msg) - 1);
                printLcd(msg);     // 힌트로 변경되면 바꿔야 돼서 나중에 수정해야함
                clearFnd();
                close(fnds);
                preDip = 0; // 이전 DIP 스위치 상태를 갱신
            }
        } else if(dipNum != preDip){
            switch (dipNum){
                case 1: /* 주제 다시 보기 */
                    printf("주제 다시 보기\n");
                    clearLcd();
                    strncpy(msg, select.subj, sizeof(msg) - 1);
                    printLcd(msg);
                    break;

                case 2: /* 힌트 보기 */
                    printf("힌트 보기\n");
                    clearLcd();
                    strncpy(msg, select.hint, sizeof(msg) - 1);
                    printLcd(msg);
                    break;
                
                case 4: /* 암호 공개 모드 */
                    printf("암호 공개 모드\n");

                    while(1){   // 암호 공개 모드 진입
                        getArr = whichN(newStat);   // 남은 암호(중복, 공백X)
                        printf("getArr1: %s\n", getArr);
                        
                        int rNum = tactFnd(getArr);   // 인덱스 받음
                        printf("rNum: %d\n", rNum);   // 디버깅용

                        if(rNum == -1) {
                            printf("암호 공개 모드 종료\n");
                            free(getArr);
                            break;  // 암호 공개 모드 종료
                        } else {
                            printf("getArr2: %c\n", getArr[rNum]);
                            newStat = reveal(newStat, getArr[rNum], select);
                            printf("newStat: %s\n", newStat);   // 디버깅용

                            strncpy(msg, newStat, sizeof(msg) - 1);
                            printLcd(msg);
                        }
                        
                        clearFnd();
                        
                        // DIP 상태 다시 확인
                        read(dipSw, &dipNum, sizeof(dipNum));
                        if (dipNum != 4) {
                            printf("암호 공개 모드 종료\n");
                            free(getArr);
                            clearFnd();
                            close(fnds);
                            break;  // 암호 공개 모드 종료
                        }

                        usleep(500000);
                    }
                    break;
            }
            preDip = dipNum; // 이전 DIP 스위치 상태를 갱신
        }   
        usleep(500000);
    }

    close(fnds);
    close(clcd);
    close(dipSw);

    free(encrypted_message);
    
    return 0;
}

/* -- Main End -- */

/*  Operate function  */
unsigned char* toFnd(char *chNum){
    // 입력: 남아있는 암호 문자열 e.g. '325671'
    int len = strlen(chNum);
    int i;
    unsigned char *trans = (unsigned char*)malloc(len * sizeof(unsigned char));

    for(i=0; i<len; i++){
        switch (chNum[i])
        {
        case '0':
            trans[i] = ZERO;
            break;
        case '1':
            trans[i] = ONE;
            break;
        case '2':
            trans[i] = TWO;
            break;
        case '3':
            trans[i] = THREE;
            break;
        case '4':
            trans[i] = FOUR;
            break;
        case '5':
            trans[i] = FIVE;
            break;
        case '6':
            trans[i] = SIX;
            break;
        case '7':
            trans[i] = SEVEN;
            break;
        case '8':
            trans[i] = EIGHT;
            break;
        case '9':
            trans[i] = NINE;
            break;
        }
    }

    return trans;
}

char *replace_char(char *str, char find, char replace) {     // 선택한 암호를 답으로 replace
    int i;
    char *new_str = strdup(str); // 문자열을 복사하여 새 메모리 영역에 저장
    for (i = 0; new_str[i] != '\0'; i++) {
        if (new_str[i] == find) {
            new_str[i] = replace;
        }
    }

    return new_str;
}

char* whichN(char *numb){  
    // 남아있는 암호를 공백, 중복 제거해 fnd로 변환
    // numb: 진행 상태
    int i;
    int j = 0;
    
    int len = strlen(numb);
    char *arr = (char*)malloc(32 * sizeof(char));
    memset(arr, 0, 32);  // 배열을 초기화
    char a;
    
    for(i=0; i<len; i++){
        a = numb[i];
        printf("a: %c\n", a);

        if((isdigit(a)) && (strchr(arr, numb[i]) == NULL)){   // 공백, 중복 제거, 숫자만 저장
            arr[j] = a;
            j++;
            arr[j] = '\0';
        }
    }
    
    printf("arr: %s\n", arr);

    unsigned char *fndD = toFnd(arr);

    strcpy(fnd_data,fndD);
    // memcpy(fnd_data, fndD, sizeof(fndD)); // fnd_data에 변환된 값 저장
    free(fndD);

    return arr;
}

char* reveal(char *numb, char num, cryptogram q) {
    // numb: 진행 상태, num: 변환할 암호, q: 문제
    int t;
    int len = strlen(numb);
    char* newNumb = strdup(numb); // 원본 문자열을 복사하여 수정할 수 있는 메모리 영역에 저장

    switch(num) {
        case '0':
            for(t=0; t<len; t++){
                if(numb[t] == '0')
                    break;
            }
            newNumb = replace_char(newNumb, '0', q.sent[t]);
            break;
        case '1':
            for(t=0; t<len; t++){
                if(numb[t] == '1')
                    break;
            }
            newNumb = replace_char(newNumb, '1', q.sent[t]);
            break;
        case '2':
            for(t=0; t<len; t++){
                if(numb[t] == '2')
                    break;
            }
            newNumb = replace_char(newNumb, '2', q.sent[t]);
            break;
        case '3':
            for(t=0; t<len; t++){
                if(numb[t] == '3')
                    break;
            }
            newNumb = replace_char(newNumb, '3', q.sent[t]);
            break;
        case '4':
            for(t=0; t<len; t++){
                if(numb[t] == '4')
                    break;
            }
            newNumb = replace_char(newNumb, '4', q.sent[t]);
            break;
        case '5':
            for(t=0; t<len; t++){
                if(numb[t] == '5')
                    break;
            }
            newNumb = replace_char(newNumb, '5', q.sent[t]);
            break;
        case '6':
            for(t=0; t<len; t++){
                if(numb[t] == '6')
                    break;
            }
            newNumb = replace_char(newNumb, '6', q.sent[t]);
            break;
        case '7':
            for(t=0; t<len; t++){
                if(numb[t] == '7')
                    break;
            }
            newNumb = replace_char(newNumb, '7', q.sent[t]);
            break;
        case '8':
            for(t=0; t<len; t++){
                if(numb[t] == '8')
                    break;
            }
            newNumb = replace_char(newNumb, '8', q.sent[t]);
            break;
        case '9':
            for(t=0; t<len; t++){
                if(numb[t] == '9')
                    break;
            }
            newNumb = replace_char(newNumb, '9', q.sent[t]);
            break;
    }

    printf("newNumb: %s\n", newNumb);
    return newNumb;
}

/* 암호화 */
void generate_random_mapping(char *unique_alphabets, int unique_count, int *mapping, const char *message) {
    srand(time(NULL));

    // Make a copy of the message to use with strtok
    char *message_copy = strdup(message);
    if (!message_copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    // Track already encrypted letters
    char encrypted[ALPHABET_COUNT] = {0};
    int encrypted_count = 0;
    int used_numbers[10] = {0}; // Track used numbers

    // Ensure each word has at least one letter encrypted
    char *word = strtok(message_copy, " ");
    while (word != NULL) {
        int len = strlen(word);
        int i;
        for (i = 0; i < len; i++) {
            char ch = tolower(word[i]);
            if (isalpha(ch) && !encrypted[ch - 'a']) {
                int random_number;
                do {
                    random_number = rand() % 10;
                } while (used_numbers[random_number]); // Ensure unique mapping
                mapping[ch - 'a'] = random_number;
                used_numbers[random_number] = 1; // Mark number as used
                encrypted[ch - 'a'] = 1;
                encrypted_count++;
                break;
            }
        }
        word = strtok(NULL, " ");
    }

    // Encrypt additional letters up to the 70% requirement
    while (encrypted_count < unique_count * 0.7) {
        int index = rand() % unique_count;
        char ch = unique_alphabets[index];
        if (!encrypted[ch - 'a']) {
            int random_number;
            do {
                random_number = rand() % 10;
            } while (used_numbers[random_number]); // Ensure unique mapping
            mapping[ch - 'a'] = random_number;
            used_numbers[random_number] = 1; // Mark number as used
            encrypted[ch - 'a'] = 1;
            encrypted_count++;
        }
    }

    free(message_copy);
}

char* encrypt_message(const char *message, int *mapping) {
    int len = strlen(message);
    char *encrypted_message = (char *)malloc(len + 1);
    if (!encrypted_message) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int i;

    for (i=0; i < len; i++) {
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


/*  IO function  */ 
int tactFnd(char* rest){ // 실행 전에 fnd_data 세팅해야함
    unsigned char tact_num;
    unsigned char dipNum;
    int current_index = 0;
    int len = strlen(rest);

    // FND 초기 설정
    fnds = open(fnd, O_RDWR);
    if(fnds < 0) {
        perror("fnd open error");
        exit(1);
    }
    display_fnd(current_index);

    // Tact Switch 초기 설정
    tactSw = open(tact, O_RDWR);
    if(tactSw < 0) {
        perror("tact open error");
        exit(1);
    }

    while(1) {
        get_tact(&tact_num);
        read(dipSw, &dipNum, sizeof(dipNum)); // DIP 상태 확인

        if (dipNum != 4) {
            close(fnds);
            close(tactSw);
            return -1; // 암호 공개 모드 종료 신호
        }

        switch(tact_num) {
            case TACT_4: // 이전 숫자
                current_index = (current_index == 0) ? (len-1) : current_index - 1;
                display_fnd(current_index);
                break;
            case TACT_5: // 현재 숫자 선택, 암호 공개
                printf("Selected Index: %d\n", current_index);
                close(fnds);
                close(tactSw);

                return current_index;
            case TACT_6: // 다음 숫자
                current_index = (current_index == (len-1)) ? 0 : current_index + 1;
                display_fnd(current_index);
                break;
            default:
                break;
        }

        usleep(300000); // 0.5초 대기
    }
    close(fnds);
    close(tactSw);
}

void clearFnd(){
    fnds = open(fnd, O_RDWR);
    unsigned char buf[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // FND 데이터 준비
    write(fnds, buf, sizeof(buf)); // FND에 데이터 쓰기
    close(fnds);
}

void display_fnd(int index) {
    unsigned char buf[4] = {fnd_data[index], 0xFF, 0xFF, 0xFF}; // FND 데이터 준비
    write(fnds, buf, sizeof(buf)); // FND에 데이터 쓰기
}

void get_tact(unsigned char *tact_num) {
    read(tactSw, tact_num, sizeof(*tact_num));
}

int clearLcd(){
    clcd = open(lcd, O_RDWR);
    unsigned char clear[32];
    memset(clear, 0, 32);

    if(clcd < 0){
        perror("dip open error");
        exit(1);
    }

    write(clcd, clear, sizeof(clear));
    close(clcd);
    return 0;
}

int printLcd(char str[]){
    clcd = open(lcd, O_RDWR);

    if(clcd < 0){
        perror("dip open error");
        exit(1);
    }

    write(clcd, str, 32);
    return 0;
}

