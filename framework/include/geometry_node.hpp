#pragma once
#ifndef GEOMETRY_NODE_HPP
#define GEOMETRY_NODE_HPP
#include "node.hpp"
#include "model.hpp"

// class used to draw planets even though model is not used
class geometryNode : public node
{
	public:
		geometryNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 color, model geometry);
		model* getGeometry();
		void setGeometry(model*);

	private:
		model* geometry;
};


#endif