#pragma once
#include "Common.h"

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
public:
    double calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]); // EAR�� ����ϴ� �Լ�
    double calculateAngle(Point p1, Point p2); //�� �� �������
    double eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6); // EAR�� ����ϴ� �Լ�
    
    void initializeCamera(int i, VideoCapture& cap); //ī�޶� �ʱ⼳��
    void initializeDlib(dlib::frontal_face_detector& detector, dlib::shape_predictor& landmark_predictor); //dlib �ʱ⼳��
    void calculateFPS(Mat& frame, int& frameCount, chrono::time_point<chrono::high_resolution_clock>& start_fps); //FPS ���
    void detectEyesAndSleep(Mat& frame, dlib::frontal_face_detector detector, dlib::shape_predictor landmark_predictor, bool& sleep, clock_t& start, clock_t& end); //EAR�� ��� �� �������� �Ǵ�
    int videotoframe(Mat& frame, VideoCapture& cap); //���� -> �̹���
};
