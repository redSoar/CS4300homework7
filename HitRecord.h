#ifndef __HITRECORD_H__
#define __HITRECORD_H__

#include <glm/glm.hpp>
#include "Material.h"

/**
 * This class implements a hit record to record information of the time of intersection, point of intersection, 
 * normal of intersection, material attributes and the texture color of the hit by a ray.
 */
class HitRecord 
{
public:
    HitRecord(float time, glm::vec3 intersection, glm::vec3 normalVec, util::Material material, glm::vec4 texture) {
        t = time;
        hit = false;
        intersect = intersection;
        normal = normalVec;
        mat = material;
        textureColor = texture;
    }

    HitRecord() {
        hit = false;
        t = -1.0f;
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

    glm::vec4 getTextureColor() {
        return textureColor;
    }

    void setTextureColor(glm::vec4 texture) {
        textureColor = texture;
    }
    
private:
    float t;
    bool hit;
    glm::vec3 intersect;
    glm::vec3 normal;
    util::Material mat;
    glm::vec4 textureColor;
};

#endif