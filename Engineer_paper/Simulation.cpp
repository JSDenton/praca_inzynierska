#include "Simulation.h"


Cell::Cell(glm::vec3 loc) {
	location = loc;
	border = false;
	energy = 0;
	normal = glm::vec3();
}

Cell::Cell() {
	Cell(glm::vec3(0, 0, 0));
}

Photon::Photon(glm::vec3 dimentions) {
	location = glm::vec3(dimentions[0]/2, 73.f, dimentions[2]/2); //set starting point in the center (x,z) and max up
	float rand1 = rand();
	float rand2 = rand();
	direction = glm::vec3(rand1 / RAND_MAX - 0.5f, -1.0f, rand2 / RAND_MAX - 0.5f);
	direction = glm::normalize(direction);
	inside_model = false;
	inside_model_prev = false;
	out_of_space = false;
	absorbed = false;

	Vertex vertex;
	vertex.Position = location;
	vertex.Normal = direction;
	std::vector<Vertex> vec;
	vec.push_back(vertex);
	std::vector<unsigned int>indices;
	indices.push_back(0);
	mesh = Mesh(vec, indices, true);
}

void Photon::save() {
	locations_historical.push_back(location);
	Vertex vertex;
	vertex.Position = location;
	vertex.Normal = direction;
	mesh.update_mesh(vertex, mesh.indices.size());
}

Material::Material() { //reflection < absorbtion < transmittance
	reflection = 0.1f;
	transmittance = 0.95f;
	absorption = 0.15f;
	/*float n1 = rand();
	n1 /= RAND_MAX;
	float n2 = rand();
	n2 /= RAND_MAX;
	float n3 = rand();
	n3 /= RAND_MAX;
	if (n1 < n2 && n1 < n3) {
		reflection = n1;
	}
	else if (n2 < n1 && n2 < n3) {
		reflection = n2;
	}
	else if (n3 < n2 && n3 < n1) {
		reflection = n3;
	}

	if (n1 > n2 && n1 > n3) {
		transmittance = n1;
		if (reflection == n2) {
			absorption = n3;
		}
		else {
			absorption = n2;
		}
	}
	else if (n3 > n2 && n3 > n1) {
		transmittance = n3;
		if (reflection == n2) {
			absorption = n1;
		}
		else {
			absorption = n2;
		}
	}
	else if (n2 > n1 && n2 > n3) {
		transmittance = n2;
		if (reflection == n1) {
			absorption = n3;
		}
		else {
			absorption = n1;
		}
	}*/
};


void get_dimentions(std::pair<float, float> &x, std::pair<float, float>&y, std::pair<float, float>&z, Object3D model_in) {
	float x_min = FLT_MAX, y_min = FLT_MAX, z_min = FLT_MAX, x_max = -FLT_MAX, y_max = -FLT_MAX, z_max = -FLT_MAX;
	std::vector<Mesh> meshes = model_in.get_meshes();

	for (int m = 0; m < meshes.size(); m++) {
		Mesh mesh = meshes[m];
		for (int i = 0; i < mesh.vertices.size(); i++) {
			Vertex vertex = mesh.vertices[i];
			if (vertex.Position[0] > x_max)
				x_max = vertex.Position[0];
			else if (vertex.Position[0] < x_min)
				x_min = vertex.Position[0];

			if (vertex.Position[1] > y_max)
				y_max = vertex.Position[1];
			else if (vertex.Position[1] < y_min)
				y_min = vertex.Position[1];

			if (vertex.Position[2] > z_max)
				z_max = vertex.Position[2];
			else if (vertex.Position[2] < z_min)
				z_min = vertex.Position[2];
		}
	}
	x.first = x_max;
	x.second = x_min;
	y.first = y_max;
	y.second = y_min;
	z.first = z_max;
	z.second = z_min;
}

