#include "Common.h"
#include <iostream>
#include <opencv2/opencv.hpp>

const String window_capture_name0 = "0. Set Roi";
const String window_capture_name1 = "1. Lane_detection_video";
const String window_capture_name2 = "2. ROI rect";
const String window_capture_name3 = "3. birdview_Line";
const String window_capture_name4 = "4. Final";

bool ROISelect = false;
int cnt = 0;
Point2f srcPt[4];

void on_mouse(int event, int x, int y, int flags, void* userdata);
void SetROI(Mat src);
Mat TransPersfective(Mat src, Point2f* srcPts, Point2f* dstPts);
Mat PreprocessFrame(Mat src);

int main()
{
    namedWindow(window_capture_name0);

    //������ �ҷ�����
    VideoCapture video("input.mp4"); //opencv 480�� �ִ� ���� opencv_videoio_ffmpeg480_64�� build�� �־�� �����
    Mat img_frame;

    if (!video.isOpened())
    {
        cout << "������ ������ �� �� �����ϴ�. \n" << endl;
        return -1;
    }

    video.read(img_frame);
    resize(img_frame, img_frame, Size(210, 200));
    // 1: RoI ������ ���콺�� ����
    //      1) ���콺 Ŭ�� �ݹ� �Լ�
    setMouseCallback(window_capture_name0, on_mouse, &img_frame);
        
    while (!ROISelect)
    {
        waitKey(20);
    } 
    
    destroyWindow(window_capture_name0);

    if (img_frame.empty())
    {
        return -1;
    }

    while (true)
    {
        if (!video.read(img_frame))
        {
            break;
        }

        resize(img_frame, img_frame, Size(210, 200));
        Mat src_color = img_frame.clone();
        Mat src_gray;
        cvtColor(src_color, src_gray, COLOR_BGR2GRAY);

        //      2) Region of Interest (ROI) ���� ����
        SetROI(img_frame);
           
        // 2: ���� ��ȯ
        //      1) ���� ��ȯ (Perspective Transformation)
        //      Top view �� ��ȯ�ϱ� ���� �����ϴ� ��ǥ��
        Point2f dstPt[4] = {
            Point2f(0, 0), //���� ��;
            Point2f(src_color.cols, 0), // ������ ��
            Point2f(src_color.cols, src_color.rows), // ������ �Ʒ�
            Point2f(0, src_color.rows) //���� �Ʒ�
        }; 
        
        Mat img_topview = TransPersfective(src_color, srcPt, dstPt);

        // 3: �̹��� ��ó��
        //      1) �̹��� ��ó��
        Mat LineContoure;
        Mat Line_img = PreprocessFrame(img_topview);;
        cvtColor(Line_img, LineContoure, COLOR_BGR2GRAY);
        threshold(LineContoure, LineContoure, 128, 255, ThresholdTypes::THRESH_OTSU);

        //      2) ���� ���� ����
        //      3) ������ ����
        

        // 4: ���� ����
        //      1) ���� ����
        //      ���� ���� ���簢������ ���� ������
        int numRect = 10;
        int rectHeight = img_topview.rows / numRect;

        // ���簢�� ��ġ�� �����ϴ� ���� �ʱ�ȭ
        // ���� ���� ���� ����
        vector<Rect> rectangles_left;
        for (int i = 0; i < numRect; i++)
        {
            // ���簢�� ��ǥ ���
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;
            int x = 0;
                
            Rect rect(x, y1, 70, y2 - y1);

            rectangles_left.push_back(rect);
        }

        //rectangles_left �迭�� ���� rect�� �������
        for (const auto& rect : rectangles_left)
        {
            rectangle(Line_img, rect, Scalar(255, 0, 0), 3);
        }

        // �߽� ��ǥ�� ������ ���� �ʱ�ȭ
        vector<Point2f> centroids_left;

        //rectangles_left �迭�� ���� �߽��� ã�Ƽ� ����
        for (const Rect& rect : rectangles_left)
        {

            Mat roi = LineContoure(rect);
            // rectangles_left ���� �簢������ ������ ����
            vector<vector<Point>> contours;
            findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            /*
                ROI �������� ����� contours ó��
                rectangles_left ���� �簢������ ó���ϱ� ������ contours �Ѱ��� ����
            */

            // 1) ���� �������� ������ ���� ���
            if (contours.empty())
            {
                //������ ���簢���� �߽����� ���Ƿ� ����
                Point2f centcircle(rect.x+rect.width/2, rect.y + rect.height / 2);
                centroids_left.push_back(centcircle);
                //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
            }

            // 2) ���� �������� ������ ���� ���
            else
            {
                for (const vector<Point>& contour : contours)
                {
                        
                    double area = contourArea(contour);
                    double minContourArea = 10.0; // ������ ������ ����� ���ֱ� ���� �Ӱ谪

                    if (area >= minContourArea)
                    {
                        /*
                            Moments�� ������(������)�� Ư¡�� ���
                            mu.m00: 0�� ���Ʈ, ������ ���� (���� ��)
                            mu.m10: 1�� ���Ʈ X(X ��ǥ�� ���� ��)
                            mu.m01 : 1�� ���Ʈ Y(Y ��ǥ�� ���� ��)
                            mu.m20 : 2�� ���Ʈ X(X ��ǥ�� ������ ���� ��)
                            mu.m02 : 2�� ���Ʈ Y(Y ��ǥ�� ������ ���� ��)
                            mu.m11 : 2�� ���Ʈ XY(X�� Y ��ǥ�� ���� ���� ��)
                        */
                        Moments mu = moments(contour);

                        // �߽� ��ǥ ���
                        Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);

                        // contours �߽� ��ǥ�� �� �׸���
                        Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
                        centroids_left.push_back(centcircle);
                        //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
                    }
                }
            }
        }

        /*
        �� ���簢�� ����(ROI)���� �߽���ǥ�� �̾ �� �������� �׸���

        Mat roiLine_left = Mat::zeros(img_topview.size(), CV_8UC3);
        for (size_t i = 0; i < centroids_left.size() - 1; i++)
        {
            line(roiLine_left, centroids_left[i], centroids_left[i + 1], Scalar(255, 0, 0), 7);
        }
        */
        // ���簢�� ��ġ�� �����ϴ� ���� �ʱ�ȭ
        // ������ ���� ���� ����
        vector<Rect> rectangles_right;
        for (int i = 0; i < numRect; i++)
        {
            // ���簢�� ��ǥ ���
            int y1 = i * rectHeight;
            int y2 = (i + 1) * rectHeight;
            int x = 140;

            Rect rect(x, y1, 70, y2 - y1);

            rectangles_right.push_back(rect);
        }

        //rectangles_right �迭�� ���� rect�� �������
        for (const auto& rect : rectangles_right)
        {
            rectangle(Line_img, rect, Scalar(255, 0, 0), 3); 
        }


        // �߽� ��ǥ�� ������ ���� �ʱ�ȭ
        vector<Point2f> centroids_right;

        for (const Rect& rect : rectangles_right)
        {

            Mat roi = LineContoure(rect);
            vector<vector<Point>> contours;

            // rectangles_left  ���� �簢������ ������ ����
            findContours(roi, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            /*
                ROI �������� ����� contours ó��
                rectangles_left  ���� �簢������ ó���ϱ� ������ contours �Ѱ��� ����
            */

            // 1) ���� �������� ������ ���� ���
            if (contours.empty())
            {
                //������ ���簢���� �߽����� ���Ƿ� ����
                Point2f centcircle(rect.x + rect.width / 2, rect.y + rect.height / 2);
                centroids_right.push_back(centcircle);
                //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
            }

            // 2) ���� �������� ������ ���� ���
            else
            {

                for (const vector<Point>& contour : contours)
                {
                    double area = contourArea(contour);
                    double minContourArea = 10.0; // ������ ������ ����� ���ֱ� ���� �Ӱ谪

                    if (area >= minContourArea)
                    {
                        /*
                            Moments�� ������(������)�� Ư¡�� ���
                            mu.m00: 0�� ���Ʈ, ������ ���� (���� ��)
                            mu.m10: 1�� ���Ʈ X(X ��ǥ�� ���� ��)
                            mu.m01 : 1�� ���Ʈ Y(Y ��ǥ�� ���� ��)
                            mu.m20 : 2�� ���Ʈ X(X ��ǥ�� ������ ���� ��)
                            mu.m02 : 2�� ���Ʈ Y(Y ��ǥ�� ������ ���� ��)
                            mu.m11 : 2�� ���Ʈ XY(X�� Y ��ǥ�� ���� ���� ��)
                        */
                        Moments mu = moments(contour);

                        // �߽� ��ǥ ���
                        Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);

                        // contours �߽� ��ǥ�� �� �׸���
                        Point2f centcircle(centroid.x + rect.x, centroid.y + rect.y);
                        centroids_right.push_back(centcircle);
                        //circle(birdview, centcircle, 4, Scalar(0, 255, 0), -1);
                    }
                }
            }
        }

        /*
            �� ���簢�� ����(ROI)���� �߽���ǥ�� �̾ �� �������� �׸���
        Mat roiLine_right = Mat::zeros(img_topview.size(), CV_8UC3);
        for (size_t i = 0; i < centroids_right.size() - 1; i++)
        {
            line(roiLine_right, centroids_right[i], centroids_right[i + 1], Scalar(255, 0, 0), 7);
        }
        */

        /* 
        ����, ������ �������� ���� ���� ��ĥ Frame ����
        Mat roiLine_sum = Mat::zeros(img_topview.size(), CV_8UC3);
        addWeighted(roiLine_left, 1, roiLine_right, 1, 0, roiLine_sum);
        */
        Mat img_fill = Mat::zeros(img_topview.size(), CV_8UC3);
        Point img_fill_point[4]{
            Point(centroids_left.back()), //���� �Ʒ�
            Point(centroids_left.front()), //���� ��
            Point(centroids_right.front()), //������ ��
            Point(centroids_right.back()) //������ �Ʒ�
        };

        fillConvexPoly(img_fill, img_fill_point, 4, Scalar(255, 0, 0));
        Mat inv_src_color = TransPersfective(img_fill, dstPt, srcPt);

        addWeighted(src_color, 1, inv_src_color, 1, 0, inv_src_color);
            
        resize(src_color, src_color, Size(210, 200));
        resize(img_frame, img_frame, Size(210, 200));
        resize(img_topview, img_topview, Size(210, 200));
        resize(inv_src_color, inv_src_color, Size(210, 200));

        imshow(window_capture_name1, src_color);
        moveWindow(window_capture_name1, 300, 300);
        imshow(window_capture_name2, img_frame);
        moveWindow(window_capture_name2, 600, 300);
        imshow(window_capture_name3, img_topview);
        moveWindow(window_capture_name3, 900, 300);
        imshow(window_capture_name4, inv_src_color);
        moveWindow(window_capture_name4, 1200, 300);

        char key = (char)waitKey(1);
        if (key == 'q' || key == 27)
        {
            break;
        }
    }

    return 0;
}

