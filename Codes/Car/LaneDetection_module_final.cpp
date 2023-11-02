#include "Common.h"
#include <iostream>

const String window_capture_name1 = "1. Set Roi";
const String window_capture_name2 = "2. Top View";
const String window_capture_name3 = "3. Image Preprocessing";
const String window_capture_name4 = "4. Lane Detection";

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

void ConvertColor(Mat src, Mat& dst);
void RemoveNoise(Mat src, Mat& dst);
void PreprocessFrame(Mat src, Mat& dst);

double CalcDirection(Mat& img_draw);

void PutText(Mat& draw, const bool isSleep, const double amount);

void LaneDetection(Mat& src, Mat& dst);
void DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids);
vector<Point2f> findCentroids(const Mat& roi, const vector<Rect>& rectangles);
vector<Rect> DivideRoi(Mat src, int x);

int main(int argc, char* argv[])
{
	VideoCapture video("input.mp4");	// INPUT : 도로주행 동영상
	if (!video.isOpened())	return -1;			// ERROR : 동영상 파일 열기 실패

	Mat img_frame;
	video.read(img_frame);

	width = img_frame.cols;
	height = img_frame.rows;

	Point2f srcPts[4] = { };
	Point2f dstPts[4] = {
		Point2f(0,0),
		Point2f(img_frame.cols * 0.3, 0),
		Point2f(img_frame.cols * 0.3, img_frame.rows * 0.3),
		Point2f(0, img_frame.rows * 0.3)
	};

	// 1. Set RoI (첫번째 frame에서 마우스 클릭으로 ROI 영역 지정)
	SetROI(img_frame, srcPts);

	// RoI 영역 지정이 완료된 후, 차선 검출 프로그램 시작
	namedWindow(window_capture_name4);
	while (video.read(img_frame))
	{
		// 2. Perspective Transform (3D -> 2D : top view)
		Mat img_top;
		TransPersfective(img_frame, img_top, srcPts, dstPts);

		// 3. Image Preprocessing
		Mat img_preprocessed;
		PreprocessFrame(img_top, img_preprocessed);

		// 4. Get Lanes
		Mat img_lane = img_preprocessed;
		LaneDetection(img_lane, img_top);
		
		// 5. Perspective Transform (2D -> 3D)
		Mat img_roi;
		TransPersfective(img_top, img_roi, dstPts, srcPts);

		Mat img_result;
		addWeighted(img_frame, 1, img_roi, 1, 0, img_result);

		// 6. 방향 구하기
		// 좌 / 직진 / 우

		/*
		7. Put Text

		Mat img_result;
		bool flag = true;
		PutText(img_result, flag, 0.0);
		*/
#if 0
		bool flag = true;
		if (flag)
		{
			putText(img_result, "Auto", Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
		}
#endif
		imshow(window_capture_name4, img_result);
		waitKey(1);
	}

	return 0;
}



/*
[1. ROI 설정]
마우스 클릭 이벤트 발생 시 호출되는 콜백 함수입니다.
- 클릭 포인트에 빨간 점 찍기
- 클릭 포인트를 RoI 사각형을 구성하는 포인트로 설정하기 (좌측 상단부터 시계 방향으로 지정)
*/
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
			imshow(window_capture_name1, img_roi);

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

/*
[1. ROI 설정] 첫번째 frame에서 마우스 클릭으로 ROI 영역 지정
인자로 받은 src 이미지에서 마우스 클릭으로 RoI 영역을 지정하는 함수입니다.

Parameters
- Mat src : 동영상의 첫번째 프레임 이미지
- Point2f* srcPts : RoI 영역을 이루는 사각형의 꼭지점 4개 (srcPts[4])
*/
void SetROI(Mat src, Point2f* srcPts)
{
	namedWindow(window_capture_name1);

	Mat img_roi = src.clone();
	UserData userdata = { img_roi, srcPts };

	putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
	imshow(window_capture_name1, img_roi); // ROI 설정을 위한 화면 출력

	// 마우스 클릭 이벤트 발생 시 콜백 함수(OnMouse) 호출
	setMouseCallback(window_capture_name1, OnMouse, &userdata);

	while (!ROISelect)
	{	// 20ms 간격으로 RoI 지정 여부 확인
		waitKey(20);
	}

	destroyWindow(window_capture_name1);
}

/*
[2. 시점 변경]
인자로 받은 src 이미지에서 네 점(srcPts[])이 이루는 영역을
같은 크기를 갖는 새 이미지의 네점(dstPts[])으로 투시 변환하는 함수입니다.

Parameters
- Mat src : 원본 이미지
- Point2f srcPts[] : 원본 이미지 상의 점 4개 (사각형의 꼭지점, 좌측 상단부터 시계방향)
- Point2f dstPts[] : 결과 이미지 상의 점 4개 (사각형의 꼭지점, 좌측 상단부터 시계방향)
*/
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


/*
[3. 이미지 전처리] 컬러 영역 변경
차선 검출이 용이하도록 컬러 이미지를 흰색, 노란색 영역으로 변경해주는 함수이다.
- Input : RGB 3-채널 이미지(src)
- Output : HSV(노랑), LAB(흰색)을 조합한 1-채널 이미지(dst)
*/
void ConvertColor(Mat src, Mat& dst)
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

/*
[3. 이미지 전처리] 노이즈 제거
차선 검출이 용이하도록 노이즈를 제거해주는 함수이다.
*/
void RemoveNoise(Mat src, Mat& dst)
{
	// Morphology Opening (erosion -> dilation)
	erode(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
	dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

/*
[3. 이미지 전처리]
영상 프레임에서 차선을 검출하기 전, 검출이 용이하도록 이미지를 전처리하는 함수이다.
*/
void PreprocessFrame(Mat src, Mat& dst)
{
	// 1) 색상 영역 변경
	ConvertColor(src, dst);

	// 2) 노이즈 제거 : Morphology Opening
	RemoveNoise(src, dst);
}


// 4. LaneDetection 
vector<Rect> DivideRoi(Mat src, int x)
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

vector<Point2f> findCentroids(const Mat& roi, const vector<Rect>& rectangles)
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

void DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids)
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

void LaneDetection(Mat& src, Mat& dst)
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

//	구현할것
double CalcDirection(Mat& img_draw);
void PutText(Mat& draw, const bool isSleep, const double amount);