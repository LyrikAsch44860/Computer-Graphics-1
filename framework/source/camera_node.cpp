#include "camera_node.hpp"

	cameraNode::cameraNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 color, bool isPerspective, bool isEnabled, glm::mat4 projectionMatrix) : node { parent, children, name, path, depth, localTransform, worldTransform, color }, isPerspective{ isPerspective }, isEnabled{ isEnabled }, projectionMatrix{ projectionMatrix } {
	}
	
	// Just some getters and setters
	bool cameraNode::getPerspective() {
		return isPerspective;
	}

	bool cameraNode::getEnabled() {
		return isEnabled;
	}

	void cameraNode::setEnabled(bool newEnabled) {
		isEnabled = newEnabled;
	}

	glm::mat4 cameraNode::getProjectionMatrix() {
		return projectionMatrix;
	}

	void cameraNode::setProjectionMatrix(glm::mat4 newMatrix) {
		projectionMatrix = newMatrix;
	}
