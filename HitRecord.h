#ifndef __HITRECORD_H__
#define __HITRECORD_H__

#include <glm/glm.hpp>
#include "Material.h"

class HitRecord 
{
public:
    HitRecord(float time, glm::vec3 intersection, glm::vec3 normalVec, util::Material material) {
        t = time;
        hit = false;
        intersect = intersection;
        normal = normalVec;
        mat = material;
    }

    HitRecord() {
        hit = false;
    }

    float getTime() {
        return t;
    }

    void setTime(float time) {
        t = time;
    }

    bool getHit() {
        return hit;
    }

    void triggerHit() {
        hit = true;
    }

    glm::vec3 getIntersect() {
        return intersect;
    }

    void setIntersect(glm::vec3 intersection) {
        intersect = intersection;
    }

    glm::vec3 getNormal() {
        return normal;
    }

    void setNormal(glm::vec3 normalVec) {
        normal = normalVec;
    }

    util::Material getMaterial() {
        return mat;
    }

    void setMaterial(const util::Material& material) {
        mat = material;
    }
    
private:
    float t;
    bool hit;
    glm::vec3 intersect;
    glm::vec3 normal;
    util::Material mat;
};

#endif