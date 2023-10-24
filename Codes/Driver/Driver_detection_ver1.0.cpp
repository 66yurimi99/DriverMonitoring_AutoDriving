#include "ISP.h"

const String windowName = "Sleep Detection";

const double eyeAspectRatioThreshold = 0.23;
const double sleeptime_threshold = 3;

int main() {

    ISP* _isp = new ISP();

    VideoCapture cap(0);  // 카메라 열기(0은 내장카메라 우선 시, 외부 캠 사용시 1로)
    if (!cap.isOpened()) {
        cerr << "Error: 카메라를 열 수 없습니다." << endl;
        return -1;
    }

    Mat frame;
    vector<dlib::rectangle> faces; // dlib의 rectangle을 사용하므로 std::vector로 변경

    //Sleep에 관한 변수 선언
    clock_t start = 0;
    clock_t end = 0;

    double scaler = 0.3;

    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

    dlib::shape_predictor landmark_predictor;
    //shape_predictor_68_face_landmarks.dat path 확인 요망
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> landmark_predictor;

    double angle = 0;

    bool sleep = false;

    // FPS 측정을 위한 변수들 초기화
    auto start_fps = chrono::high_resolution_clock::now();
    int frameCount = 0;

    while (true) {
        cap >> frame;  // 프레임 캡처
        //cap >> sub_frame;
        if (frame.empty()) {
            cerr << "비디오 스트림이 종료되었습니다." << endl;
            break;
        }
        dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);

        // 얼굴 감지
        faces = detector(dlibFrame);
        
        //감지 못했을 때 프레임 넘어감
        //if (faces.empty()) {
        //    cout << "no face" << endl;
        //    //puttext(sub_frame, "empty", point(280, 200), cv::font_hershey_simplex, 1, scalar(0, 0, 255), 2);
        //    //cv::imshow(windowname, sub_frame);
        //    continue;
        //}

        for (const auto& face : faces) {
            dlib::draw_rectangle(dlibFrame, face, dlib::rgb_pixel(255, 0, 0));
            // 랜드마크 검출
            dlib::full_object_detection landmarks = landmark_predictor(dlibFrame, face);
            shapes.push_back(landmarks);

            dlib::rectangle rect = face;

            // 눈 위치를 사용하여 각 눈의 EAR 계산
            double leftEAR = _isp->eyeAspectRatio(landmarks, 36, 37, 38, 39, 40, 41);
            double rightEAR = _isp->eyeAspectRatio(landmarks, 42, 43, 44, 45, 46, 47);
            //랜드마크 그림
            for (int i = 36; i < 48; i++) {
                cv::Point point(landmarks.part(i).x(), landmarks.part(i).y());
                cv::circle(frame, point, 2, Scalar(0, 0, 255), -1);
            }

            // EAR 평균 계산
            double ear = (leftEAR + rightEAR) / 2.0;

            //클락으로 시간 계산 (클락당 1ms)
            if (ear <= eyeAspectRatioThreshold) {
                if (start == 0) {
                    start = clock();
                }
                else {
                    end = clock();
                    //잠들었다고 판단한 시간을 클락 단위에서 초로 변환)
                    double sleeptime = (double)(end - start) / CLOCKS_PER_SEC;
                    string text = "Closed: "+to_string(sleeptime);
                    putText(frame, text, Point(50, 430), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

                    if (sleeptime >= sleeptime_threshold) {//3초가 넘어가서 잔다고 판단)
                        sleep = true;
                    }
                }
            }
            else {
                start = 0;
            }
        }
        if (sleep == true) putText(frame, "Sleep", Point(10, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
        else putText(frame, "Good", Point(10, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        // 다음 프레임 처리를 위한 변수 업데이트
         // FPS 측정 및 표시
        auto end_fps = chrono::high_resolution_clock::now();
        double fps = frameCount / chrono::duration<double>(end_fps - start_fps).count();
        cv::putText(frame, "FPS: " + to_string(int(fps)), Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
        frameCount++;
        cv::imshow(windowName, frame);

        if (waitKey(1) == 27)  // ESC 키로 종료
            break;
    }

    

    destroyAllWindows();
    return 0;
}