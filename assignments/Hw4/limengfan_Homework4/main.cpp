#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) {
//    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) {
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.0001) {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) {
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() == 1) {
        return control_points[0];
    } else {
        std::vector<cv::Point2f> control_points_new;
        control_points_new.reserve(control_points.size() - 1);
        for (int i = 0; i < control_points.size() - 1; i++) {
            control_points_new.push_back(control_points[i] * t + (1 - t) * control_points[i + 1]);
        }
        return recursive_bezier(control_points_new, t);
    }
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) {
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's recursive Bezier algorithm.
    for (double t = 0.0; t <= 1.0; t += 0.0001) {
        cv::Point2f point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255;

        // 反走样操作（没有考虑边界的越界操作）
        // 每个点采样一个全颜色点和三个部分颜色点，需要注意加上原颜色并取模防止覆盖
        cv::Point2f center{int(point.x) + .5f, int(point.y) + .5f};
        cv::Point2f distance = point - center;
        // 左或右点根据距离取颜色
        window.at<cv::Vec3b>(point.y, distance.x > 0 ? point.x + 1 : point.x - 1)[1] =
                int((window.at<cv::Vec3b>(point.y, distance.x > 0 ? point.x + 1 : point.x - 1)[1] +
                     255 * (distance.x * distance.x) * 4)) % 255; // 0~0.25*4 -> 0~1
        // 上或下点根据距离取颜色
        window.at<cv::Vec3b>(distance.y > 0 ? point.y + 1 : point.y - 1, point.x)[1] =
                int(window.at<cv::Vec3b>(distance.y > 0 ? point.y + 1 : point.y - 1, point.x)[1] +
                    255 * (distance.y * distance.y) * 4) % 255; // 0~0.25*4 -> 0~1
        // 左(右)上或左(右)下点根据距离取颜色
        window.at<cv::Vec3b>(distance.y > 0 ? point.y + 1 : point.y - 1,
                             distance.x > 0 ? point.x + 1 : point.x - 1)[1] =
                int(window.at<cv::Vec3b>(distance.y > 0 ? point.y + 1 : point.y - 1,
                                         distance.x > 0 ? point.x + 1 : point.x - 1)[1] +
                    255 * (distance.x * distance.x + distance.y * distance.y) * 2) % 255; // 0~0.5*2 -> 0~1
    }
}

int main() {
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) {
        window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
        for (auto &point : control_points) {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() >= 4) {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
//            cv::imwrite("my_bezier_curve.png", window);
//            key = cv::waitKey(0);

//            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