void on_mouse(int event, int x, int y, int flags, void* userdata)
{
    
    if (event == EVENT_LBUTTONDOWN) 
    {
        if (cnt < 4)
        { 
            //���� ���� ��� -> ���� ��� -> ���� �ϴ� -> ���� �ϴ�
            srcPt[cnt] = Point2f(x, y);
            circle((*(Mat*)userdata), srcPt[cnt], 3, Scalar(0, 0, 255), 5);
            imshow(window_capture_name0, (*(Mat*)userdata));
            cnt++;
            if (cnt == 4)
            {
                ROISelect = true;
                
                srcPt[1].y = srcPt[0].y; //y��ġ�� �ϳ��� ����
                srcPt[3].y = srcPt[2].y; //y��ġ�� �ϳ��� ����
            }
        }
    }
    putText((*(Mat*)userdata), "Click Roi Point", Point(20, 60), 1, 4, Scalar(0, 255, 255), 5);
    imshow(window_capture_name0, (*(Mat*)userdata));
    moveWindow(window_capture_name0, 900, 300);
}

void SetROI(Mat src)
{
    line(src, srcPt[0], srcPt[1], Scalar(0, 255, 0), 2);
    line(src, srcPt[0], srcPt[3], Scalar(0, 255, 0), 2);
    line(src, srcPt[3], srcPt[2], Scalar(0, 255, 0), 2);
    line(src, srcPt[2], srcPt[1], Scalar(0, 255, 0), 2);
}

