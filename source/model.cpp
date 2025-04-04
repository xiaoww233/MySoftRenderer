#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(), tex_img() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v.raw[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt "))
		{
			iss >> trash >> trash;
			Vec2f v;
			for (int i = 0; i < 2; i++) iss >> v.raw[i];
			tex_verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vn "))
		{
			iss >> trash >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v.raw[i];
			nor_verts_.push_back(v);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			std::vector<int> t;
			std::vector<int> n;
			int itrash, idx, textureidx, normalidx;
			iss >> trash;
			while (iss >> idx >> trash >> textureidx >> trash >> normalidx) {
				idx--;
				textureidx--;
				normalidx--;// in wavefront obj all indices start at 1, not zero
				f.push_back(idx);
				t.push_back(textureidx);
				n.push_back(normalidx);
			}
			faces_.push_back(f);
			texture_.push_back(t);
			normal_.push_back(n);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

void Model::settex(const char* filename) {
	tex_img.read_tga_file(filename);
	tex_img.flip_vertically();
}

TGAImage& Model::gettex() {
	return tex_img;
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::ntex_vert() {
	return (int)tex_verts_.size();
}

int Model::nnor_vert() {
	return (int)nor_verts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

Vec2f Model::tex_vert(int i) {
	return tex_verts_[i];
}

Vec3f Model::nor_vert(int i) {
	return nor_verts_[i];
}

std::vector<int> Model::face(int idx) {
	return faces_[idx];
}

std::vector<int> Model::texture(int idx) {
	return texture_[idx];
}

std::vector<int> Model::normal(int idx) {
	return normal_[idx];
}

