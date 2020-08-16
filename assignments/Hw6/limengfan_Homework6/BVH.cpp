#include <algorithm>
#include <cassert>
#include "BVH.hpp"

#include <iostream>
#include <ctime>

using namespace std;

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
        : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
          primitives(std::move(p)) {
    clock_t start_clock = clock();

    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    switch (this->splitMethod) {
        case SplitMethod::NAIVE:
            root = recursiveBuild(primitives);
            break;
        case SplitMethod::SAH:
            root = recursiveBuildSAH(primitives);
            break;
    }

    time(&stop);

    clock_t end_clock = clock();

    double diff = difftime(stop, start);
    int hrs = (int) diff / 3600;
    int mins = ((int) diff / 60) - (hrs * 60);
    int secs = (int) diff - (hrs * 3600) - (mins * 60);

    printf("\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs, total %lf ms, with primitives size: %d\n\n",
           hrs, mins, secs, (double) (end_clock - start_clock) / CLOCKS_PER_SEC * 1000 ,primitives.size());
}

BVHBuildNode *BVHAccel::recursiveBuild(std::vector<Object *> objects) {
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    } else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    } else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                    Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
            case 0:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().x <
                           f2->getBounds().Centroid().x;
                });
                break;
            case 1:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().y <
                           f2->getBounds().Centroid().y;
                });
                break;
            case 2:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().z <
                           f2->getBounds().Centroid().z;
                });
                break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

// 给定 midIndex，计算 cost
double BVHAccel::computeSAHCost(std::vector<Object *> objects, int midIndex) {
    auto beginning = objects.begin();
    auto middling = objects.begin() + (midIndex);
    auto ending = objects.end();

    auto leftshapes = std::vector<Object *>(beginning, middling);
    auto rightshapes = std::vector<Object *>(middling, ending);

    assert(objects.size() == (leftshapes.size() + rightshapes.size()));

    Bounds3 leftBounds;
    for (Object *o:leftshapes) {
        leftBounds = Union(leftBounds, o->getBounds());
    }
    Bounds3 rightBounds;
    for (Object *o:rightshapes) {
        rightBounds = Union(rightBounds, o->getBounds());
    }
    Bounds3 totalBounds = Union(leftBounds, rightBounds);
    double cost = (leftBounds.SurfaceArea() + rightBounds.SurfaceArea()) / totalBounds.SurfaceArea();
    return cost;
}

// 计算一个最好的 midIndex 出来
int BVHAccel::tryBuildSAH(std::vector<Object *> objects, double cost, int begin, int step) {
    // 去除 0 和 objects.size() - 1，防止一边模型数量为 0
    for (int i = begin; i > 0 && i < objects.size() - 1; i += step) {
        double tryCost = BVHAccel::computeSAHCost(objects, i);
        if (tryCost <= cost) {
            cost = tryCost;
        } else {
            return i - step;
        }
    }
    // 没有出现转折点
    return begin;
}

BVHBuildNode *BVHAccel::recursiveBuildSAH(std::vector<Object *> objects) {
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    } else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    } else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                    Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
            case 0:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().x <
                           f2->getBounds().Centroid().x;
                });
                break;
            case 1:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().y <
                           f2->getBounds().Centroid().y;
                });
                break;
            case 2:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().z <
                           f2->getBounds().Centroid().z;
                });
                break;
        }

        int midIndex = objects.size() / 2;
//        以此值作为初始值
        double cost = BVHAccel::computeSAHCost(objects, midIndex);

        double cost_1 = BVHAccel::computeSAHCost(objects, midIndex - 1); // 左移一个
        double cost1 = BVHAccel::computeSAHCost(objects, midIndex + 1); // 右移一个

//        cout << "total: " << objects.size() << ", beginMid: " << midIndex;

        if (cost <= cost_1 && cost <= cost1) {
            // 当前值即为最好值
        } else if (cost > cost_1) {
            // 向左寻找最优 midIndex
            midIndex = BVHAccel::tryBuildSAH(objects, cost, midIndex, -1);
        } else if (cost > cost1) {
            // 向右寻找最优 midIndex
            midIndex = BVHAccel::tryBuildSAH(objects, cost, midIndex, 1);
        }

//        cout << ", endMid: " << midIndex << endl;

        auto beginning = objects.begin();
        auto middling = objects.begin() + (midIndex);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuildSAH(leftshapes);
        node->right = recursiveBuildSAH(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray &ray) const {
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node, const Ray &ray) const {
    // TODO Traverse the BVH to find intersection
    Intersection isect;

    std::array<int, 3> dirIsNeg{
            int(ray.direction.x > 0),
            int(ray.direction.y > 0),
            int(ray.direction.z > 0)};

    if (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg)) {
        if (node->left == nullptr && node->right == nullptr) {
            return node->object->getIntersection(ray);
        } else if (node->left != nullptr && node->right != nullptr) {
            Intersection hit1 = BVHAccel::getIntersection(node->left, ray);
            Intersection hit2 = BVHAccel::getIntersection(node->right, ray);

            return hit1.distance < hit2.distance ? hit1 : hit2;
        }
    }
    return isect;
}