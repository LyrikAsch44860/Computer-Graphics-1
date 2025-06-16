#include "geometry_node.hpp"

		geometryNode::geometryNode(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::mat4 localTransform, glm::mat4 worldTransform, glm::vec3 color, model geometry) : geometry{ }, node { parent, children, name, path, depth, localTransform, worldTransform, color } {
		}

		//just some getters and setters
		model* geometryNode::getGeometry() {
			return geometry;
		}
		void geometryNode::setGeometry(model* newGeometry) {
			geometry = newGeometry;
			return;
		}