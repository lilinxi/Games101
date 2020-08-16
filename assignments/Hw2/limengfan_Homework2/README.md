---

李梦凡同学**已完成**提高题，但是在`rst::rasterizer::draw`中的调用`rasterize_triangle_msaa`已经注释掉了。

- 简单采样结果如下图：

![](./images/simple_sample.png)

放大简单采样可以看到锯齿

![](./images/zoom_in_simple_sample.png)

- MSAA采样结果如下图：

![](./images/super_sample.png)

放大MSAA采样可以看到锯齿被模糊处理，并且融合了前景色和背景色，没有黑边

![](./images/zoom_in_super_sample.png)