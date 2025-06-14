#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include <glm/gtx/string_cast.hpp>
#include "cmath"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 10.0f})}
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
  , lastView{}
{
  initializeGeometry();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render(node* currentNode, float angle) const {
  std::vector<node*> childList = currentNode->getChildrenList();
  // bind shader to upload uniforms
  
  // get world transform from parent and add own local transform to it
  // unnamed matrix is used to rotatet the children around the parent based on the time
  glm::fmat4 ModelMatrix = currentNode->getParent()->getWorldTransform() * glm::fmat4{ {cos(angle), 0, -1 * sin(angle),0}, {0,1,0,0}, { sin(angle), 0, cos(angle),0}, {0,0,0,1} } * currentNode->getLocalTransform();
 
  // sends information to orbit shaders and draws the orbits 
  glUseProgram(m_shaders.at("orbit").handle);
  glBindVertexArray(orbit.vertex_AO);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
    1, GL_FALSE, glm::value_ptr(glm::mat4{ glm::inverse(m_view_transform)}));
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ParentMatrix"),
    1, GL_FALSE, glm::value_ptr(currentNode->getParent()->getWorldTransform()));
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
    1, GL_FALSE, glm::value_ptr(currentNode->getLocalTransform()));
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));
  glDrawArrays(orbit.draw_mode, 0, orbit.num_elements);

  glUseProgram(m_shaders.at("planet").handle);

  // update own world transform to give it to children
  currentNode->setWorldTransform(ModelMatrix);

  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
    1, GL_FALSE, glm::value_ptr(ModelMatrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * ModelMatrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
    1, GL_FALSE, glm::value_ptr(normal_matrix));

  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

  if (currentNode->getName() == "sun") {
    // Debugging std::cout << "here";
    // printing stars in the if statement so they get only printed once
    // send information to orbit shaders and prints the stars
    glUseProgram(m_shaders.at("star").handle);
    glBindVertexArray(star.vertex_AO);
    glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
      1, GL_FALSE, glm::value_ptr(glm::mat4{ glm::inverse(m_view_transform) }));
    glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
      1, GL_FALSE, glm::value_ptr(m_view_projection));
    glDrawArrays(star.draw_mode, 0, 50);
  }

  //call the shader function for all children
  for (int i = 0; i < childList.size(); ++i) {
    ApplicationSolar::render(childList[i], angle);
  }
  return;
}

void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  //glUseProgram(m_shaders.at("star").handle);
  
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  //glUseProgram(m_shaders.at("star").handle);
  
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  glUseProgram(m_shaders.at("planet").handle);
  // upload uniform values to new locations
  uploadView();
  //glUseProgram(m_shaders.at("planet").handle);
  uploadProjection();

  // update the view for the new shaders (Does not really work how i wanted it)
  glUseProgram(m_shaders.at("star").handle);
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("planet").handle);

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("planet").handle);
}

///////////////////////////// intialisation functions /////////////////////////
// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  
  // added extra shaders for stars and orbits
  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});

  m_shaders.emplace("orbit", shader_program{ {{GL_VERTEX_SHADER,m_resource_path + "shaders/orbit.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/orbit.frag"}} });
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -2;
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -2;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -3;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -3;
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -3;
  m_shaders.at("orbit").u_locs["ParentMatrix"] = -3;
}

// load models

void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
  
  // added one model for stars and one for orbits
  star = { 1, 1, 1, GL_POINTS, 50 };
  // orbits have one vertex every five degrees
  orbit = { 2, 2, 2, GL_POINTS, 360 / 5 };
  std::vector<float> stars = {};
  // Arrays for DrawArray function because i couldnt get DrawElements to work the way i wanted it to
  float star_points[50*6];
  float orbit_angles[360 / 5];
  for (int i = 0; i < star.num_elements; ++i) {
    // generate random positions between 16 and 100 in positive and negative
    stars.push_back(16 + std::rand() % 84 * (std::rand() % 3 - 1));
    stars.push_back(16 + std::rand() % 84 * (std::rand() % 3 - 1));
    // always -1 to have them infront of the starting view
    stars.push_back(16 + std::rand() % 84 * -1);
    stars.push_back((std::rand() % 255) / 255.0f);
    stars.push_back((std::rand() % 255) / 255.0f);
    stars.push_back((std::rand() % 255) / 255.0f);
  }
  // copy elements into array
  std::copy(stars.begin(), stars.end(), star_points);

  float angle = 0;
  // generate angles with five degrees difference each
  for (int i = 0; i < 360 / 5; ++i) {
    angle = angle + 5.0f;
    orbit_angles[i] = angle;
    // Debugging std::cout << "angle: " << angle << "\n";
  }

  // Classic open Gl object creating
  glGenVertexArrays(1, &star.vertex_AO);
  glGenBuffers(1, &star.vertex_BO);
  glBindVertexArray(star.vertex_AO);
  glBindBuffer(GL_ARRAY_BUFFER, star.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(star_points), &star_points, GL_STATIC_DRAW);
  // first three elements of each Vertex are the position coordinates
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // second three are the rgb color values
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
  glEnableVertexAttribArray(1);

  // same for orbits
  glGenVertexArrays(1, &orbit.vertex_AO);
  glGenBuffers(1, &orbit.vertex_BO);
  glBindVertexArray(orbit.vertex_AO);
  glBindBuffer(GL_ARRAY_BUFFER, orbit.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(orbit_angles), &orbit_angles, GL_STATIC_DRAW);
  // only one data per vertex which is the angle
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);
  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);
  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

///////////////////////////// callback functions for window events ////////////
// handle key input
// added movment for key A, D, Space and Ctrl for rigth, left, up and down
void ApplicationSolar::keyCallback(int key, int action, int mods) {
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    uploadView();
  }

  else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{ -0.1f, 0.0f, 0.0f });
    uploadView();
  }

  else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 0.1f, 0.0f, 0.0f });
    uploadView();
  }

  else if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 0.0f, 0.1f, 0.0f });
    uploadView();
  }

  else if (key == GLFW_KEY_LEFT_CONTROL && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{ 0.0f, -0.1f, 0.0f });
    uploadView();
  }

  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    uploadView();
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
  // transform the view based on the mouse movement at the y achis
  // when used first rotates your camera a lot due to the potentially large x and y positions
  // bad handeling when using both y and x achsis

    //m_view_projection = glm::rotate(m_view_projection, float(0.004 * pos_x ), glm::fvec3{ 0.0f, 1.0f, 0.0f });
    m_view_projection = glm::rotate(m_view_projection, float(0.004 * pos_y), glm::fvec3{ 1.0f, 0.0f, 0.0f });
    uploadProjection();
  
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
  // upload new projection matrix
  uploadProjection();
}


// exe entry point
int main(int argc, char* argv[]) {

  Application::run<ApplicationSolar>(argc, argv, 3, 2);
}

