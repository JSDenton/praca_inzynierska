#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class Object3D
{
	std::vector<float> vertices;
	std::vector<float> faces;
	std::vector<float> vertices_normals;
	std::vector<float> vertices_textures;

public:
	Object3D(std::string filename);

	//float* get_vertices_to_render();	
	void get_vertices(std::string line);
};

