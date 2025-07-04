#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"


// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // react to key input
  void keyCallback(int key, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  //handle resizing
  void resizeCallback(unsigned width, unsigned height);

  // draw all objects
  // added a node and an angle to draw all nodes with their parents transformation and to make them rotate around them at equal speed
  void render(node* currentNode, float angle) const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  // update uniform values
  void uploadUniforms();
  // upload projection matrix
  void uploadProjection();
  // upload view matrix
  void uploadView();

  // cpu representation of model
  model_object planet_object;
  model_object star;
  model_object orbit;
  model_object skyBox_top;
  model_object skyBox_bottom;
  model_object skyBox_back;
  model_object skyBox_front;
  model_object skyBox_left;
  model_object skyBox_right;
  std::vector<model_object*> boxObjects;
  std::vector<GLuint> boxTexId;
  std::map < std::string, std::pair< int, GLenum > > texname_texindex_map;
  std::vector < GLuint > texids;
  std::vector<std::string> texpaths;
  std::vector<std::string> texnames;

  // camera transform matrix
  glm::fmat4 m_view_transform;
  // camera projection matrix
  glm::fmat4 m_view_projection;
  glm::fmat4 lastView;

};

#endif