#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Object3D.h"
#include <omp.h>
#include <cstdlib>
#include <ctime>

#define CELLS_COUNT 500

const unsigned int scatter_counter_const = 20, border_margin_const = 5;

struct Photon {
	glm::vec3 location, direction;
	std::vector<glm::vec3> locations_historical;
	bool inside_model, absorbed, out_of_space, reflected;
	unsigned int border_passed = 1, scatter_counter = scatter_counter_const, border_margin = border_margin_const;
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

struct Material { // (0, n1) - reflection; (n1-n2) - absorption; (n2, 1) - transmittance
	float n1;
	float n2;

	Material();
};

class Simulation
{
	const unsigned int PHOTONS_COUNT = 1;
	//const unsigned int CELLS_COUNT = 1000; //the number of cells IN EVERY DIRECTION (x, y, z)
	const float CELL_SIZE_COEF = 1.6f; //1.2 means that we want space covered by cells by 20% above what model takes 

	std::vector<Photon> photons;
	Cell*** cells;
	glm::vec3 max_dimentions;
	glm::vec3 min_dimentions;
	float cell_size;
	Object3D model;
	
	Material material;
	int absorbed_photons;
	int out_of_space_photons;
	bool display_cells = false;

public:
	Simulation(Object3D model_in);
	Simulation();
	void simulate();
	bool model_boundary(Photon p);
	void draw();	
	~Simulation();
};