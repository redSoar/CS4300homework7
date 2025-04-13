#ifndef __RAY3D_H__
#define __RAY3D_H__

#include <glm/glm.hpp>

class Ray3D 
{
public:
    Ray3D() {
        point = glm::vec3(0,0,0);
        direction = glm::vec3(0,0,0);
    }

    Ray3D(glm::vec3 pnt, glm::vec3 dir) {
        point = pnt;
        direction = dir;
    }

    glm::vec3 getPoint() {
        return point;
    }

    glm::vec3 getDirection() {
        return direction;
    }

    void setPoint(glm::vec3 pnt) {
        point = pnt;
    }

    void setDirection(glm::vec3 dir) {
        direction = dir;
    }

private:
    glm::vec3 point;
    glm::vec3 direction;
};

#endif