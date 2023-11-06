#include "Car.h"

const String window_capture_name1 = "1. Set Roi";
const String window_capture_name2 = "2. Top View";
const String window_capture_name3 = "3. Image Preprocessing";
const String window_capture_name4 = "4. Lane Detection";


int main(int argc, char* argv[])
{   
    AutoDriving* _car = new AutoDriving();

    VideoCapture video("lane_detect.mp4");   // INPUT : 도로주행 동영상
    if (!video.isOpened()) {
        cout << "video is not opened" << endl;
        return -1;         // ERROR : 동영상 파일 열기 실패
    }
    Mat img_frame;
    video.read(img_frame);

    int width = img_frame.cols;
    int height = img_frame.rows;
    bool ROISelect = false;

    Point2f srcPts[4] = { };
    Point2f dstPts[4] = {
       Point2f(0,0),
       Point2f(img_frame.cols * 0.3, 0),
       Point2f(img_frame.cols * 0.3, img_frame.rows * 0.3),
       Point2f(0, img_frame.rows * 0.3)
    };

    // 1. Set RoI (첫번째 frame에서 마우스 클릭으로 ROI 영역 지정)
    _car->SetROI(img_frame, srcPts, window_capture_name1);

    // RoI 영역 지정이 완료된 후, 차선 검출 프로그램 시작
    namedWindow(window_capture_name4);
    while (video.read(img_frame))
    {
        // 2. Perspective Transform (3D -> 2D : top view)
        Mat img_top;
        _car->TransPersfective(img_frame, img_top, srcPts, dstPts, width, height);

        // 3. Image Preprocessing
        Mat img_preprocessed;
        _car->PreprocessFrame(img_top, img_preprocessed);

        // 4. Get Lanes
        Mat img_lane = img_preprocessed;
        _car->LaneDetection(img_lane, img_top);

        // 5. Perspective Transform (2D -> 3D)
        Mat img_roi;
        _car->TransPersfective(img_top, img_roi, dstPts, srcPts, width, height);

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
