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
    const int leftEye[6] = { 41, 42, 43, 44, 45, 46 };
    const int rightEye[6] = { 36, 37, 38, 39, 40, 41 };
    const int nose[1] = { 30 };
public:
    // EAR을 계산하는 함수
    double calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]);
    double calculateAngle(Point p1, Point p2);
    double eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6);
};
