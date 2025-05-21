#include "point_light_node.hpp"

	pointLightNode::pointLightNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 lightColor, float lightIntensity) : lightColor{ lightColor }, lightIntensity{ lightIntensity }, node{ parent, children, name, path, depth, localTransform, worldTransform } {
	}

	glm::vec3 pointLightNode::getLightColor() {
		return lightColor;
	}

	void pointLightNode::setLightColor(glm::vec3 newColor) {
		lightColor = newColor;
		return;
	}

	float pointLightNode::getLightIntensity() {
		return lightIntensity;
	}

	void pointLightNode::setLightIntensity(float newIntensity) {
		lightIntensity = newIntensity;
		return;
	}
