#pragma once

/* default */
#include <iostream>
#include <string>
#include <vector>

/* Driver */
#include <chrono>
#include <math.h>
#define _USE_MATH_DEFINES
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

/* Driver - dlib */
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <opencv2/core.hpp>


#ifdef _DEBUG
#pragma comment(lib, "opencv_world470d.lib")
#else
#pragma comment(lib, "opencv_world470.lib")
#endif

using namespace std;
using namespace cv;
using namespace dlib;
