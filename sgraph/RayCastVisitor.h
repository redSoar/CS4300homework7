#ifndef _RAYCASTVISITOR_H_
#define _RAYCASTVISITOR_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include <ShaderProgram.h>
#include <ShaderLocationsVault.h>
#include "ObjectInstance.h"
#include "../Ray3D.h"
#include "../include/TextureImage.h"
#include "../PPMImageLoader.h"
#include <stack>
#include <iostream>
using namespace std;

namespace sgraph {
    /**
     * This visitor implements a text rendering that includes the tabs and hyphens used to show levels, and prints the names of each node as shown.
     */
    class RayCastVisitor: public SGNodeVisitor {
        public:
        RayCastVisitor(stack<glm::mat4>& mv, Ray3D ray) : modelview(mv), ray(ray){
        }
        RayCastVisitor(stack<glm::mat4>& mv, Ray3D ray, map<string,util::TextureImage*> textures) : modelview(mv), ray(ray), textures(textures){
        }
        void visitGroupNode(GroupNode *groupNode) {
            for (int i=0;i<groupNode->getChildren().size();i=i+1) {
                groupNode->getChildren()[i]->accept(this);
            }
        }

        void visitLeafNode(LeafNode *leafNode) {
            string instance = leafNode->getInstanceOf();
            glm::mat4 currentModelView = modelview.top();
            glm::vec4 transformedOrigin = glm::inverse(currentModelView) * glm::vec4(ray.getPoint(), 1.0f);
            glm::vec4 transformedDirection = glm::inverse(currentModelView) * glm::vec4(ray.getDirection(), 0.0f);
            
            if(instance == "box") {
                checkForBoxIntersection(transformedOrigin, transformedDirection, leafNode->getMaterial(), leafNode->getTextureName());
            }
            else if(instance == "sphere") {
                checkForSphereIntersection(transformedOrigin, transformedDirection, leafNode->getMaterial());
            }
            else if(instance == "cylinder") {
                // checkForCylinderIntersection(transformedOrigin, transformedDirection, leafNode->getMaterial());
            }
            else if(instance == "cone") {

            }
        }

        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            modelview.top() = modelview.top() * transformNode->getTransform();
            if (transformNode->getChildren().size()>0) {
                transformNode->getChildren()[0]->accept(this);
            }
            modelview.pop();
        }

        void visitScaleTransform(ScaleTransform *scaleNode) {
            visitTransformNode(scaleNode);
        }

        void visitTranslateTransform(TranslateTransform *translateNode) {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode) {
            visitTransformNode(rotateNode);
        }

        HitRecord getHit() {
            return hit;
        }

        private:
        stack<glm::mat4>& modelview;   
        Ray3D ray; 
        HitRecord hit;
        int depth = 0;
        int indentation = 3;
        map<string,util::TextureImage*> textures;

        void checkForBoxIntersection(glm::vec3 s, glm::vec3 v, util::Material mat, string textureName) {
            float minimums[3];
            float maximums[3];
            for (int i = 0; i < 3; i++) {
                float num1 = (-0.5f - s[i]) / v[i];
                float num2 = (0.5f - s[i]) / v[i];
                minimums[i] = std::min(num1, num2);
                maximums[i] = std::max(num1, num2);
            }

            float minimum = std::max(minimums[0], std::max(minimums[1], minimums[2]));
            float maximum = std::min(maximums[0], std::min(maximums[1], maximums[2]));

            if (maximum >= minimum) {
                glm::vec3 poi = s + minimum*v;
                glm::vec4 viewPoi = modelview.top() * glm::vec4(poi, 1.0f);

                glm::vec3 normal = calculateNormalForBox(poi);
                glm::vec4 viewNormal = glm::inverse(glm::transpose(modelview.top())) * glm::vec4(normal, 0.0f);
                
                if(minimum < 0.0f) {
                    return;
                }
                glm::vec4 textureColor;
                float width = 0.0f;
                float height = 0.0f;
                float x = poi.x + 0.5f;
                float y = poi.y + 0.5f;
                float z = poi.z + 0.5f;

                if (normal == glm::vec3(1, 0, 0)) {
                    // Right
                    width = 0.5f + 0.25f * z;
                    height = 0.25f + 0.25f * y;
                }
                else if (normal == glm::vec3(-1, 0, 0)) {
                    // Left
                    width = 0.25f - 0.25f * z;
                    height = 0.25f + 0.25f * y;
                }
                else if (normal == glm::vec3(0, 1, 0)) {
                    // Top
                    width = 0.25f + 0.25f * x;
                    height = 0.5f + 0.25f * z;
                }
                else if (normal == glm::vec3(0, -1, 0)) {
                    // Bottom
                    width = 0.25f + 0.25f * x;
                    height = 0.25f - 0.25f * z;
                }
                else if (normal == glm::vec3(0, 0, 1)) {
                    // Back
                    width = 0.25f + 0.25f * x;
                    height = 0.25f + 0.25f * y;
                }
                else if (normal == glm::vec3(0, 0, -1)) {
                    // Front
                    width = 1.0f - 0.25f * x;
                    height = 0.25f + 0.25f * y;
                }
                textureColor = textures[textureName]->getColor(width, height);

                textureColor.r /= 255.0f;
                textureColor.g /= 255.0f;
                textureColor.b /= 255.0f;

                if (hit.getHit()) {
                    if (hit.getTime() > minimum) {
                        editHit(minimum, viewPoi, viewNormal, mat, textureColor);
                    }
                }
                else {
                    hit.triggerHit();
                    editHit(minimum, viewPoi, viewNormal, mat, textureColor);
                }
            }
        }

