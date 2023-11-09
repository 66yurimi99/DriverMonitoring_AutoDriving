#include "Car.h"

const String window_capture_name1 = "1. Set Roi";
const String window_capture_name2 = "2. Top View";
const String window_capture_name3 = "3. Image Preprocessing";
const String window_capture_name4 = "4. Lane Detection";

int main(int argc, char* argv[])
{
    Car* _car = new Car();

    VideoCapture video("input3.mp4");   // INPUT : 도로주행 동영상
    if (!video.isOpened()) {
        cout << "video is not opened" << endl;
        return -1;         // ERROR : 동영상 파일 열기 실패
    }

    dnn::Net net = dnn::readNet("yolov7.weights", "yolov7.cfg");
    if (net.empty()) {
        cout << "Yolo model is not opened" << endl;
        return -1;
    }

    Mat img_frame;
    vector<string> classes;
    vector<float> confidences;
    vector<Rect> boxes;
    vector<Point2f> lines;

    classes = _car->loadYolo(net);
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

    // Set RoI (첫번째 frame에서 마우스 클릭으로 ROI 영역 지정)
    _car->SetROI(img_frame, srcPts, window_capture_name1);

    // RoI 영역 지정이 완료된 후, 차선 검출 프로그램 시작
    namedWindow(window_capture_name4);
    while (video.read(img_frame))
    {
        
        // Perspective Transform (3D -> 2D : top view)
        Mat img_top;
        _car->TransPersfective(img_frame, img_top, srcPts, dstPts, width, height);

        // Image Preprocessing
        Mat img_preprocessed;
        _car->PreprocessFrame(img_top, img_preprocessed);
        
        // Car Detect
        _car->getObject(img_frame, srcPts, net, classes, 0.5, confidences, boxes, lines);

        // Draw Car RoundingBox 
        Mat img_object;
        _car->drawObject(img_frame, img_preprocessed, img_object, srcPts, confidences, boxes, lines, img_frame.cols, 0.5);
        
        // Get Lanes
        Mat img_lane = img_preprocessed;
        double amount = _car->LaneDetection(img_lane, img_top);

        // Perspective Transform (2D -> 3D)
        Mat img_roi;
        _car->TransPersfective(img_top, img_roi, dstPts, srcPts, width, height);
        Mat img_result;
        addWeighted(img_object, 1, img_roi, 1, 0, img_result);
        
        // Put Text
        bool flag = true;
        _car->PutText(img_result, flag, amount);

        imshow(window_capture_name4, img_result);
        if (waitKey(1) == 27)
        {
            break; // Press ESC to exit
        }
    }

    return 0;
}
