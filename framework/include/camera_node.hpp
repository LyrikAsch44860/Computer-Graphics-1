#pragma once
#ifndef CAMERA_NODE_HPP
#define CAMERA_NODE_HPP
#include "node.hpp"

// class for camera node is not in use other than being an element in scene graph
class cameraNode : public node
{
	public:
		cameraNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 color, bool isPerspective, bool isEnabled, glm::mat4 projectionMatrix);
		bool getPerspective();
		bool getEnabled();
		void setEnabled(bool);
		glm::mat4 getProjectionMatrix();
		void setProjectionMatrix(glm::mat4);

	private:
		bool isPerspective;
		bool isEnabled;
		glm::mat4 projectionMatrix;
};

#endif