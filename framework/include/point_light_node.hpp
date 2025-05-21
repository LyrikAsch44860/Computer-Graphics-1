#pragma once
#ifndef POINT_LIGHT_NODE_HPP
#define POINT_LIGHT_NODE_HPP
#include "node.hpp"

class pointLightNode : public node
{
public:
	pointLightNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 LightColor, float LightIntensity);
	float getLightIntensity();
	void setLightIntensity(float);
	glm::vec3 getLightColor();
	void setLightColor(glm::vec3);

private:
	glm::vec3 lightColor;
	float lightIntensity;
};


#endif