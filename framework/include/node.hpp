#pragma once
#ifndef NODE_HPP
#define NODE_HPP
#include <glm/glm.hpp>
#include <vector>
#include <string>

class node
{
public:
	node(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::fmat4 localTransform, glm::fmat4 worldTransform);
	~node();
	node* getParent();
	void setparent(node*);
	node* getChildren(std::string);
	std::vector<node*> getChildrenList();
	std::string getName();
	std::string getPath();
	int getDepth(int* i, int* j);
	glm::fmat4 getLocalTransform();
	void setLocalTransform(glm::fmat4);
	glm::fmat4 getWorldTransform();
	void setWorldTransform(glm::fmat4);
	void addChildren(node*);
	node* removeChildren(std::string);

protected:
	node* parent;
	std::vector<node*> children;
	std::string name;
	std::string path;
	int depth;
	glm::fmat4 localTransform;
	glm::fmat4 worldTransform;
};
#endif