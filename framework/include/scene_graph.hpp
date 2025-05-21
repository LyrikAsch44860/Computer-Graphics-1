#pragma once
#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP
#include "node.hpp"

// class to represent the scene graph but its not used to render anything right now

class sceneGraph
{
	public:
		sceneGraph(std::string name, node* root);
		~sceneGraph();
		node* getRoot();
		void setRoot(node*);
		std::string getName();
		void setName(std::string);

	private:
		node* root;
		std::string name;
};


#endif