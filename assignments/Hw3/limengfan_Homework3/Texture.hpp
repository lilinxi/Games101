//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H

#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

class Texture {
private:
    cv::Mat image_data;

public:
    Texture(const std::string &name) {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v, int deltaU = 0, int deltaV = 0) {
        auto u_img = u * width;
        u_img = u_img + deltaU;
        if (u_img < 0) {
            u_img = 0;
        } else if (u_img >= width) {
            u_img = width - 1;
        }
        auto v_img = (1 - v) * height;
        v_img = v_img + deltaV;
        if (v_img < 0) {
            v_img = 0;
        } else if (v_img >= height) {
            v_img = height - 1;
        }
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color00 = image_data.at<cv::Vec3b>(v_img, u_img);
        auto color01 = image_data.at<cv::Vec3b>(v_img, u_img + 1);
        auto color10 = image_data.at<cv::Vec3b>(v_img + 1, u_img);
        auto color11 = image_data.at<cv::Vec3b>(v_img + 1, u_img + 1);
        float s = u_img - int(u_img);
        float t = v_img - int(v_img);
        auto color0 = lerp(s, color00, color01);
        auto color1 = lerp(s, color10, color11);
        auto color = lerp(t, color0, color1);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

private:
    static cv::Vec3b
    lerp(float x, const cv::Vec3b &v0, const cv::Vec3b &v1) {
        return v0 + x * (v1 - v0);
    }

};

#endif //RASTERIZER_TEXTURE_H
