#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sgraph/GLScenegraphRenderer.h"
#include "VertexAttrib.h"
#include "sgraph/LightGatherer.h"
#include "sgraph/Scenegraph.h"
#include "Ray3D.h"

View::View() {

}

View::~View(){

}

void View::init(Callbacks *callbacks,map<string,util::PolygonMesh<VertexAttrib>>& meshes,map<string,util::TextureImage*>& textures) 
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    width = 800;
    height = 800;
    fov = 60.0f;

    window = glfwCreateWindow(width, height, "Lights and Textures in a Scenegraph", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
     glfwSetWindowUserPointer(window, (void *)callbacks);

    //using C++ functions as callbacks to a C-style library
    glfwSetKeyCallback(window, 
    [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->onkey(key,scancode,action,mods);
    });

    glfwSetWindowSizeCallback(window, 
    [](GLFWwindow* window, int width,int height)
    {
        reinterpret_cast<Callbacks*>(glfwGetWindowUserPointer(window))->reshape(width,height);
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // create the shader program
    program.createProgram(string("shaders/lights-textures.vert"),
                          string("shaders/lights-textures.frag"));
    // assuming it got created, get all the shader variables that it uses
    // so we can initialize them at some point
    // enable the shader program
    program.enable();
    shaderLocations = program.getAllShaderVariables();

    
    /* In the mesh, we have some attributes for each vertex. In the shader
     * we have variables for each vertex attribute. We have to provide a mapping
     * between attribute name in the mesh and corresponding shader variable
     name.
     *
     * This will allow us to use PolygonMesh with any shader program, without
     * assuming that the attribute names in the mesh and the names of
     * shader variables will be the same.

       We create such a shader variable -> vertex attribute mapping now
     */
    map<string, string> shaderVarsToVertexAttribs;

    shaderVarsToVertexAttribs["vPosition"] = "position";
    shaderVarsToVertexAttribs["vNormal"] = "normal";
    shaderVarsToVertexAttribs["vTexCoord"] = "texcoord";
    
    //objects
    for (typename map<string,util::PolygonMesh<VertexAttrib> >::iterator it=meshes.begin();
           it!=meshes.end();
           it++) {
        util::ObjectInstance * obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations,shaderVarsToVertexAttribs,it->second);
        objects[it->first] = obj;
    }

    //textures
    for (typename map<string,util::TextureImage*>::iterator it=textures.begin();
           it!=textures.end();
           it++) {
        GLuint textureId;
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D,textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //if the s-coordinate goes outside (0,1), repeat it
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //if the t-coordinate goes outside (0,1), repeat it
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, it->second->getWidth(),it->second->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,it->second->getImage());
        glGenerateMipmap(GL_TEXTURE_2D);
        textureIds[it->first] = textureId;
        
    } 
    
	int window_width,window_height;
    glfwGetFramebufferSize(window,&window_width,&window_height);

    //prepare the projection matrix for perspective projection
	projection = glm::perspective(glm::radians(fov),(float)window_width/window_height,0.1f,10000.0f);
    glViewport(0, 0, window_width,window_height);

    frames = 0;
    time = glfwGetTime();

    renderer = new sgraph::GLScenegraphRenderer(modelview,objects,textureIds,shaderLocations);
    lookat = glm::lookAt(glm::vec3(0.0f,300.0f,300.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
    
}

void View::raytrace(sgraph::IScenegraph *scenegraph) {
    cout << "Processing Ray Trace" << endl;
    std::vector<std::vector<glm::vec3>> image(width, std::vector<glm::vec3>(height));
    modelview.push(glm::mat4(1.0));
    modelview.top() = modelview.top() * lookat;
    int currentPrint = -1;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            glm::vec3 s = glm::vec3(0, 0, 0);
            int Vx = -(width/2) + x;
            int Vy = -(height/2) + y;
            int Vz = (-0.5f * height)/(tan(0.5f * glm::radians(fov)));
            glm::vec3 d = glm::vec3(Vx, Vy, Vz);
            Ray3D ray(s, d);
            sgraph::Scenegraph* sgraph = (dynamic_cast<sgraph::Scenegraph*>(scenegraph));
            HitRecord hit = sgraph->raycast(ray, modelview.top());
            if(hit.getHit()) {
                sgraph::LightGatherer * gatherer = new sgraph::LightGatherer(modelview);
                scenegraph->getRoot()->accept(gatherer);
                vector<util::Light> lightsInViewSpace = gatherer->getLightsInViewSpace();
                image[x][y] = shade(hit, lightsInViewSpace);
                //image[x][y] = glm::vec3(hit.getMaterial().getAmbient().x, hit.getMaterial().getAmbient().y, hit.getMaterial().getAmbient().z);
            }
            else {
                image[x][y] = glm::vec3(1, 1, 1);
            }
            int currentProgress = ((y * width + x) * 100) / (height * width);
            if (currentProgress >= (currentPrint + 1)) {
                printf("Loading: %d%%\r", currentProgress);
                currentPrint = currentProgress;
            }
        }
    }
    cout << "Loading Complete" << endl;
    cout << "PPM Image has been created at root directory : output.ppm" << endl;
    // for(int i = 0; i < height; i++) {
    //     cout << endl;
    //     for(int j = 0; j < width; j++) {
    //         cout << image[i][j].x << " ";
    //     }
    // }

    // Draw image
    imageToPPM(image);
    modelview.pop();
}



