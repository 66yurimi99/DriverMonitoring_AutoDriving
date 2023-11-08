# DriverMonitoring_AutoDriving
[Intel] 엣지 AI SW 아카데미 2기 - 팀 프로젝트 : 운전자 상태 모니터링 및 자율주행 시스템

## Integrated Development Enviroment
*   Driver's drowsiness Detection Part
    -  IDE: Visual Studio 2022
    -  Language: C++
    -  OS: Windows
    -  opencv
*   Car Detection Part
    -
    -
## Installation
### Opencv
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/132d8fb1-f6c2-409f-b38d-1fa6e02f2984" width="640" height="480"/>

*  https://opencv.org/releases/

*  OpeCV Release 최신버전 설치 (Visual Studio 2022)
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/06026ca3-5684-4f03-8818-9fe3a58ee754/image.png" width="640" height="480"/>

*  C++ 빈 프로젝트 생성
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/37b38e5d-69a5-4105-8f1f-3a5df844f3e4/image.png" width="640" height="480"/>

*  같은 디렉터리에 배치
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/b2b8a933-d470-4c6e-b2dc-d66b60ab34cc/image.png" width="480" height="320"/>

*  소스파일에 main.cpp 생성
<br/>

<img src=https://velog.velcdn.com/images/lcooldong/post/001a81b7-e5c1-4f39-a7ca-85738159c006/image.png alt="2" style="width: 240px; height: 480px;"> | <img src=https://velog.velcdn.com/images/lcooldong/post/344eb828-9d32-449f-85c6-d6044fdccf35/image.png alt="1" style="width: 240px; height: 480px;">
--- | --- |

*  프로젝트 속성 열기
*  이 후 파일명은 설치 버전에 맞게 변경
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/85b0cfff-78f1-403a-97c2-f568fb783a34/image.png" width="480" height="360"/>
<img src="https://velog.velcdn.com/images/lcooldong/post/a884d7da-2a4e-4685-b7d6-f1892c001d49/image.png" width="480" height="360"/>

*  구성   : 모든 구성
*  플랫폼 : x64
*  C/C++ -> 일반 / 추가 포함 디렉터리에서 opencv\build\include를 포함
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/cd2c5c7f-ca49-43b9-97a8-5d3f15134d87/image.png" width="480" height="360"/>
<img src="https://velog.velcdn.com/images/lcooldong/post/646e68a4-2e54-4431-8bc8-8472080d473e/image.png" width="480" height="360"/>

*  링커 -> 일반 / 추가 라이브러 디렉터리에서 build\x64\vc17\lib를 포함
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/54a395de-ffb8-4bc1-8711-dcbae8e4ffa0/image.png" width="480" height="360"/>
<img src="https://velog.velcdn.com/images/lcooldong/post/c10f0945-3059-4cf9-bb19-0b027eb85459/image.png" width="480" height="360"/>

*  링커 -> 입력 -> 추가 종속 / opencv_world[버전]d.lib를 포함
<br/>
<img src="https://velog.velcdn.com/images/lcooldong/post/216bcc62-790b-4234-849c-e204f4df772f/image.png" width="480" height="360"/>
<img src="https://velog.velcdn.com/images/lcooldong/post/6d267223-4242-4a24-9053-5541212f2f25/image.png" width="480" height="360"/>

*  opencv\build\x64\vc17\bin에 있는 opencv_world[].dll / opencv_world[]d.dll 파일 복사 및 디렉터리에 복사
<br/>
*  참조 : https://velog.io/@lcooldong/C-OpenCV-%EC%84%A4%EC%B9%98

<br/><br/>

### Dlib
*  http://dlib.net/ 홈페이지 접속하여 라이브러리 다운로드
<br/>
*  CMake 다운로드 https://cmake.org/download/ 
<br/>
*  Dlib 다운로드 http://dlib.net/
*  이 후 버전은 설치 버전으로 변경
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/310c137f-7655-4eb3-abec-52aaaef0403f" width="480" height="360"/>

*  zip 압축을 풀고, 빌드를 위해 build폴더를 생성
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F246A4B37575E495240" width="480" height="360"/>

*  CMake에서 다운받은 파일 압축풀기 / cmake-gui.exe 파일 실행
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F22017336575E497808" width="480" height="360"/>

*  source code -> dlib-xx.xx\examples 선택
*  binaries -> build 폴더 선택 / configure
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F23019537575E498F28" width="480" height="360"/>

*  Visual studio 버전을 선택 /  Finish
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F271AAE37575E49BC10" width="480" height="360"/>

*  OpenCV_DIR 에 사용하고 있는 OpenCV/build경로 선택
*  체크박스 확인<br/>
   - DLIB_JPEG_SUPPORT<br/>
   - DLIB_PNG_SUPPORT<br/>
   - USE_AVX_INSTRUCTIONS<br/>
   - USE_SSE2_INSTRUCTIONS<br/>
   - USE_SSE4_INSTRUCTIONS<br/>
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F22539535575E4A582B" width="480" height="360"/>

*  빨간 박스가 안생길때 까지 Configure
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F250EC13C575E4A832B" width="480" height="360"/>

*  Generate / Generating done이 나오면 완
<br/>
<img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Ft1.daumcdn.net%2Fcfile%2Ftistory%2F2654CD36575E4ACF38" width="480" height="360"/>

*  dlib-18.18\build\dlib_build\dlib.sln Visual Studio로 실행
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/1c611dab-fd1e-492c-98f2-25aca8ec3d79" width="480" height="360"/>

*  Release / x64로 변경
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/8bc2fa6f-e5a5-40e2-9870-e9f571c01d1c" width="480" height="360"/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/cdeda01f-f441-49ff-a268-8e16157420a2" width="480" height="80"/>


*  빌드 -> 구성 관리자 / ALL_BUILD까지 체크 후 솔루션 빌드
*  Release폴더 lib파일 확인
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/f1c51d59-18ea-4973-8255-c20de3f1d9b6" width="240" height="80"/>

*  적용 할 프로젝트 또는 새 프로젝트 시작 후 Release x64로 변경
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/97216bbf-3d25-4e0b-b46d-0aa8cfa79cfc" width="480" height="360"/>

*  프로젝트 속성에서 구성과 플랫폼은 Release x64로 변경
*  VC++ 디렉터리 메뉴 포함 디렉터리에 Opencv/build/include와 dlib-xx.xx 추가
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/ee044d69-e0c8-468d-9ff9-47880db80783" width="480" height="360"/>

*  라이브러리 디렉터리에는 opencv/build/x64/vc17/lib와 dlib-xx.xx/build/dlib_build/Release 추가
<br/>
<img src="https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/141535958/35573f9e-1969-495d-beca-7c20051ef603" width="480" height="360"/>

*  링커 -> 입력 -> 추가 종속성의 opencv와 Dlib의 라이브러리(.lib) 추가
<br/> 

* 참조 : https://hhahn.tistory.com/10
