#include"my_gl.h"
#include"globals.h"

vertex_shader::vertex_shader(const char* filename, bool istexture, const char* filenametex) :_model(filename) {
	if (istexture) {
		_model.settex(filenametex);
	}
}
Model& vertex_shader::getmodel() {
	return _model;
}
TGAImage& vertex_shader::gettex() {
	return _model.gettex();
}
std::vector<Vec4f> cuttriangle(std::vector<Vec4f>& trangle, float coord, CutXYZ mark) {
	switch (mark)
	{
	case x: {
		for (int i = 0; i < 3; i++) {
			if(trangle[i].x)<
		}
	}
		break;
	case y:
		break;
	case z:
		break;
	default:
		break;
	}
}
std::vector<std::vector<Vec4f>> cut2triangle(std::vector<Vec4f>& becut) {

}
o2v vertex_shader::MVPtrans(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale, const Vec3f& camera_coord, const Vec3f& camera_direction, const float& left, const float& right, const float& bottom, const float& top, const float& znear, const float& zfar) {
	o2v result;
	int numface = _model.nfaces();
	Matrix4x4f Matrix_M = obj2world(worldcoord, rotate_angle, scale);
	Matrix4x4f Matrix_V = world2view(camera_coord, camera_direction);
	Matrix4x4f Matrix_P = view2cilp(left, right, bottom, top, znear, zfar);
	for (int i = 0; i < numface; i++) {
		std::vector<Vec3f> trangle;
		std::vector<Vec2f> tex_trangle;
		std::vector<Vec3f> normal_trangle;
		std::vector<Vec3f> test_trangle;
		for (int j = 0; j < 3; j++) {
			Vec3f vertex = Matrix_V * Matrix_M * _model.vert(_model.face(i)[j]);
			test_trangle.push_back(vertex);
			Vec3f vertex2 = Matrix_P * vertex;
			trangle.push_back(vertex2);
			tex_trangle.push_back(_model.tex_vert(_model.texture(i)[j]));
			normal_trangle.push_back(_model.nor_vert(_model.normal(i)[j]));
		}
		std::vector<Vec3f>_trangle = cuttriangle(trangle, znear, z);
		if (trangle.size() != 3) {
			std::vector<std::vector<Vec3f>> new_triangles = cut2triangle(_trangle);
		}
		Vec3f normal = (test_trangle[2] - test_trangle[0]) ^ (test_trangle[1] - test_trangle[0]);
		normal.normalize();
		if (normal * Vec3f(0, 0, -1) > 0&& bculling) {
			result.vertex_coord.push_back(trangle);
			result.tex_coord.push_back(tex_trangle);
			result.nor_coord.push_back(normal_trangle);
		}
		else if (!bculling) {
			result.vertex_coord.push_back(trangle);
			result.tex_coord.push_back(tex_trangle);
			result.nor_coord.push_back(normal_trangle);
		}
	}
	return result;
}

