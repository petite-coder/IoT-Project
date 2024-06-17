# IoT 프로그래밍 최종 발표 보고서
</br>

**5조**   

팀장: 오규원

팀원: 민병수, 이예은, 이준희   
</br>
발표일: 2024년 6월 18일

발표자: 민병수
</br>
</br>

## 게임 설명
+ 알고리즘  
  + Flow Chart
 
    <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/%ED%94%8C%EB%A1%9C%EC%9A%B0%EC%B0%A8%ED%8A%B8.png"/>
 
+ 게임 방법
  + LCD에 3초간 주제 출력 후, 랜덤으로 선택되고 암호화된 문제 출력되며 타이머 시작
 
    <img src="https://github.com/IoT-programing-team-5/IoT-Project/assets/156408029/1ec1c98b-a4d5-4ac5-91ff-44b41179001d"/>

  + <문제 풀이 모드>

      ✔ Dip Switch가 모두 꺼져있는 상태
    
      ✔ FND에 남아있는 암호를 문제에 표시된 순서대로 출력
    
          - 두 번 이상 포함된 암호는 처음 한번만 출력
    
      ✔ Tact Switch를 이용해 해석할 암호 선택
    
          -  Tact 4번: 이전 암호
    
          -  Tact 5번: 선택
    
          -  Tact 6번: 다음 암호
    
      ✔ 선택한 암호를 어떤 알파벳으로 변환할 지 선택
    
      ✔ DOT Matrix에 현재 진행 상태에 없는 알파벳을 사전순으로 표시
    
      ✔ 암호 선택과 마찬가지 방법으로 Tact Switch를 이용해 선택

      ✔ 모든 답을 입력했을 때 정답 여부 확인
    
         - 정답일 경우, 3초간 점수 출력 후 문제를 해결하는 데 걸린 시간 LCD에 출력
    
         - 오답일 경우, Dip Switch 5번을 통해 수정하라는 안내 문구 출력, LED ON
    
  + <주제 다시 보기>
 
      ✔ Dip Switch 1번

  + <힌트 보기>
  
      ✔ Dip Switch 2번
    
      ✔ 점수 -5점 차감

  + <암호 공개 모드>
  
      ✔ Dip Switch 3번
    
      ✔ 한 암호를 공개할 때마다 점수 -5점 차감
    
      ✔ 문제 풀이 모드와 같은 방식으로 FND에 남아있는 암호 출력
    
      ✔ Tact Switch를 이용해 공개할 암호 선택
    
      ✔ 공개된 암호가 반영된 문제가 LCD에 표시
    
      ✔ 더 이상 공개할 암호가 없으면 정답 여부를 확인
    
         - 정답일 경우, 3초간 점수 출력 후 문제를 해결하는 데 걸린 시간이 LCD에 출력
    
         - 오답일 경우, Dip Switch 5번을 통해 수정하라는 안내 문구 출력, LED ON

  + <수정 모드>

      ✔ Dip Switch 5번
 
      ✔ 문제 풀이 모드에서 선택했던 알파벳을 다시 암호로 복구 가능
    
      ✔ Tact Switch를 이용해 복구할 알파벳 선택

          - 암호 공개 모드에서 공개된 알파벳과 기존 문제에 포함된 알파벳은 출력X
    
      ✔ 더 이상 복구할 수 있는 알파벳이 없으면 LCD에 "NO CHARACTERS TO RECOVER" 안내 문구 출력
    

+ 구현 기능
  + 독창성
 
    + While문을 사용해서 Dip Switch 상태를 계속 확인해서 Dip Switch가 언제 켜지든 반응을 할 수 있다.

    + Dip Switch를 활용해서 다양한 기능을 구현했다.


 
+ [시연 영상](https://youtu.be/ZJcun-a4ZXE?si=bPZ5OPqYbS7Te_jF)

## 핵심 코드
 
## 참고 자료
