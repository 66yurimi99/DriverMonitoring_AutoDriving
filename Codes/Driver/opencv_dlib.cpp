#include "ISP.h"

const String windowName = "Sleep Detection";

const double eyeAspectRatioThreshold = 0.23;
const double sleeptime_threshold = 3;

//double eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6) {
//
//    dlib::point p1_point = landmarks.part(p1);
//    dlib::point p2_point = landmarks.part(p2);
//    dlib::point p3_point = landmarks.part(p3);
//    dlib::point p4_point = landmarks.part(p4);
//    dlib::point p5_point = landmarks.part(p5);
//    dlib::point p6_point = landmarks.part(p6);
//
//    double a = std::sqrt(std::pow(p2_point.x() - p6_point.x(), 2) + std::pow(p2_point.y() - p6_point.y(), 2));
//    double b = std::sqrt(std::pow(p3_point.x() - p5_point.x(), 2) + std::pow(p3_point.y() - p5_point.y(), 2));
//    double c = std::sqrt(std::pow(p1_point.x() - p4_point.x(), 2) + std::pow(p1_point.y() - p4_point.y(), 2));
//
//    return (a + b) / (2.0 * c);
//}
//
//double calculateAngle(Point p1, Point p2) {
//    double angle = atan2(p2.y - p1.y, p2.x - p1.x);
//    return angle * 180. / CV_PI; //degree
//}


int main() {

    ISP* _isp = new ISP();

    VideoCapture cap(0);  // 카메라 열기
    if (!cap.isOpened()) {
        cerr << "Error: 카메라를 열 수 없습니다." << endl;
        return -1;
    }

    Mat frame;
    Mat sub_frame;
    std::vector<dlib::rectangle> faces; // dlib의 rectangle을 사용하므로 std::vector로 변경
    std::vector<full_object_detection> shapes; // dlib의 full_object_detection을 사용하므로 std::vector로 변경

    clock_t start = 0;
    clock_t end = 0;

    double scaler = 0.3;

    frontal_face_detector detector = get_frontal_face_detector();

    shape_predictor landmark_predictor;
    deserialize("shape_predictor_68_face_landmarks.dat") >> landmark_predictor;

    double angle = 0;

    bool sleep = false;

    while (true) {
        cap >> frame;  // 프레임 캡처
        cap >> sub_frame;
        if (frame.empty()) {
            cerr << "비디오 스트림이 종료되었습니다." << endl;
            break;
        }
       /* cv::Point2f center(frame.cols / 2.0, frame.rows / 2.0);
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::Mat rotatedframe;
        cv::warpAffine(frame, rotatedframe, rotationMatrix, frame.size());
        */

        cv_image<bgr_pixel> dlibFrame(frame);

        // 얼굴 감지
        faces = detector(dlibFrame);

        if (faces.empty()) {
            cout << "No face" << endl;
            putText(sub_frame, "empty", Point(280, 200), cv::FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            cv::imshow(windowName, sub_frame);
            continue;
        }

        for (const auto& face : faces) {
            dlib::draw_rectangle(dlibFrame, face, dlib::rgb_pixel(255, 0, 0));
            // 랜드마크 검출
            full_object_detection landmarks = landmark_predictor(dlibFrame, face);
            shapes.push_back(landmarks);

            dlib::rectangle rect = face;

            int num_of_points_out = 17;
            int num_of_points_in = landmarks.num_parts() - num_of_points_out;
            double gx_out = 0, gy_out = 0, gx_in = 0, gy_in = 0;

            for (int i = 0; i < landmarks.num_parts(); ++i) {
                point shape_point = landmarks.part(i);

                if (i < num_of_points_out) {
                    gx_out += shape_point.x() / scaler / num_of_points_out;
                    gy_out += shape_point.y() / scaler / num_of_points_out;
                }
                else {
                    gx_in += shape_point.x() / scaler / num_of_points_in;
                    gy_in += shape_point.y() / scaler / num_of_points_in;
                }
            }

            double theta = asin(2 * (gx_in - gx_out) / (rect.right() / scaler - rect.left() / scaler));
            double radian = theta * 180 / M_PI;


            // 눈 위치를 사용하여 EAR 계산
            double leftEAR = _isp->eyeAspectRatio(landmarks, 36, 37, 38, 39, 40, 41);
            double rightEAR = _isp->eyeAspectRatio(landmarks, 42, 43, 44, 45, 46, 47);

            for (int i = 36; i < 48; i++) {
                cv::Point point(landmarks.part(i).x(), landmarks.part(i).y());
                cv::circle(frame, point, 2, Scalar(0, 0, 255), -1);
            }

            angle += radian;

            string textShow;
            if (radian < 0) {
                textShow = "left" + to_string(abs(radian)) + " deg.";
            }
            else {
                textShow = "right" + to_string(abs(radian)) + " deg.";
            }
            putText(frame, textShow, Point(400, 430), cv::FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

            // EAR 평균 계산
            double ear = (leftEAR + rightEAR) / 2.0;

            //클락으로 시간 계산
            if (ear <= eyeAspectRatioThreshold) {
                if (start == 0) {
                    start = clock();
                }
                else {
                    end = clock();
                    double sleeptime = (double)(end - start) / CLOCKS_PER_SEC;
                    std::string text = std::to_string(sleeptime);
                    putText(frame, text, Point(50, 430), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
                    if (sleeptime >= sleeptime_threshold) {
                        sleep = true;
                    }
                }
            }
            else {
                start = 0;
            }
        }
        if (sleep == true) putText(frame, "잠", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

        cv::imshow(windowName, frame);

        if (waitKey(1) == 27)  // ESC 키로 종료
            break;
    }

    

    destroyAllWindows();
    return 0;
}