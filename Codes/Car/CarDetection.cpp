#include "Common.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>


void SetROI(Mat src, Point2f* srcPts);
void OnMouse(int event, int x, int y, int flags, void* userdata);

const String window_roi_name = "1. RoI Selection";
bool ROISelect = false;
bool ddd = true;
typedef struct UserData {
    Mat img;
    Point2f* pts;
};

int main() {
    //Open video capture
    //VideoCapture video("D:\\git\\DriverMonitoring_AutoDriving\\Video\\Line.mp4");
    VideoCapture video("D:\\git\\DriverMonitoring_AutoDriving\\Video\\Line2.mp4");

    if (!video.isOpened()) {
         cerr << "Error: Video not opened." << endl;
        return -1;
    }

    Mat frame, detect_frame;

    video.read(frame);
    detect_frame = Mat::zeros(frame.size(), CV_8UC3);

    Point2f srcPts[8] = { };
    SetROI(frame, srcPts);
    Point carDetPts[4] = {
    Point{ 0, (int)srcPts[4].y },
    Point{ frame.cols, (int)srcPts[5].y },
    Point{ frame.cols,  frame.rows},
    Point{ 0, frame.rows }
    };
    // 시야의 앞쪽 차량 검출 위해 범위 설정
    float m1 = (srcPts[7].y - srcPts[4].y)/(srcPts[7].x - srcPts[4].x);
    float m2 = (srcPts[6].y - srcPts[5].y) / (srcPts[6].x - srcPts[5].x);

    float meet_point_x = ((m1 * srcPts[4].x) - (m2 * srcPts[5].x) + srcPts[5].y - srcPts[4].y) / (m1 - m2);
    int min_point_y = (int)(m1 * (meet_point_x - srcPts[4].x) + srcPts[4].y);

    // Load YOLO model
      dnn::Net net =  dnn::readNet("D:\\git\\DriverMonitoring_AutoDriving\\Yolo\\yolov7.weights", "D:\\git\\DriverMonitoring_AutoDriving\\Yolo\\yolov7.cfg");
    //dnn::Net net =  dnn::readNet("D:\\git\\DriverMonitoring_AutoDriving\\Yolo\\yolov4-tiny.weights", 
     //   "D:\\git\\DriverMonitoring_AutoDriving\\Yolo\\yolov4-tiny.cfg");
     //dnn::Net net =  dnn::readNet("yolov4-tiny.weights", 
     //   "yolov4-tiny.cfg");
    
    if (net.empty()) 
    {
        cerr << "Error: YOLO model is empty." << endl;
        return -1;
    }

    // Load class names
    vector<string> classes;
    ifstream ifs("D:\\git\\DriverMonitoring_AutoDriving\\Yolo\\coco.names");

    if (!ifs.is_open())
    {
        cerr << "Error: coco.names not opened." << endl;
        return -1;
    }
        
    string line;
    while (getline(ifs, line)) 
    {
        classes.push_back(line);
    }

    //+GPU
    net.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(dnn::DNN_TARGET_CUDA);


    //time
    int t0, t1 = 0;
    
    // Process video frames
    while (true)
    {
        t0 = getTickCount();
        video >> frame;
        //frame = imread("C:\\Users\\PC\\Desktop\\Hello\\img1.png");
        //resize(frame, frame, Size(854, 480));

        if (frame.empty())
        {
            break;
        }
     
        // Detect objects
        Mat blob;
        dnn::blobFromImage(frame, blob, 1 / 255.0, Size(416, 416), Scalar(0, 0, 0), true, false);
        net.setInput(blob);

        vector<Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames()); //yolo 결과 outs에 저장

        // Process detection results
        vector<int> class_ids;
        vector<float> confidences;
        vector<Rect> boxes;
        vector<Point2f> lines;
        double conf_value = 0.4;

        for (int i = 0; i < outs.size(); ++i) 
        {
            float* data = (float*)outs[i].data;
            for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) 
            {
                Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
                Point class_id_point;
                double confidence;
                minMaxLoc(scores, nullptr, &confidence, nullptr, &class_id_point);

                if (confidence > conf_value)
                {
                    int center_x = (int)(data[0] * frame.cols);
                    int center_y = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = center_x - width / 2;
                    int top = center_y - height / 2;
                    if (classes[class_id_point.x] == "car" || classes[class_id_point.x] == "bus" || classes[class_id_point.x] == "truck")
                    {
                        class_ids.push_back(class_id_point.x);
                        confidences.push_back((float)confidence);

                        // Only draw bounding box and label for 'car' class
                        boxes.push_back(Rect(left, top, width, height));
                        lines.push_back(Point(center_x, center_y));
                    }
                }
            }
        }
        // Apply non-maximum suppression -> 중복 박스 중 최적의 박스만 검출
         vector<int> indices;
         dnn::NMSBoxes(boxes, confidences, conf_value, conf_value, indices);
         Mat img_top = Mat::zeros(frame.size(), CV_8UC3);
         int final_value = frame.cols;
         Rect check_box;
        // Draw bounding boxes around detected objects
        for (int i = 0; i < indices.size(); i++) 
        {
            int idx = indices[i];
            Rect box = boxes[idx];
            Point line = lines[idx];
            int class_id = class_ids[idx];
            String label_accuracy = cv::format("%.2f", confidences[idx]);
            rectangle(frame, box,  Scalar(0, 0, 255), 2);
            rectangle(detect_frame, box, Scalar(255, 255, 255), -1);

            putText(frame, label_accuracy, Point(box.x, box.y - 10),  FONT_HERSHEY_SIMPLEX, 0.5,  Scalar(0, 0, 255), 2);
            //yolo 객체 탐지
            if (line.y > min_point_y)
            { 
                int check_x1 = ((float)line.y - srcPts[4].y + (m1 * srcPts[4].x)) / m1;
                int check_x2 = ((float)line.y - srcPts[5].y + (m2 * srcPts[5].x)) / m2;
                if (line.x > check_x1 && line.x < check_x2)
                {
                    int com_value = abs((frame.cols)/2 - line.x);
                    if (com_value < final_value)
                    {
                        check_box = box;
                        final_value = com_value;
                    }
                }               
            }
        }
        //검출된 앞쪽 차량에 대하여 ROI좌표 0, 1 보다 y 값이 커질 경우 충돌 위험 
        if (indices.size() != 0 && final_value != frame.cols)
        {
            Point points_dis[4]{
                Point(check_box.x, check_box.y + check_box.height),
                Point(check_box.x + check_box.width , check_box.y + check_box.height),
                Point(srcPts[5].x,srcPts[5].y),
                Point(srcPts[4].x,srcPts[4].y),
            };
            fillConvexPoly(frame, points_dis, 4, Scalar(0, 255, 0));
            // bitwise_and(frame, detect_frame, detect_frame);
            string judge;
            int red, green, blue = 0;
            if ((check_box.y + check_box.height) > srcPts[4].y)
            {
                judge = "Collision Detection";
                red = 255;
                green = 0;
                blue = 0;
            }
            else
            {
                judge = "Safe";
                red = 0;
                green = 0;
                blue = 255;
            }
            putText(frame, judge, Point(0, frame.rows / 2 * 0.7), 1, 4, Scalar(blue, green, red), 5);
        }

        //영상처리 속도 비교
        t1 = getTickCount();
        //초 단위 처리 속도 = (t1-t0)/getTickFrequency
        double fps =  getTickFrequency() / (t1 - t0); // FPS 계산
        string fpsText = "FPS: " + to_string(fps);
        putText(frame, fpsText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0));


        imshow("Object Detection", frame);

        ddd = false;
        if ( waitKey(1) == 27) 
        {
            break; // Press ESC to exit
        }
    }

    video.release();
    destroyAllWindows();

    return 0;
}

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
            srcPts[cnt + 4] = Point2f(x, y);
            srcPts[cnt++] = Point2f(x, y);
            circle(img_roi, Point(x, y), 5, Scalar(255, 0, 255), -1);
            imshow(window_roi_name, img_roi);

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