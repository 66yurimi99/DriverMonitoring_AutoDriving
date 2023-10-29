#pragma once
#include "Common.h"
#include <iostream>

const String window_capture_name1 = "1. Set Roi";
const String window_capture_name2 = "2. Top View";
const String window_capture_name3 = "3. Image Preprocessing";
const String window_capture_name4 = "4. Lane Detection";

void ConvertColor(Mat src, Mat& dst);
void RemoveNoise(Mat src, Mat& dst);
void PreprocessFrame(Mat src, Mat& dst);

int main(int argc, char* argv[])
{
	// 도로주행 영상 읽어오기
	
	// 1. Set ROI

	Mat img_frame;
	// while loop : 동영상 한 프레임씩 처리
	while (1) // video.read(img_frame)
	{
		// 2. Perspective Transform (3D -> 2D : Bird View)
		Mat img_top;
		
		// 3. Image Preprocessing
		Mat img_preprocessed;
		PreprocessFrame(img_top, img_preprocessed);

		// 4. Get Lanes
		Mat img_lane;

		// 5. Perspective Transform (2D -> 3D)
		Mat img_result;

		// 6. Calculate direction

		// 7. Put text


		// 최종 결과를 화면에 출력
		imshow(window_capture_name3, img_preprocessed);
		imshow(window_capture_name4, img_result);
		waitKey(1);
	}

	return 0;
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