Simulation::Simulation(Object3D model_in) {
	model = model_in;
	out_of_space_photons = 0;
	absorbed_photons = 0;
	//init a 3d vector

	cells = new Cell * *[CELLS_COUNT] {};

	std::pair<float, float> x(.0f, .0f), y(.0f, .0f), z(.0f, .0f);
	get_dimentions(x, y, z, model_in);
	
	float x_diff = x.first - x.second;
	float y_diff = y.first - y.second;
	float z_diff = z.first - z.second;

	cell_size = CELL_SIZE_COEF*(x_diff + y_diff + z_diff) / (3*CELLS_COUNT);
	float delta_space = (CELL_SIZE_COEF - 1) / 2;	

	float x_shift = x.second - (x_diff * delta_space); //x_min - 0.1*model_width.x (expand by 10%)
	float y_shift = y.second - (y_diff * delta_space); //y_min - 0.1*model_width.y
	float z_shift = z.second - (z_diff * delta_space); //z_min - 0.1*model_width.z

	max_dimentions = glm::vec3(x.first + (x_diff * delta_space), y.first + (y_diff * delta_space), z.first + (z_diff * delta_space));
	min_dimentions = glm::vec3(x_shift, y_shift, z_shift);
	
	std::cout << "INIT CELLS" << std::endl;

#pragma omp parallel for
	for (int i = 0; i < CELLS_COUNT; i++) {
		cells[i] = new Cell * [CELLS_COUNT];
		float cell_x = x_shift + i * cell_size;
		for (int j = 0; j < CELLS_COUNT; j++) {
			cells[i][j] = new Cell[CELLS_COUNT];
			float cell_y = y_shift + j * cell_size;
			for (int k = 0; k < CELLS_COUNT; k++) {
				float cell_z = z_shift + k * cell_size;
				cells[i][j][k] = Cell(glm::vec3(cell_x, cell_y, cell_z));
			}
		}
	}


	std::cout << "INIT CELLS END" << std::endl;

	std::vector<Mesh> meshes = model.get_meshes();

	std::cout << "BIND CELLS" << std::endl;

	std::cout << glm::length(glm::vec3(1, 1, 1) - glm::vec3(0, 0, 0)) << std::endl;
	
	//decide if a cell is intersecting with model boundary
#pragma omp parallel for
	for (int m = 0; m < meshes.size(); m++) {
		Mesh mesh = meshes[m];
		//take every triangle of the model and check if ray intersects with it
		for (int i = 0; i < mesh.vertices.size()-2; i+=3) { 
			glm::vec3 v1 = mesh.vertices[i].Position;
			glm::vec3 v2 = mesh.vertices[i+1].Position;
			glm::vec3 v3 = mesh.vertices[i+2].Position;

			//vector of the side of triangle
			glm::vec3 l1 = v2 - v1;
			glm::vec3 l2 = v3 - v2;
			glm::vec3 l3 = v3 - v1;

			glm::vec3 normal_to_side = glm::cross(l1, l2);

			//dividing a vector to evenly-sized portions
			short n1 = std::round(glm::length(v2 - v1) / (cell_size*0.5));
			short n2 = std::round(glm::length(v3 - v2) / (cell_size*0.5));
			short n3 = std::round(glm::length(v3 - v1) / (cell_size*0.5));

			

			//iterate through every triangle side
			//v1->v2 (l1)
			for (int i1 = 0; i1 < n1; i1++) {
				//find index of cell
				int i_temp = std::floor(((i1 * l1[0] / n1) + (v1[0] - x_shift)) / cell_size);
				int j_temp = std::floor(((i1 * l1[1] / n1) + (v1[1] - y_shift)) / cell_size);
				int k_temp = std::floor(((i1 * l1[2] / n1) + (v1[2] - z_shift)) / cell_size);

				cells[i_temp][j_temp][k_temp].border = true;				
				cells[i_temp][j_temp][k_temp].normal = normal_to_side;		
			}
			//v2->v3 (l2)
			for (int i1 = 0; i1 < n2; i1++) {
				//find index of cell
				int i_temp = std::floor(((i1 * l2[0] / n2) + (v2[0] - x_shift)) / cell_size);
				int j_temp = std::floor(((i1 * l2[1] / n2) + (v2[1] - y_shift)) / cell_size);
				int k_temp = std::floor(((i1 * l2[2] / n2) + (v2[2] - z_shift)) / cell_size);

				cells[i_temp][j_temp][k_temp].border = true;
				cells[i_temp][j_temp][k_temp].normal = normal_to_side;
			}
			//v1->v3 (l3)
			for (int i1 = 0; i1 < n3; i1++) {
				//find index of cell
				int i_temp = std::floor(((i1 * l3[0] / n3) + (v3[0] - x_shift)) / cell_size);
				int j_temp = std::floor(((i1 * l3[1] / n3) + (v3[1] - y_shift)) / cell_size);
				int k_temp = std::floor(((i1 * l3[2] / n3) + (v3[2] - z_shift)) / cell_size);

				cells[i_temp][j_temp][k_temp].border = true;
				cells[i_temp][j_temp][k_temp].normal = normal_to_side;
			}
		}
	}
	std::cout << "BIND CELLS END" << std::endl;
	std::cout << "PHOTONS INIT" << std::endl;

	for (int i = 0; i < PHOTONS_COUNT; i++) {
		photons.push_back(Photon(max_dimentions + min_dimentions));
	}
	std::cout << "PHOTONS INIT END" << std::endl;
}

