#ifndef __VIEW_H__
#define __VIEW_H__

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ShaderProgram.h>
#include "sgraph/SGNodeVisitor.h"
#include "ObjectInstance.h"
#include "PolygonMesh.h"
#include "TextureImage.h"
#include "VertexAttrib.h"
#include "Callbacks.h"
#include "sgraph/IScenegraph.h"
#include "HitRecord.h"

#include <stack>
using namespace std;

class View
{
public:
    View();
    ~View();
    void init(Callbacks *callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes,map<string,util::TextureImage*>& textures);
    void display(sgraph::IScenegraph *scenegraph);
    void raytrace(sgraph::IScenegraph *scenegraph);
    bool shouldWindowClose();
    void closeWindow();
    void imageToPPM(const std::vector<std::vector<glm::vec3>>& image);
    glm::vec3 shade(HitRecord hitrec, vector<util::Light> lightsInViewSpace);


private: 

    GLFWwindow* window;
    util::ShaderProgram program;
    util::ShaderLocationsVault shaderLocations;
    map<string,util::ObjectInstance *> objects;
    map<string,GLuint> textureIds;
    glm::mat4 projection;
    stack<glm::mat4> modelview;
    sgraph::SGNodeVisitor *renderer;
    glm::mat4 lookat;
    int width;
    int height;
    int frames;
    float fov;
    double time;

};

#endif