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
#include <windows.h> 
#include <wininet.h> //window내장 http 전송 라이브러리

using namespace std;
using namespace cv;
using namespace dlib;

class ISP {
public:
    ISP();
    ~ISP();
private:
    const int leftEye[6] = { 41, 42, 43, 44, 45, 46 }; //�޴� ���帶ũ ��ȣ
    const int rightEye[6] = { 36, 37, 38, 39, 40, 41 }; //������ ���帶ũ ��ȣ
    const int nose[1] = { 30 }; //�� ���帶ũ ��ȣ
    const double eyeAspectRatioThreshold = 0.23; //�� ���� EAR �Ӱ谪
    const double sleeptime_threshold = 3; //���� ���ӽð� �Ӱ谪
    const double notfound_threshold = 5; //��ã�� �� ���ӽð� �Ӱ谪
    double calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]); // EAR�� ����ϴ� �Լ�
    double calculateAngle(Point p1, Point p2); //�� �� �������
    double eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6); // EAR�� ����ϴ� �Լ�
    
public:
    void initializeCamera(int i, VideoCapture& cap); //ī�޶� �ʱ⼳��
    void initializeDlib(dlib::frontal_face_detector& detector, dlib::shape_predictor& landmark_predictor); //dlib �ʱ⼳��
    void calculateFPS(Mat& frame, int& frameCount, chrono::time_point<chrono::high_resolution_clock>& start_fps); //FPS ���
    void detectEyesAndSleep(Mat& frame, dlib::frontal_face_detector detector, dlib::shape_predictor landmark_predictor, bool& sleep, clock_t& start, clock_t& end); //EAR�� ��� �� �������� �Ǵ�
    int videotoframe(Mat& frame, VideoCapture& cap); //���� -> �̹���
    void preprocessing(Mat& frame, Mat& dst, Mat& dst_hsv); //preprocessing input image
    void gammatransform(Mat& frame, Mat& gamma_t, float gamma_var);
    void logtransform(Mat& frame, Mat& log_t, int log_var);
    void initalizeWininet(HINTERNET& hInternet, HINTERNET& hConnect); //http 통신 초기화
    void request_Wininet_Get(HINTERNET& hInternet, HINTERNET& hConnect, bool sleep, bool& pre_sleep); //wininet_get
};