void Simulation::simulate() {
	if (absorbed_photons + out_of_space_photons >= PHOTONS_COUNT) {
		//std::cout << "END_OF_SIMULATION" << std::endl;
		return;
	}
	//std::cout << absorbed_photons  << " " <<  out_of_space_photons << std::endl;
	//#pragma omp parallel for
	for	(int i = 0; i < PHOTONS_COUNT; i++) {
		if (photons[i].absorbed || photons[i].out_of_space) {
			continue;
		}
		photons[i].save();
		photons[i].location += photons[i].direction;

		//check if new location is on the border of the model
		//find index of cell photon is in
		int i1 = std::floor((photons[i].location[0] - min_dimentions[0]) / cell_size);
		int i2 = std::floor((photons[i].location[1] - min_dimentions[1]) / cell_size);
		int i3 = std::floor((photons[i].location[2] - min_dimentions[2]) / cell_size);

		//std::cout << i << ": " << photons[i].direction[0] << " " << photons[i].direction[1] << " " << photons[i].direction[2] << std::endl;

		

		if (i1 <= 0 || i2 <= 0 || i3 <= 0 || i1 >= CELLS_COUNT - 1 || i2 >= CELLS_COUNT - 1 || i3 >= CELLS_COUNT - 1) {
			photons[i].out_of_space = true;
			out_of_space_photons++;
			continue;
		}

		//printf("%f, %f, %f = %d\n", cells[i1][i2][i3].location[0], cells[i1][i2][i3].location[1], cells[i1][i2][i3].location[2], cells[i1][i2][i3].border);
		
		float rand_num = rand();
		rand_num /= RAND_MAX;
		glm::vec3 new_angle = photons[i].direction;


		if (cells[i1][i2][i3].border || photons[i].inside_model) {
			if (rand_num < material.absorption && rand_num > material.reflection) { //absorption
				photons[i].absorbed = true;
				absorbed_photons++;
			}
			else if (rand_num > material.absorption){ //transmittance
				float p = rand();
				p /= RAND_MAX;
				float a1 = 6.28 * p; //zenith angle
				float a2 = 1 / cos(1 - p); //azimuth angle
				new_angle = glm::vec3(cos(a2), a1, sin(a2));
				std::cout << new_angle[0] << " " << new_angle[1] << " " << new_angle[2] << std::endl;
			}
			else if (rand_num < material.reflection && cells[i1][i2][i3].border && !photons[i].inside_model) { //reflection
				new_angle = glm::reflect(photons[i].direction, cells[i1][i2][i3].normal);
			}
		}

		photons[i].direction = glm::normalize(new_angle);

		if (cells[i1][i2][i3].border && photons[i].inside_model_prev) {
			continue;
		}
		else if (cells[i1][i2][i3].border && photons[i].inside_model) {
			photons[i].inside_model_prev = true;
		}
		else if (cells[i1][i2][i3].border) {
			photons[i].inside_model = true;
		}
		else if (!cells[i1][i2][i3].border && photons[i].inside_model) {
			photons[i].inside_model = false;
		}
		else if (!cells[i1][i2][i3].border && photons[i].inside_model_prev) {
			photons[i].inside_model_prev = false;
		}

		
	}
}

void Simulation::draw() {
	for (int i = 0; i < PHOTONS_COUNT; i++) {
		photons[i].mesh.draw();
	}
}

Simulation::~Simulation() {
	delete[] cells;
}