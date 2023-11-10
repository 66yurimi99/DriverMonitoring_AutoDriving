# DriverMonitoring_AutoDriving
[Intel] 엣지 AI SW 아카데미 2기 - 팀 프로젝트 : 운전자 상태 모니터링 및 자율주행 시스템

## Integrated Development Enviroment
*   Lane and object detection Part
    -  IDE: Visual Studio 2022
    -  Language: C++
    -  OS: Windows
    -  opencv
    -  opencv contrib
    -  CUDA
    -  CuDNN
    -  Yolo

## 1. Installation
 ## Opencv

   ![KakaoTalk_20231109_182750407](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/4a056c52-7b5a-4fce-be9e-b635d3986cc8)

  - https://opencv.org/releases/ 

 ## Opencv contrib

 ![image-10](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/7e7ee2b9-e3dd-4665-9a73-e5d2f5013809)

  - https://www.raoyunsoft.com/opencv/opencv_contrib/ (Opencv 버전과 동일)

 ## CMake 

  ![image-12](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/bf8a5047-452f-4719-9896-a63e277c89fe)

  - https://cmake.org/download/

 ## CUDA

 ![image-13](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/d05fdc55-30db-4357-930e-ad45fda5bdf0)

  - https://developer.nvidia.com/cuda-toolkit-archive
  - https://en.wikipedia.org/wiki/CUDA#GPUs_supported (GPU와 호환되는 CUDA 버전 확인)

 ## CuDNN

 ![image-16](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/d385cf22-259b-4ca4-84cf-30fb065ee3ba)

  - https://developer.nvidia.com/rdp/cudnn-archive (CUDA와 호환되는 CuDNN 버전 확인)

 ## YOLOv7

 ![image-17](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/6a83607a-ec5f-4670-9365-6148e791cbed)

  - YOLOv7: https://github.com/WongKinYiu/yolov7
  
## 2. Setup
 ![image-1](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/f59c0652-1def-47f1-a7fa-d49639b5176a)
 * CMake 실행하여 Opencv build 경로 설정

![KakaoTalk_20231109_182739202](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/c3274647-53d3-4778-a7d4-9a33fab7475a)
 
 * build 환경 선택

![image-5](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/085d2415-6359-4cbb-aded-f3ef1aab0220)

 * CMake 내 아래 아이템 설정
   * BUILD_opencv_world = ON
   * OPENCV_ENABLE_NONFREE = ON
   * OPENCV_EXTRA_MODULES_PATH = D:/opencv470/opencv_contrib-4.7.0/modules
   * OPENCV_DNN_CUDA = ON
   * WITH_CUDA = ON
   * WITH_CUDNN = ON
   * CUDA_FAST_MATH = ON
   * CUDA_ARCH_BIN = 7.5
     * (https://en.wikipedia.org/wiki/CUDA#GPUs_supported 내 GPU compute capability 참조) 
     * 아이템 추가 후 Configure -> Generate -> Open Project

![image](https://github.com/66yurimi99/DriverMonitoring_AutoDriving/assets/148414302/4b1e6552-9542-45be-9cf6-b261a0dfcb64)

 * CMakeTargets 폴더 내 ALL_BUILD 프로젝트 우 클릭 하여 빌드
 
 * CMakeTargets 폴더 내 INSTALL 프로젝트 우 클릭 하여 빌드
