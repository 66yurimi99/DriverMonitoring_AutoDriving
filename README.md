# DriverMonitoring_AutoDriving
[Intel] 엣지 AI SW 아카데미 2기 - 팀 프로젝트 : 운전자 상태 모니터링 및 자율주행 시스템

* 프로젝트 기간: 2023.10.12-2023.11.10
* 프로젝트 지도 교수: 노상수 교수님
* 프로젝트 수행 팀:
  * Driver's Drowsiness Detection: 정인성, 나준희
  * Car Detection: 박영문, 유나영
  * Lane Detection: 조영식, 이유림

프로젝트 소개
---
자율주행 시대의 도래함에 따라 차량은 단순히 운전자에게 운송수단을 넘어 새로운 가치를 제공하려 하고 있다. 

운전자에게 제공할 편의성을 확대하기 위해 실내 영상 제어를 통해 운전자의 상태 파악하여 졸음 상태라 판단되면 자동으로 차선 인식과 HUD에 세부 정보를 제공하는 모드로 넘어가는 자율주행 “님아 그 눈을 감지 마오” 시스템을 제안한다.

프로젝트 배경
---

<img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/978d3f75-9765-4157-84fa-57123db496e2 alt="2" style="width: 320px; height: 240px;"> | <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/f380734e-a15b-4204-8f39-48387e142ac3 alt="1" style="width: 320px; height: 240px;">
--- | --- |

 한국도로공사에 따르면 고속도로 교통사고 원인별 사망자 현황 1위는 졸음 및 주시 태만으로 대략 70%의 비율을 보였다. 따라서, 졸음 운전에 대한 대처가 미흡하다는 것을 알 수 있다. 
 
 본 시스템은 운전자의 졸음 및 상태를 파악하여 졸음 운전이라 판단되면 자동으로 자율 주행 모드로 넘어가서 운전자에게는 편리함과 안전성을 제공하고 사고의 발생률을 낮추기 위해 개발하게 되었다.

기대효과
---
* 졸음 운전에 대한 운전자의 스트레스를 해소하고 편리함과 안전성의 가치를 제공할 수 있습니다.
* 교통 사고 발생률 1위인 졸음 운전에 대한 사고를 현저하게 줄일 수 있습니다.

구성도
---
* 시스템 구성도
  
  <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/50bf5c79-a821-4c8a-b02e-2f65cd83c449 alt="2" style="width: 840px; height: 360px;">
  
* SW 구성도
  
  <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/71be7783-a3a0-4c25-924c-6055c4750b65 alt="1" style="width: 840px; height: 480px;">

프로젝트 기능
---
- 실시간 얼굴 및 눈 인식 : Gamma Transformation을 적용한 전처리와 Dlib의 학습모델인 get_frontal_face_detector()를 활용하여 실시간으로 사람 얼굴을 인식 후, 눈을 인식함
- 졸음 여부 판단 기능 : 눈에 찍힌 랜드마크 좌표값을 활용하여 EAR알고리즘을 계산 후, 비율 값이 임계값보다 작으면 감았다 판단, 일정시간이 넘어가면 졸음이라 판단
- 알림 서비스 : HTTP프로토콜을 이용한 통신으로 실시간 운전자 정보 확인
- 차선 인식 시스템 : ROI 설정 후, ROI를 Top View로 변환과 전처리로 차선을 검출하고 주행 방향을 실시간으로 HUD에 투영
- 안전거리 유지 : YoloV7을 활용해 차들을 인식하고 앞차와의 거리 판단 후 속도 조절 및 방향 전환 유도

프로젝트 결과
---
* 운전자가 졸지 않을 때,

   <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/5a601673-92bb-437a-9a09-0d47f5eb6c7f alt="2" style="width: 320px; height: 240px;"> | <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/843399f3-45a3-4c8b-b19c-732549024644 style="width: 320px; height: 240px;">
  --- | --- |

* 운전자가 잔다고 판단될 때, 자율 주행(안전할 때) 

  <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/94097b46-328d-46e2-8ce3-e2314f86fc4d alt="2" style="width: 320px; height: 240px;"> | <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/58150f4e-bf16-45e7-a494-70757275f979 alt="1" style="width: 320px; height: 240px;">
  --- | --- |

* 운전자가 잔다고 판단될 떄, 자율 주행(위험할 때)

  <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/94097b46-328d-46e2-8ce3-e2314f86fc4d alt="2" style="width: 320px; height: 240px;"> | <img src=https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/86766617/fe9122d4-7e4d-4973-bd30-6b4599528073 alt="1" style="width: 320px; height: 240px;">
  --- | --- |




