# H-Smart4412를 이용한 암호 해독(CRYPTOGRAM) 게임 - 한경대 IoT 프로그래밍 팀 프로젝트

- Cryptogram - 컴퓨터 또는 중요한 파일에 대한 액세스를 제한하기 위해 사용자가 식별 코드로 입력하는 고유 문자열

</br>

**5조**   

팀장: 오규원

팀원: 민병수, 이예은, 이준희   

</br>
</br>

## 게임 설명
+ 알고리즘  
  + Flow Chart
 
    <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/%ED%94%8C%EB%A1%9C%EC%9A%B0%EC%B0%A8%ED%8A%B8.png"/>
 
+ 게임 방법
  + LCD에 3초간 주제 출력 후, 랜덤으로 선택되고 암호화된 문제 출력되며 타이머 시작
 
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/game%20start.gif"/>

  + <문제 풀이 모드>

      ✔ Dip Switch가 모두 꺼져있는 상태
    
      ✔ FND에 남아있는 암호를 문제에 표시된 순서대로 출력
    
          - 두 번 이상 포함된 암호는 처음 한번만 출력
 
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/select%20number.gif"/>
    
      ✔ Tact Switch를 이용해 해석할 암호 선택
    
          -  Tact 4번: 이전 암호
    
          -  Tact 5번: 선택
    
          -  Tact 6번: 다음 암호
    
      ✔ 선택한 암호를 어떤 알파벳으로 변환할 지 선택
    
          - DOT Matrix에 현재 진행 상태에 없는 알파벳을 사전순으로 표시

      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/select%20Alpha1.gif"/>
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/select%20Alpha.gif"/>
    
      ✔ 암호 선택과 마찬가지 방법으로 Tact Switch를 이용해 선택

      ✔ 모든 답을 입력했을 때 정답 여부 확인
    
         - 정답일 경우, 3초간 점수 출력 후 문제를 해결하는 데 걸린 시간 LCD에 출력
    
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/ending.gif"/>
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/play%20time.gif"/>
      
         - 오답일 경우, Dip Switch 5번을 통해 수정하라는 안내 문구 출력, LED ON
    
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/wrong%20answer.gif"/>
    
  + <주제 다시 보기>
 
      ✔ Dip Switch 1번

      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/f_dip1.gif"/>

  + <힌트 보기>
  
      ✔ Dip Switch 2번
    
      ✔ 점수 -5점 차감

      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/f_dip2.gif"/>

  + <암호 공개 모드>
  
      ✔ Dip Switch 3번
    
      ✔ 한 암호를 공개할 때마다 점수 -5점 차감
    
      ✔ 문제 풀이 모드와 같은 방식으로 FND에 남아있는 암호 출력
    
      ✔ Tact Switch를 이용해 공개할 암호 선택
    
      ✔ 공개된 암호가 반영된 문제가 LCD에 표시
 
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/f_dip3.gif"/>
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/open%20a%20character.gif"/>
    
      ✔ 더 이상 공개할 암호가 없으면 정답 여부를 확인
    
         - 정답일 경우, 3초간 점수 출력 후 문제를 해결하는 데 걸린 시간이 LCD에 출력
    
         - 오답일 경우, Dip Switch 5번을 통해 수정하라는 안내 문구 출력, LED ON

  + <수정 모드>

      ✔ Dip Switch 5번
 
      ✔ 문제 풀이 모드에서 선택했던 알파벳을 다시 암호로 복구 가능
    
      ✔ Tact Switch를 이용해 복구할 알파벳 선택

          - 암호 공개 모드에서 공개된 알파벳과 기존 문제에 포함된 알파벳은 출력X
 
      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/recover1.gif"/>
    
      ✔ 더 이상 복구할 수 있는 알파벳이 없으면 LCD에 "NO CHARACTERS TO RECOVER" 안내 문구 출력

      <img src="https://github.com/IoT-programing-team-5/IoT-Project/blob/main/report/img/recover.gif"/>    

## **센서 및 장치**

1. Character LCD - 주제, 문제, 점수
2. Tact Switch - 알파벳, 암호 선택
3. Dot Matrix - 알파벳 선택
4. Dip Switch - 힌트 출력
5. FND - 암호 출력
6. LED - 수정 dip 스위치 안내

+ [최종 코드](https://github.com/IoT-programing-team-5/IoT-Project/blob/main/Codes/iot_project.c)
 
+ [시연 영상](https://youtu.be/ZJcun-a4ZXE?si=bPZ5OPqYbS7Te_jF)

  [![Video Label](http://img.youtube.com/vi/ZJcun-a4ZXE/0.jpg)](https://youtu.be/ZJcun-a4ZXE)



