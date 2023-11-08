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

void GetContours(Mat src, Mat& dst);
void GetLines(Mat src, vector<Vec4i>& dst);
void DrawLines(vector<Vec4i> lines, Mat& dst);
void GetLane(Mat src, Mat& dst);

double CalcDirection(Mat& img_draw);

void PutText(Mat& draw, const bool isSleep, const double amount);

int main(int argc, char* argv[])
{
	VideoCapture video("Line.mp4");	// INPUT : 도로주행 동영상
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

		// 3. Image Preprocessing (Convert "RGB Color" Image to "HSV-Value + HSV-Yellow")

		// 4. Get Lanes

		// 5. Perspective Transform (2D -> 3D)

		// 6. 방향 구하기
		// 좌 / 직진 / 우

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
	각자 알고리즘 테스트 후 함수 작성해두기
*/
void ConvertColor(Mat src, Mat& dst);
void RemoveNoise(Mat src, Mat& dst);
void PreprocessFrame(Mat src, Mat& dst);

void GetContours(Mat src, Mat& dst);
void GetLines(Mat src, vector<Vec4i>& dst);
void DrawLines(vector<Vec4i> lines, Mat& dst);
void GetLane(Mat src, Mat& dst);

double CalcDirection(Mat& img_draw);

void PutText(Mat& draw, const bool isSleep, const double amount);