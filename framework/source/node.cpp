#include "node.hpp"
		
	node::node(node* parent, std::vector<node*> children, std::string name, std::string path, int depth, glm::fmat4 localTransform, glm::fmat4 worldTransform) :
		parent{ parent }, children{ children }, name{ name }, path{ name }, depth{depth}, localTransform{localTransform}, worldTransform{worldTransform} {
	}
	node::~node() {}

	//mostly just getters and setters

	node* node::getParent() {
		return parent;
	}


	// sets parent and updates depth and path
	// calls itself for all children to update path and depth
	void node::setparent(node* parentNode) {
		parent = parentNode;
		if (parent != nullptr) {
			depth = parentNode->depth + 1;
			path = parent->getPath() + "_" + name;
			for (int i = 0; i < children.size(); ++i)
			{
				children[i]->setparent(this);
			}
		}
	}

	//gets the node with the name childName by iterating over all children
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

	int node::getDepth() {
		return depth;
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
		return;
	}

	//adds a new node pointer at the end of children vector
	void node::addChildren(node* newChild) {
		children.push_back(newChild);
		newChild->setparent(this);
		return;
	}

	//searches for the node with the name childName and removes its pointer from the child vector
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
