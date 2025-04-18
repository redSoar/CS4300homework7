#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include "IScenegraph.h"
#include "SGNode.h"
#include "glm/glm.hpp"
#include "IVertexData.h"
#include "PolygonMesh.h"
#include "../Ray3D.h"
#include "../HitRecord.h"
#include "RayCastVisitor.h"
#include <string>
#include <map>
using namespace std;

namespace sgraph {

  /**
 * A specific implementation of this scene graph. This implementation is still independent
 * of the rendering technology (i.e. OpenGL)
 * \author Amit Shesh
 */

  class Scenegraph: public IScenegraph {
    /**
     * The root of the scene graph tree
     */
  protected:
    SGNode *root;
    map<string,util::PolygonMesh<VertexAttrib> > meshes;
    map<string,string> meshPaths;
    map<string,util::TextureImage*> textures;


    /**
     * A map to store the (name,node) pairs. A map is chosen for efficient search
     */
    map<string,SGNode *> nodes;
    

  public:
    Scenegraph() {
      root = NULL;
    }

    ~Scenegraph() {
      dispose();
    }

    void dispose() {

      if (root!=NULL) {
          delete root;
          root = NULL;
      }
      for (typename map<string,util::TextureImage*>::iterator it=textures.begin();
           it!=textures.end();
           it++) {
            delete it->second;
      }
    }

    

    /**
     * Set the root of the scenegraph, and then pass a reference to this scene graph object
     * to all its node. This will enable any node to call functions of its associated scene graph
     * \param root
     */

    void makeScenegraph(SGNode *root) {
      this->root = root;
      if (root!=NULL) {
        this->root->setScenegraph(this);
      }

    }

   

    void addNode(const string& name, SGNode *node) {
      nodes[name]=node;
    }


    SGNode *getRoot() {
      return root;
    }



    map<string, SGNode *> getNodes() {
      return nodes;
    }

    void setMeshes(map<string,util::PolygonMesh<VertexAttrib> >& meshes) {
      this->meshes = meshes;
    }

    map<string,util::PolygonMesh<VertexAttrib> > getMeshes() {
      return this->meshes;
    }

    void setMeshPaths(map<string,string>& meshPaths) {
      this->meshPaths = meshPaths;
    }

    map<string,string> getMeshPaths() {
      return this->meshPaths;
    }

    void setTextures(map<string,util::TextureImage*>& textures) {
      this->textures = textures;
    }

    map<string,util::TextureImage*> getTextures() {
      return textures;
    }

    HitRecord raycast(Ray3D ray, glm::mat4 mv){
      stack<glm::mat4> modelviews;
      modelviews.push(mv);
      RayCastVisitor *rcv = new sgraph::RayCastVisitor(modelviews, ray, getTextures());
      root->accept(rcv);
      return rcv->getHit();
    }
  };
}
#endif


/*

Scenegraph
-> raycast(3dray, mv, *hit);
  raycast{
    RayCastVisitor rcv;
    root.accept(rcv);
    return rcv.getHit();
  }

RayTraceVisitor
  field:
    bool hit;
  functions:
    visitGroupNOde
    visitLeafNode
      check instance
      do the respective math calculation
      set hit to true if it hit
      do nothing if not
    getHit(){
      return hit
    }




*/