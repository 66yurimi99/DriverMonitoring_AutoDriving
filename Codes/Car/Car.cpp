#include "Car.h"

Car::Car()
{
    cout << "Start AutoDriving mode..." << endl;
}

Car::~Car()
{
    cout << "End AutoDriving mode..." << endl;
}

/*** 1. RoI 설정 : 마우스 클릭으로 RoI 사각형을 구성하는 네 점의 좌표 설정 (클릭 순서: 좌측 상단부터 시계 방향) ***/
// 1-1. 마우스 클릭 이벤트에 대한 콜백 함수
void Car::OnMouse(int event, int x, int y, int flags, void* userdata)
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

// 1-2. 첫번째 frame에서 마우스 클릭으로 ROI 영역 지정 (PUBLIC)
void Car::SetROI(Mat src, Point2f* srcPts, string windowname)
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


/*** 2. 시점 변경 : 투시 변환을 통한 Original View(3D) <-> Top View(2D) 상호 전환 ***/
// 2-1. 시점 변경 (PUBLIC)
void Car::TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[], int width, int height)
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


/*** 3. 이미지 전처리 : 차선 검출이 용이하도록 이미지 변환 ***/
// 3-1. 컬러 영역 변경 : RGB 3채널 -> HSV(노랑)+LAB(흰색) 1채널 이미지
void Car::ConvertColor(Mat src, Mat& dst)
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

// 3-2. 노이즈 제거
void Car::RemoveNoise(Mat src, Mat& dst)
{
    // Morphology Opening (erosion -> dilation)
    erode(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
    dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

// 3-3. 이미지 전처리 함수 (PUBLIC)
void Car::PreprocessFrame(Mat src, Mat& dst)
{
    // 1) 색상 영역 변경
    ConvertColor(src, dst);

    // 2) 노이즈 제거 : Morphology Opening
    RemoveNoise(src, dst);
}


/*** 4. 차선 검출 및 주행 방향 계산 ***/
// 4-1. 주행 방향 계산
// 4-1-1. x 좌표 평균 구하기
double Car::GetAverage(vector<Point2f> centroids)
{
	float sum_x = 0.0;
	for (const Point2f& point : centroids) {
		sum_x += point.x;
	}

	float average_x = sum_x / centroids.size();

	return average_x;
}
// 4-1-2. 방향 계산
double Car::CalcDirection(double left, double right)
{
	double diff = (prev_average_left - left) + (prev_average_right - right);

	prev_average_left = left;
	prev_average_right = right;

	return diff;
}

// 4-2. 차선 검출
// 4-2-1. RoI 영역을 좌우 2개로 분할
vector<Rect> Car::DivideRoi(Mat src, int x, int divide_flag)
{
    vector<Rect> rectangles;
    int numRect = 10;
    int rectHeight = src.rows / numRect;
    if (divide_flag == 0)
    {
        for (int i = 0; i < numRect; i++)
        {
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;
            
            Rect rect(0, y1, x/2, y2 - y1);
            rectangles.push_back(rect);
        }
    }
    else if (divide_flag == 1)
    {
        for (int i = 0; i < numRect; i++)
        {
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;

            Rect rect(x / 2, y1, x / 2, y2 - y1);
            rectangles.push_back(rect);
        }
    }

    return rectangles;
}
// 4-2-2. 차선으로 이어줄 점들의 좌표 구하기
vector<Point2f> Car::findCentroids(const Mat& roi, const vector<Rect>& rectangles)
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
// 4-2-3. 검출된 차선을 기준으로 도로 영역 그리기
void Car::DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids)
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

// 4-3. 차선 검출 (PUBLIC)
double Car::LaneDetection(Mat& src, Mat& dst)
{
    vector<Rect> rectangles_left = DivideRoi(src, src.cols,0);
    vector<Point2f> centroids_left = findCentroids(src, rectangles_left);
	double average_left = GetAverage(centroids_left);
	
    vector<Rect> rectangles_right = DivideRoi(src, src.cols,1);
    vector<Point2f> centroids_right = findCentroids(src, rectangles_right);
	double average_right = GetAverage(centroids_right);
	double amount = CalcDirection(average_left, average_right);
	
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
	
	return amount;
}


/*** 5. 결과 데이터 출력 : 결과 데이터(자동주행 여부 및 주행 방향)를 화면에 출력 ***/
// 5-1. 결과 데이터 출력 (PUBLIC)
void Car::PutText(Mat& draw, const bool isSleep, const double amount)
{
    String driving_mode = isSleep ? "Auto" : "Manual";
    String direction;
    if (amount < -5)	direction = "Left";
    else if (amount < 5)	direction = "Straight";
    else	direction = "Right";
    putText(draw, driving_mode, Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
    putText(draw, direction, Point(20, 120), 1, 4, Scalar(0, 255, 255), 5);
}
