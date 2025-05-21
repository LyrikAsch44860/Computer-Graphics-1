#include "node.hpp"



		node::node(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::fmat4 localTransform, glm::fmat4 worldTransform) :
			parent{ parent }, children{ children }, name{ name }, path{ path }, depth{ depth }, localTransform{ localTransform }, worldTransform{ worldTransform } {
		}
		node::~node() {}

		node* node::getParent() {
			return parent;
		}

		void node::setparent(node* parentNode) {
			parent = parentNode;
		}

		node* node::getChildren(std::string childName) {
			for (int i = 0; i < children.size(); ++i) {
				if (childName == children[i]->getName()) {
					return children[i];
				}
			}
			return nullptr;
		}

		std::vector<node*> node::getChildrenList() {
			return children;
		}

		std::string node::getName() {
			return name;
		}

		std::string node::getPath() {
			return path;
		}

		int node::getDepth(int* maxDepth, int* currentDepth) {
			if (*maxDepth < *currentDepth) {
				*maxDepth = *currentDepth;
			}
			for (int i = 0; i <= children.size(); ++i) {
				*currentDepth = *currentDepth + 1;
				children[i]->getDepth(maxDepth, currentDepth);
				*currentDepth = *currentDepth - 1;
			}
			if (*currentDepth = 0) {
				return *maxDepth;
			}
			return 0;
		}

		glm::fmat4 node::getLocalTransform() {
			return localTransform;
		}

		void node::setLocalTransform(glm::fmat4 newTransform) {
			localTransform = newTransform;
			return;
		}

		glm::mat4 node::getWorldTransform() {
			return worldTransform;
		}

		void node::setWorldTransform(glm::fmat4 newTransform) {
			worldTransform = newTransform;
		}

		void node::addChildren(node* newChild) {
			children.push_back(newChild);
			newChild->setparent(this);
			return;
		}

		node* node::removeChildren(std::string childName) {
			for (int i = 0; i < children.size(); ++i) {
				if (childName == children[i]->getName()) {
					node* returnNode = children[i];
					children.erase(children.begin() + i);
					return returnNode;
				}
			}
			return nullptr;
		}
