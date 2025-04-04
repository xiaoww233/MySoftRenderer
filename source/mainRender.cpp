#include "tgaimage.h"
#include "model.h"
#include"my_gl.h"

struct o2v
{
	std::vector<std::vector<Vec3f>> vertex_coord;
	std::vector<std::vector<Vec2f>> tex_coord;
	std::vector<std::vector<Vec3f>> nor_coord;
};

class vertex_shader {
private:
	Model _model;
public:
	vertex_shader(const char* filename, bool istexture, const char* filenametex = NULL) :_model(filename) {
		if (istexture) {
			_model.settex(filenametex);
		}
	}
	TGAImage& gettex() {
		return _model.gettex();
	}
	o2v MVPtrans(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale, const Vec3f& camera_coord, const Vec3f& camera_direction, const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far) {
		o2v result;
		int numface = _model.nfaces();
		Matrix4x4f Matrix_M = obj2world(worldcoord, rotate_angle, scale);
		Matrix4x4f Matrix_V = world2view(camera_coord, camera_direction);
		Matrix4x4f Matrix_P = view2cilp(left, right, bottom, top, near, far);
		for (int i = 0; i < numface; i++) {
			std::vector<Vec3f> trangle;
			std::vector<Vec2f> tex_trangle;
			std::vector<Vec3f> normal_trangle;
			std::vector<Vec3f> test_trangle;
			for (int j = 0; j < 3; j++) {
				Vec3f vertex = Matrix_V * Matrix_M * _model.vert(_model.face(i)[j]);
				test_trangle.push_back(vertex);
				Vec3f vertex2 = (Matrix_P * vertex) / vertex.z;
				if (vertex2.x > 1 || vertex2.x < -1 || vertex2.y>1 || vertex2.y < -1 || vertex2.z > 1 || vertex2.z < -1) {
					continue;
				}
				trangle.push_back(vertex2);
				tex_trangle.push_back(_model.tex_vert(_model.texture(i)[j]));
				normal_trangle.push_back(_model.nor_vert(_model.normal(i)[j]));
			}
			if (trangle.size() != 3) {
				continue;
			}
			Vec3f normal = (test_trangle[2] - test_trangle[0]) ^ (test_trangle[1] - test_trangle[0]);
			normal.normalize();
			if (normal * Vec3f(0, 0, -1) > 0) {
				result.vertex_coord.push_back(trangle);
				result.tex_coord.push_back(tex_trangle);
				result.nor_coord.push_back(normal_trangle);
			}
		}
		return result;
	}
};
class frangment_shader {
private:
	o2v vertex;
public:
	frangment_shader(const o2v& a) {
		vertex = a;
	}
	void viewtrans(const int& width, const int& height) {
		int num = vertex.vertex_coord.size();
		for (int i = 0; i < num; i++) {
			int num2 = vertex.vertex_coord[i].size();
			for (int j = 0; j < num2; j++) {
				vertex.vertex_coord[i][j].x = (vertex.vertex_coord[i][j].x + 1) * width / 2;
				vertex.vertex_coord[i][j].y = (vertex.vertex_coord[i][j].y + 1) * height / 2;
				vertex.vertex_coord[i][j].z = (vertex.vertex_coord[i][j].z + 1) / 2 * 255;
			}
		}
	}
 	void drawcall(const int& width, const int& height, float* zbuffer, TGAImage& target, TGAImage& texture,Vec3f light) {
		int facenum = vertex.vertex_coord.size();
		for (int i = 0; i < facenum; i++) {
			std::vector<Vec3f> triangle = vertex.vertex_coord[i];
			std::vector<Vec2f> tex_coord = vertex.tex_coord[i];
			std::vector<Vec3f> normal_coord = vertex.nor_coord[i];
			Vec2i A, B;
			A.x = std::min({ triangle[0].x, triangle[1].x, triangle[2].x });
			B.x = std::max({ triangle[0].x, triangle[1].x, triangle[2].x });
			A.y = std::max({ triangle[0].y, triangle[1].y, triangle[2].y });
			B.y = std::min({ triangle[0].y, triangle[1].y, triangle[2].y });
			Vec3f P;
			Vec2f Tex_P;
			for (P.x = A.x; P.x <= B.x; P.x++) {
				for (P.y = B.y; P.y <= A.y; P.y++) {
					Vec3f bc_screen = barycentric(triangle[0], triangle[1], triangle[2], P);
					//质心坐标有一个负值，说明点在三角形外
					if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
					P.z = 0;
					//计算zbuffer
					for (int i = 0; i < 3; i++) P.z += triangle[i].raw[2] * bc_screen.raw[i];
					//计算纹理坐标
					Tex_P = { 0.0f,0.0f };
					for (int i = 0; i < 3; i++) {
						Tex_P.x  += tex_coord[i].raw[0] * bc_screen.raw[i] * texture.get_width();
						Tex_P.y += tex_coord[i].raw[1] * bc_screen.raw[i] * texture.get_height();
					}
					//计算表面亮度
					float intensity_P;
					Vec3f normal_P={0,0,0};
					for (int i = 0; i < 3; i++) {
						normal_P.x += normal_coord[i].raw[0] * bc_screen.raw[i];
						normal_P.y += normal_coord[i].raw[1] * bc_screen.raw[i];
						normal_P.z += normal_coord[i].raw[2] * bc_screen.raw[i];
					}
					intensity_P = normal_P * light;
					if (zbuffer[int(P.x + P.y * width)] < P.z) {
						zbuffer[int(P.x + P.y * width)] = P.z;
						TGAColor temp2(255, 255, 255, 255);
						if (texture.buffer() != 0) {
							temp2 = texture.get(Tex_P.x, Tex_P.y);
						}
						target.set(P.x, P.y, TGAColor(temp2.r , temp2.g, temp2.b , temp2.a));
					}
				}
			}
		}
	}
};


int main() {
	float width = 1600;
	float height = 1600;
	TGAImage test(width, height, 4);
	Vec3f light(0, 0, -1);
	float* _zbuffer = new float[width * height];
	for (int i = width * height; i--; _zbuffer[i] = -std::numeric_limits<float>::max());

	vertex_shader normalvertex("diablo3_pose.obj", true, "diablo3_pose_diffuse.tga");
	frangment_shader normalfrangment(normalvertex.MVPtrans(Vec3f(0, 0, 0), Vec3f(0, 0, 0), Vec3f(0, 0, 0), Vec3f(1, -1, 2), Vec3f(0, 0, 0), -1, 1, -1, 1, -1.0f, -100.0f));
	normalfrangment.viewtrans(width, height);
	normalfrangment.drawcall(width, height, _zbuffer, test, normalvertex.gettex(), light);

	test.write_tga_file("lineout.tga");
	std::cin.get();
	return 0;
}