#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> tex_verts_;
	std::vector<Vec3f> nor_verts_;
	std::vector<std::vector<int>> faces_;
	std::vector<std::vector<int>> texture_;
	std::vector<std::vector<int>> normal_;
	TGAImage tex_img;
public:
	Model(const char *filename);
	~Model();
	void settex(const char* filename);
	TGAImage& gettex();
	int nverts();
	int ntex_vert();
	int nnor_vert();
	int nfaces();
	Vec3f vert(int i);
	Vec2f tex_vert(int i);
	Vec3f nor_vert(int i);
	std::vector<int> face(int idx);
	std::vector<int> texture(int idx);
	std::vector<int> normal(int idx);
};

#endif //__MODEL_H__