#pragma once
#include "Common.h"

static bool ROISelect = false;
static string windowname = "1. Set Roi";

class Car {
public:
	Car();
	~Car();

private:
	typedef struct UserData {
		Mat img;
		Point2f* pts;
	};

	double prev_average_left = 0.0;
	double prev_average_right = 0.0;

private:
	/*** 1. RoI 설정 : 마우스 클릭으로 RoI 사각형을 구성하는 네 점의 좌표 설정 (클릭 순서: 좌측 상단부터 시계 방향) ***/
	static void OnMouse(int event, int x, int y, int flags, void* userdata);

	/*** 3. 이미지 전처리 : 차선 검출이 용이하도록 이미지 변환 ***/
	void ConvertColor(Mat src, Mat& dst);
	void RemoveNoise(Mat src, Mat& dst);

	/*** 4. 차선 검출 및 주행 방향 계산 ***/
	double GetAverage(vector<Point2f> centroids);
	double CalcDirection(double left, double right);

	vector<Rect> DivideRoi(Mat src, int x, int divide_flag);
	vector<Point2f> findCentroids(const Mat& roi, const vector<Rect>& rectangles);
	void DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids);

public:
	/*** 1. RoI 설정 : 마우스 클릭으로 RoI 사각형을 구성하는 네 점의 좌표 설정 (클릭 순서: 좌측 상단부터 시계 방향) ***/
	void SetROI(Mat src, Point2f* srcPts, string windowname);

	/*** 2. 시점 변경 : 투시 변환을 통한 Original View(3D) <-> Top View(2D) 상호 전환 ***/
	void TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[], int width, int height);

	/*** 3. 이미지 전처리 : 차선 검출이 용이하도록 이미지 변환 ***/
	void PreprocessFrame(Mat src, Mat& dst);

	/*** 4. 차선 검출 및 주행 방향 계산 ***/
	double LaneDetection(Mat& src, Mat& dst);

	/*** 5. 결과 데이터 출력 : 결과 데이터(자동주행 여부 및 주행 방향)를 화면에 출력 ***/
	void PutText(Mat& draw, const bool isSleep, const double amount);

};