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
struct cutpack
{
	std::vector<Vec4f> vertex;
	std::vector<Vec4f> normal;
	std::vector<Vec2f> tex;
};

struct cutpack3 {
	std::vector<Vec3f> vertex; // 转换后的三维顶点
	std::vector<Vec3f> normal; // 转换后的三维法线
	std::vector<Vec2f> tex;    // 纹理坐标直接传递
	std::vector<float> vertex_w;
};

// 转换函数：将 cutpack 中的 4D 数据转换为 3D 数据
cutpack3 convertCutpackTo3d(const cutpack& cp) {
	cutpack3 cp3;

	// 转换顶点：进行齐次除法
	for (const auto& v : cp.vertex) {
		// 如果 v.w 非 0 则计算 1/v.w，否则直接使用（避免除0错误）
		float invW = (v.w != 0.0f) ? (1.0f / v.w) : 1.0f;
		cp3.vertex.push_back(Vec3f(v.x * invW, v.y * invW, v.z * invW));
		cp3.vertex_w.push_back(invW);
	}

	// 转换法线：通常法线的 w 分量无效，直接丢弃即可
	for (const auto& n : cp.normal) {
		cp3.normal.push_back(Vec3f(n.x, n.y, n.z));
	}

	// 纹理数据保持不变
	cp3.tex = cp.tex;

	return cp3;
}
cutpack cuthelp(cutpack& pack, CutWhere mark) {
	cutpack result;

	auto inside = [&](const Vec4f& v) -> bool {
		switch (mark) {
		case left:   return (v.x + v.w) >= 0;
		case right:  return (v.x - v.w) <= 0;
		case bottom: return (v.y + v.w) >= 0;
		case top:    return (v.y - v.w) <= 0;
		case znear:  return (v.z + v.w) >= 0;
		case zfar:   return (v.z - v.w) <= 0;
		}
		return false;
	};

	auto computeT = [&](const Vec4f& v1, const Vec4f& v2) -> float {
		float f1, f2;
		switch (mark) {
		case left:
			f1 = v1.x + v1.w;
			f2 = v2.x + v2.w;
			break;
		case right:
			f1 = v1.x - v1.w;
			f2 = v2.x - v2.w;
			break;
		case bottom:
			f1 = v1.y + v1.w;
			f2 = v2.y + v2.w;
			break;
		case top:
			f1 = v1.y - v1.w;
			f2 = v2.y - v2.w;
			break;
		case znear:
			f1 = v1.z + v1.w;
			f2 = v2.z + v2.w;
			break;
		case zfar:
			f1 = v1.z - v1.w;
			f2 = v2.z - v2.w;
			break;
		default:
			return 0.0f;
		}
		float denom = f1 - f2;
		if (denom == 0.0f) return 0.0f; // 避免除0情况
		return f1 / denom;
	};

	int n = pack.vertex.size();
	for (int i = 0; i < n; ++i) {
		int curr = i;
		int prev = (i + n - 1) % n;

		bool curr_in = inside(pack.vertex[curr]);
		bool prev_in = inside(pack.vertex[prev]);

		const Vec4f& v1 = pack.vertex[prev];
		const Vec4f& v2 = pack.vertex[curr];
		const Vec4f& n1 = pack.normal[prev];
		const Vec4f& n2 = pack.normal[curr];
		const Vec2f& t1 = pack.tex[prev];
		const Vec2f& t2 = pack.tex[curr];

		if (prev_in && curr_in) {
			// 保留当前顶点
			result.vertex.push_back(v2);
			result.normal.push_back(n2);
			result.tex.push_back(t2);
		}
		else if (prev_in && !curr_in) {
			// 边界 → 外部：加入交点
			float t = computeT(v1, v2);
			result.vertex.push_back(v1 * (1 - t) + v2 * t);
			result.normal.push_back(n1 * (1 - t) + n2 * t);
			result.tex.push_back(t1 * (1 - t) + t2 * t);
		}
		else if (!prev_in && curr_in) {
			// 外部 → 内部：加入交点和当前点
			float t = computeT(v1, v2);
			result.vertex.push_back(v1 * (1 - t) + v2 * t);
			result.normal.push_back(n1 * (1 - t) + n2 * t);
			result.tex.push_back(t1 * (1 - t) + t2 * t);

			result.vertex.push_back(v2);
			result.normal.push_back(n2);
			result.tex.push_back(t2);
		}
	}
	return result;
}

