#include "Car.h"

Car::Car()
{
    cout << "Start AutoDriving mode..." << endl;
}

Car::~Car()
{
    cout << "End AutoDriving mode..." << endl;
}

/*** 1. RoI 설정 : 마우스 클릭으로 RoI 사각형을 구성하는 네 점의 좌표 설정 (클릭 순서: 좌측 상단부터 시계 방향) ***/
// 1-1. 마우스 클릭 이벤트에 대한 콜백 함수
void Car::OnMouse(int event, int x, int y, int flags, void* userdata)
{
    static int cnt = 0;

    UserData* userData = static_cast<UserData*>(userdata);
    Point2f* srcPts = userData->pts;
    Mat img_roi = userData->img;


    if (event == EVENT_LBUTTONDOWN)
    {
        if (cnt < 4)
        {
            // 선택한 위치를 점으로 그려서 보여주기
            srcPts[cnt++] = Point2f(x, y);
            circle(img_roi, Point(x, y), 5, Scalar(255, 0, 255), -1);
            imshow(windowname, img_roi);
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

// 1-2. 첫번째 frame에서 마우스 클릭으로 ROI 영역 지정 (PUBLIC)
void Car::SetROI(Mat src, Point2f* srcPts, string windowname)
{
    namedWindow(windowname);

    Mat img_roi = src.clone();
    UserData userdata = { img_roi, srcPts };

    putText(img_roi, "Click RoI Points", Point(img_roi.cols * 0.3, 60), 1, 4, Scalar(255, 255, 255), 5);
    imshow(windowname, img_roi); // ROI 설정을 위한 화면 출력

    //마우스 클릭 이벤트 발생 시 콜백 함수(OnMouse) 호출
    setMouseCallback(windowname, OnMouse, &userdata);

    while (!ROISelect)
    {   // 20ms 간격으로 RoI 지정 여부 확인
        waitKey(20);
    }

    destroyWindow(windowname);
}


/*** 2. 시점 변경 : 투시 변환을 통한 Original View(3D) <-> Top View(2D) 상호 전환 ***/
// 2-1. 시점 변경 (PUBLIC)
void Car::TransPersfective(Mat src, Mat& dst, Point2f srcPts[], Point2f dstPts[], int width, int height)
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


/*** 3. 이미지 전처리 : 차선 검출이 용이하도록 이미지 변환 ***/
// 3-1. 컬러 영역 변경 : RGB 3채널 -> HSV(노랑)+LAB(흰색) 1채널 이미지
void Car::ConvertColor(Mat src, Mat& dst)
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

// 3-2. 노이즈 제거
void Car::RemoveNoise(Mat src, Mat& dst)
{
    // Morphology Opening (erosion -> dilation)
    erode(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
    dilate(dst, dst, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1));
}

// 3-3. 이미지 전처리 함수 (PUBLIC)
void Car::PreprocessFrame(Mat src, Mat& dst)
{
    // 1) 색상 영역 변경
    ConvertColor(src, dst);

    // 2) 노이즈 제거 : Morphology Opening
    RemoveNoise(src, dst);
}


/*** 4. 차선 검출 및 주행 방향 계산 ***/
// 4-1. 주행 방향 계산
// 4-1-1. x 좌표 평균 구하기
double Car::GetAverage(vector<Point2f> centroids)
{
    float sum_x = 0.0;
    for (const Point2f& point : centroids) {
        sum_x += point.x;
    }

    float average_x = sum_x / centroids.size();

    return average_x;
}
// 4-1-2. 방향 계산
double Car::CalcDirection(double left, double right)
{
    double diff = (prev_average_left - left) + (prev_average_right - right);

    prev_average_left = left;
    prev_average_right = right;

    return diff;
}

// 4-2. 차선 검출
// 4-2-1. RoI 영역을 좌우 2개로 분할
vector<Rect> Car::DivideRoi(Mat src, int x, int divide_flag)
{
    vector<Rect> rectangles;
    int numRect = 10;
    int rectHeight = src.rows / numRect;
    if (divide_flag == 0)
    {
        for (int i = 0; i < numRect; i++)
        {
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;

            Rect rect(0, y1, x / 2, y2 - y1);
            rectangles.push_back(rect);
        }
    }
    else if (divide_flag == 1)
    {
        for (int i = 0; i < numRect; i++)
        {
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;

            Rect rect(x / 2, y1, x / 2, y2 - y1);
            rectangles.push_back(rect);
        }
    }

    return rectangles;
}
// 4-2-2. 차선으로 이어줄 점들의 좌표 구하기
vector<Point2f> Car::findCentroids(const Mat& roi, const vector<Rect>& rectangles)
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
// 4-2-3. 검출된 차선을 기준으로 도로 영역 그리기
void Car::DrawLine(Mat& image, const vector<Rect>& rectangles, const vector<Point2f>& centroids)
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

// 4-3. 차선 검출 (PUBLIC)
double Car::LaneDetection(Mat& src, Mat& dst)
{
    vector<Rect> rectangles_left = DivideRoi(src, src.cols, 0);
    vector<Point2f> centroids_left = findCentroids(src, rectangles_left);
    double average_left = GetAverage(centroids_left);

    vector<Rect> rectangles_right = DivideRoi(src, src.cols, 1);
    vector<Point2f> centroids_right = findCentroids(src, rectangles_right);
    double average_right = GetAverage(centroids_right);
    double amount = CalcDirection(average_left, average_right);

    DrawLine(src, rectangles_left, centroids_left);
    DrawLine(src, rectangles_right, centroids_right);

    Mat img_fill = Mat::zeros(src.size(), CV_8UC3);
    Point img_fill_point[4]
    {
       Point(centroids_left.back()),
       Point(centroids_left.front()),
       Point(centroids_right.front()),
       Point(centroids_right.back())
    };

    fillConvexPoly(img_fill, img_fill_point, 4, Scalar(255, 0, 0));
    dst = img_fill;

    return amount;
}


/*** 5. 결과 데이터 출력 : 결과 데이터(자동주행 여부 및 주행 방향)를 화면에 출력 ***/
// 5-1. 결과 데이터 출력 (PUBLIC)
void Car::PutText(Mat& draw, const bool isSleep, const double amount)
{
    String driving_mode = isSleep ? "Auto" : "Manual";
    String direction;
    if (amount < -5)	direction = "Left";
    else if (amount < 5)	direction = "Straight";
    else	direction = "Right";
    putText(draw, driving_mode, Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
    putText(draw, direction, Point(20, 120), 1, 4, Scalar(0, 255, 255), 5);
}

/*** 6. 도로 내 차량 검출 ***/
//6-1. 차량 검출을 위한 model name load
vector<string> Car::loadYolo(dnn::Net net) {
    vector<string> classes;
    //+GPU
    net.setPreferableBackend(dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(dnn::DNN_TARGET_CUDA);

    // Load class names
    ifstream ifs("coco.names");
    if (!ifs.is_open())
    {
        cout << "Error: coco.names not opened." << endl;
    }

    //line 내 coco.names 파일 내용 넣기
    string line;
    while (getline(ifs, line))
    {
        classes.push_back(line);
    }
    return classes;
}
//6-2 Frame 내 차량 관련 객체 검출
void Car::getObject(Mat& Src, Point2f* srcPts, dnn::Net net, vector<string> classes,
    double conf_value, vector<float>& confidences, vector<Rect>& boxes, vector<Point2f>& lines)
{
    Mat blob;
    // Detect objects
    confidences.clear();
    boxes.clear();
    lines.clear();
    dnn::blobFromImage(Src, blob, 1 / 255.0, Size(416, 416), Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    vector<Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames()); //yolo 결과 outs에 저장
    // Process detection results
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
                int center_x = (int)(data[0] * Src.cols);
                int center_y = (int)(data[1] * Src.rows);
                int width = (int)(data[2] * Src.cols);
                int height = (int)(data[3] * Src.rows);
                int left = center_x - width / 2;
                int top = center_y - height / 2;

                if (classes[class_id_point.x] == "car" || classes[class_id_point.x] == "bus" || classes[class_id_point.x] == "truck")
                {
                    //class_ids.push_back(class_id_point.x);
                    confidences.push_back((float)confidence);
                    // Only draw bounding box and label for 'car' class
                    boxes.push_back(Rect(left, top, width, height));
                    lines.push_back(Point(center_x, center_y));
                }
            }
        }
    }
}
//6-3 객체 표시 및 거리 판단이 필요한 객체 확인
void Car::drawObject(Mat Src, Mat Src2, Mat& dst, Point2f* srcPts, vector<float>& confidences, vector<Rect>& boxes,
    vector<Point2f>& lines, int final_value, double conf_value)
{
    float m1 = (srcPts[3].y - srcPts[0].y) / (srcPts[3].x - srcPts[0].x);
    float m2 = (srcPts[2].y - srcPts[1].y) / (srcPts[2].x - srcPts[1].x);
    float meet_point_x = ((m1 * srcPts[0].x) - (m2 * srcPts[1].x) + srcPts[1].y - srcPts[0].y) / (m1 - m2);
    int min_point_y = (int)(m1 * (meet_point_x - srcPts[0].x) + srcPts[0].y);
    dst = Src.clone();
    vector<int> indices;
    Rect check_box;
    vector <Rect> detect_boxes;
    // Apply non-maximum suppression -> 중복 박스 중 최적의 박스만 검출
    dnn::NMSBoxes(boxes, confidences, conf_value, conf_value, indices);
    // Draw bounding boxes around detected objects
    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        Point line = lines[idx];
        detect_boxes.push_back(box);
        String label_accuracy = cv::format("%.2f", confidences[idx]);
        rectangle(dst, box, Scalar(0, 0, 255), 2);
        putText(dst, label_accuracy, Point(box.x, box.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
        //yolo 객체 탐지
        if (line.y > min_point_y)
        {
            int check_x1 = ((float)line.y - srcPts[0].y + (m1 * srcPts[0].x)) / m1;
            int check_x2 = ((float)line.y - srcPts[1].y + (m2 * srcPts[1].x)) / m2;
            if (line.x > check_x1 && line.x < check_x2)
            {
                int com_value = abs((Src.cols) / 2 - line.x);
                if (com_value < final_value)
                {
                    check_box = box;
                    final_value = com_value;
                }
            }
        }
    }
    judgeObject(Src, dst, srcPts, check_box, final_value);
    LaneChangeStatus(Src2, dst, srcPts, detect_boxes);
}
//6-4 거리 판단 필요한 객체(차량)에 대하여 safe, dangerous 판단
void Car::judgeObject(Mat& Src, Mat& dst, Point2f* srcPts, Rect check_box, int final_value) {
    if (final_value != Src.cols)
    {
        Point points_dis[4]{
            Point(check_box.x, check_box.y + check_box.height),
            Point(check_box.x + check_box.width , check_box.y + check_box.height),
            Point(srcPts[1].x,srcPts[1].y),
            Point(srcPts[0].x,srcPts[0].y),
        };

        fillConvexPoly(dst, points_dis, 4, Scalar(0, 235, 0));

        string judge;
        int red, green, blue = 0;
        if ((check_box.y + check_box.height) > srcPts[0].y)
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
        putText(dst, judge, Point(20, 180), 1, 4, Scalar(blue, green, red), 5);
    }
}
//6-5 차선 변경 여부 판단을 위한 차선 판단(실선, 점선) 및 해당 차선 진행 차량 여부 판단
void Car::LaneChangeStatus(Mat& Src, Mat& dst, Point2f* srcPts, vector<Rect>& boxes) {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(Src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);//객체의 외곽선을 검출하는 함수
    int left_height = 0;
    int right_height = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        Rect rt = boundingRect(contours[i]); 
        if (rt.x < Src.cols / 2)
        {
            left_height += rt.height;
        }
        else
        {
            right_height += rt.height;
        }
        cv::rectangle(Src, rt, Scalar(255, 0, 0), 3);
    }

    String left_move = "O";
    String right_move = "O";

    if (left_height > (Src.rows * 0.8))
    {
        left_move = "X";
    }
    else
    {
        for (const Rect& box : boxes)
        {
            if (box.y + box.height > srcPts[0].y)
            {
                left_move = "X";
                break;
            }
            else
            {
                left_move = "O";
            }
        }
    }
    putText(dst, left_move, Point(dst.cols / 4, dst.rows * 0.8), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 225, 255), 2);

    if (right_height > (Src.rows * 0.9))
    {
        right_move = "X";
    }
    else
    {
        for (const Rect& box : boxes)
        {
            if (box.y + box.height > srcPts[0].y)
            {
                right_move = "X";
                break;
            }
            else
            {
                right_move = "O";
            }
        }
    }
    putText(dst, right_move, Point(dst.cols * 0.75, dst.rows * 0.8), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 225, 255), 2);
}