#include "Common.h"
#include <iostream>

const String window_roi_name = "ROI Setting";
const String window_capture_name0 = "0. Original";
const String window_capture_name1 = "1. Gray";
const String window_capture_name2 = "2. Yellow + LAB White";
const String window_capture_name3 = "3. Yellow + Value";

int width = 0;
int height = 0;

bool ROISelect = false;
typedef struct UserData {
    Mat img;
    Point2f* pts;
};
void OnMouse(int event, int x, int y, int flags, void* userdata);
void SetROI(Mat src, Point2f* srcPts);
void TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[]);

void ColorOption1(Mat src, Mat& dst);
void ColorOption2(Mat src, Mat& dst);
void ColorOption3(Mat src, Mat& dst);
void NoiseOption(Mat src, Mat& dst);

int main()
{
    namedWindow(window_roi_name);
    namedWindow(window_capture_name0);
    namedWindow(window_capture_name1);
    namedWindow(window_capture_name2);
    namedWindow(window_capture_name3);

    string fileName = "case1.jpg";
    Mat img_test = imread(fileName, IMREAD_COLOR);

    width = img_test.cols;
    height = img_test.rows;

    Point2f srcPts[4] = {
#if 0
        // Case 1
        Point2f(500, 445),
        Point2f(690, 445),
        Point2f(1170, 630),
        Point2f(150, 630)

        // Case 2
        Point2f(540, 410),
        Point2f(670, 410),
        Point2f(1180, 630),
        Point2f(90, 630)

        // Case 3
        Point2f(520, 440),
        Point2f(710, 440),
        Point2f(1150, 635),
        Point2f(65, 635)
#endif
    };
    Point2f dstPts[4] = {
        Point2f(0,0),
        Point2f(width * 0.3, 0),
        Point2f(width * 0.3, height * 0.3),
        Point2f(0, height * 0.3)
    };

#if 0
    // 1. Set RoI (첫번째 frame에서 마우스 클릭으로 ROI 영역 지정)
    SetROI(img_test, srcPts);
#endif

    Mat img_original;
    TransPersfective(img_test, img_original, srcPts, dstPts);

    Mat img_option1, img_option2, img_option3;

    // TEST 1: Color Conversion
#if 1
    ColorOption1(img_original, img_option1);
    ColorOption2(img_original, img_option2);
    ColorOption3(img_original, img_option3);
#endif

    // TEST 2: Darkening
#if 1
    img_option1 = img_option1 * 0.5;
    img_option2 = img_option2 * 0.5;
    img_option3 = img_option3 * 0.5;
#endif

    // TEST 3: Noise Calcelation
#if 1
    NoiseOption(img_option1, img_option1);
    NoiseOption(img_option2, img_option2);
    NoiseOption(img_option3, img_option3);
#endif


    // RESULT: 이진화 - 오츠 알고리즘
#if 0
    threshold(img_option1, img_option1, 0, 255, ThresholdTypes::THRESH_OTSU);
    threshold(img_option2, img_option2, 0, 255, ThresholdTypes::THRESH_OTSU);
    threshold(img_option3, img_option3, 0, 255, ThresholdTypes::THRESH_OTSU);
#endif

    imshow(window_capture_name0, img_original);
    imshow(window_capture_name1, img_option1);
    imshow(window_capture_name2, img_option2);
    imshow(window_capture_name3, img_option3);

    waitKey(0);

    return 0;
}

void ColorOption1(Mat src, Mat& dst)
{
    // RESULT: Gray, 1-channel
    cvtColor(src, dst, COLOR_BGR2GRAY);
}

void ColorOption2(Mat src, Mat& dst)
{
    // STEP 1: Yellow, 1-channel (based on HSV 3-channels)
    Mat img_hsv = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat img_yellow = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, img_hsv, COLOR_BGR2HSV);

    Scalar low_yellow = Scalar(15, 120, 200);
    Scalar high_yellow = Scalar(25, 255, 255);
    inRange(img_hsv, low_yellow, high_yellow, img_yellow);

    // STEP 2: White, 1-channel (based on LAB 3-channels)
    Mat img_lab = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat img_white = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, img_lab, COLOR_BGR2Lab);

    Scalar low_white = Scalar(200, 120, 125);
    Scalar high_white = Scalar(255, 130, 140);
    inRange(img_lab, low_white, high_white, img_white);

    // RESULT: Yellow & White
