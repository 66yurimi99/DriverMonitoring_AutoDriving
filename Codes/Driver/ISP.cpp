#include "ISP.h"

ISP::ISP() {
    cout << "Program Start" << endl;
}

ISP::~ISP() {
    cout << "Program end.." << endl;
}

double ISP::calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]) { // EAR을 계산하는 함수
    // 왼쪽 눈의 좌표 추출
    std::vector<Point> leftEyePoints;
    for (int i = leftEye[0]; i < leftEye[(int)sizeof(leftEye) / sizeof(int) - 1]; i++) {
        leftEyePoints.push_back(Point(shape.part(i).x(), shape.part(i).y()));
    }

    // 오른쪽 눈의 좌표 추출
    std::vector<Point> rightEyePoints;
    for (int i = rightEye[0]; i < rightEye[(int)sizeof(rightEye) / sizeof(int) - 1]; i++) {
        rightEyePoints.push_back(Point(shape.part(i).x(), shape.part(i).y()));
    }

    // 눈의 수직 길이 계산
    double verticalDistLeft1 = sqrt(pow(leftEyePoints[1].x - leftEyePoints[5].x, 2) + pow(leftEyePoints[1].y - leftEyePoints[5].y, 2));
    //norm(leftEyePoints[1] - leftEyePoints[5]);
    double verticalDistLeft2 = sqrt(pow(leftEyePoints[2].x - leftEyePoints[4].x, 2) + pow(leftEyePoints[2].y - leftEyePoints[4].y, 2));
    //norm(leftEyePoints[2] - leftEyePoints[4]);
    double verticalDistLeft = (verticalDistLeft1 + verticalDistLeft2);

    double verticalDistRight1 = sqrt(pow(rightEyePoints[1].x - rightEyePoints[5].x, 2) + pow(rightEyePoints[1].y - rightEyePoints[5].y, 2));
    //norm(rightEyePoints[1] - rightEyePoints[5]);
    double verticalDistRight2 = sqrt(pow(rightEyePoints[2].x - rightEyePoints[4].x, 2) + pow(rightEyePoints[2].y - rightEyePoints[4].y, 2));
    //norm(rightEyePoints[2] - rightEyePoints[4]);
    double verticalDistRight = (verticalDistRight1 + verticalDistRight2);

    // 눈의 수평 길이 계산
    double horizontalDistLeft = sqrt(pow(leftEyePoints[0].x - leftEyePoints[3].x, 2) + pow(leftEyePoints[0].y - leftEyePoints[3].y, 2));
    //norm(leftEyePoints[0] - leftEyePoints[3]);
    double horizontalDistRight = sqrt(pow(rightEyePoints[0].x - rightEyePoints[3].x, 2) + pow(rightEyePoints[0].y - rightEyePoints[3].y, 2));
    //norm(rightEyePoints[0] - rightEyePoints[3]);

// EAR 계산
    double earLeft = verticalDistLeft / (2.0 * horizontalDistLeft);
    double earRight = verticalDistRight / (2.0 * horizontalDistRight);
    //std::cout << "Left: " << earLeft << "== Right: " << earRight << endl;
    // 두 눈의 EAR을 평균하여 반환
    return (earLeft + earRight) / 2.0;
}

double ISP::calculateAngle(Point p1, Point p2) { //두 점 각도계산
    double angle = atan2(p2.y - p1.y, p2.x - p1.x);
    return angle * 180. / CV_PI; //degree
}

double ISP::eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6) { // EAR을 계산하는 함수

    dlib::point p1_point = landmarks.part(p1);
    dlib::point p2_point = landmarks.part(p2);
    dlib::point p3_point = landmarks.part(p3);
    dlib::point p4_point = landmarks.part(p4);
    dlib::point p5_point = landmarks.part(p5);
    dlib::point p6_point = landmarks.part(p6);

    double a = std::sqrt(std::pow(p2_point.x() - p6_point.x(), 2) + std::pow(p2_point.y() - p6_point.y(), 2));
    double b = std::sqrt(std::pow(p3_point.x() - p5_point.x(), 2) + std::pow(p3_point.y() - p5_point.y(), 2));
    double c = std::sqrt(std::pow(p1_point.x() - p4_point.x(), 2) + std::pow(p1_point.y() - p4_point.y(), 2));

    return (a + b) / (2.0 * c);
}

void ISP::initializeCamera(int i, VideoCapture& cap) //카메라 초기세팅
{ 
    cap.open(i);
    if (!cap.isOpened()) {
        cerr << "Error: 카메라를 열 수 없습니다." << endl;
        exit(1);
    }
}

void ISP::initializeDlib(dlib::frontal_face_detector& detector, dlib::shape_predictor& landmark_predictor) //dlib - face_detector, face_landmarks 설정
{ 
    detector = dlib::get_frontal_face_detector();
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> landmark_predictor;
}