cutpack cuttriangle(cutpack& pack) {
	cutpack current = pack;

	std::vector<CutWhere> planes = {
		left,
		right,
		bottom,
		top ,
		znear,
		zfar
	};

	for (auto& where : planes) { // 使用c代替coord避免冲突
			current = cuthelp(current, where);
			if (current.vertex.size() < 3) break; // 被完全裁掉了
	}

	return current;
}

std::vector<cutpack> cut2triangle(cutpack pack) {
	std::vector<cutpack> result;

	size_t n = pack.vertex.size();
	if (n < 3) return result; // 无法组成三角形，直接返回空

	for (size_t i = 1; i + 1 < n; ++i) {
		cutpack tri;
		// 顶点
		tri.vertex.push_back(pack.vertex[0]);
		tri.vertex.push_back(pack.vertex[i]);
		tri.vertex.push_back(pack.vertex[i + 1]);
		// 法线
		tri.normal.push_back(pack.normal[0]);
		tri.normal.push_back(pack.normal[i]);
		tri.normal.push_back(pack.normal[i + 1]);
		// 纹理
		tri.tex.push_back(pack.tex[0]);
		tri.tex.push_back(pack.tex[i]);
		tri.tex.push_back(pack.tex[i + 1]);

		result.push_back(tri);
	}

	return result;
}
o2v vertex_shader::MVPtrans(const Vec3f& worldcoord, const Vec3f& rotate_angle, const Vec3f& scale, const Vec3f& camera_coord, const Vec3f& camera_direction, const float& left, const float& right, const float& bottom, const float& top, const float& znear, const float& zfar) {
	o2v result;
	int numface = _model.nfaces();
	Matrix4x4f Matrix_M = obj2world(worldcoord, rotate_angle, scale);
	Matrix4x4f normal_M;
	bool isusenormal = 0;
	if (scale.x != scale.y || scale.y != scale.z || scale.x != scale.y || scale.z != scale.x) {
		normal_M = Matrix_M.get_normalM();
		isusenormal = 1;
	}
	Matrix4x4f Matrix_V = world2view(camera_coord, camera_direction);
	Matrix4x4f Matrix_P = view2cilp(left, right, bottom, top, znear, zfar);
	Matrix4x4f MVP = Matrix_P * Matrix_V * Matrix_M;
	for (int i = 0; i < numface; i++) {
		std::vector<Vec4f> triangle;
		std::vector<Vec2f> tex_triangle;
		std::vector<Vec4f> normal_triangle;
		std::vector<Vec4f> test_triangle;
		for (int j = 0; j < 3; j++) {
			Vec4f vertex = Vec4f(_model.vert(_model.face(i)[j]), 1.0f);
			vertex = Matrix_V * Matrix_M * vertex;
			test_triangle.push_back(vertex);
			vertex = Matrix_P * vertex;
			triangle.push_back(vertex);
			tex_triangle.push_back(_model.tex_vert(_model.texture(i)[j]));

			if (isusenormal) {
				Vec4f normal = Matrix_V * normal_M * Vec4f(_model.vert(_model.face(i)[j]), 0.0f);
				normal.normalize();
				normal_triangle.push_back(normal);
			}
			else {
				Vec4f normal = Matrix_V * Matrix_M * Vec4f(_model.vert(_model.face(i)[j]), 0.0f);
				normal.normalize();
				normal_triangle.push_back(normal);
			}
		}
		if (triangle.size() != 3) {
			continue;
		}
		cutpack temppack = { triangle,normal_triangle,tex_triangle };
		std::vector<cutpack> triangles = cut2triangle(cuttriangle(temppack));
		Vec3f facenormal = ((test_triangle[2] - test_triangle[0]).change2vec3() ^ (test_triangle[1] - test_triangle[0]).change2vec3());
		if ( facenormal * Vec3f(0, 0, -1.0f) > 0 && bculling) {
			int n = triangles.size();
				for (int i = 0; i < n; i++) {
					cutpack3 temp = convertCutpackTo3d(triangles[i]);
						result.vertex_coord.push_back(temp.vertex);
						result.nor_coord.push_back(temp.normal);
						result.tex_coord.push_back(temp.tex);
						result.vertex_w.push_back(temp.vertex_w);
				}
		}
		else if (!bculling) {
			int n = triangles.size();
			for (int i = 0; i < n; i++) {
				cutpack3 temp = convertCutpackTo3d(triangles[i]);
				result.vertex_coord.push_back(temp.vertex);
				result.nor_coord.push_back(temp.normal);
				result.tex_coord.push_back(temp.tex);
				result.vertex_w.push_back(temp.vertex_w);
			}
		}
	}
	return result;
}

