#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"

//---------VERTEX
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	std::string print();
};

//---------MESH
class Mesh {
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, bool copy);
	Mesh();
	void update_mesh(Vertex vertex, unsigned int index);
	void bind_buffers();
	//~Mesh();

	void draw();
	void draw_photons();
	std::string print();
private:
	unsigned int VAO, VBO, EBO;
protected:
	void setup_mesh();
};


//--------OBJECT3D
class Object3D
{
public:
    Object3D(std::string path)
    {
        load_model(path);
    }
	Object3D()
	{
		load_model("resources/sphere.obj");
	}
    void draw(Shader& shader);
	void draw();
	void print();
	std::vector<Mesh> get_meshes();

private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;

protected:
	void load_model(std::string path);
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);	
};