void ISP::calculateFPS(Mat& frame, int& frameCount, chrono::time_point<chrono::high_resolution_clock>& start_fps) //초당 프레임 수 계산
{ 

    auto end_fps = chrono::high_resolution_clock::now();
    double fps = frameCount / chrono::duration<double>(end_fps - start_fps).count();
    cv::putText(frame, "FPS: " + to_string(int(fps)), Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
    //start_fps = end_fps;;

    frameCount++;
}

void ISP::detectEyesAndSleep(Mat& frame, dlib::frontal_face_detector detector, dlib::shape_predictor landmark_predictor, bool& sleep,clock_t& start, clock_t& end) //EAR값 검출 및 졸음 여부 판단
{
    std::vector<dlib::rectangle> faces;
    dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);

    faces = detector(dlibFrame);

    //When cannot find driver
    if (faces.empty()) {
        if (start == 0) start = clock();
        else {
            end = clock();
            double notfound = double(end - start) / CLOCKS_PER_SEC;
            string text = "Not Found: " + to_string(notfound);
            putText(frame, text, Point(50, 430), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            if (notfound >= notfound_threshold) {
                sleep = true;
            }
        }
        if (sleep == true) putText(frame, "Sleep", Point(10, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
    }

    else {
        sleep = false;
        for (const auto& face : faces) {
            dlib::draw_rectangle(dlibFrame, face, dlib::rgb_pixel(255, 0, 0));
            dlib::full_object_detection landmarks = landmark_predictor(dlibFrame, face);

            double leftEAR = eyeAspectRatio(landmarks, 36, 37, 38, 39, 40, 41);
            double rightEAR = eyeAspectRatio(landmarks, 42, 43, 44, 45, 46, 47);

            // 눈 랜드마크 그림
            for (int i = 36; i < 48; i++) {
                cv::Point point(landmarks.part(i).x(), landmarks.part(i).y());
                cv::circle(frame, point, 2, Scalar(0, 0, 255), -1);
            }
            double ear = (leftEAR + rightEAR) / 2.0; //눈 수직:수평 비율

            if (ear <= eyeAspectRatioThreshold) {
                if (start == 0) {
                    start = clock();
                }
                else {
                    end = clock();
                    double sleeptime = (double)(end - start) / CLOCKS_PER_SEC;
                    string text = "Closed: " + to_string(sleeptime);
                    putText(frame, text, Point(50, 430), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

                    if (sleeptime >= sleeptime_threshold) {
                        sleep = true;
                    }
                }
            }
            else {
                start = 0;
            }
            if (sleep == true) putText(frame, "Sleep", Point(10, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2); // 수면 시
            else putText(frame, "Good", Point(10, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2); // 정상상태 시
        }
    }
}

int ISP::videotoframe(Mat& frame, VideoCapture& cap) // 영상 -> 이미지
{
    cap >> frame;
    if (frame.empty()) {
        cerr << "비디오 스트림이 종료되었습니다." << endl;
        return 0;
    }
    return 1;
}

void ISP::preprocessing(Mat& frame, Mat& dst, Mat& dst_hsv) {
    dst = frame.clone();
    //cvtColor(frame, dst, COLOR_BGR2GRAY);
    cvtColor(frame, dst, COLOR_BGR2HSV);
    const int H_Low = 0, S_Low = 0, V_Low = 0;
    const int H_High = 40, S_High = 60, V_High = 160;
    inRange(dst, (H_Low, S_Low, V_Low), (H_High, S_High, V_High), dst_hsv);
}

void ISP::gammatransform(Mat& frame, Mat& gamma_t, float gamma_var) {
    const int a = 1;
    for (size_t i = 0; i < frame.rows; i++) {
        for (size_t j = 0; j < frame.cols; j++) {
            //float pixel_value = frame.at<uchar>(i, j);
            for (size_t c = 0; c < frame.channels(); c++) {
                int idx = ((i * frame.cols) + j) * frame.channels() + c;
                float val = pow(((float)frame.data[idx] / 255.), gamma_var) * 255;
                gamma_t.data[idx] = (uchar)val;
            }
        }
    }
}

void ISP::logtransform(Mat& frame, Mat& log_t, int log_var) {
    for (size_t i = 0; i < frame.rows; i++) {
        for (size_t j = 0; j < frame.cols; j++) {
            for (size_t c = 0; c < frame.channels(); c++) {
                int idx = ((i * frame.cols) + j) * frame.channels() + c;
                float val = log_var * log(1.+(float)frame.data[idx])*(255./log(256));
                log_t.data[idx] = (uchar)val;
            }
        }
    }
}

void ISP::initalizeWininet(HINTERNET& hInternet, HINTERNET& hConnect) {
    hInternet = InternetOpen(L"HTTP Example", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    wstring url(L"http://54.175.8.12/flag.php");
    wstring query(L"?sleep=0");
    hConnect = InternetOpenUrl(hInternet, (url + query).c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hInternet) {
        std::cerr << "InternetOpen failed." << std::endl;
        exit(1);
    }
}

void ISP::request_Wininet_Get(HINTERNET& hInternet, HINTERNET& hConnect, bool sleep, bool& pre_sleep) {
    if (sleep && !pre_sleep) {
        //wstring url = DB_URL;
        //wstring query = SLEEP;
        wstring url(L"http://54.175.8.12/flag.php");
        wstring query(L"?sleep=1");
        hConnect = InternetOpenUrl(hInternet, (url + query).c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            std::cerr << "InternetOpenUrl failed." << std::endl;
            InternetCloseHandle(hInternet);
            exit(1);
        }
    }
    else if (!sleep && pre_sleep) {
        //wstring url = DB_URL;
        //wstring query = GOOD;
        wstring url(L"http://54.175.8.12/flag.php");
        wstring query(L"?sleep=0");
        hConnect = InternetOpenUrl(hInternet, (url + query).c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            std::cerr << "InternetOpenUrl failed." << std::endl;
            InternetCloseHandle(hInternet);
            exit(1);
        }
    }
    pre_sleep = sleep;
}