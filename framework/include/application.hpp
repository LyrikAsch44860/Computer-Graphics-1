#ifndef APPLICATION_HPP
#define APPLICATION_HPP

//#include <node.hpp>
#include "point_light_node.hpp"
#include "geometry_node.hpp"
#include "scene_graph.hpp"
#include "camera_node.hpp"
#include "structs.hpp"
#include <glm/gtc/type_precision.hpp>
#include <map>



struct GLFWwindow;
// gpu representation of model
class Application {
 public:
  template<typename T>
  static void run(int argc, char* argv[], unsigned ver_major, unsigned ver_minor);

  // allocate and initialize objects
  Application(std::string const& resource_path);
  // free shader resources
  virtual ~Application();

  // update viewport and field of view
  void resize_callback(unsigned width, unsigned height);
  // handle key input
  void key_callback(GLFWwindow* window, int key, int action, int mods);
  //handle mouse movement input
  void mouse_callback(GLFWwindow* window, double pos_x, double pos_y);
  // recompile shaders form source files
  void reloadShaders(bool throwing);

// functiosn which are implemented in derived classes
  // update uniform locations and values
  inline virtual void uploadUniforms() {};
  // react to key input
  inline virtual void keyCallback(int key, int action, int mods) {};
  //handle delta mouse movement input
  inline virtual void mouseCallback(double pos_x, double pos_y) {};
  // update framebuffer textures
  inline virtual void resizeCallback(unsigned width, unsigned height) {};
  // draw all objects
  virtual void render(node* currentNode, float angle) const = 0;

 protected:
  void updateUniformLocations();

  std::string m_resource_path; 

  // container for the shader programs
  std::map<std::string, shader_program> m_shaders{};

  // resolution when 
  static const glm::uvec2 initial_resolution; 
  static const float initial_aspect_ratio; 
};


#include "utils.hpp"
#include "window_handler.hpp"

template<typename T>
void Application::run(int argc, char* argv[], unsigned ver_major, unsigned ver_minor) {  

    GLFWwindow* window = window_handler::initialize(initial_resolution, ver_major, ver_minor);
    
    std::string resource_path = utils::read_resource_path(argc, argv);
    T* application = new T{resource_path};

    window_handler::set_callback_object(window, application);

    // do intial shader load an uniform upload
    application->reloadShaders(true);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::vector<node*> root_children = {};

    // builds the scene graph with one root node, one camera node, one pointLightNode and 9 geometry nodes
    // geometry nodes have a translation matrix multiplied by a rotation matrix as their local Transform
    // rotation is only an offset 

    node root = { nullptr, root_children, "root", "no_clue", 0, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}} };

    sceneGraph graph = { "Scene Graph" , &root };

    cameraNode camera = { nullptr, root_children, "camera", "no_clue", 0, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, true, true, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}} };
    root.addChildren(&camera);

    pointLightNode sun_1 = { nullptr, root_children, "sun", "no_clue", 0, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, glm::vec3{1, 1, 0.8}, 1 };
    root.addChildren(&sun_1);

    geometryNode planet_1 = { nullptr, root_children, "mercury", "no_clue", 0, glm::fmat4{{cos(60), 0, -1 * sin(60),0}, {0,1,0,0}, { sin(60), 0, cos(60),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-2,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_1);

    geometryNode planet_2 = { nullptr, root_children, "venus", "no_clue", 0, glm::fmat4{{cos(30), 0, -1 * sin(30),0}, {0,1,0,0}, { sin(30), 0, cos(30),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-4,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_2);

    geometryNode planet_3 = { nullptr, root_children, "super-earth", "no_clue", 0, glm::fmat4{{cos(170), 0, -1 * sin(170),0}, {0,1,0,0}, {sin(170),0, cos(170),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-6,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };

    geometryNode planet_4 = { nullptr, root_children, "mars", "no_clue", 0, glm::fmat4{{cos(50), 0, -1 * sin(50),0}, {0,1,0,0}, { sin(50), 0, cos(50),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-8,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_4);

    geometryNode planet_5 = { nullptr, root_children, "jupiter", "no_clue", 0, glm::fmat4{{cos(30), 0, -1 * sin(30),0}, {0,1,0,0}, { sin(30), 0, cos(30),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-10,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_5);

    geometryNode planet_6 = { nullptr, root_children, "saturn", "no_clue", 0, glm::fmat4{{cos(80), 0, -1 * sin(80),0}, {0,1,0,0}, { sin(80), 0, cos(80),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-12,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_6);

    geometryNode planet_7 = { nullptr, root_children, "uranus", "no_clue", 0, glm::fmat4{{cos(40), 0, -1 * sin(40),0}, {0,1,0,0}, { sin(40), 0, cos(40),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-14,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}},model{} };
    sun_1.addChildren(&planet_7);

    geometryNode planet_8 = { nullptr, root_children, "neptun", "no_clue", 0, glm::fmat4{{cos(20), 0, -1 * sin(20),0}, {0,1,0,0}, { sin(20), 0, cos(20),0}, {0,0,0,1}} * glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-16,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    sun_1.addChildren(&planet_8);

    geometryNode moon_1 = { nullptr, root_children, "moon", "no_clue", 0, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1,1}}, glm::fmat4{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}, model{} };
    planet_3.addChildren(&moon_1);
    sun_1.addChildren(&planet_3);


    // rendering loop
    while (!glfwWindowShouldClose(window)) {
      // query input
      glfwPollEvents();
      // clear buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // draw geometry
      // angle is used for rotation, same angle for all so they rotate at the same speed
      double angle = glfwGetTime();
      //render get a node to start and an angle for rotation
      // I started at sun because it is the center for the actual scene
      application->render(&sun_1, angle);
      // swap draw buffer to front
      glfwSwapBuffers(window);
      // display fps
      window_handler::show_fps(window);
    }

    delete application;
    window_handler::close_and_quit(window, EXIT_SUCCESS);
}


#endif