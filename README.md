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

 ![Alt text](image-8.png)

  - https://opencv.org/releases/

 ## Opencv contrib

 ![Alt text](image-10.png)

  - https://www.raoyunsoft.com/opencv/opencv_contrib/ (Opencv 버전과 동일)

 ## CMake 

 ![Alt text](image-12.png)

  - https://cmake.org/download/

 ## CUDA

 ![Alt text](image-15.png)

  - https://developer.nvidia.com/cuda-toolkit-archive
  - https://en.wikipedia.org/wiki/CUDA#GPUs_supported (GPU와 호환되는 CUDA 버전 확인)

 ## CuDNN

 ![Alt text](image-16.png)

  - https://developer.nvidia.com/rdp/cudnn-archive (CUDA와 호환되는 CuDNN 버전 확인)

 ## YOLOv7

 ![Alt text](image-17.png)

  - YOLOv7: https://github.com/WongKinYiu/yolov7
  
## 2. Setup
 ![Alt text](image-1.png)
 * CMake 실행하여 Opencv build 경로 설정

 ![Alt text](image-4.png)
 
 * build 환경 선택

 ![Alt text](image-5.png)

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

 ![Alt text](image.png)

 * CMakeTargets 폴더 내 ALL_BUILD 프로젝트 우 클릭 하여 빌드
 
 * CMakeTargets 폴더 내 INSTALL 프로젝트 우 클릭 하여 빌드
