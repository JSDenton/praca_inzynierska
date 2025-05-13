#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Object3D.h"
#include <omp.h>
#include <cstdlib>
#include <ctime>

#define CELLS_COUNT 10 //the number of cells IN EVERY DIRECTION (x, y, z)

const unsigned int scatter_counter_const = 0, border_margin_const = 5;

struct Photon {
	glm::vec3 location, direction;
	bool inside_model, absorbed, out_of_space, reflected;
	unsigned int border_passed = 1, scatter_counter = scatter_counter_const, border_margin = border_margin_const;
	Mesh mesh;

	Photon();
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

struct Material { //  reflection; (0, n1) - transmittance; (n1-n2) - absorption; (n2, 1) - scattering
	float reflection_chance;
	float n1, n2;

	Material();
};

class Simulation
{
	const unsigned int PHOTONS_COUNT = 1;
	const float CELL_SIZE_COEF = 1.6f; //1.x means that we want space covered by cells by x*10% above what model takes 

	Photon* photons = new Photon[PHOTONS_COUNT];
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
	bool simulate();
	bool model_boundary(Photon p);
	void draw();	
	void detect_temperature(glm::vec3 axis, glm::vec3 axis_begin, std::vector<unsigned int>&);
	~Simulation();
};