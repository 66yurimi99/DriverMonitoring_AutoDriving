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

double CalcDirection(Mat& img_draw);

void PutText(Mat& draw, const bool isSleep, const double amount);

void LaneDetection(const Mat& img_topview, Mat& src_color);
void DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids);
vector<Point2f> findCentroids(const Mat& roi, const vector<Rect>& rectangles);
vector<Rect> DivideRoi(Mat src, int x);

int main(int argc, char* argv[])
{
	VideoCapture video("Line.mp4");	// INPUT : �������� ������
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
	namedWindow(window_capture_name4);
	while (video.read(img_frame))
	{
		// 2. Perspective Transform (3D -> 2D : top view)

		// 3. Image Preprocessing (Convert "RGB Color" Image to "HSV-Value + HSV-Yellow")

		// 4. Get Lanes

		// 5. Perspective Transform (2D -> 3D)

		// 6. ���� ���ϱ�
		// �� / ���� / ��

		// 7. Put Text

		Mat img_result;
		bool flag = true;
		PutText(img_result, flag, 0.0);
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
			imshow(window_capture_name1, img_roi);

			if (cnt == 4)
			{
				// x ��ǥ ���� : RoI �簢�� �ʺ� ���� �ʺ�� ����ϵ��� x ��ǥ�� ����
				for (size_t i = 0; i < 4; i = i + 2)
				{
					int margin = (srcPts[i].x - srcPts[i + 1].x) * 0.2;
					srcPts[i].x += margin;
					srcPts[i + 1].x += (-margin);
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
	namedWindow(window_capture_name1);

	Mat img_roi = src.clone();
	UserData userdata = { img_roi, srcPts };

	putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
	imshow(window_capture_name1, img_roi); // ROI ������ ���� ȭ�� ���

	// ���콺 Ŭ�� �̺�Ʈ �߻� �� �ݹ� �Լ�(OnMouse) ȣ��
	setMouseCallback(window_capture_name1, OnMouse, &userdata);

	while (!ROISelect)
	{	// 20ms �������� RoI ���� ���� Ȯ��
		waitKey(20);
	}

	destroyWindow(window_capture_name1);
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
	���� �˰��� �׽�Ʈ �� �Լ� �ۼ��صα�
*/

// LaneDetection 
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

void LaneDetection(const Mat& img_topview, Mat& src_color)
{

	vector<Rect> rectangles_left = DivideRoi(src_color, 0);
	vector<Point2f> centroids_left = findCentroids(src_color, rectangles_left);

	vector<Rect> rectangles_right = DivideRoi(src_color, 140);
	vector<Point2f> centroids_right = findCentroids(src_color, rectangles_right);

	DrawLine(src_color, rectangles_left, centroids_left);
	DrawLine(src_color, rectangles_right, centroids_right);

	Point img_fill_point[4]
	{
		Point(centroids_left.back()),
		Point(centroids_left.front()),
		Point(centroids_right.front()),
		Point(centroids_right.back())
	};

	fillConvexPoly(img_topview, img_fill_point, 4, Scalar(255, 0, 0));
}

//	�����Ұ�
double CalcDirection(Mat& img_draw);
void PutText(Mat& draw, const bool isSleep, const double amount);