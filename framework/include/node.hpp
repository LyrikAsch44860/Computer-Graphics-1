#pragma once
#ifndef NODE_HPP
#define NODE_HPP
#include <glm/glm.hpp>
#include <vector>
#include <string>

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

//class for nodes in the scene graph

class node
{
public:
	node(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::fmat4 localTransform, glm::fmat4 worldTransform, glm::vec3 color);
	~node();
	node* getParent();
	void setparent(node*);
	node* getChildren(std::string);
	std::vector<node*> getChildrenList();
	std::string getName();
	std::string getPath();
	int getDepth();
	glm::fmat4 getLocalTransform();
	void setLocalTransform(glm::fmat4);
	glm::fmat4 getWorldTransform();
	void setWorldTransform(glm::fmat4);
	void addChildren(node*);
	node* removeChildren(std::string);
	glm::vec3 node::getColor();
	void node::setcolor(glm::vec3);

protected:
	node* parent;
	std::vector<node*> children;
	std::string name;
	std::string path;
	int depth;
	glm::fmat4 localTransform;
	glm::fmat4 worldTransform;
	glm::vec3 color;
};
#endif