void View::imageToPPM(const std::vector<std::vector<glm::vec3>>& image){
    std::ofstream imageFile("output.ppm", std::ios::binary);
    if (imageFile.is_open()) {
        imageFile << "P3\n" << width << " " << height << "\n255\n";
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                int r = static_cast<int>(image[x][y].x * 255.0f);
                r = r > 255 ? 255 : r;
                int g = static_cast<int>(image[x][y].y * 255.0f);
                g = g > 255 ? 255 : g;
                int b = static_cast<int>(image[x][y].z * 255.0f);
                b = b > 255 ? 255 : b;
                imageFile << r << " " << g << " " << b << " ";
            }
            imageFile << "\n";
        }
        imageFile.close();
    } else {
        std::cerr << "Error opening file." << std::endl;
    }
}

glm::vec3 View::shade(HitRecord hitrec, vector<util::Light> light) {
    glm::vec4 fColor = glm::vec4(0,0,0,1);
    int numLights = light.size();
    glm::vec3 lightVec,viewVec,reflectVec;
    glm::vec3 normalView;
    glm::vec3 ambient,diffuse,specular;
    float nDotL,rDotV;
    for (int i=0;i<numLights;i++)
    {
        glm:: vec3 spotdirection= glm::vec3(0,0,0);
        if (glm::length(light[i].getSpotDirection())>0.01f){
            spotdirection = glm::normalize(glm::vec3(light[i].getSpotDirection()));
        }
        if (light[i].getPosition().w!=0) {
            lightVec = glm::normalize(glm::vec3(light[i].getPosition()) - hitrec.getIntersect());
        }
        else{
            lightVec = glm::normalize(-glm::vec3(light[i].getPosition()));
        }
        if (glm::dot(-lightVec,spotdirection)<=glm::cos(glm::radians(light[i].getSpotCutoff()))) {
            continue;
        }

        normalView = glm::normalize(hitrec.getNormal());
        nDotL = glm::dot(normalView,lightVec);
        viewVec = -hitrec.getIntersect();
        viewVec = glm::normalize(viewVec);

        reflectVec = glm::reflect(-lightVec,normalView);

        rDotV = std::max(glm::dot(reflectVec,viewVec),0.0f);

        ambient = glm::vec3(hitrec.getMaterial().getAmbient()) * light[i].getAmbient();
        diffuse = glm::vec3(hitrec.getMaterial().getDiffuse()) * light[i].getDiffuse() * std::max(nDotL,0.0f);
        if (nDotL>0.0f) {
            specular = glm::vec3(hitrec.getMaterial().getSpecular()) * light[i].getSpecular() * std::pow(rDotV,hitrec.getMaterial().getShininess());
        }
        else {
            specular = glm::vec3(0,0,0);
        }
        fColor = fColor + glm::vec4(ambient+diffuse+specular,1.0);
    }
    return glm::vec3(fColor);
}


void View::display(sgraph::IScenegraph *scenegraph) {
    
    program.enable();
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT_FACE);
    
    modelview.push(glm::mat4(1.0));
    modelview.top() = modelview.top() * lookat;
    //send projection matrix to GPU    
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    //get all the lights in view space
    sgraph::LightGatherer * gatherer = new sgraph::LightGatherer(modelview);
    scenegraph->getRoot()->accept(gatherer);
    vector<util::Light> lightsInViewSpace = gatherer->getLightsInViewSpace();

    //send them down to the gpu

    //pass light color properties to shader
    glUniform1i(shaderLocations.getLocation("numLights"),lightsInViewSpace.size());
    

    for (int i = 0; i < lightsInViewSpace.size(); i++) {
        stringstream name;

        name << "light[" << i << "]";
        glUniform3fv(shaderLocations.getLocation(name.str()+".ambient"), 1, glm::value_ptr(lightsInViewSpace[i].getAmbient()));
        glUniform3fv(shaderLocations.getLocation(name.str()+".diffuse"), 1, glm::value_ptr(lightsInViewSpace[i].getDiffuse()));
        glUniform3fv(shaderLocations.getLocation(name.str()+".specular"), 1,glm::value_ptr(lightsInViewSpace[i].getSpecular()));
        glUniform4fv(shaderLocations.getLocation(name.str()+".position"), 1,glm::value_ptr(lightsInViewSpace[i].getPosition()));

        glUniform4fv(shaderLocations.getLocation(name.str()+".spotdirection"), 1,glm::value_ptr(lightsInViewSpace[i].getSpotDirection()));
        glUniform1f(shaderLocations.getLocation(name.str()+".cosSpotCutoff"), glm::cos(glm::radians(lightsInViewSpace[i].getSpotCutoff())));
    }

    //enable texture mapping
    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    //tell the shader to look for GL_TEXTURE"0"
    glUniform1i(shaderLocations.getLocation("image"), 0);


    //draw scene graph here
    scenegraph->getRoot()->accept(renderer);

    
    
    modelview.pop();
    glFlush();
    program.disable();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double currenttime = glfwGetTime();
    if ((currenttime-time)>1.0) {
        printf("Framerate: %2.0f\r",frames/(currenttime-time));
        frames = 0;
        time = currenttime;
    }
    

}

bool View::shouldWindowClose() {
    return glfwWindowShouldClose(window);
}



void View::closeWindow() {
    for (map<string,util::ObjectInstance *>::iterator it=objects.begin();
           it!=objects.end();
           it++) {
          it->second->cleanup();
          delete it->second;
    }

    for (typename map<string,GLuint>::iterator it=textureIds.begin();
           it!=textureIds.end();
           it++) { 
        glDeleteTextures(1,&it->second);
    }
    glfwDestroyWindow(window);

    glfwTerminate();
}





