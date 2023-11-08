#include "Common.h"
#include <iostream>
#include <opencv2/opencv.hpp>

const String window_capture_name0 = "0. Set Roi";
const String window_capture_name1 = "1. Lane_detection_video";
const String window_capture_name2 = "2. ROI rect";
const String window_capture_name3 = "3. birdview_Line";
const String window_capture_name4 = "4. Final";

bool ROISelect = false;
int cnt = 0;
Point2f srcPt[4];

void on_mouse(int event, int x, int y, int flags, void* userdata);
void SetROI(Mat src);
Mat TransPersfective(Mat src, Point2f* srcPts, Point2f* dstPts);
Mat PreprocessFrame(Mat src);

int main()
{
    namedWindow(window_capture_name0);

    //동영상 불러오기
    VideoCapture video("input.mp4"); //opencv 480에 있는 파일 opencv_videoio_ffmpeg480_64을 build에 넣어야 실행됨
    Mat img_frame;

    if (!video.isOpened())
    {
        cout << "동영상 파일을 열 수 없습니다. \n" << endl;
        return -1;
    }

    video.read(img_frame);
    resize(img_frame, img_frame, Size(210, 200));
    // 1: RoI 영역을 마우스로 지정
    //      1) 마우스 클릭 콜백 함수
    setMouseCallback(window_capture_name0, on_mouse, &img_frame);
        
    while (!ROISelect)
    {
        waitKey(20);
    } 
    
    destroyWindow(window_capture_name0);

    if (img_frame.empty())
    {
        return -1;
    }

    while (true)
    {
        if (!video.read(img_frame))
        {
            break;
        }

        resize(img_frame, img_frame, Size(210, 200));
        Mat src_color = img_frame.clone();
        Mat src_gray;
        cvtColor(src_color, src_gray, COLOR_BGR2GRAY);

        //      2) Region of Interest (ROI) 지정 지정
        SetROI(img_frame);
           
        // 2: 시점 변환
        //      1) 투시 변환 (Perspective Transformation)
        //      Top view 로 변환하기 위해 설정하는 좌표들
        Point2f dstPt[4] = {
            Point2f(0, 0), //왼쪽 위;
            Point2f(src_color.cols, 0), // 오른쪽 위
            Point2f(src_color.cols, src_color.rows), // 오른쪽 아래
            Point2f(0, src_color.rows) //왼쪽 아래
        }; 
        
        Mat img_topview = TransPersfective(src_color, srcPt, dstPt);

        // 3: 이미지 전처리
        //      1) 이미지 전처리
        Mat LineContoure;
        Mat Line_img = PreprocessFrame(img_topview);;
        cvtColor(Line_img, LineContoure, COLOR_BGR2GRAY);
        threshold(LineContoure, LineContoure, 128, 255, ThresholdTypes::THRESH_OTSU);

        //      2) 색상 영역 변경
        //      3) 노이즈 제거
        

        // 4: 차선 검출
        //      1) 차선 검출
        //      높이 방향 직사각형으로 영역 나누기
        int numRect = 10;
        int rectHeight = img_topview.rows / numRect;

        // 직사각형 위치를 저장하는 벡터 초기화
        // 왼쪽 영역 직선 따기
        vector<Rect> rectangles_left;
        for (int i = 0; i < numRect; i++)
        {
            // 직사각형 좌표 계산
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;
            int x = 0;
                
            Rect rect(x, y1, 70, y2 - y1);

            rectangles_left.push_back(rect);
        }

        //rectangles_left 배열을 돌며 rect에 집어넣음
        for (const auto& rect : rectangles_left)
        {
            rectangle(Line_img, rect, Scalar(255, 0, 0), 3);
        }

        // 중심 좌표를 저장할 벡터 초기화
        vector<Point2f> centroids_left;

        //rectangles_left 배열을 돌며 중심점 찾아서 넣음
        for (const Rect& rect : rectangles_left)
        {

            Mat roi = LineContoure(rect);
            // rectangles_left 나뉜 사각형에서 컨투어 검출
            vector<vector<Point>> contours;
            findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            /*
                ROI 영역에서 검출된 contours 처리
                rectangles_left 나뉜 사각형에서 처리하기 때문에 contours 한개만 나옴
            */

            // 1) 나뉜 구역에서 차선이 없을 경우
            if (contours.empty())
            {
                //생성된 직사각형의 중심점을 임의로 잡음
                Point2f centcircle(rect.x+rect.width/2, rect.y + rect.height / 2);
                centroids_left.push_back(centcircle);
                //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
            }

            // 2) 나뉜 구역에서 차선이 있을 경우
            else
            {
                for (const vector<Point>& contour : contours)
                {
                        
                    double area = contourArea(contour);
                    double minContourArea = 10.0; // 점선을 제외한 노이즈를 없애기 위한 임계값

                    if (area >= minContourArea)
                    {
                        /*
                            Moments는 컨투어(윤곽선)의 특징을 계산
                            mu.m00: 0차 모멘트, 영역의 면적 (무게 합)
                            mu.m10: 1차 모멘트 X(X 좌표의 무게 합)
                            mu.m01 : 1차 모멘트 Y(Y 좌표의 무게 합)
                            mu.m20 : 2차 모멘트 X(X 좌표의 제곱의 무게 합)
                            mu.m02 : 2차 모멘트 Y(Y 좌표의 제곱의 무게 합)
                            mu.m11 : 2차 모멘트 XY(X와 Y 좌표의 곱의 무게 합)
                        */
                        Moments mu = moments(contour);

                        // 중심 좌표 계산
                        Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);

                        // contours 중심 좌표에 원 그리기
                        Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
                        centroids_left.push_back(centcircle);
                        //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
                    }
                }
            }
        }

        /*
        각 직사각형 영역(ROI)내의 중심좌표를 이어서 한 라인으로 그리기

        Mat roiLine_left = Mat::zeros(img_topview.size(), CV_8UC3);
        for (size_t i = 0; i < centroids_left.size() - 1; i++)
        {
            line(roiLine_left, centroids_left[i], centroids_left[i + 1], Scalar(255, 0, 0), 7);
        }
        */
        // 직사각형 위치를 저장하는 벡터 초기화
        // 오른쪽 영역 직선 따기
        vector<Rect> rectangles_right;
        for (int i = 0; i < numRect; i++)
        {
            // 직사각형 좌표 계산
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;
            int x = 140;

            Rect rect(x, y1, 70, y2 - y1);

            rectangles_right.push_back(rect);
        }

        //rectangles_right 배열을 돌며 rect에 집어넣음
        for (const auto& rect : rectangles_right)
        {
            rectangle(Line_img, rect, Scalar(255, 0, 0), 3); 
        }


        // 중심 좌표를 저장할 벡터 초기화
        vector<Point2f> centroids_right;

        for (const Rect& rect : rectangles_right)
        {

            Mat roi = LineContoure(rect);
            vector<vector<Point>> contours;

            // rectangles_left  나뉜 사각형에서 컨투어 검출
            findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            /*
                ROI 영역에서 검출된 contours 처리
                rectangles_left  나뉜 사각형에서 처리하기 때문에 contours 한개만 나옴
            */

            // 1) 나뉜 구역에서 차선이 없을 경우
            if (contours.empty())
            {
                //생성된 직사각형의 중심점을 임의로 잡음
                Point2f centcircle(rect.x + rect.width / 2, rect.y + rect.height / 2);
                centroids_right.push_back(centcircle);
                //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
            }

            // 2) 나뉜 구역에서 차선이 있을 경우
            else
            {

                for (const vector<Point>& contour : contours)
                {
                    double area = contourArea(contour);
                    double minContourArea = 10.0; // 점선을 제외한 노이즈를 없애기 위한 임계값

                    if (area >= minContourArea)
                    {
                        /*
                            Moments는 컨투어(윤곽선)의 특징을 계산
                            mu.m00: 0차 모멘트, 영역의 면적 (무게 합)
                            mu.m10: 1차 모멘트 X(X 좌표의 무게 합)
                            mu.m01 : 1차 모멘트 Y(Y 좌표의 무게 합)
                            mu.m20 : 2차 모멘트 X(X 좌표의 제곱의 무게 합)
                            mu.m02 : 2차 모멘트 Y(Y 좌표의 제곱의 무게 합)
                            mu.m11 : 2차 모멘트 XY(X와 Y 좌표의 곱의 무게 합)
                        */
                        Moments mu = moments(contour);

                        // 중심 좌표 계산
                        Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);

                        // contours 중심 좌표에 원 그리기
                        Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
                        centroids_right.push_back(centcircle);
                        //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
                    }
                }
            }
        }

        /*
            각 직사각형 영역(ROI)내의 중심좌표를 이어서 한 라인으로 그리기
        Mat roiLine_right = Mat::zeros(img_topview.size(), CV_8UC3);
        for (size_t i = 0; i < centroids_right.size() - 1; i++)
        {
            line(roiLine_right, centroids_right[i], centroids_right[i + 1], Scalar(255, 0, 0), 7);
        }
        */

        /* 
        왼쪽, 오른쪽 영역에서 얻은 정보 합칠 Frame 생성
        Mat roiLine_sum = Mat::zeros(img_topview.size(), CV_8UC3);
        addWeighted(roiLine_left, 1, roiLine_right, 1, 0, roiLine_sum);
        */
        Mat img_fill = Mat::zeros(img_topview.size(), CV_8UC3);
        Point img_fill_point[4]{
            Point(centroids_left.back()), //왼쪽 아래
            Point(centroids_left.front()), //왼쪽 위
            Point(centroids_right.front()), //오른쪽 위
            Point(centroids_right.back()) //오른쪽 아래
        };

        fillConvexPoly(img_fill, img_fill_point, 4, Scalar(255, 0, 0));
        Mat inv_src_color = TransPersfective(img_fill, dstPt, srcPt);

        addWeighted(src_color, 1, inv_src_color, 1, 0, inv_src_color);
            
        resize(src_color, src_color, Size(210, 200));
        resize(img_frame, img_frame, Size(210, 200));
        resize(img_topview, img_topview, Size(210, 200));
        resize(inv_src_color, inv_src_color, Size(210, 200));

        imshow(window_capture_name1, src_color);
        moveWindow(window_capture_name1, 300, 300);
        imshow(window_capture_name2, img_frame);
        moveWindow(window_capture_name2, 600, 300);
        imshow(window_capture_name3, img_topview);
        moveWindow(window_capture_name3, 900, 300);
        imshow(window_capture_name4, inv_src_color);
        moveWindow(window_capture_name4, 1200, 300);

        char key = (char)waitKey(1);
        if (key == 'q' || key == 27)
        {
            break;
        }
    }

    return 0;
}

