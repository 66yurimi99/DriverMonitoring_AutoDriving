#pragma once

/* default */
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

/* Car - Car Detection */
#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <fstream>
#include <math.h>


#ifdef _DEBUG
#pragma comment(lib, "opencv_world470d.lib")
#else
#pragma comment(lib, "opencv_world470.lib")
#endif

using namespace std;
using namespace cv;