frangment_shader::frangment_shader(const o2v& a):original_coords(),vertex(a) {}
void frangment_shader::viewtrans(const int& width, const int& height) {
	int num = original_coords.size();
	vertex.vertex_coord.resize(num); // 调整目标数组大小
	for (int i = 0; i < num; i++) {
		int num2 = original_coords[i].size();
		vertex.vertex_coord[i].resize(num2);
		for (int j = 0; j < num2; j++) {
			// 使用原始坐标进行视口转换，覆盖当前数据
			vertex.vertex_coord[i][j].x = (original_coords[i][j].x + 1) * width / 2;
			vertex.vertex_coord[i][j].y = (original_coords[i][j].y + 1) * height / 2;
			vertex.vertex_coord[i][j].z = (original_coords[i][j].z + 1) / 2 * 255;
		}
	}
}
void frangment_shader::drawcall(const int& width, const int& height, float* zbuffer, TGAImage& target, TGAImage& texture, Vec3f light) {
	viewtrans(width, height);
	float texwidth = texture.get_width();
	float texheight = texture.get_height();
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
					Tex_P.x += tex_coord[i].raw[0] * bc_screen.raw[i] * texwidth;
					Tex_P.y += tex_coord[i].raw[1] * bc_screen.raw[i] * texheight;
				}
				//计算表面亮度
				float intensity_P;
				Vec3f normal_P = normal_coord[0] * bc_screen.x + normal_coord[1] * bc_screen.y + normal_coord[2] * bc_screen.z;
				normal_P.normalize();
				intensity_P = -(normal_P * light);
				if (intensity_P < 0) {
					intensity_P = 0;
				}
				if (zbuffer[int(P.x + P.y * width)] < P.z) {
					zbuffer[int(P.x + P.y * width)] = P.z;
					TGAColor temp2(255, 255, 255, 255);
					if (texture.buffer() != 0) {
						temp2 = texture.get(Tex_P.x, Tex_P.y);
					}
					target.set(P.x, P.y, TGAColor(temp2.r * intensity_P, temp2.g * intensity_P, temp2.b * intensity_P, temp2.a));
				}
			}
		}
	}
}

Matrix4x4f obj2world(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale) {
	Matrix4x4f rotate;
	Matrix4x4f translate;
	Matrix4x4f scalee;
	rotate.identity(); translate.identity(); scalee.identity();
	float pi = 3.141592653589793;
	double angle_x = rotate_angle.x * pi / 180;
	double angle_y = rotate_angle.y * pi / 180;
	double angle_z = rotate_angle.z * pi / 180;

	Matrix4x4f Rx, Ry, Rz;
	Rx.identity();
	Rx[5] = std::cos(angle_x); Rx[6] = -std::sin(angle_x);
	Rx[9] = std::sin(angle_x); Rx[10] = std::cos(angle_x);

	Ry.identity();
	Ry[0] = std::cos(angle_y); Ry[2] = std::sin(angle_y);
	Ry[8] = -std::sin(angle_y); Ry[10] = std::cos(angle_y);

	Rz.identity();
	Rz[0] = std::cos(angle_z); Rz[1] = -std::sin(angle_z);
	Rz[4] = std::sin(angle_z); Rz[5] = std::cos(angle_z);

	rotate = Rz * Ry * Rx; // ZYX 顺序

	translate[3] = worldcoord.x;
	translate[7] = worldcoord.y;
	translate[11] = worldcoord.z;

	if (scale.x != 0 && scale.y != 0 && scale.z != 0) {
		scalee[0] = scale.x;
		scalee[5] = scale.y;
		scalee[10] = scale.z;
	}

	Matrix4x4f result = translate * rotate * scalee;
	return translate * rotate * scalee;
}

