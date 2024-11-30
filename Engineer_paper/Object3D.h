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
};

//---------MESH
class Mesh {
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	//~Mesh();

	void draw();
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
    void draw(Shader& shader);
	void draw();

private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;

protected:
	void load_model(std::string path);
	void process_node(aiNode* node, const aiScene* scene);
	Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
};

