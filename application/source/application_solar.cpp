#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
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
 ,texname_texindex_map{}
  , texids{}
  , texpaths{}
  , texnames{}
  ,skyBox_top{}
  ,skyBox_bottom{}
  ,skyBox_back{}
  ,skyBox_front{}
  ,skyBox_left{}
  ,skyBox_right{}
  ,boxTexId{}
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
  
  // get world transform from parent and add own local transform to it
  // unnamed matrix is used to rotatet the children around the parent based on the time
  glm::fmat4 ModelMatrix = currentNode->getParent()->getWorldTransform() * glm::fmat4{ {cos(angle), 0, -1 * sin(angle),0}, {0,1,0,0}, { sin(angle), 0, cos(angle),0}, {0,0,0,1} } * currentNode->getLocalTransform();
 
  // sends information to orbit shaders and draws the orbits 
  glUseProgram(m_shaders.at("orbit").handle);
  glBindVertexArray(orbit.vertex_AO);
  
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ParentMatrix"),
    1, GL_FALSE, glm::value_ptr(currentNode->getParent()->getWorldTransform()));
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
    1, GL_FALSE, glm::value_ptr(currentNode->getLocalTransform()));
  
  glDrawArrays(orbit.draw_mode, 0, orbit.num_elements);

  if (currentNode->getName() == "sun") {
    // upload color of the light to planet shaders
    //glUseProgram(m_shaders.at("planet").handle);
    //glUniform3f(m_shaders.at("planet").u_locs.at("LightColor"), currentNode->getColor()[0], currentNode->getColor()[1], currentNode->getColor()[2]);

    glUseProgram(m_shaders.at("square").handle);
    glActiveTexture(texname_texindex_map.at(currentNode->getName()).second);
    glBindTexture(GL_TEXTURE_2D, texids[texname_texindex_map.at(currentNode->getName()).first]);
    glUniform1i(m_shaders.at("square").u_locs.at("Texture"), texname_texindex_map.at(currentNode->getName()).first);
    glUniformMatrix4fv(m_shaders.at("square").u_locs.at("ModelMatrix"),
      1, GL_FALSE, glm::value_ptr(ModelMatrix));

  }

  else {
    // upload planet color to planet shader

    glUseProgram(m_shaders.at("planet").handle);
    glActiveTexture(texname_texindex_map.at(currentNode->getName()).second);
    glBindTexture(GL_TEXTURE_2D, texids[texname_texindex_map.at(currentNode->getName()).first]);
    glUniform1i(m_shaders.at("planet").u_locs.at("Texture"), texname_texindex_map.at(currentNode->getName()).first);
    glUniform3f(m_shaders.at("planet").u_locs.at("PlanetColor"), currentNode->getColor()[0], currentNode->getColor()[1], currentNode->getColor()[2]);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
      1, GL_FALSE, glm::value_ptr(ModelMatrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * ModelMatrix);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
      1, GL_FALSE, glm::value_ptr(normal_matrix));
  }
  // update own world transform to give it to children
  currentNode->setWorldTransform(ModelMatrix);

  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

  if (currentNode->getName() == "uranus") {
    // Debugging std::cout << "here";
    // printing stars in the if statement so they get only printed once
    // send information to star shaders and prints the stars
    glUseProgram(m_shaders.at("star").handle);
    glBindVertexArray(star.vertex_AO);
    
    glDrawArrays(star.draw_mode, 0, star.num_elements);
    
    // render skyshpere only once per iteration
    ModelMatrix = glm::fmat4{{50,0,0,0},{0,50,0,0},{0,0,50,0},{0,0,0,1}};
    glUseProgram(m_shaders.at("square").handle);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, boxTexId[0]);
    glUniform1i(m_shaders.at("square").u_locs.at("Texture"), 10);
    glBindVertexArray(planet_object.vertex_AO);
    // no longer used glBindVertexArray(skyBox_bottom.vertex_AO);
    glUniformMatrix4fv(m_shaders.at("square").u_locs.at("ModelMatrix"),
      1, GL_FALSE, glm::value_ptr(ModelMatrix));
    // no longer used glDrawArrays(skyBox_bottom.draw_mode, 0, skyBox_bottom.num_elements);
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
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

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("square").handle);
  glUniformMatrix4fv(m_shaders.at("square").u_locs.at("ViewMatrix"),
    1, GL_FALSE, glm::value_ptr(view_matrix));
  
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("square").handle);
  glUniformMatrix4fv(m_shaders.at("square").u_locs.at("ProjectionMatrix"),
    1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("planet").handle);
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  glUseProgram(m_shaders.at("planet").handle);
  // upload uniform values to new locations
  uploadView();
  //glUseProgram(m_shaders.at("planet").handle);
  uploadProjection();
  
}

