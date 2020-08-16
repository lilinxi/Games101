---

李梦凡同学已完成作业

得分点完成情况：

1. 完成（格式正确，5分）
2. 完成（路径追踪，45分）
3. 完成（多线程，10分）
4. 未完成（微表面，10分）

下面展示结果图（spp=66，分辨率784*784，六个线程）

![](./images/binary_v2_spp66_thread6.png)

使用spp=66，开启六个线程，每个线程计算spp_per_thread=11的耗时如下

```c++
Render complete: 
Time taken: 0 hours
          : 52 minutes
          : 3173 seconds
```

## 多线程实现方法

通过定义了多个线程，每个线程计算若干了spp的方式进行实现。本文使用机器共6核，所以上面展示的效果为spp=66，开启六个线程，每个线程计算spp_per_thread=11。

多线程方法定义为`RenderThread`在main函数中被注释掉，程序默认运行的效果为单线程spp=1的效果。

其中`RenderThread`循环创建了若干线程，每个线程的执行程序定义在`OneThread`方法中，其主要作用是每个线程计算spp_per_thread，并最终除以总spp，达到了分离任务的效果。

## 微表面（未实现）

未实现