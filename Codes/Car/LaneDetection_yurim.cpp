#pragma once
#include "Common.h"
#include <iostream>

using namespace cv;

const String window_roi_name = "1. RoI Selection";
const String window_detection_name = "2. Lane Detection";

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

void GetContours(Mat src, Mat& dst);
void GetLines(Mat src, vector<Vec4i>& dst);
void DrawLines(vector<Vec4i> lines, Mat& dst);
void GetLane(Mat src, Mat& dst);

double CalcDirection(Mat& img_draw);

void PutText(Mat& draw, const bool isSleep, const double amount);


int main(int argc, char* argv[])
{
	VideoCapture video("input_1st_lane.mp4");	// INPUT : 도로주행 동영상
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
	namedWindow(window_detection_name);
	while (video.read(img_frame))
	{
		// 2. Perspective Transform (3D -> 2D : top view)
		Mat img_top;
		TransPersfective(img_frame, img_top, srcPts, dstPts);

		// 3. Image Preprocessing (Convert "RGB Color" Image to "HSV-Value + HSV-Yellow")
		Mat img_preprocessed;
		PreprocessFrame(img_top, img_preprocessed);

		// 4. Get Lanes
		Mat img_lane = Mat::zeros(img_preprocessed.size(), CV_8UC3);
		GetLane(img_preprocessed, img_lane);

		// 5. Perspective Transform (2D -> 3D)
		Mat img_roi;
		TransPersfective(img_lane, img_roi, dstPts, srcPts);
		
		Mat img_result;
		addWeighted(img_frame, 1, img_roi, 0.5, 0, img_result);

		// 6. 방향 구하기
		// 좌 / 직진 / 우
		

		// 7. Put Text
		bool flag = true;
		PutText(img_result, flag, 0.0);
#if 0
		bool flag = true;
		if (flag)
		{
			putText(img_result, "Auto", Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
		}
#endif
		imshow(window_detection_name, img_result);
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
			imshow(window_roi_name, img_roi);

			if (cnt == 4) 
			{
				// x 좌표 설정 : RoI 사각형 너비가 차선 너비와 비례하도록 x 좌표값 조정
				for (size_t i = 0; i < 4; i=i+2)
				{
					int margin = (srcPts[i].x - srcPts[i+1].x) * 0.2;
					srcPts[i].x += margin;
					srcPts[i+1].x += (-margin);
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
- Output : HSV Value 채널과 Yellow 색상 채널을 조합한 1-채널 이미지
*/
void ConvertColor(Mat src, Mat& dst)
{
	Mat img_hsv, img_value, img_yellow;

	// Convert RBG Color to HSV-Value
	cvtColor(src, img_hsv, COLOR_BGR2HSV);

	// HSV -> H, S, V 채널 추출
	vector<Mat> HSV_channels(3);
	split(img_hsv, HSV_channels);

	// 1) HSV-Value 값 추출
	img_value = HSV_channels[2];

	// 2) HSV 컬러 영역에서 노란색 영역 추출
	Scalar lower_yellow(15, 120, 200);
	Scalar upper_yellow(25, 255, 255);
	inRange(img_hsv, lower_yellow, upper_yellow, img_yellow);

	// 3) HSV-Value + HSV-Yellow
	addWeighted(img_value, 1, img_yellow, 0.3, 0, dst);
}

/*
[3. 이미지 전처리] 노이즈 제거
차선 검출이 용이하도록 노이즈를 제거해주는 함수이다.
*/
void RemoveNoise(Mat src, Mat& dst)
{
	// Morphology Opening (erosion -> dilation)
	erode(dst, dst, Mat::ones(Size(5, 5), CV_8UC1), Point(-1, -1));
	dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

/*
[3. 이미지 전처리]
영상 프레임에서 차선을 검출하기 전, 검출이 용이하도록 이미지를 전처리하는 함수이다.
*/
void PreprocessFrame(Mat src, Mat& dst)
{
	// 1) 이미지 밝기를 어둡게 조정 (밝은 도로에서도 차선을 잘 검출하기 위해)
	Mat tmp = src * 0.5;

	// 2) 색상 영역 변경
	ConvertColor(src, tmp);
	
	// 3) 1차 노이즈 제거 : 이미지 블러링
	medianBlur(tmp, tmp, 3);

	// 4) 이진화 : 오츠 알고리즘 사용
	threshold(tmp, tmp, 0, 255, ThresholdTypes::THRESH_OTSU);

	// 5) 2차 노이즈 제거 : Morphology Opening
	erode(tmp, dst, Mat::ones(Size(5, 5), CV_8UC1), Point(-1, -1));
	dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

// 4. Get Lines
//     4-3. Find Contours
//     4-4. Get Canny Edges
//     4-5. Get Hough Lines
// 5. Draw Lines (filter by angle)
void GetContours(Mat src, Mat& dst)
{
	// Find Contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	// Draw Contours
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if (contours[i].size() < 10) continue;
		if (area < 100) continue;
		drawContours(dst, contours, i, CV_RGB(255, 255, 255), LineTypes::FILLED);
	}
}

void GetLines(Mat src, vector<Vec4i>& dst)
{
	Mat edges;
	
	int lowThreshold = 50;  // 임계값 (조절 가능)
	Canny(src, edges, lowThreshold, lowThreshold * 3, 3);
	HoughLinesP(edges, dst, 1, CV_PI / 180, 10, 30, 30);
}

void DrawLines(vector<Vec4i> lines, Mat& dst)
{
	if (!lines.empty())
	{
		for (size_t i = 0; i < lines.size(); i++)
		{
			Vec4i line = lines[i];

			// 기울기(m) 계산
			int x1 = line[0];
			int y1 = line[1];
			int x2 = line[2];
			int y2 = line[3];
			float m = 1.0 * (y2 - y1) / (x2 - x1);
			m = (m < 0) ? -m : m;	// 직선 기울기의 절댓값 계산 (직선 각도 범위 지정을 위해 사용)

			// 각도(θ) 계산 (단위: 라디안)
			float angle_radian = atan(m);

			// 각도 변환 (단위: 라디안 -> 도)
			float angle_degree = angle_radian * 180.0 / CV_PI;

			// 특정 각도 범위만 얻기 (중심 세로선=90도를 기준으로 좌우 30도 범위)
			if (angle_degree > 60 && angle_degree < 120)
			{
				cv::line(dst, Point(line[0], line[1]), Point(line[2], line[3]), Scalar(255, 0, 255), 5);
			}
		}
	}
}

void GetLane(Mat src, Mat& dst)
{
	Mat contours = Mat::zeros(src.size(), CV_8UC1);
	GetContours(src, contours);

	vector<Vec4i> lines;
	GetLines(contours, lines);
	DrawLines(lines, dst);
}

// 6. Calculate Direction
double CalcDirection(Mat& img_draw);


/* 
[7. 결과 데이터 출력]
결과 데이터(주행 방향, 자동주행 여부 등)를 화면에 출력하는 함수입니다
- 주행 방향 : Left / Right / Straight
- 자동주행 여부 : Auto / Manual

Parameters
- bool isSleep : 졸음 여부를 알려주는 플래그. if true, "Auto".
- double amount : 평균 중심점과의 거리 (x 좌표)
	1) -5 미만 :	"Left"
	2) -5 ~ 5 사이 : "Straight"
	3) 5 초과 : "Right"
*/
void PutText(Mat& draw, const bool isSleep, const double amount)
{
	String driving_mode = isSleep ? "Auto" : "Manual";
	String direction;

	if (amount < -5)	direction = "Left";
	else if (amount < 5)	direction = "Straight";
	else	direction = "Right";

	putText(draw, driving_mode, Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
	putText(draw, direction, Point(20, 120), 1, 4, Scalar(0, 255, 255), 5);
}
