#include "ISP.h"

ISP::ISP() {
    cout << "Program Start" << endl;
}

ISP::~ISP() {
    cout << "Program end.." << endl;
}



// EAR을 계산하는 함수
double ISP::calculateEar(const dlib::full_object_detection& shape, const int leftEye[], const int rightEye[]) {
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

double ISP::calculateAngle(Point p1, Point p2) {
    double angle = atan2(p2.y - p1.y, p2.x - p1.x);
    return angle * 180. / CV_PI; //degree
}

double ISP::eyeAspectRatio(const dlib::full_object_detection& landmarks, int p1, int p2, int p3, int p4, int p5, int p6) {

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