        void checkForSphereIntersection(glm::vec3 s, glm::vec3 v, util::Material mat) {
            float A = glm::dot(v, v);
            float B = 2 * glm::dot(v, s);
            float C = glm::dot(s, s) - 1;
            if((B * B - 4 * A * C) >= 0) {
                float t1 = (-B + std::sqrt(B * B - 4 * A * C))/(2 * A);
                float t2 = (-B - std::sqrt(B * B - 4 * A * C))/(2 * A);
                float t;

                if (t1 > 0 && t2 > 0) {
                    t = std::min(t1, t2);
                }
                else if (t1 > 0) {
                    t = t1;
                }
                else if (t2 > 0) {
                    t = t2;
                }
                else {
                    return;
                }

                glm::vec3 poi = s + t * v;
                glm::vec4 viewPoi = modelview.top() * glm::vec4(poi, 1.0f);
                glm::vec3 normal = glm::normalize(poi);
                glm::vec4 viewNormal = glm::inverse(glm::transpose(modelview.top())) * glm::vec4(normal, 0.0f);
                if (t < 0.0f ) {
                    return;
                }
                if (hit.getHit()) {
                    if (hit.getTime() > t) {
                        editHit(t, viewPoi, viewNormal, mat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }
                } else {
                    hit.triggerHit();
                    editHit(t, viewPoi, viewNormal, mat, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
            }
        }

        // void checkForCylinderIntersection(glm::vec3 s, glm::vec3 v, util::Material mat){
        //     float A = glm::dot(v, v) - std::pow(glm::dot(v, glm::vec3(0, 1, 0)), 2);
        //     float B = 2 * (glm::dot(v, s) - glm::dot(v, glm::vec3(0, 1, 0)) * glm::dot(s, glm::vec3(0, 1, 0)));
        //     float C = glm::dot(s, s) - std::pow(glm::dot(s, glm::vec3(0, 1, 0)), 2) - 1;
        //     if((B * B - 4 * A * C) >= 0) {
        //         float t1 = (-B + std::sqrt(B * B - 4 * A * C))/(2 * A);
        //         float t2 = (-B - std::sqrt(B * B - 4 * A * C))/(2 * A);
        //         float t;

        //         if (t1 > 0 && t2 > 0) {
        //             t = std::min(t1, t2);
        //         }
        //         else if (t1 > 0) {
        //             t = t1;
        //         }
        //         else if (t2 > 0) {
        //             t = t2;
        //         }
        //         else {
        //             return;
        //         }

        //         glm::vec3 poi = s + t * v;
        //         glm::vec4 viewPoi = modelview.top() * glm::vec4(poi, 1.0f);

        //         float m = glm::dot(v, glm::vec3(0, 1, 0)) * t + glm::dot(s, glm::vec3(0, 1, 0));
        //         glm::vec3 normal = glm::normalize(poi - glm::vec3(0, 1, 0) * m);
        //         glm::vec4 viewNormal = glm::inverse(glm::transpose(modelview.top())) * glm::vec4(normal, 0.0f);
        //         if (hit.getHit()) {
        //             if (hit.getTime() > t) {
        //                 editHit(t, viewPoi, viewNormal, mat);
        //             }
        //         } else {
        //             hit.triggerHit();
        //             editHit(t, viewPoi, viewNormal, mat);
        //         }
        //     }
        // }

        void editHit(float time, glm::vec4 intersection, glm::vec4 normalVec, util::Material material, glm::vec4 textureColor) {
            hit.setTime(time);
            hit.setIntersect(glm::vec3(intersection.x, intersection.y, intersection.z));
            hit.setNormal(glm::vec3(normalVec.x, normalVec.y, normalVec.z));
            hit.setMaterial(material);
            hit.setTextureColor(textureColor);
        }

        glm::vec3 calculateNormalForBox(glm::vec3 poi) {
            const float epsilon = 1e-3f;
            if (fabs(poi.x - 0.5f) < epsilon) {
                return glm::vec3(1,0,0); //Right
            }
            else if (fabs(poi.x + 0.5f) < epsilon) {
                return glm::vec3(-1,0,0); //Left
            }
            else if (fabs(poi.y - 0.5f) < epsilon) {
                return glm::vec3(0,1,0); //Top
            }
            else if (fabs(poi.y + 0.5f) < epsilon) {
                return glm::vec3(0,-1,0); //Bottom
            }
            else if (fabs(poi.z - 0.5f) < epsilon) {
                return glm::vec3(0,0,1); //Back
            }
            else if (fabs(poi.z + 0.5f) < epsilon) {
                return glm::vec3(0,0,-1); //Front
            }
            cout << "shoudl never print out" << endl;
            return glm::vec3(0, 0, 0);
        }

   };
}

#endif