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
	location = glm::vec3(dimentions[0]/2, 85.f, dimentions[2]/2); //set starting point in the center (x,z) and max up
	float rand1 = (float)rand();
	float rand2 = (float)rand();
	//direction = glm::vec3((rand1 / RAND_MAX) - 0.5f, -1.0f, (rand2 / RAND_MAX) - 0.5f);
	direction = glm::vec3((rand1 / RAND_MAX)/.4f - 0.12f, -1.0f, (rand2 / RAND_MAX) / .4f - 0.12f);
	//direction = glm::vec3(0.0f, -1.0f, 0.0f); //for tests only
	direction = glm::normalize(direction);
	inside_model = false;
	border_passed = 1;
	out_of_space = false;
	absorbed = false;
	reflected = false;

	Vertex vertex;
	vertex.Position = location;
	vertex.Color = glm::vec3(0.0f, 0.0f, 1.0f);
	std::vector<Vertex> vec;
	vec.push_back(vertex);
	std::vector<unsigned int>indices;
	//indices.push_back(0);
	//indices.push_back(0);
	//mesh = Mesh(vec, indices, true);
}

Photon::Photon() {
	Photon(glm::vec3(130.f, 0.f, 130.f));
}

void Photon::save() {
	Vertex vertex;
	vertex.Position = location;
	if(inside_model || (border_passed%2==0 && border_passed % 4 != 0 && !reflected))
		vertex.Color = glm::vec3(1.0f, 0.0f, 0.0f);
	else
		vertex.Color = glm::vec3(0.0f, 0.0f, 1.0f);
	mesh.update_mesh(vertex, mesh.indices.back());
}

Material::Material() { //r; (0, n1) - transmittance; (n1-n2) - absorption; (n2, 1) - scattering
	reflection_chance = 0.0f;
	n1 = 0.9f;
	n2 = 0.99f;
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
	
	std::cout << "INIT CELLS START" << std::endl;
	clock_t t1 = clock();

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
	clock_t t2 = clock();

	std::cout << "INIT CELLS END - " << (float)(t2- t1)/CLOCKS_PER_SEC << "sec" << std::endl;

	std::vector<Mesh> meshes = model.get_meshes();

	std::cout << "BIND CELLS" << std::endl;

	t1 = clock();
	
	//decide if a cell is intersecting with model boundary

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

			glm::vec3 normal_to_side = glm::normalize(glm::cross(l1, l2));

			glm::vec3 min = glm::vec3(FLT_MAX);
			for (int n = 0; n < 3; n++) {
				if (v1[n] <= v2[n] && v1[n] <= v3[n])
					min[n] = v1[n];
				else if (v2[n] <= v1[n] && v2[n] <= v3[n])
					min[n] = v2[n];
				else
					min[n] = v3[n];
			}

			glm::vec3 max = glm::vec3(FLT_MIN);
			for (int n = 0; n < 3; n++) {
				if (v1[n] >= v2[n] && v1[n] >= v3[n])
					max[n] = v1[n];
				else if (v2[n] >= v1[n] && v2[n] >= v3[n])
					max[n] = v2[n];
				else
					max[n] = v3[n];
			}
						
			//creating space in which the triangle is in
			/*int n1 = std::round((max[0] - min[0]) / (cell_size));
			int n2 = std::round((max[1] - min[1]) / (cell_size));
			int n3 = std::round((max[2] - min[2]) / (cell_size));*/
			int n1 = 40;
			int n2 = 40;
			int n3 = 40;

			glm::vec3 range_vec = max - min;

#pragma omp parallel for
			for (int i1 = 0; i1 < n1; i1++) {
				float x = (i1 * range_vec[0] / n1) + min[0];				

				for (int i2 = 0; i2 < n2; i2++) {
					float y = (i2 * range_vec[1] / n2) + min[1];

					for (int i3 = 0; i3 < n3; i3++) {
						float z = (i3 * range_vec[2] / n3) + min[2];

						glm::vec3 p = glm::vec3(x, y, z);

						float V = glm::length((v1 - p)) * glm::length((glm::cross(v2 - p, v3 - p))); //calculate volume of tetrahedron (v1, v2, v3, p)
						if (V < 500) {
							float A1 = 0.5f * glm::length(glm::cross(v1 - p, v2 - p));
							float A2 = 0.5f * glm::length(glm::cross(v1 - p, v3 - p));
							float A3 = 0.5f * glm::length(glm::cross(v2 - p, v3 - p));
							float A123 = 0.5f * glm::length(glm::cross(l1, l3));
							if (abs(A123 - (A1 + A2 + A3)) < 10 || A1<0.5 || A2<0.5 || A3<0.5) {
								int i_temp = std::floor((x - x_shift) / cell_size);
								int j_temp = std::floor((y - y_shift) / cell_size);
								int k_temp = std::floor((z - z_shift) / cell_size);
								cells[i_temp][j_temp][k_temp].border = true;
								cells[i_temp][j_temp][k_temp].normal = normal_to_side;
							}
							
						}
					}
				}
			}
		}
	}
	t2 = clock();

	std::cout << "BIND CELLS END - " << (float)(t2 - t1) / CLOCKS_PER_SEC << "sec" << std::endl;

	std::cout << "PHOTONS INIT" << std::endl;
	t1 = clock();

	for (int i = 0; i < PHOTONS_COUNT; i++) {
		photons[i] = Photon(max_dimentions + min_dimentions);
	}
	t2 = clock();
	std::cout << "PHOTONS INIT END" << (float)(t2 - t1) / CLOCKS_PER_SEC << "sec" << std::endl;
}