void on_mouse(int event, int x, int y, int flags, void* userdata)
{
    
    if (event == EVENT_LBUTTONDOWN) 
    {
        if (cnt < 4)
        { 
            //순서 좌측 상단 -> 우측 상단 -> 우측 하단 -> 좌측 하단
            srcPt[cnt] = Point2f(x, y);
            circle((*(Mat*)userdata), srcPt[cnt], 3, Scalar(0, 0, 255), 5);
            imshow(window_capture_name0, (*(Mat*)userdata));
            cnt++;
            if (cnt == 4)
            {
                ROISelect = true;
                
                srcPt[1].y = srcPt[0].y; //y위치를 하나로 통일
                srcPt[3].y = srcPt[2].y; //y위치를 하나로 통일
            }
        }
    }
    putText((*(Mat*)userdata), "Click Roi Point", Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
    imshow(window_capture_name0, (*(Mat*)userdata));
    moveWindow(window_capture_name0, 900, 300);
}

void SetROI(Mat src)
{
    line(src, srcPt[0], srcPt[1], Scalar(0, 255, 0), 2);
    line(src, srcPt[0], srcPt[3], Scalar(0, 255, 0), 2);
    line(src, srcPt[3], srcPt[2], Scalar(0, 255, 0), 2);
    line(src, srcPt[2], srcPt[1], Scalar(0, 255, 0), 2);
}

Mat TransPersfective(Mat src, Point2f* srcPts, Point2f* dstPts)
{
    Mat M_Transper = getPerspectiveTransform(srcPts, dstPts);
    Mat img_TransPer;
    //Top view 변환
    warpPerspective(src, img_TransPer, M_Transper, Size(src.cols, src.rows));

    return img_TransPer;
}

Mat PreprocessFrame(Mat src)
{
    Mat src_HSV = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat YLine = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, src_HSV, COLOR_BGR2HSV);

    //노란색 추출
    Scalar low_yellow = Scalar(15, 120, 200);
    Scalar high_yellow = Scalar(25, 255, 255);
    inRange(src_HSV, low_yellow, high_yellow, YLine);

    //차선 색 분리 -> 흰색
    Mat src_Lab = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat WLine = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, src_Lab, COLOR_BGR2Lab);

    //흰색 추출
    Scalar low_white = Scalar(200, 120, 125);
    Scalar high_white = Scalar(255, 130, 140);
    inRange(src_Lab, low_white, high_white, WLine);

    Mat Line_img; //bridview에서 라인만 분리 
    Mat LineContoure; //bridview에서 라인만 분리 
    bitwise_and(src, src, Line_img, YLine);
    bitwise_and(src, src, Line_img, WLine);

    return Line_img;
}