Mat TransPersfective(Mat src, Point2f* srcPts, Point2f* dstPts)
{
    Mat M_Transper = getPerspectiveTransform(srcPts, dstPts);
    Mat img_TransPer;
    //Top view ��ȯ
    warpPerspective(src, img_TransPer, M_Transper, Size(src.cols, src.rows));

    return img_TransPer;
}

Mat PreprocessFrame(Mat src)
{
    Mat src_HSV = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat YLine = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, src_HSV, COLOR_BGR2HSV);

    //����� ����
    Scalar low_yellow = Scalar(15, 120, 200);
    Scalar high_yellow = Scalar(25, 255, 255);
    inRange(src_HSV, low_yellow, high_yellow, YLine);

    //���� �� �и� -> ���
    Mat src_Lab = Mat::zeros(src.rows, src.cols, CV_8UC1);
    Mat WLine = Mat::zeros(src.rows, src.cols, CV_8UC1);
    cvtColor(src, src_Lab, COLOR_BGR2Lab);

    //��� ����
    Scalar low_white = Scalar(200, 120, 125);
    Scalar high_white = Scalar(255, 130, 140);
    inRange(src_Lab, low_white, high_white, WLine);

    Mat Line_img; //bridview���� ���θ� �и� 
    Mat LineContoure; //bridview���� ���θ� �и� 
    bitwise_and(src, src, Line_img, YLine);
    bitwise_and(src, src, Line_img, WLine);

    return Line_img;
}