#include "Common.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace cv;

const String window_capture_name1 = "1. Live";
const String window_capture_name2 = "2. Canny";
const String window_capture_name3 = "3. Hough";
const String window_capture_name4 = "4. TopView";

Point2f srcPts[4], dstPts[4];
void on_mouse(int event, int x, int y, int flags, void* userdata);

int main(int argc, char* argv[])
{
    namedWindow(window_capture_name1);
    //namedWindow(window_capture_name2);
    //namedWindow(window_capture_name3);
    namedWindow(window_capture_name4);

    VideoCapture video("Line.mp4"); //opencv 480에 있는 파일 build에 넣어야 실행됨
    Mat img_frame, img_edges, img_lines, img_threshold,img_bin;
    Mat img_frame_gray, blob;
    int cnt = 0;
    int sum_left = 0;
    int sum_right = 0;
    int avg_left = 0;
    int avg_right = 10;

    std::vector<std::string> classes;
    std::ifstream ifs("coco.names");
    std::string line;
    while (getline(ifs, line)) {
        classes.push_back(line);
    }

    cv::dnn::Net net = cv::dnn::readNet("yolov7.weights", "yolov7.cfg"); //학습된 모델 불러오기 dnn

    if (!video.isOpened())
    {
        cout << "동영상 파일을 열 수 없습니다. \n" << endl;
        return -1;
    }


    while (true)
    {

        video >> img_frame;
       
        if (img_frame.empty())
        {
            break;
        }

       
        cvtColor(img_frame, img_frame_gray, COLOR_BGR2GRAY);

        Canny(img_frame, img_edges, 50, 150);
        //1. 가운데 지점에서 for loop 돌려서 가장 가까운 왼쪽 오른쪽의 값을 통해 스레스 홀드 값 지정.

        int width = img_frame.cols;
        int height = img_frame.rows;

        
        

        Mat output;
        Mat img_mask = Mat::zeros(height, width, CV_8UC1);
        Mat img_point = Mat::zeros(height, width, CV_8UC3);
        Mat img_unwarped = Mat::zeros(height, width, CV_8UC1);

        //관심 영역 정점 계산

      /*  double poly_bottom_width2 = 0.85;  //사다리꼴 아래쪽 가장자리 너비 계산을 위한 백분율
        double poly_top_width2 = 0.15;     //사다리꼴 위쪽 가장자리 너비 계산을 위한 백분율
        double poly_height2 = 0.3;         //사다리꼴 높이 계산을 위한 백분율
        */
        double poly_bottom_width = 1;  //사다리꼴 아래쪽 너비 계산을 위한 백분율
        double poly_top_width = 0.2;  //사다리꼴 위쪽 너비 계산을 위한 백분율
        double poly_height = 0.35;     //사다리꼴 높이 계산을 위한 백분율

        Point points_int[4]{
           Point((width * (1 - poly_bottom_width)) / 2, height),
           Point((width * (1 - poly_top_width)) / 2, height - height * poly_height),
           Point(width - (width * (1 - poly_top_width)) / 2, height - height * poly_height),
           Point(width - (width * (1 - poly_bottom_width)) / 2, height)
        };

        //=========================================================================
        //perspective

        Point2f points[4]{
           Point2f((width * (1 - poly_top_width)) / 2, height - height * poly_height),
           Point2f(width - (width * (1 - poly_top_width)) / 2, height - height * poly_height),
           Point2f((width * (1 - poly_bottom_width)) / 2, height),
           Point2f(width - (width * (1 - poly_bottom_width)) / 2, height)
        };


        //Warping 후의 좌표
        Size warp_size(width, height);
        Mat img_top(warp_size, img_frame_gray.type());

        std::vector<int> profile(img_top.cols, 0);
        std::vector<int> profile_conv(img_top.cols, 0);
        string profileValues = "";

        Point2f warp_corners[4]{
        Point2f(0, 0),
        Point2f(img_top.cols, 0),
        Point2f(0, img_top.rows),
        Point2f(img_top.cols, img_top.rows)
        };


        Mat trans = getPerspectiveTransform(points, warp_corners);
        Mat Inv_trans = getPerspectiveTransform(warp_corners, points);

        warpPerspective(img_frame_gray, img_top, trans, warp_size);

        for (int col = 0; col < img_top.cols; col++)
        {
            for (int row = 0; row < img_top.rows; row++)
            {
                profile[col] += img_top.data[row * (img_top.cols) + col];
            }
            profileValues += to_string(profile[col]);
            profileValues += ",";
        }

        /*const int kernelSz = 5;
        int kernel[kernelSz] = { -10,-5,0,5,10 };

        for (int i = kernelSz / 2; i < profile.size() - kernelSz / 2; i++)
        {
            //kernel
            int convValue = 0;
            for (size_t k = 0; k < kernelSz; k++)
            {
                profile_conv[i] += profile[(i - kernelSz / 2) + k] * kernel[k];
            }

            profileValues += to_string(profile_conv[i]);
            profileValues += ",";
        }*/

        int maxIndex_X_left = 0;
        int maxIndex_X_right = 0;
        int maxValue_left = -99999999;
        int maxValue_right = -99999999;

        for (int i = 0; i < profile.size(); i++)
        {
            if (i < profile.size() / 2)
            {
                if (maxValue_left < profile[i])
                {
                    maxValue_left = profile[i];
                    maxIndex_X_left = i;
                }
            }

            else
            {
                if (maxValue_right < profile[i])
                {
                    maxValue_right = profile[i];
                    maxIndex_X_right = i;
                }
            }
        }

        sum_left += maxIndex_X_left;
        sum_right += maxIndex_X_right;
        cnt++;

        if (cnt == 10)
        {
            avg_left = sum_left / cnt;
            avg_right = sum_right / cnt;
            cnt = 0;
            sum_left = 0;
            sum_right = 0;
        }

        cv::circle(img_point, cv::Point(avg_left, img_point.rows / 2), 5, cv::Scalar(0, 255, 0), 2);
        cv::circle(img_point, cv::Point(avg_right, img_point.rows / 2), 5, cv::Scalar(0, 255, 0), 2);
        cv::line(img_point, cv::Point(avg_left, img_point.rows / 2), cv::Point(avg_right, img_point.rows / 2), cv::Scalar(0, 0, 255), 10);
        //cv::line(img_point, cv::Point(avg_left, img_point.rows / 2), cv::Point(avg_left, img_point.rows), cv::Scalar(0, 0, 255), 10);
        //cv::line(img_point, cv::Point(avg_right, img_point.rows / 2), cv::Point(avg_right, img_point.rows), cv::Scalar(0, 0, 255), 10);
        
        warpPerspective(img_point, img_point, Inv_trans, warp_size);
        addWeighted(img_point, 1, img_frame, 1, 1, img_frame);

        //정점으로 정의된 다각형 내부의 색상을 채워 그린다.
        fillConvexPoly(img_mask, points_int, 4, Scalar(255, 0, 0));

        //결과를 얻기 위해 edges 이미지와 mask를 곱한다.
        bitwise_and(img_edges, img_mask, output);


        vector<Vec4i> line;
        //HoughLinesP(img_mask, line, 1, CV_PI / 180, 20, 10, 20);

        imshow(window_capture_name1, img_frame);
        //imshow(window_capture_name2, img_edges);
        //imshow(window_capture_name3, img_mask);
        imshow(window_capture_name4, img_top);

        char key = (char)waitKey(30);
        if (key == 'q' || key == 27)
        {
            break;
        }
    }
    return 0;
}