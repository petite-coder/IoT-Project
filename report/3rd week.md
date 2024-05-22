# 3주차 보고서
</br>

**5조**   

팀장: 오규원

팀원: 민병수, 이예은, 이준희   
</br>
발표일: 2024년 5월 23일

발표자: 오규원
</br>
</br>

## 2주차 활동 내용
+ 호스트 컴퓨터와 타겟 시스템 연결
+ 랜덤으로 퍼즐을 생성하고 암호화 하는 임시 코드 작성 및 테스트
  + 이 코드를 바탕으로 암호화 코드를 추가 작성 및 수정할 예정 
  
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define ALPHABET_SIZE 26
#define MAX_PUZZLE_LENGTH 256

const char ALPHABET[ALPHABET_SIZE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

typedef struct {
    char puzzle[MAX_PUZZLE_LENGTH];
    char hashed_puzzle[MAX_PUZZLE_LENGTH];
    char guessed_puzzle[MAX_PUZZLE_LENGTH];
    char alpha_to_hash[ALPHABET_SIZE];
    char hash_to_alpha[ALPHABET_SIZE];
    char alpha_to_guesses[ALPHABET_SIZE];
} Puzzle;

void init_puzzle(Puzzle* p) {
    memset(p->puzzle, 0, MAX_PUZZLE_LENGTH);
    memset(p->hashed_puzzle, 0, MAX_PUZZLE_LENGTH);
    memset(p->guessed_puzzle, 0, MAX_PUZZLE_LENGTH);
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        p->alpha_to_hash[i] = 0;
        p->hash_to_alpha[i] = 0;
        p->alpha_to_guesses[i] = '_';
    }
}

void select_puzzle(Puzzle* p) {
    const char* puzzles[] = {
        "HELLO WORLD",
        "C PROGRAMMING",
        "PUZZLE GAME",
        "OPENAI GPT"
    };
    int num_puzzles = sizeof(puzzles) / sizeof(puzzles[0]);
    srand(time(NULL));
    strncpy(p->puzzle, puzzles[rand() % num_puzzles], MAX_PUZZLE_LENGTH - 1);
}

void shuffle_alphabet(char* alphabet) {
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        int j = rand() % ALPHABET_SIZE;
        char temp = alphabet[i];
        alphabet[i] = alphabet[j];
        alphabet[j] = temp;
    }
}

void hash_puzzle(Puzzle* p) {
    char shuffled_alphabet[ALPHABET_SIZE + 1];
    strcpy(shuffled_alphabet, ALPHABET);
    shuffle_alphabet(shuffled_alphabet);
    
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        p->alpha_to_hash[ALPHABET[i] - 'A'] = shuffled_alphabet[i];
        p->hash_to_alpha[shuffled_alphabet[i] - 'A'] = ALPHABET[i];
    }
    
    for (int i = 0; i < strlen(p->puzzle); ++i) {
        char letter = toupper(p->puzzle[i]);
        if (isalpha(letter)) {
            p->hashed_puzzle[i] = p->alpha_to_hash[letter - 'A'];
        } else {
            p->hashed_puzzle[i] = letter;
        }
    }
}

void update_guesses(Puzzle* p, char alpha, char guess) {
    p->alpha_to_guesses[toupper(alpha) - 'A'] = toupper(guess);
}

int guess_already_used(Puzzle* p, char guess) {
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (p->alpha_to_guesses[i] == toupper(guess)) {
            return 1;
        }
    }
    return 0;
}

int letter_in_hash(Puzzle* p, char letter_to_replace) {
    for (int i = 0; i < strlen(p->hashed_puzzle); ++i) {
        if (p->hashed_puzzle[i] == toupper(letter_to_replace)) {
            return 1;
        }
    }
    return 0;
}

void update_guessed_puzzle(Puzzle* p) {
    for (int i = 0; i < strlen(p->hashed_puzzle); ++i) {
        char letter = p->hashed_puzzle[i];
        if (isalpha(letter)) {
            p->guessed_puzzle[i] = p->alpha_to_guesses[letter - 'A'];
        } else {
            p->guessed_puzzle[i] = letter;
        }
    }
}

int puzzle_matches_key(Puzzle* p) {
    for (int i = 0; i < strlen(p->hashed_puzzle); ++i) {
        char letter = p->hashed_puzzle[i];
        if (isalpha(letter) && p->alpha_to_guesses[letter - 'A'] != p->hash_to_alpha[letter - 'A']) {
            return 0;
        }
    }
    return 1;
}

void display_puzzle(const char* puzzle) {
    for (int i = 0; i < strlen(puzzle); ++i) {
        if (puzzle[i] == ' ') {
            printf("  ");
        } else {
            printf("%c ", puzzle[i]);
        }
    }
    printf("\n");
}

int main() {
    Puzzle puzzle;
    init_puzzle(&puzzle);
    
    select_puzzle(&puzzle);
    printf("Original Puzzle:\n");
    display_puzzle(puzzle.puzzle);
    
    hash_puzzle(&puzzle);
    printf("Hashed Puzzle:\n");
    display_puzzle(puzzle.hashed_puzzle);
    
    // Example update guesses
    update_guesses(&puzzle, 'H', 'P');
    update_guesses(&puzzle, 'E', 'Q');
    
    update_guessed_puzzle(&puzzle);
    printf("Guessed Puzzle:\n");
    display_puzzle(puzzle.guessed_puzzle);
    
    if (puzzle_matches_key(&puzzle)) {
        printf("Puzzle matches the key!\n");
    } else {
        printf("Puzzle does not match the key.\n");
    }

    return 0;
}

```
</br>

## 변경 사항   
+ 제한 시간 기능을 문제를 푸는 데 걸린 시간 측정으로 변경   
+ 점수 기능 도입
</br>

## 기능 구체화   
<img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/target%20board.png" width="400" height="500"/>  

1. CLCD   
  + 게임을 시작하면 문제의 주제 3초간 출력   
  + 크립토그램 문제 출력   
  + 문제를 해결하면 점수 출력   
2. Dip Switch   
  + 첫 번째 스위치: 주제 다시 보기(점수 차감 없이 다시 볼 수 있음)   
  + 두 번째 스위치: 힌트 제공   
  + 세 번째 스위치: 한 암호의 답을 공개하는 모드로 진입   
    + Tact 스위치로 알고 싶은 암호 선택 > 공개   
3. FND   
  + 변환할 암호 출력   
    + Tact 스위치로 선택   
  + 문제를 해결하면 문제를 푸는 데 걸린 시간 출력   
4. Dot Matrix   
  + 입력할 문자 출력    
    + Tact 스위치로 선택    
    + 이미 변환된 문자는 표시되지 않음   
5. Tact Switch   
  + 3개의 스위치를 이용하여 이전 문자, 입력, 다음 문자로 지정해 문자 입력
