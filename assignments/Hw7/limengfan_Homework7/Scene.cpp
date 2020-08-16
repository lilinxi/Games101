//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include <iostream>

using namespace std;


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const {
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum) {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object *> &objects,
        float &tNear, uint32_t &index, Object **hitObject) {
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter = this->intersect(ray);
    if (!inter.happened) {
        return 0;
    }
    if (inter.m->hasEmission()) {
        return inter.m->getEmission();
    }
    Vector3f wo = -ray.direction;
    Vector3f l_dir = 0;
    Intersection lightSampler;
    float lightPDF;
    this->sampleLight(lightSampler, lightPDF);
    Vector3f lightDir = normalize(inter.coords - lightSampler.coords);
    Vector3f ws = -lightDir;
    Ray lightRay = Ray(lightSampler.coords, lightDir);
    Intersection lightInter = this->intersect(lightRay);
    Vector3f lightDelta = lightInter.coords - inter.coords;
    if (lightDelta.norm() < 0.01) {
            l_dir = lightSampler.emit * inter.m->eval(wo, ws, inter.normal) * dotProduct(ws, inter.normal) *
                    dotProduct(ws, lightInter.normal) /
                    (lightSampler.coords - lightInter.coords).norm() /
                    (lightSampler.coords - lightInter.coords).norm() /
                    lightPDF;
    }

    Vector3f l_indir = 0;
    if (get_random_float() < this->RussianRoulette) {
        Vector3f wi = normalize(inter.m->sample(wo, inter.normal));
        Ray indirRay = Ray(inter.coords, wi);
        Intersection indirInter = this->intersect(indirRay);
        if (indirInter.happened && !indirInter.obj->hasEmit()) {
            l_indir = this->castRay(indirRay, depth + 1) * inter.m->eval(wo, wi, inter.normal) *
                      dotProduct(wi, inter.normal) /
                      inter.m->pdf(wo, wi, inter.normal) / this->RussianRoulette;
        }
    }


    return l_dir + l_indir;
}