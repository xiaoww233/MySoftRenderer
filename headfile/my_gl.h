#ifndef _MY_GL_H_
#define _MY_GL_H_
#include "model.h"
#include<vector>
#include <algorithm>
#include<cmath>

enum CutXYZ
{
	x,y,z
};

struct o2v
{
	std::vector<std::vector<Vec3f>> vertex_coord;
	std::vector<std::vector<Vec2f>> tex_coord;
	std::vector<std::vector<Vec3f>> nor_coord;
};



class vertex_shader{
private:
	Model _model;
public:
	vertex_shader(const char* filename, bool istexture, const char* filenametex = NULL);
	Model& getmodel();
	TGAImage& gettex();
	std::vector<Vec3f> cuttriangle(std::vector<Vec3f>& trangle,float coord,CutXYZ mark);
	std::vector<std::vector<Vec3f>> cut2triangle(std::vector<Vec3f>& becut);
	o2v MVPtrans(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale, 
							const Vec3f& camera_coord, const Vec3f& camera_direction, 
							const float& left, const float& right, 
							const float& bottom, const float& top, 
							const float& znear, const float& zfar);
};

class frangment_shader{
public:
	o2v vertex;
	std::vector<std::vector<Vec3f>> original_coords;
	frangment_shader(const o2v& a);
	void viewtrans(const int& width, const int& height);
	void drawcall(const int& width, const int& height, float* zbuffer, TGAImage& target, TGAImage& texture, Vec3f light);
};

Matrix4x4f obj2world(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale);

Matrix4x4f world2view(const Vec3f& camera_coord, const Vec3f& camera_direction, bool iscoord = true);

Matrix4x4f view2cilp(const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far, bool isperspective = true);

void drawpoint(int x, int y, const TGAColor& color, TGAImage& image);

void drawline(int startx, int starty, int endx, int endy, const TGAColor& color, TGAImage& image);

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
#endif // !_MY_GL_H_

