#include "ISP.h"

const String windowName = "Sleep Detection";
const String subwindowName = "Preprocessing";
const int log_var = 2; //log var 크기만큼 맵핑
const float gamma_var = 1.1; //커질수록 어두워짐
ISP* _isp = nullptr;
VideoCapture cap;
dlib::frontal_face_detector detector;
dlib::shape_predictor landmark_predictor;
int frameCount = 1;
int camera_num = 0; //내장 및 첫번째 카메라
int gammaframe = 1;
HINTERNET hInternet, hConnect; //인터넷 세션, 리소스 핸들

int main() {
    _isp = new ISP();
    _isp->initializeCamera(camera_num, cap);
    _isp->initializeDlib(detector, landmark_predictor);
    _isp->initalizeWininet(hInternet, hConnect);
    Mat frame;
    Mat dst; // image preprocessing
    Mat dst_hsv;

    Mat gamma_t;
    Mat log_t;

    bool sleep = false;
    bool pre_sleep = false;
    bool found = true;
    clock_t start = 0;
    clock_t end = 0;
    
    
    auto start_fps = chrono::high_resolution_clock::now();
    
    while (true) {
        if (!_isp->videotoframe(frame, cap)) break;
        gamma_t = frame.clone();
        log_t = frame.clone();
        _isp->gammatransform(frame, gamma_t, gamma_var);
        _isp->detectEyesAndSleep(gamma_t, detector, landmark_predictor, sleep, start, end);
        _isp->calculateFPS(gamma_t, gammaframe, start_fps);
        _isp->request_Wininet_Get(hInternet, hConnect, sleep, pre_sleep);
        
        cv::imshow("gamma trans", gamma_t);
        if (waitKey(1) == 27) {
            break;
        }
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    destroyAllWindows();
    delete _isp;

    return 0;
}
