---

李梦凡同学**已完成**提高题，但是在main.cpp中的调用已经注释掉了。

摘抄过来的`Eigen::Matrix4f get_rotation(Vector3f axis, float angle)`函数的代码如下：

```c++
Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f axis_hat;
    float x = axis[0];
    float y = axis[1];
    float z = axis[2];
    axis_hat
            <<
            0, -z, y,
            z, 0, -x,
            -y, x, 0;
    Eigen::Matrix3f R = cos(angle / 180 * MY_PI) * I +
                        (1 - cos(angle / 180 * MY_PI)) * axis * axis.transpose() +
                        sin(angle / 180 * MY_PI) * axis_hat;

    Eigen::Matrix4f model;
    model
            <<
            R(0, 0), R(0, 1), R(0, 2), 0,
            R(1, 0), R(1, 1), R(1, 2), 0,
            R(2, 0), R(2, 1), R(2, 2), 0,
            0, 0, 0, 1;
    return model;
}
```