bool Simulation::simulate() {
	if (absorbed_photons + out_of_space_photons >= PHOTONS_COUNT) {
		//std::cout << "END_OF_SIMULATION" << std::endl;
		return false;
	}
	//std::cout << absorbed_photons  +  out_of_space_photons << std::endl;

	/*for (int i = 0; i < PHOTONS_COUNT; i++) {
		if (photons[i].absorbed || photons[i].out_of_space) {
			continue;
		}
		photons[i].save();
	}	*/
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < PHOTONS_COUNT; i++) {
			if (photons[i].absorbed || photons[i].out_of_space) {
				continue;
			}

			//#pragma omp critical
			
			photons[i].location += photons[i].direction;

			//check if new location is on the border of the model
			//find index of cell photon is in
			int i1 = std::floor((photons[i].location[0] - min_dimentions[0]) / cell_size);
			int i2 = std::floor((photons[i].location[1] - min_dimentions[1]) / cell_size);
			int i3 = std::floor((photons[i].location[2] - min_dimentions[2]) / cell_size);

			if (i1 <= 0 || i2 <= 0 || i3 <= 0 || i1 >= CELLS_COUNT - 1 || i2 >= CELLS_COUNT - 1 || i3 >= CELLS_COUNT - 1) {
				photons[i].out_of_space = true;
#pragma omp atomic
				out_of_space_photons++;
				continue;
			}

			//printf("%f, %f, %f = %d\n", cells[i1][i2][i3].location[0], cells[i1][i2][i3].location[1], cells[i1][i2][i3].location[2], cells[i1][i2][i3].border);

			float rand_num = (float)rand();
			rand_num /= RAND_MAX;
			glm::vec3 new_angle = photons[i].direction;


			if (cells[i1][i2][i3].border) {
				if (rand_num < material.reflection_chance && !photons[i].inside_model && photons[i].border_passed % 2 != 0 && !photons[i].reflected) { //reflection
					new_angle = glm::reflect(photons[i].direction, cells[i1][i2][i3].normal);
					photons[i].reflected = true;
				}
			}
			else if (photons[i].inside_model && photons[i].border_passed % 2 != 0) {
				if (photons[i].scatter_counter == 0) {
					if (rand_num < material.n2 && rand_num > material.n1) { //absorption
						photons[i].absorbed = true;
						cells[i1][i2][i3].energy++;
#pragma omp atomic
						absorbed_photons++;
					}
					else if (rand_num > material.n2) { //scattering
						float p1 = (float)rand();
						float p2 = (float)rand();
						float p3 = (float)rand();
						p1 /= RAND_MAX;
						p2 /= RAND_MAX;
						p3 /= RAND_MAX;
						new_angle = glm::vec3(p1 - 0.5f, p2 - 0.5f, p3 - 0.5f);
					}
					photons[i].scatter_counter = scatter_counter_const;
				}
				else {
					photons[i].scatter_counter--;
				}
			}

			photons[i].direction = (cell_size * 0.5f) * glm::normalize(new_angle);


			//boundary of model
			if (cells[i1][i2][i3].border) {
				if (photons[i].border_passed % 2 == 1) { //getting on border
					photons[i].border_passed++;
				}
				if (photons[i].border_margin != border_margin_const) {
					photons[i].border_margin = border_margin_const;
				}

				//photons[i].inside_model = true;
			}
			else {
				if (photons[i].border_passed % 3 == 0 && photons[i].reflected) { //case of reflection			
					photons[i].reflected = false;
					photons[i].border_passed = 1;
				}
				else if (photons[i].border_passed % 4 == 0) { //getting outside model
					if (photons[i].border_margin == 0) {
						photons[i].inside_model = false;
						photons[i].border_passed++;
						photons[i].border_margin = border_margin_const;
					}
					else {
						photons[i].border_margin--;
					}
				}
				else if (photons[i].border_passed % 2 == 0 && !photons[i].reflected) { //leaving border, but still being in the model
					if (photons[i].border_margin == 0) {
						photons[i].border_passed++;
						photons[i].inside_model = true;
						photons[i].border_margin = border_margin_const;
					}
					else {
						photons[i].border_margin--;
					}
				}
				//photons[i].inside_model = false;
			}
		}
	}
	return true;
}

void Simulation::detect_temperature(glm::vec3 axis, glm::vec3 axis_begin, std::vector<unsigned int> &energies) {
	glm::vec3 point = axis_begin;
	int i = 0;
	while (true && i<CELLS_COUNT * 2) {
		int i1 = std::floor((point.x - min_dimentions[0]) / cell_size);
		int i2 = std::floor((point.y - min_dimentions[1]) / cell_size);
		int i3 = std::floor((point.z - min_dimentions[2]) / cell_size);
		if (i1 <= 0 || i2 <= 0 || i3 <= 0 || i1 >= CELLS_COUNT - 1 || i2 >= CELLS_COUNT - 1 || i3 >= CELLS_COUNT - 1) {
			return;
		}
		energies.push_back(cells[i1][i2][i3].energy);
		glm::vec3 a = (float)i * glm::normalize(axis) / cell_size;
		point += a;
		i++;
	}
}

void Simulation::draw() {
	for (int i = 0; i < PHOTONS_COUNT; i++) {
		photons[i].mesh.draw_photons();
	}
}

Simulation::~Simulation() {
	delete[] cells;
}