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
	VideoCapture video("input_1st_lane.mp4");	// INPUT : �������� ������
	if (!video.isOpened())	return -1;			// ERROR : ������ ���� ���� ����

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

	// 1. Set RoI (ù��° frame���� ���콺 Ŭ������ ROI ���� ����)
	SetROI(img_frame, srcPts);

	// RoI ���� ������ �Ϸ�� ��, ���� ���� ���α׷� ����
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

		// 6. ���� ���ϱ�
		// �� / ���� / ��
		

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
[1. ROI ����] 
���콺 Ŭ�� �̺�Ʈ �߻� �� ȣ��Ǵ� �ݹ� �Լ��Դϴ�.
- Ŭ�� ����Ʈ�� ���� �� ���
- Ŭ�� ����Ʈ�� RoI �簢���� �����ϴ� ����Ʈ�� �����ϱ� (���� ��ܺ��� �ð� �������� ����)
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
			// ������ ��ġ�� ������ �׷��� �����ֱ�
			srcPts[cnt++] = Point2f(x, y);
			circle(img_roi, Point(x, y), 5, Scalar(255, 0, 255), -1);
			imshow(window_roi_name, img_roi);

			if (cnt == 4) 
			{
				// x ��ǥ ���� : RoI �簢�� �ʺ� ���� �ʺ�� ����ϵ��� x ��ǥ�� ����
				for (size_t i = 0; i < 4; i=i+2)
				{
					int margin = (srcPts[i].x - srcPts[i+1].x) * 0.2;
					srcPts[i].x += margin;
					srcPts[i+1].x += (-margin);
				}

				// y ��ǥ ���� : RoI �簢���� ����, �Ʒ����� �̷�� �� ���� y ��ǥ�� ��ġ��Ű��
				srcPts[1].y = srcPts[0].y;
				srcPts[3].y = srcPts[2].y;

				ROISelect = true;
			}
		}
	}
}

/*
[1. ROI ����] ù��° frame���� ���콺 Ŭ������ ROI ���� ����
���ڷ� ���� src �̹������� ���콺 Ŭ������ RoI ������ �����ϴ� �Լ��Դϴ�.

Parameters
- Mat src : �������� ù��° ������ �̹���
- Point2f* srcPts : RoI ������ �̷�� �簢���� ������ 4�� (srcPts[4])
*/
void SetROI(Mat src, Point2f* srcPts)
{
	namedWindow(window_roi_name);

	Mat img_roi = src.clone();
	UserData userdata = { img_roi, srcPts };

	putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
	imshow(window_roi_name, img_roi); // ROI ������ ���� ȭ�� ���

	// ���콺 Ŭ�� �̺�Ʈ �߻� �� �ݹ� �Լ�(OnMouse) ȣ��
	setMouseCallback(window_roi_name, OnMouse, &userdata);

	while (!ROISelect)
	{	// 20ms �������� RoI ���� ���� Ȯ��
		waitKey(20);
	}

	destroyWindow(window_roi_name);
}

/*
[2. ���� ����]
���ڷ� ���� src �̹������� �� ��(srcPts[])�� �̷�� ������
���� ũ�⸦ ���� �� �̹����� ����(dstPts[])���� ���� ��ȯ�ϴ� �Լ��Դϴ�.

Parameters
- Mat src : ���� �̹���
- Point2f srcPts[] : ���� �̹��� ���� �� 4�� (�簢���� ������, ���� ��ܺ��� �ð����)
- Point2f dstPts[] : ��� �̹��� ���� �� 4�� (�簢���� ������, ���� ��ܺ��� �ð����)
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
[3. �̹��� ��ó��] �÷� ���� ����
���� ������ �����ϵ��� �÷� �̹����� ���, ����� �������� �������ִ� �Լ��̴�.
- Input : RGB 3-ä�� �̹���(src)
- Output : HSV Value ä�ΰ� Yellow ���� ä���� ������ 1-ä�� �̹���
*/
void ConvertColor(Mat src, Mat& dst)
{
	Mat img_hsv, img_value, img_yellow;

	// Convert RBG Color to HSV-Value
	cvtColor(src, img_hsv, COLOR_BGR2HSV);

	// HSV -> H, S, V ä�� ����
	vector<Mat> HSV_channels(3);
	split(img_hsv, HSV_channels);

	// 1) HSV-Value �� ����
	img_value = HSV_channels[2];

	// 2) HSV �÷� �������� ����� ���� ����
	Scalar lower_yellow(15, 120, 200);
	Scalar upper_yellow(25, 255, 255);
	inRange(img_hsv, lower_yellow, upper_yellow, img_yellow);

	// 3) HSV-Value + HSV-Yellow
	addWeighted(img_value, 1, img_yellow, 0.3, 0, dst);
}

/*
[3. �̹��� ��ó��] ������ ����
���� ������ �����ϵ��� ����� �������ִ� �Լ��̴�.
*/
void RemoveNoise(Mat src, Mat& dst)
{
	// Morphology Opening (erosion -> dilation)
	erode(dst, dst, Mat::ones(Size(5, 5), CV_8UC1), Point(-1, -1));
	dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

/*
[3. �̹��� ��ó��]
���� �����ӿ��� ������ �����ϱ� ��, ������ �����ϵ��� �̹����� ��ó���ϴ� �Լ��̴�.
*/
void PreprocessFrame(Mat src, Mat& dst)
{
	// 1) �̹��� ��⸦ ��Ӱ� ���� (���� ���ο����� ������ �� �����ϱ� ����)
	Mat tmp = src * 0.5;

	// 2) ���� ���� ����
	ConvertColor(src, tmp);
	
	// 3) 1�� ������ ���� : �̹��� ����
	medianBlur(tmp, tmp, 3);

	// 4) ����ȭ : ���� �˰��� ���
	threshold(tmp, tmp, 0, 255, ThresholdTypes::THRESH_OTSU);

	// 5) 2�� ������ ���� : Morphology Opening
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
	
	int lowThreshold = 50;  // �Ӱ谪 (���� ����)
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

			// ����(m) ���
			int x1 = line[0];
			int y1 = line[1];
			int x2 = line[2];
			int y2 = line[3];
			float m = 1.0 * (y2 - y1) / (x2 - x1);
			m = (m < 0) ? -m : m;	// ���� ������ ���� ��� (���� ���� ���� ������ ���� ���)

			// ����(��) ��� (����: ����)
			float angle_radian = atan(m);

			// ���� ��ȯ (����: ���� -> ��)
			float angle_degree = angle_radian * 180.0 / CV_PI;

			// Ư�� ���� ������ ��� (�߽� ���μ�=90���� �������� �¿� 30�� ����)
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
[7. ��� ������ ���]
��� ������(���� ����, �ڵ����� ���� ��)�� ȭ�鿡 ����ϴ� �Լ��Դϴ�
- ���� ���� : Left / Right / Straight
- �ڵ����� ���� : Auto / Manual

Parameters
- bool isSleep : ���� ���θ� �˷��ִ� �÷���. if true, "Auto".
- double amount : ��� �߽������� �Ÿ� (x ��ǥ)
	1) -5 �̸� :	"Left"
	2) -5 ~ 5 ���� : "Straight"
	3) 5 �ʰ� : "Right"
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
