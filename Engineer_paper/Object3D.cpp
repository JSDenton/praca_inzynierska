#include "Object3D.h"
	
Mesh::Mesh(std::vector<Vertex> vertices_in, std::vector<unsigned int> indices_in, bool copy) {
    if (copy) {
        vertices = vertices_in;
        indices = indices_in;
    }
    else {
        this->vertices = vertices_in;
        this->indices = indices_in;
    }
    setup_mesh();
}

Mesh::Mesh() {
    ;
}

void Mesh::update_mesh(Vertex vertex, unsigned int index) {
    vertices.push_back(vertex);
    indices.push_back(index);
    indices.push_back(index+1);

    bind_buffers();
}

void Mesh::setup_mesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    bind_buffers();
}

void Mesh::bind_buffers() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_DYNAMIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glBindVertexArray(0);
}

void Mesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::draw_photons() {
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//Mesh::~Mesh() {
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteBuffers(1, &EBO);
//}

//----------------------------------------------------------------------------------------------------------------------------

void Object3D::draw() {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].draw();
}

void Object3D::load_model(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
       std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    process_node(scene->mRootNode, scene);
}

void Object3D::process_node(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Object3D::process_mesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = glm::vec3(0.f, 1.0f, 0.f);//vector;

        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    return Mesh(vertices, indices, false);

}

std::string Vertex::print() {
    return std::to_string(Position[0]) + ' ' + std::to_string(Position[1]) + ' ' + std::to_string(Position[2]);
}

std::string Mesh::print() {
    std::string final_string = "";
    for (int i = 0; i < indices.size(); i++) {
        final_string += "\t" + std::to_string(indices[i]) + ": " + vertices[i].print() + "\n";
    }
    return final_string;
}

void Object3D::print() {
    std::string final_string = "";
    for (int i = 0; i < meshes.size(); i++) {
        final_string += std::to_string(i) + ": " + meshes[i].print() + "\n";
    }
    std::cout << final_string;
}

std::vector<Mesh> Object3D::get_meshes() {
    return meshes;
}