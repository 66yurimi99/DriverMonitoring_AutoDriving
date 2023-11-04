#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <chrono>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <vector>


using namespace std;
using namespace cv;
using namespace dlib;

class ISP {
public:
    ISP();
    ~ISP();
private:
    const int leftEye[6] = { 41, 42, 43, 44, 45, 46 }; //왼눈 랜드마크 번호
    const int rightEye[6] = { 36, 37, 38, 39, 40, 41 }; //오른눈 랜드마크 번호
    const int nose[1] = { 30 }; //코 랜드마크 번호
    const double eyeAspectRatioThreshold = 0.23; //눈 감김 EAR 임계값
    const double sleeptime_threshold = 3; //감김 지속시간 임계값
    const double notfound_threshold = 5; //못찾을 때 지속시간 임계값
public:
    double calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]); // EAR을 계산하는 함수
    double calculateAngle(Point p1, Point p2); //두 점 각도계산
    double eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6); // EAR을 계산하는 함수
    
    void initializeCamera(int i, VideoCapture& cap); //카메라 초기설정
    void initializeDlib(dlib::frontal_face_detector& detector, dlib::shape_predictor& landmark_predictor); //dlib 초기설정
    void calculateFPS(Mat& frame, int& frameCount, chrono::time_point<chrono::high_resolution_clock>& start_fps); //FPS 계산
    void detectEyesAndSleep(Mat& frame, dlib::frontal_face_detector detector, dlib::shape_predictor landmark_predictor, bool& sleep, clock_t& start, clock_t& end); //EAR값 계산 및 졸음여부 판단
    int videotoframe(Mat& frame, VideoCapture& cap); //영상 -> 이미지
    void preprocessing(Mat& frame, Mat& dst, Mat& dst_hsv); //preprocessing input image
    void gammatransform(Mat& frame, Mat& gamma_t, float gamma_var);
    void logtransform(Mat& frame, Mat& log_t, int log_var);
};
