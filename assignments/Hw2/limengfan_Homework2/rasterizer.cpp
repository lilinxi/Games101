// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions) {
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices) {
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols) {
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f &v3, float w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static double v2_cross(const Vector2f &a, const Vector2f &b) {
    return a.x() * b.y() - b.x() * a.y();
}

static bool insideTriangle(float x, float y, const Vector3f *_v) {
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector2f point{x, y};
    bool state = v2_cross((_v[0] - _v[1]).head<2>(), point - _v[1].head<2>()) > 0;
    if (v2_cross((_v[1] - _v[2]).head<2>(), point - _v[2].head<2>()) > 0 == state) {
        if (v2_cross((_v[2] - _v[0]).head<2>(), point - _v[0].head<2>()) > 0 == state) {
            return true;
        }
    }
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f *v) {
    float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
               (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() -
                v[2].x() * v[1].y());
    float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
               (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() -
                v[0].x() * v[2].y());
    float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
               (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() -
                v[1].x() * v[0].y());
    return {c1, c2, c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type) {
    auto &buf = pos_buf[pos_buffer.pos_id];
    auto &ind = ind_buf[ind_buffer.ind_id];
    auto &col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto &i : ind) {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto &vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto &vert : v) {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i) {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

//        rasterize_triangle(t);
        rasterize_triangle_msaa(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle &t) {
    auto v = t.toVector4();
    // TODO : Find out the bounding box of current triangle.
    Vector2i min_xy{INT_MAX, INT_MAX};
    Vector2i max_xy{INT_MIN, INT_MIN};
    for (const Vector3f &tv :t.v) {
        min_xy[0] = std::min<double>(min_xy.x(), tv.x()); // 舍去
        min_xy[1] = std::min<double>(min_xy.y(), tv.y()); // 舍去
        max_xy[0] = std::max<double>(max_xy.x(), tv.x()) + 0.99; // 进位
        max_xy[1] = std::max<double>(max_xy.y(), tv.y()) + 0.99; // 进位
    }
    // iterate through the pixel and find if the current pixel is inside the triangle
    for (int _x = min_xy[0]; _x <= max_xy[0]; _x++) {
        for (int _y = min_xy[1]; _y <= max_xy[1]; _y++) {
            float x = _x + 0.5;
            float y = _y + 0.5;
            if (insideTriangle(x, y, t.v)) {
                // If so, use the following code to get the interpolated z value.
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated =
                        alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                if (judge_and_set_depth(_x, _y, z_interpolated)) {
                    this->set_pixel(Vector3f{_x, _y, 1}, t.getColor());
                }
            }
        }
    }
}

void rst::rasterizer::rasterize_triangle_msaa(const Triangle &t) {
    auto v = t.toVector4();
    // TODO : Find out the bounding box of current triangle.
    Vector2i min_xy{INT_MAX, INT_MAX};
    Vector2i max_xy{INT_MIN, INT_MIN};
    for (const Vector3f &tv :t.v) {
        min_xy[0] = std::min<double>(min_xy.x(), tv.x()); // 舍去
        min_xy[1] = std::min<double>(min_xy.y(), tv.y()); // 舍去
        max_xy[0] = std::max<double>(max_xy.x(), tv.x()) + 0.99; // 进位
        max_xy[1] = std::max<double>(max_xy.y(), tv.y()) + 0.99; // 进位
    }
    // iterate through the pixel and find if the current pixel is inside the triangle
    for (int _x = min_xy[0]; _x <= max_xy[0]; _x++) {
        for (int _y = min_xy[1]; _y <= max_xy[1]; _y++) {
            int count = 0;
            float x = _x + 1. / 3.;
            float y = _y + 1. / 3.;
            if (insideTriangle(x, y, t.v)) {
                count += 1;
                // If so, use the following code to get the interpolated z value.
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated =
                        alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                judge_and_set_depth_and_pixel_msaa(x, y, 0, z_interpolated, t.getColor());
            }

            x = _x + 2. / 3.;
            y = _y + 1. / 3.;
            if (insideTriangle(x, y, t.v)) {
                count += 1;
                // If so, use the following code to get the interpolated z value.
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated =
                        alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                judge_and_set_depth_and_pixel_msaa(x, y, 1, z_interpolated, t.getColor());
            }


            x = _x + 1. / 3.;
            y = _y + 2. / 3.;
            if (insideTriangle(x, y, t.v)) {
                count += 1;
                // If so, use the following code to get the interpolated z value.
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated =
                        alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                judge_and_set_depth_and_pixel_msaa(x, y, 2, z_interpolated, t.getColor());
            }

            x = _x + 2. / 3.;
            y = _y + 2. / 3.;
            if (insideTriangle(x, y, t.v)) {
                count += 1;
                // If so, use the following code to get the interpolated z value.
                auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated =
                        alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                judge_and_set_depth_and_pixel_msaa(x, y, 3, z_interpolated, t.getColor());
            }

            if (count > 0) {
                auto ind = get_index(_x, _y);
                Vector3f color = (frame_list_buf[ind * 2] * (4 - count) + frame_list_buf[ind] * count) / 4; // 混合前景和背景
                set_pixel(Vector3f{_x, _y, 1}, color);
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f &m) {
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f &v) {
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f &p) {
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff) {
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(frame_list_buf.begin(), frame_list_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth) {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        std::fill(depth_list_buf.begin(), depth_list_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    frame_list_buf.resize(w * h * 2);
    depth_list_buf.resize(w * h * 4);
}

int rst::rasterizer::get_index(int x, int y) {
    return (height - 1 - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f &point, const Eigen::Vector3f &color) {
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}

// 深度测试通过，成功设置则返回 true
bool rst::rasterizer::judge_and_set_depth(int x, int y, float z) {
    auto ind = (height - 1 - y) * width + x;
    if (depth_buf[ind] > z) {
        depth_buf[ind] = z;
        return true;
    }
    return false;
}

// MSAA, sample list, 深度测试通过，成功设置则返回 true
bool
rst::rasterizer::judge_and_set_depth_and_pixel_msaa(int x, int y, int count, float z, const Eigen::Vector3f &color) {
    auto ind = (height - 1 - y) * width + x;
    if (depth_list_buf[ind * count] > z) {
        depth_list_buf[ind * count] = z; // 存储四个深度
        frame_list_buf[ind * 2] = frame_list_buf[ind]; // 背景
        frame_list_buf[ind] = color; // 前景
        return true;
    }
    return false;
}

// clang-format on