Matrix4x4f world2view(const Vec3f& camera_coord, const Vec3f& camera_direction, bool iscoord) {
	Matrix4x4f translate;
	Matrix4x4f rotate;
	translate.identity(); rotate.identity();

	translate[3] = -camera_coord.x;
	translate[7] = -camera_coord.y;
	translate[11] = -camera_coord.z;

	if (iscoord) {
		Vec3f direction_z = camera_coord - camera_direction;
		direction_z.normalize();
		Vec3f direction_x = direction_z ^ Vec3f(0, 1, 0);
		direction_x.normalize();
		Vec3f direction_y = direction_z ^ direction_x;
		direction_y.normalize();

		for (int i = 0; i < 3; i++) {
			rotate[i] = direction_x.raw[i];
			rotate[i + 4] = direction_y.raw[i];
			rotate[i + 8] = direction_z.raw[i];
		}
	}
	else {
		float pi = 3.141592653589793;
		double angle_x = camera_direction.x * pi / 180;
		double angle_y = camera_direction.y * pi / 180;
		double angle_z = camera_direction.z * pi / 180;

		Matrix4x4f rotate_x, rotate_y, rotate_z;
		rotate_x.identity();
		rotate_y.identity();
		rotate_z.identity();

		// 绕X轴旋转（Pitch）
		float cos_x = cos(angle_x);
		float sin_x = sin(angle_x);
		rotate_x[5] = cos_x;
		rotate_x[6] = -sin_x;
		rotate_x[9] = sin_x;
		rotate_x[10] = cos_x;

		// 绕Y轴旋转（Yaw）
		float cos_y = cos(angle_y);
		float sin_y = sin(angle_y);
		rotate_y[0] = cos_y;
		rotate_y[2] = sin_y;
		rotate_y[8] = -sin_y;
		rotate_y[10] = cos_y;

		// 绕Z轴旋转（Roll）
		float cos_z = cos(angle_z);
		float sin_z = sin(angle_z);
		rotate_z[0] = cos_z;
		rotate_z[1] = -sin_z;
		rotate_z[4] = sin_z;
		rotate_z[5] = cos_z;

		// 组合旋转矩阵：R_view = R_z * R_x * R_y
		rotate = rotate_z * rotate_x * rotate_y;
	}
	return rotate * translate;
}

Matrix4x4f view2cilp(const float& left, const float& right, const float& bottom, const float& top, const float& znear, const float& zfar, bool isperspective) {
	Matrix4x4f result;
	result[0] = 2 / (right - left);
	result[5] = 2 / (top - bottom);
	result[10] = 2 / (znear - zfar);
	result[3] = -(right + left) / (right - left);
	result[7] = -(top + bottom) / (top - bottom);
	result[11] = -(znear + zfar) / (znear - zfar);
	result[15] = 1;
	if (isperspective) {
		Matrix4x4f perspective;
		perspective[0] = znear;
		perspective[5] = znear;
		perspective[10] = znear + zfar;
		perspective[11] = -(znear * zfar);
		perspective[14] = 1;
		result = result * perspective;
	}
	return result;
}

int  lerp(int a, int b, float x) {
	return a + x * (b - a);
}
void drawpoint(int x, int y, const TGAColor& color, TGAImage& image) {
	image.set(x, y, color);
}
void drawline(int startx, int starty, int endx, int endy, const TGAColor& color, TGAImage& image) {
	bool steep = 0;
	if (std::abs(endy - starty) > std::abs(endx - startx)) {
		std::swap(startx, starty);
		std::swap(endx, endy);
		steep = 1;
	}
	if (startx > endx) {
		std::swap(startx, endx);
		std::swap(starty, endy);
	}
	int dy = endy - starty;
	int dx = endx - startx;
	int k = std::abs(2 * dy);
	int isrise = 0;
	int risey = starty;
	if (steep) {
		for (int x = startx; x <= endx; x++) {
			image.set(risey, x, color);
			isrise += k;
			if (isrise > dx) {
				risey += (endy > starty ? 1 : -1);
				isrise -= 2 * dx;
			}
		}
	}
	else {
		for (int x = startx; x <= endx; x++) {
			image.set(x, risey, color);
			isrise += k;
			if (isrise > dx) {
				risey += (endy > starty ? 1 : -1);
				isrise -= 2 * dx;
			}
		}
	}
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	//计算[AB,AC,PA]的x和y分量
	for (int i = 2; i--; ) {
		s[i].raw[0] = C.raw[i] - A.raw[i];
		s[i].raw[1] = B.raw[i] - A.raw[i];
		s[i].raw[2] = A.raw[i] - P.raw[i];
	}
	//[u,v,1]和[AB,AC,PA]对应的x和y向量都垂直，所以叉乘
	Vec3f u = s[0] ^ s[1];
	//三点共线时，会导致u[2]为0，此时返回(-1,1,1)
	if (std::abs(u.raw[2]) > 1e-2)
		//若1-u-v，u，v全为大于0的数，表示点在三角形内部
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);
}



