#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Object3D.h"
#include <omp.h>
#include <cstdlib>
#include <ctime>

#define CELLS_COUNT 300

struct Photon {
	glm::vec3 location, direction;
	std::vector<glm::vec3> locations_historical;
	bool inside_model, inside_model_prev, absorbed, out_of_space;
	Mesh mesh;

	Photon(glm::vec3);
	void save();
};

struct Cell {
	glm::vec3 location;
	int energy;
	bool border; //if true, this means that cell is on the border of a model
	glm::vec3 normal;
	Cell(glm::vec3 loc);
	Cell();
};

struct Material {
	float absorption;
	float reflection;
	float transmittance;
	Material();
};

class Simulation
{
	const unsigned int PHOTONS_COUNT = 100;
	//const unsigned int CELLS_COUNT = 1000; //the number of cells IN EVERY DIRECTION (x, y, z)
	const float CELL_SIZE_COEF = 1.2f; //1.2 means that we want space covered by cells by 20% above what model takes 

	std::vector<Photon> photons;
	Cell*** cells;
	glm::vec3 max_dimentions;
	glm::vec3 min_dimentions;
	float cell_size;
	Object3D model;
	Material material;
	int absorbed_photons;
	int out_of_space_photons;

public:
	Simulation(Object3D model_in);
	Simulation();
	void simulate();
	bool model_boundary(Photon p);
	void draw();
	~Simulation();
};