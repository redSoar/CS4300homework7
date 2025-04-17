#include "Controller.h"
#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "ObjImporter.h"
using namespace sgraph;
#include <iostream>
using namespace std;

#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"

Controller::Controller(Model& m,View& v) {
    model = m;
    view = v;

    initScenegraph();
    rayTraceMode = false;
    count = 0;
}

void Controller::initScenegraph() {

     
    
    //read in the file of commands
    ifstream inFile("scenegraphmodels/finalscene.txt");
    //ifstream inFile("tryout.txt");
    sgraph::ScenegraphImporter importer;
    

    IScenegraph *scenegraph = importer.parse(inFile);
    //scenegraph->setMeshes(meshes);
    model.setScenegraph(scenegraph);
    cout <<"Scenegraph made" << endl;   

}

Controller::~Controller()
{
    
}

void Controller::run()
{
    sgraph::IScenegraph * scenegraph = model.getScenegraph();
    map<string,util::PolygonMesh<VertexAttrib> > meshes = scenegraph->getMeshes();
    map<string,util::TextureImage *> textures = scenegraph->getTextures();
    view.init(this,meshes,textures);
    while (!view.shouldWindowClose()) {
        if(rayTraceMode){
            view.raytrace(scenegraph);
            rayTraceMode = !rayTraceMode;
        } else {
            view.display(scenegraph);
        }
    }
    view.closeWindow();
    exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) { // 's' or 'S' for ray tracing.
        rayTraceMode = !rayTraceMode;
        count = 0;
    }
}

void Controller::reshape(int width, int height) 
{
    //cout <<"Window reshaped to width=" << width << " and height=" << height << endl;
    glViewport(0, 0, width, height);
}

void Controller::dispose()
{
    view.closeWindow();
}

void Controller::error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}