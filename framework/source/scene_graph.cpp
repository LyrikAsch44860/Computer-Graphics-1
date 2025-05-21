#include "scene_graph.hpp"

		sceneGraph::sceneGraph(std::string name, node* root) : name { name }, root{ root } {
		}

		// just some getters and setters

		sceneGraph::~sceneGraph() {}

		node* sceneGraph::getRoot() {
			return root;
		}
		void sceneGraph::setRoot(node* newRoot) {
			root = newRoot;
			return;
		}

		std::string sceneGraph::getName() {
			return name;
		}

		void sceneGraph::setName(std::string newName) {
			name = newName;
		}