///////////////////////////// intialisation functions /////////////////////////
// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  
  // added extra shader for sun
  m_shaders.emplace("sun", shader_program{ {{GL_VERTEX_SHADER,m_resource_path + "shaders/sun.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/sun.frag"}} });

  // added extra shaders for stars and orbits
  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});

  m_shaders.emplace("orbit", shader_program{ {{GL_VERTEX_SHADER,m_resource_path + "shaders/orbit.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/orbit.frag"}} });

  // shader for skysphere (previously used for skybox)
  m_shaders.emplace("square", shader_program{ {{GL_VERTEX_SHADER,m_resource_path + "shaders/square.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/square.frag"}} });
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["PlanetColor"] = -1;
  m_shaders.at("planet").u_locs["LightColor"] = -1;
  m_shaders.at("planet").u_locs["Texture"] = -1;
  m_shaders.at("planet").u_locs["outline"] = -1;
  m_shaders.at("sun").u_locs["NormalMatrix"] = -1;
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ParentMatrix"] = -1;
  m_shaders.at("square").u_locs["ModelMatrix"] = -1;
  m_shaders.at("square").u_locs["ViewMatrix"] = -1;
  m_shaders.at("square").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("square").u_locs["Texture"] = -1;
  
}

// load models

void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL
    | model::TEXCOORD);

  // added one model for stars and one for orbits
  star = { 1, 1, 1, GL_POINTS, 200 };

  // no longer used model for skybox (using skysphere instead)
  skyBox_top = { 6,6,6,GL_TRIANGLES, 6 };
  skyBox_bottom = { 6,6,6,GL_TRIANGLES, 6 * 6 };
  skyBox_front = { 6,6,6,GL_TRIANGLES, 6 };
  skyBox_back = { 6,6,6,GL_TRIANGLES, 6 };
  skyBox_left = { 6,6,6,GL_TRIANGLES, 6 };
  skyBox_right = { 6,6,6,GL_TRIANGLES, 6 };
  // orbits have one vertex every five degrees
  orbit = { 2, 2, 2, GL_LINE_STRIP, 360 / 5 + 1 };


  std::vector<float> stars = {};
  // Arrays for DrawArray function because i couldnt get DrawElements to work the way i wanted it to
  float star_points[200 * 6];
  float orbit_angles[360 / 5 + 1];
  for (int i = 0; i < star.num_elements; ++i) {
    // generate random positions between 16 and 100 in positive and negative
    stars.push_back(16 + std::rand() % (84 * 2) - 84);
    stars.push_back(16 + std::rand() % (84 * 2) - 84);
    stars.push_back(16 + std::rand() % (84 * 2) - 84);
    stars.push_back((std::rand() % 255) / 255.0f);
    stars.push_back((std::rand() % 255) / 255.0f);
    stars.push_back((std::rand() % 255) / 255.0f);
  }
  // copy elements into array
  std::copy(stars.begin(), stars.end(), star_points);

  float angle = 0;
  // generate angles with five degrees difference each
  for (int i = 0; i <= 360 / 5; ++i) {
    angle = angle + 5.0f;
    orbit_angles[i] = angle;
    // Debugging std::cout << "angle: " << angle << "\n";
  }

  // no longer used vertecies for box sides
  std::vector<float> box_bottom_points{ 0.0,0.0,0.0, 0.0,0.0 ,1.0,0.0,0.0, 1.0,0.0 ,0.0,0.0,1.0, 0.0,1.0 , 1.0,0.0,1.0, 1.0,1.0 ,1.0,0.0,0.0, 1.0,0.0 ,0.0,0.0,1.0, 0.0,1.0, 0.0,1.0,0.0, 0.0,0.0 ,1.0,1.0,0.0, 1.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 , 1.0,1.0,1.0, 1.0,1.0 ,1.0,1.0,0.0, 1.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 };
  std::vector<float> box_top_points{ 0.0,1.0,0.0, 0.0,0.0 ,1.0,1.0,0.0, 1.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 , 1.0,1.0,1.0, 1.0,1.0 ,1.0,1.0,0.0, 1.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 };
  std::vector<float> box_left_points{ 0.0,0.0,0.0, 0.0,0.0 ,0.0,1.0,0.0, 1.0,0.0 ,0.0,0.0,1.0, 0.0,1.0 , 0.0,1.0,1.0, 1.0,1.0 ,0.0,1.0,0.0, 1.0,0.0 ,0.0,0.0,1.0, 0.0,1.0 };
  std::vector<float> box_right_points{ 1.0,0.0,0.0, 0.0,0.0 ,1.0,1.0,0.0, 1.0,0.0 ,1.0,0.0,1.0, 0.0,1.0 , 1.0,1.0,1.0, 1.0,1.0 ,1.0,1.0,0.0, 1.0,0.0 ,1.0,0.0,1.0, 0.0,1.0 };
  std::vector<float> box_front_points{ 0.0,0.0,1.0, 0.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 ,1.0,0.0,1.0, 1.0,0.0 , 1.0,1.0,1.0, 1.0,1.0 ,1.0,0.0,1.0, 1.0,0.0 ,0.0,1.0,1.0, 0.0,1.0 };
  std::vector<float> box_back_points{ 0.0,0.0,0.0, 0.0,0.0 ,0.0,1.0,0.0, 0.0,1.0 ,1.0,0.0,0.0, 1.0,0.0 , 1.0,1.0,0.0, 1.0,1.0 ,1.0,0.0,0.0, 1.0,0.0 ,0.0,1.0,0.0, 0.0,1.0 };

  box_bottom_points.insert(box_bottom_points.end(), box_left_points.begin(), box_left_points.end());
  box_bottom_points.insert(box_bottom_points.end(), box_right_points.begin(), box_right_points.end());
  box_bottom_points.insert(box_bottom_points.end(), box_front_points.begin(), box_front_points.end());
  box_bottom_points.insert(box_bottom_points.end(), box_back_points.begin(), box_back_points.end());
  float box_bottom_points1[5 * 6 * 6];

  std::copy(box_bottom_points.begin(), box_bottom_points.end(), box_bottom_points1);

  // no longer used box object creation
  glGenVertexArrays(1, &skyBox_bottom.vertex_AO);
  glGenBuffers(1, &skyBox_bottom.vertex_BO);
  glBindVertexArray(skyBox_bottom.vertex_AO);
  glBindBuffer(GL_ARRAY_BUFFER, skyBox_bottom.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(box_bottom_points1), &box_bottom_points1, GL_STATIC_DRAW);
  // first three elements of each Vertex are the position coordinates
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12);
  glEnableVertexAttribArray(1);
  // second two are texture coordinates

  // skybox (skysphere) texture generation
  GLuint bottomID;

  glActiveTexture(GL_TEXTURE10);
  glGenTextures(1, &bottomID);
  glBindTexture(GL_TEXTURE_2D, bottomID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  boxTexId.push_back(bottomID);

  /* just some debugging
  std::cout << "#########################################\n";
  std::cout << "vBO: " << skyBox_bottom.vertex_BO << "\n";
  std::cout << "vAO: " << skyBox_bottom.vertex_AO << "\n";
  std::cout << "ID: " << boxTexId[0] << "\n";
  std::cout << "#########################################\n";
  */

  pixel_data* Texture1 = new pixel_data{ texture_loader::file(m_resource_path + "textures/skybox.png") };
  glTexImage2D(GL_TEXTURE_2D, 0, Texture1->channels, Texture1->width, Texture1->height, 0, Texture1->channels, Texture1->channel_type, &Texture1->pixels[0]);
  

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

  
  // setting important texture attributes in containers for later use
  texnames.push_back("sun");
  texnames.push_back("mercury");
  texnames.push_back("venus");
  texnames.push_back("super-earth");
  texnames.push_back("mars");
  texnames.push_back("jupiter");
  texnames.push_back("saturn");
  texnames.push_back("uranus");
  texnames.push_back("neptun");
  texnames.push_back("moon");

  texname_texindex_map["sun"] = {0,GL_TEXTURE0} ;
  texname_texindex_map["mercury"] = {1,GL_TEXTURE1};
  texname_texindex_map["venus"] = {2,GL_TEXTURE2};
  texname_texindex_map["super-earth"] = {3,GL_TEXTURE3};
  texname_texindex_map["mars"] = {4,GL_TEXTURE4};
  texname_texindex_map["jupiter"] = {5,GL_TEXTURE5};
  texname_texindex_map["saturn"] = {6,GL_TEXTURE6};
  texname_texindex_map["uranus"] = {7,GL_TEXTURE7};
  texname_texindex_map["neptun"] = {8,GL_TEXTURE8};
  texname_texindex_map["moon"] = { 9,GL_TEXTURE9 };

  texpaths.push_back("textures/sunmap1.png");
  texpaths.push_back("textures/mercurymap.png");
  texpaths.push_back("textures/venusmap.png" );
  texpaths.push_back("textures/earthmap1k.png");
  texpaths.push_back("textures/mars_1k_color.png");
  texpaths.push_back("textures/jupitermap.png");
  texpaths.push_back("textures/saturnmap.png");
  texpaths.push_back("textures/uranusmap.png");
  texpaths.push_back("textures/neptunemap.png");
  texpaths.push_back("textures/Moon.png");
  // debugging std::cout << "vectorsize" << texnames.size();
  
  // texture generation for all planets
  for (int i = 0; i < texnames.size(); ++i)
  {
    std::cout << "iteration: " << i << "\n";
    GLuint TextureID;
    std::cout << "ID: " << TextureID << "\n";
    texids.push_back(TextureID);
    std::cout << "tex id 0: " << texids[0] << "\n";
    std::cout << "tex id befor " << i << " : " << texids[i] << "\n";
    //texname_texi_texpath_map[texnames[i]] = { {i},TextureID };
    glActiveTexture(texname_texindex_map[texnames[i]].second);
    glGenTextures(1, &texids[i]);
    std::cout << "tex id after " << i << " : " << texids[i] << "\n";
    glBindTexture(GL_TEXTURE_2D, texids[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pixel_data* Texture = new pixel_data{ texture_loader::file(m_resource_path + texpaths[i])};

    /* just some debugging
    std::cout << "data lenght" << Texture->pixels.size() << "\n";
    std::cout << "data heigth" << Texture->height << "\n";
    std::cout << "data width" << Texture->width << "\n";
    std::cout << "path: " << texpaths[i] << "\n";
    std::cout << "id: " << texids[i] << "\n";
    std::cout << "text i: " << texname_texindex_map[texnames[i]].second << "\n";
    */

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture->width, Texture->height, 0, GL_RGBA, Texture->channel_type, &Texture->pixels[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texids[i]);
  }
 
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
  std::cout << "Position Offset: " << planet_model.offsets[model::POSITION] << "\n" << "Normal Offset: " << planet_model.offsets[model::NORMAL] << "\n" << "TEX Offset: " << planet_model.offsets[model::TEXCOORD] << "\n" << "Stride: " << planet_model.vertex_bytes << "\n" << "Tex size: " << model::TEXCOORD.components << "\n";
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

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

  // upload variable for planet outline to shader
  if (key == GLFW_KEY_1 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    glUniform1f(m_shaders.at("planet").u_locs.at("outline"), 1.0f);
  }
  
  if (key == GLFW_KEY_2 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    glUniform1f(m_shaders.at("planet").u_locs.at("outline"), 0.0f);
  }

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

