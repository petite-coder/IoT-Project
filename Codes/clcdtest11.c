#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <time.h>

#define CLCD_DEVICE "/dev/clcd"

void clcd_input(const char *clcd_text) {
    int clcd_fd;
    clcd_fd = open(CLCD_DEVICE, O_RDWR);
    if (clcd_fd < 0) {
        printf("Error opening CLCD device\n");
        return;
    }

    write(clcd_fd, clcd_text, strlen(clcd_text));
    close(clcd_fd);
}

void transform_and_print_clcd_modified(const char *text, const char *selected_topic) {
    char encrypted_text[100]; 

    // 선택된 텍스트에서 중복되지 않는 랜덤 알파벳 5개 선택
    char selected_alphabets[5] = {'\0'};
    int text_len = strlen(text);
    int i, j;
    srand(time(NULL));
    for (i = 0; i < 5; i++) {
        char rand_char;
        do {
            rand_char = text[rand() % text_len];
        } while (strchr(selected_alphabets, rand_char) != NULL);
        selected_alphabets[i] = rand_char;
    }

    // 선택된 알파벳에 0부터 4까지의 숫자 대입
    int assigned_numbers[5];
    for (i = 0; i < 5; i++) {
        assigned_numbers[i] = rand() % 5;
    }

    // 텍스트 암호화
    strcpy(encrypted_text, text);
    for (i = 0; i < text_len; i++) {
        for (j = 0; j < 5; j++) {
            if (text[i] == selected_alphabets[j]) {
                encrypted_text[i] = assigned_numbers[j] + '0';
                break;
            }
        }
    }

    // 선택된 주제 출력 후 대기
    clcd_input(selected_topic);
    sleep(3);

    // 암호화된 텍스트 출력
    clcd_input(encrypted_text);
}

int main() {
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

    transform_and_print_clcd_modified(selected_text, selected_topic);

    return 0;
}