frangment_shader::frangment_shader(const o2v& a) :original_coords(), vertex(a) {}

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
			vertex.vertex_coord[i][j].z = (original_coords[i][j].z + 1);
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
		std::vector<float> vertex_w = vertex.vertex_w[i];
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
				float inv_w0 = vertex_w[0], inv_w1 = vertex_w[1], inv_w2 = vertex_w[2];
				float denom = bc_screen.x * inv_w0 + bc_screen.y * inv_w1 + bc_screen.z * inv_w2;
				float t0 = (bc_screen.x * inv_w0) / denom;
				float t1 = (bc_screen.y * inv_w1) / denom;
				float t2 = (bc_screen.z * inv_w2) / denom;//透视校正插值得到正确重心坐标
				//质心坐标有一个负值，说明点在三角形外
				if (t0 < 0 || t1 < 0 || t2 < 0) continue;
				//计算zbuffer
				P.z = triangle[0].raw[2] * t0+ triangle[1].raw[2] * t1+ triangle[2].raw[2] * t2;
				//计算纹理坐标
				Tex_P = { 0.0f,0.0f };
				Tex_P.x = (tex_coord[0].x * t0 + tex_coord[1].x * t1 + tex_coord[2].x * t2)*texwidth;
				Tex_P.y = (tex_coord[0].y * t0 + tex_coord[1].y * t1 + tex_coord[2].y * t2) * texheight;
				//计算表面亮度
				float intensity_P;
				Vec3f normal_P = normal_coord[0] * t0 + normal_coord[1] * t1 + normal_coord[2] * t2;
				normal_P.normalize();

				intensity_P = -(normal_P * light);
				if (intensity_P < 0) {
					intensity_P = 1;
				}
				if (zbuffer[int(P.x + P.y * width)] > P.z) {
					zbuffer[int(P.x + P.y * width)] = P.z;
					TGAColor temp2(255, 255, 255, 255);
					if (texture.buffer() != 0) {
						temp2 = texture.get(Tex_P.x, Tex_P.y);
					}
					target.set(P.x, P.y, TGAColor(temp2.r*intensity_P, temp2.g *intensity_P, temp2.b * intensity_P, temp2.a));
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
		Vec3f direction_x =  Vec3f(0, 1, 0)^direction_z ;
		direction_x.normalize();
		Vec3f direction_y =  direction_x^direction_z;
		direction_y.normalize();

		for (int i = 0; i < 3; i++) {
			rotate[i] = -direction_x.raw[i];
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
	if (isperspective) {
		result[0] = 2 * znear / (right - left);
		result[5] = 2 * znear / (top - bottom);
		result[8] = (right + left) / (right - left);
		result[9] = (top + bottom) / (top - bottom);
		result[10] = -(zfar + znear) / (zfar - znear);
		result[11] = -1;
		result[14] = -(2 * zfar * znear) / (zfar - znear);
		result[15] = 0;
	}
	else {
		result[0] = 2 / (right - left);
		result[5] = 2 / (top - bottom);
		result[10] = -2 / (zfar - znear);
		result[3] = -(right + left) / (right - left);
		result[7] = -(top + bottom) / (top - bottom);
		result[11] = -(zfar + znear) / (zfar - znear);
		result[15] = 1;
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



