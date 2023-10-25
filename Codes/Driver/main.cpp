#include "ISP.h"

const String windowName = "Sleep Detection";
const String subwindowName = "Preprocessing";
ISP* _isp = nullptr;
VideoCapture cap;
dlib::frontal_face_detector detector;
dlib::shape_predictor landmark_predictor;
int frameCount = 1;
int camera_num = 0; //내장 및 첫번째 카메라

int main() {
    _isp = new ISP();
    _isp->initializeCamera(camera_num, cap);
    _isp->initializeDlib(detector, landmark_predictor);
    Mat frame;
    Mat dst; // image preprocessing
    bool sleep = false;
    bool found = true;
    clock_t start = 0;
    clock_t end = 0;
    
    
    auto start_fps = chrono::high_resolution_clock::now();
    
    while (true) {
        if (!_isp->videotoframe(frame, cap)) break;

        _isp->detectEyesAndSleep(frame, detector, landmark_predictor, sleep, start, end);
        _isp->calculateFPS(frame, frameCount, start_fps);
        _isp->preprocessing(frame, dst);
        cv::imshow(windowName, frame);
        cv::imshow(subwindowName, dst);
        if (waitKey(1) == 27) {
            break;
        }
    }

    destroyAllWindows();
    delete _isp;

    return 0;
}
