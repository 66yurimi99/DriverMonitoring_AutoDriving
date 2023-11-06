#include "Car.h"

AutoDriving::AutoDriving()
{	
	cout << "Start AutoDriving mode..." << endl;
}

AutoDriving::~AutoDriving()
{
	cout << "End AutoDriving mode..." << endl;
}

void AutoDriving::OnMouse(int event, int x, int y, int flags, void* userdata)
{
    static int cnt = 0;

    UserData* userData = static_cast<UserData*>(userdata);
    Point2f* srcPts = userData->pts;
    Mat img_roi = userData->img;


    if (event == EVENT_LBUTTONDOWN)
    {
        if (cnt < 4)
        {   
            // 선택한 위치를 점으로 그려서 보여주기
            srcPts[cnt++] = Point2f(x, y);
            circle(img_roi, Point(x, y), 5, Scalar(255, 0, 255), -1);
            imshow(windowname, img_roi);
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

void AutoDriving::SetROI(Mat src, Point2f* srcPts, string windowname)
{
    namedWindow(windowname);

    Mat img_roi = src.clone();
    UserData userdata = { img_roi, srcPts };

    putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
    imshow(windowname, img_roi); // ROI 설정을 위한 화면 출력

    //마우스 클릭 이벤트 발생 시 콜백 함수(OnMouse) 호출
    setMouseCallback(windowname, OnMouse, &userdata);

    while (!ROISelect)
    {   // 20ms 간격으로 RoI 지정 여부 확인
        waitKey(20);
    }

    destroyWindow(windowname);
}

void AutoDriving::TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[], int width, int height)
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

void AutoDriving::ConvertColor(Mat src, Mat& dst)
{
    Mat img_hsv, img_lab;
    Mat img_yellow, img_white;

    // Using HSV, LAB color space
    cvtColor(src, img_hsv, COLOR_BGR2HSV);
    cvtColor(src, img_lab, COLOR_BGR2Lab);

    // STEP 1: Yellow, 1-channel (based on HSV 3-channels)
    Scalar low_yellow = Scalar(15, 120, 200);
    Scalar high_yellow = Scalar(25, 255, 255);
    inRange(img_hsv, low_yellow, high_yellow, img_yellow);

    // STEP 2: White, 1-channel (based on LAB 3-channels)
    Scalar low_white = Scalar(200, 120, 125);
    Scalar high_white = Scalar(255, 130, 140);
    inRange(img_lab, low_white, high_white, img_white);

    bitwise_or(img_yellow, img_white, dst);
}

void AutoDriving::RemoveNoise(Mat src, Mat& dst)
{
    // Morphology Opening (erosion -> dilation)
    erode(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
    dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

void AutoDriving::PreprocessFrame(Mat src, Mat& dst)
{
    // 1) 색상 영역 변경
    ConvertColor(src, dst);

    // 2) 노이즈 제거 : Morphology Opening
    RemoveNoise(src, dst);
}

double AutoDriving::CalcDirection(Mat& img_draw)
{
	return 0.0;
}

void AutoDriving::PutText(Mat& draw, const bool isSleep, const double amount)
{
    String driving_mode = isSleep ? "Auto" : "Manual";
    String direction;
    if (amount < -5)	direction = "Left";
    else if (amount < 5)	direction = "Straight";
    else	direction = "Right";
    putText(draw, driving_mode, Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
    putText(draw, direction, Point(20, 120), 1, 4, Scalar(0, 255, 255), 5);
}

void AutoDriving::LaneDetection(Mat& src, Mat& dst)
{
    vector<Rect> rectangles_left = DivideRoi(src, 0);
    vector<Point2f> centroids_left = findCentroids(src, rectangles_left);

    vector<Rect> rectangles_right = DivideRoi(src, 310);
    vector<Point2f> centroids_right = findCentroids(src, rectangles_right);

    DrawLine(src, rectangles_left, centroids_left);
    DrawLine(src, rectangles_right, centroids_right);

    Mat img_fill = Mat::zeros(src.size(), CV_8UC3);
    Point img_fill_point[4]
    {
       Point(centroids_left.back()),
       Point(centroids_left.front()),
       Point(centroids_right.front()),
       Point(centroids_right.back())
    };

    fillConvexPoly(img_fill, img_fill_point, 4, Scalar(255, 0, 0));
    dst = img_fill;
}

void AutoDriving::DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids)
{
    for (const auto& rect : rectangles)
    {
        rectangle(image, rect, Scalar(255, 0, 0), 3);
    }

    for (size_t i = 0; i < centroids.size() - 1; i++)
    {
        line(image, centroids[i], centroids[i + 1], Scalar(255, 0, 0), 7);
    }
}

vector<Point2f> AutoDriving::findCentroids(const Mat& roi, const vector<Rect>& rectangles)
{
    vector<Point2f> centroids;

    for (const Rect& rect : rectangles)
    {
        Mat roiLine = roi(rect);
        vector<vector<Point>> contours;
        findContours(roiLine, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        if (contours.empty())
        {
            Point2f centcircle(rect.x + rect.width / 2, rect.y + rect.height / 2);
            centroids.push_back(centcircle);
        }

        for (const vector<Point>& contour : contours)
        {
            double area = contourArea(contour);
            double minContourArea = 10.0;

            if (area >= minContourArea)
            {
                Moments mu = moments(contour);
                Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);
                centroids.push_back(Point2f(centroid.x + rect.x, centroid.y + rect.y));
            }

            else {
                Point2f centcircle(rect.x + rect.width / 2, rect.y + rect.height / 2);
                centroids.push_back(centcircle);
            }
        }
    }
    return centroids;
}

vector<Rect> AutoDriving::DivideRoi(Mat src, int x)
{
    vector<Rect> rectangles;
    int numRect = 10;
    int rectHeight = src.rows / numRect;
    for (int i = 0; i < numRect; i++)
    {
        int y1 = i * rectHeight;
        int y2 = (i + 1) * rectHeight;
        Rect rect(x, y1, 70, y2 - y1);
        rectangles.push_back(rect);
    }
    return rectangles;
}
