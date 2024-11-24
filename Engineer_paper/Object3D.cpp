#include "Object3D.h"


//float* Object3D::get_vertices_to_render() {
//	
//};
	
Object3D::Object3D(std::string filename) {
	std::ifstream import_file(filename);
	std::string line = "";

	while (!import_file.eof()) {
		getline(import_file, line);
		if (line[0] == 'v') {
			get_vertices(line);
		}
	}
}

void Object3D::get_vertices(std::string line) {
	int occurences = 0;
	int index = 0;
	for (int i = 0; i < line.length(); i++) {
		if (line[i] == ' ') {
			if (occurences > 0) {
				vertices.push_back(stof(line.substr(index, i - index)));
			}

			index = i;
			occurences++;
		}
	}
}