#if 1
    bitwise_or(img_yellow, img_white, dst);
#endif

#if 0
    bitwise_and(src, img_yellow, dst);
    bitwise_and(src, img_white, dst);
#endif
}

void ColorOption3(Mat src, Mat& dst)
{
    Mat img_hsv, img_value, img_yellow;

    // STEP 1: Value, 1-channel (based on HSV 3-channels)
    cvtColor(src, img_hsv, COLOR_BGR2HSV);

    vector<Mat> HSV_channels(3);
    split(img_hsv, HSV_channels);
    img_value = HSV_channels[2];

    // STEP 2: Yellow, 1-channel (based on HSV 3-channels)
    Scalar lower_yellow(15, 120, 200);
    Scalar upper_yellow(25, 255, 255);
    inRange(img_hsv, lower_yellow, upper_yellow, img_yellow);

    // RESULT: HSV-Value + HSV-Yellow * 0.3
    addWeighted(img_value, 1, img_yellow, 0.3, 0, dst);
}

void NoiseOption(Mat src, Mat& dst)
{
    Mat tmp;
    // STEP 1: 이미지 블러링
    medianBlur(src, tmp, 3);

    // STEP 2: 이진화 - 오츠 알고리즘
    threshold(tmp, tmp, 0, 255, ThresholdTypes::THRESH_OTSU);

    // STEP 3: Morphology Opening
    erode(tmp, tmp, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
    dilate(tmp, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

void OnMouse(int event, int x, int y, int flags, void* userdata)
{
    static int cnt = 0;

    Point2f* srcPts = ((UserData*)userdata)->pts;
    Mat img_roi = ((UserData*)userdata)->img;

    if (event == EVENT_LBUTTONDOWN)
    {
        if (cnt < 4)
        {
            // 선택한 위치를 점으로 그려서 보여주기
            srcPts[cnt++] = Point2f(x, y);
            circle(img_roi, Point(x, y), 5, Scalar(255, 0, 255), -1);
            imshow(window_roi_name, img_roi);

            if (cnt == 4)
            {
                // x 좌표 설정 : RoI 사각형 너비가 차선 너비와 비례하도록 x 좌표값 조정
                for (size_t i = 0; i < 4; i = i + 2)
                {
                    int margin = (srcPts[i].x - srcPts[i + 1].x) * 0.2;
                    srcPts[i].x += margin;
                    srcPts[i + 1].x += (-margin);
                }

                // y 좌표 설정 : RoI 사각형의 윗변, 아랫변을 이루는 두 점의 y 좌표를 일치시키기
                srcPts[1].y = srcPts[0].y;
                srcPts[3].y = srcPts[2].y;

                ROISelect = true;
            }
        }
    }
}

void SetROI(Mat src, Point2f* srcPts)
{
    namedWindow(window_roi_name);

    Mat img_roi = src.clone();
    UserData userdata = { img_roi, srcPts };

    putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
    imshow(window_roi_name, img_roi); // ROI 설정을 위한 화면 출력

    // 마우스 클릭 이벤트 발생 시 콜백 함수(OnMouse) 호출
    setMouseCallback(window_roi_name, OnMouse, &userdata);

    while (!ROISelect)
    {	// 20ms 간격으로 RoI 지정 여부 확인
        waitKey(20);
    }

    destroyWindow(window_roi_name);
}

void TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[])
{
    Mat persfective;

    Size size = Size(src.cols * 0.3, src.rows * 0.3);
    if (srcPts[0].x < dstPts[0].x)
    {
        size = Size(width, height);
    }

    persfective = getPerspectiveTransform(srcPts, dstPts);
    warpPerspective(src, dst, persfective, size);
}