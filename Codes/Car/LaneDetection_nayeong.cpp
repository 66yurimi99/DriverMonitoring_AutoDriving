#include "ISP.h"
#include <opencv2/videoio.hpp> //VideoCapture() header

//roi 지정 방식->자동 인식 후 지정 방식으로 변경 필요
// 가상 차선 그리기
//+곡선 계산 알고리즘 추가

int main()
{
	//동영상 불러오기,,
	VideoCapture video("input.mp4");
	if (!video.isOpened())
	{
		cout << "동영상 파일을 열 수 없습니다. \n" << endl;
		return -1;
	}
	double fps = video.get(CAP_PROP_FPS); //동영상 fps값
	int delay = cvRound(1000 / fps); //cvRound-반올림함수
	Mat img_frame;

	while (true)
	{

		video >> img_frame;

		if (img_frame.empty())
		{
			break;
		}
		Mat src_color = img_frame.clone();
		Mat src_gray;
		if (src_color.channels() == 3)	cvtColor(src_color, src_gray, COLOR_BGR2GRAY);

		//canny 
		Mat edges;
		double low_threshold = 100;
		double high_threshold = 200;
		Canny(src_gray, edges, low_threshold, high_threshold);

		/*	bird's eyes view

			sp0	sp1		ds0	ds1
			sp2 sp3		ds2	ds3		*/
	
		Point2f srcPt[4], dstPt[4];
		srcPt[0] = Point2f(520, 480);
		srcPt[1] = Point2f(770, 480);
		srcPt[2] = Point2f(260, 650);
		srcPt[3] = Point2f(1160, 650);
		line(img_frame, srcPt[0], srcPt[1], Scalar(0, 255, 0), 2);
		line(img_frame, srcPt[0], srcPt[2], Scalar(0, 255, 0), 2);
		line(img_frame, srcPt[2], srcPt[3], Scalar(0, 255, 0), 2);
		line(img_frame, srcPt[3], srcPt[1], Scalar(0, 255, 0), 2);

		dstPt[0] = Point2f(0, 0);
		dstPt[1] = Point2f(210, 0);
		dstPt[2] = Point2f(0, 200);
		dstPt[3] = Point2f(210, 200);

		//orign -> birdview 
		Mat M = getPerspectiveTransform(srcPt, dstPt);
		Mat Inv_M = getPerspectiveTransform(dstPt, srcPt);
		Mat birdview;
		warpPerspective(src_color, birdview, M, Size(210, 200));

		//차선 색 분리
		//HSV -> yellow, Lab-> white
		Mat src_HSV = Mat::zeros(birdview.rows, birdview.cols, CV_8UC1);
		Mat YLine = Mat::zeros(birdview.rows, birdview.cols, CV_8UC1);
		cvtColor(birdview, src_HSV, COLOR_BGR2HSV);

		Mat src_Lab = Mat::zeros(birdview.rows, birdview.cols, CV_8UC1);
		Mat WLine = Mat::zeros(birdview.rows, birdview.cols, CV_8UC1);
		cvtColor(birdview, src_Lab, COLOR_BGR2Lab);

		//HSV range -> Yellow + White
		Scalar low_yellow = Scalar(8, 100, 100);
		Scalar high_yellow = Scalar(30, 255, 255);
		Scalar low_white = Scalar(0, 0, 200);
		Scalar high_white = Scalar(180, 255, 255);
		
		inRange(src_HSV, low_yellow, high_yellow, YLine);
		inRange(src_HSV, low_white, high_white, WLine);

		/* //더 해보려고 했지만 그만
		//Lab range -> White + Yellow
		Mat Line_Lab = Mat::zeros(birdview.rows, birdview.cols, CV_8UC1);
		low_white = Scalar(200, 120, 120);
		//Scalar high_white = Scalar(255, 130, 140); // only White
		high_white = Scalar(255, 255, 255);//white+yellow
		
		inRange(src_Lab, low_white, high_white, Line_Lab);*/

		Mat Line_img;
		Mat LineContoure;
				
		bitwise_and(birdview, birdview, Line_img, YLine);
		bitwise_and(birdview, birdview, Line_img, WLine);
		bitwise_or(YLine, WLine, LineContoure);
		
		
		// 4. Get Lines
		Mat img_bin;
		Mat img_test_2 = LineContoure;
		//medianBlur(img_test_2, img_test_2, 3);
		threshold(img_test_2, img_bin, 128, 255, ThresholdTypes::THRESH_OTSU);
		erode(img_bin, img_bin, Mat::ones(Size(5, 5), CV_8UC1), Point(-1, -1));
		dilate(img_bin, img_bin, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
		

		//높이 방향 직사각형으로 영역 나누기
		int numRect = 10; //한 프레임을 몇개의 직사각형으로 나눌건지
		int rectWidth = 70;
		int rectHeight = birdview.rows / numRect; // 20

		// 직사각형 위치를 저장하는 벡터 초기화
		vector<Rect> L_rectangles;
		vector<Rect> R_rectangles;
		for (int i = 0; i < numRect; i++)
		{
			// 직사각형 좌표 계산
			int L_x1 = 0;
			int R_x1 = birdview.cols - rectWidth;
			int y1 = i * rectHeight;
			int x2 = rectWidth;
			int y2 = (i + 1) * rectHeight;

			Rect L_rect(L_x1, y1, x2, y2 - y1);
			Rect R_rect(R_x1, y1, x2, y2 - y1);

			L_rectangles.push_back(L_rect);
			R_rectangles.push_back(R_rect);
		}

		//디버깅
		for (const auto& L_rect : L_rectangles)
		{
			rectangle(img_bin, L_rect, Scalar(0, 255, 0), 1); // 직사각형 (초록색)
		}

		for (const auto& R_rect : R_rectangles)
		{
			rectangle(img_bin, R_rect, Scalar(0, 255, 0), 1); // 직사각형 (초록색)
		}

		// 중심 좌표를 저장할 벡터 초기화
		vector<Point2f> L_centroids; //컨투어 중심좌표
		Point2f L_previousCentroid; //이전 컨투어 중심좌표

		for (const Rect& rect : L_rectangles)
		{
			//Mat roi = LineContoure(rect);
			Mat roi = img_bin(rect);
			// 컨투어 검출
			vector<vector<Point>> contours;
			findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

			// 검출된 컨투어 처리
			if (contours.empty())
			{
				if (!L_centroids.empty())
				{
					L_previousCentroid.y += rectHeight; // 이전컨투어 중심좌표에 현재 사각형의 높이만 더해서 같은 위치에 점 그리기
					L_centroids.push_back(L_previousCentroid);
					circle(birdview, L_previousCentroid, 4, Scalar(0, 255, 0), -1);
				}
				else if (L_centroids.empty())
				{
					// 직사각형 중심 좌표 계산
					Point2f L_center(rect.x + rect.width / 2, rect.y + rect.height / 2);
					L_previousCentroid = L_center;
					L_centroids.push_back(L_center);
					circle(birdview, L_center, 4, Scalar(0, 255, 0), -1);
				}
				continue;
			}
			else
			{
				for (const vector<Point>& contour : contours)
				{
					// 면적 계산
					double area = contourArea(contour);
					double minContourArea = 1.0;

					if (area >= minContourArea)
					{
						// 컨투어의 모멘트 계산
						Moments mu = moments(contour);

						// 중심 좌표 계산
						Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);


						// 컨투어 중심 좌표에 원 그리기
						Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
						L_centroids.push_back(centcircle);
						L_previousCentroid = centcircle;
						circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
					}
				}
			}
		}
	

		//중심점과 중심좌표로 움직인 직사각형 영역 그리기
		int halfWidth = 35; // 가로 폭의 절반
		int halfHeight = 10; // 세로 높이의 절반

		for (size_t i = 0; i < L_centroids.size(); i++)
		{
			Point topLeft(L_centroids[i].x - halfWidth, L_centroids[i].y - halfHeight);
			Point bottomRight(L_centroids[i].x + halfWidth, L_centroids[i].y + halfHeight);
			// 사각형 그리기
			rectangle(birdview, topLeft, bottomRight, Scalar(0, 255, 255), 1);
		}

		//각 직사각형 영역(roi)내의 중심좌표를 이어서 한 라인으로 그리기
		Mat L_roiLine = Mat::zeros(birdview.size(), CV_8UC3);
		for (size_t i = 0; i < L_centroids.size() - 1; i++)
		{
			line(L_roiLine, L_centroids[i], L_centroids[i + 1], Scalar(242, 245, 66), 5);
		}
		// 결과 이미지에 라인을 그린 이미지를 추가
		addWeighted(birdview, 1, L_roiLine, 0.5, 0, birdview);

		//Right
		vector<Point2f> R_centroids; //컨투어 중심좌표
		Point2f R_previousCentroid; //이전 컨투어 중심좌표
		for (const Rect& rect : R_rectangles)
		{
			//Mat roi = LineContoure(rect);
			Mat roi = img_bin(rect);
			// 컨투어 검출
			vector<vector<Point>> contours;
			findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

			// 검출된 컨투어 처리
			if (contours.empty())
			{
				if (!R_centroids.empty())
				{
					R_previousCentroid.y += rectHeight; // 이전컨투어 중심좌표에 현재 사각형의 높이만 더해서 같은 위치에 점 그리기
					R_centroids.push_back(R_previousCentroid);
					circle(birdview, R_previousCentroid, 4, Scalar(0, 255, 0), -1);
				}
				else if (R_centroids.empty())//이전 컨투어 정보가 없을 때
				{
					// 직사각형 중심 좌표 계산
					Point2f R_center(rect.x + rect.width / 2, rect.y + rect.height / 2);
					R_previousCentroid = R_center;
					R_centroids.push_back(R_center);
					circle(birdview, R_center, 4, Scalar(0, 255, 0), -1);
				}
				continue;
			}
			else
			{
				for (const vector<Point>& contour : contours)
				{
					// 면적 계산
					double area = contourArea(contour);
					double minContourArea = 1.0; 

					if (area >= minContourArea)
					{
						// 컨투어의 모멘트 계산
						Moments mu = moments(contour);

						// 중심 좌표 계산
						Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);

						// 컨투어 중심 좌표에 원 그리기
						Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
						R_centroids.push_back(centcircle);
						R_previousCentroid = centcircle;
						circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
					}
				}
			}
		}
		for (size_t i = 0; i < R_centroids.size(); i++)
		{
			Point topLeft(R_centroids[i].x - halfWidth, R_centroids[i].y - halfHeight);
			Point bottomRight(R_centroids[i].x + halfWidth, R_centroids[i].y + halfHeight);
			
			rectangle(birdview, topLeft, bottomRight, Scalar(0, 255, 255), 1);// 사각형 그리기
		}

		Mat R_roiLine = Mat::zeros(birdview.size(), CV_8UC3);
		for (size_t i = 0; i < R_centroids.size() - 1; i++)//각 직사각형 영역(roi)내의 중심좌표를 이어서 한 라인으로 그리기
		{
			line(R_roiLine, R_centroids[i], R_centroids[i + 1], Scalar(242, 245, 66), 5);
		}

		// 결과 이미지에 라인을 그린 이미지를 추가
		addWeighted(birdview, 1, R_roiLine, 0.5, 0, birdview);
		
		//birdview -> orign
		warpPerspective(birdview, src_color, Inv_M, Size(src_color.cols, src_color.rows));
		addWeighted(src_color, 0.5, img_frame, 0.5, 0, src_color);

		imshow("Lane_detection_video", src_color);
		imshow("roi_video", img_frame);
		imshow("birdview_Line", Line_img);
		imshow("birdview", birdview);
		if (waitKey(delay) == 27) break;

	}
	destroyAllWindows();
}