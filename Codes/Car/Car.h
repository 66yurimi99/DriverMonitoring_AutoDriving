#pragma once
#include "Common.h"
#include <iostream>

using namespace std;

static bool ROISelect = false;
static string windowname = "1. Set Roi";
class AutoDriving {
public:
	AutoDriving();
	~AutoDriving();

private:
	//const string windowname="1. Set Roi";
	
	typedef struct UserData {
		Mat img;
		Point2f* pts;
	};
public:
	static void OnMouse(int event, int x, int y, int flags, void* userdata);
	void SetROI(Mat src, Point2f* srcPts, string windowname);

	void TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[], int width, int height);

	void ConvertColor(Mat src, Mat& dst);
	void RemoveNoise(Mat src, Mat& dst);
	void PreprocessFrame(Mat src, Mat& dst);
	void PutText(Mat& draw, const bool isSleep, const double amount);

	void LaneDetection(Mat& src, Mat& dst);
	void DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids);
	vector<Point2f> findCentroids(const Mat& roi, const vector<Rect>& rectangles);
	vector<Rect> DivideRoi(Mat src, int x);
	//아래는 구현해야함
	double CalcDirection(Mat& img_draw);
};