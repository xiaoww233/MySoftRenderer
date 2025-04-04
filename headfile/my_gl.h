#ifndef _MY_GL_H_
#define _MY_GL_H_
#include "model.h"
#include<vector>
#include <algorithm>
#include<cmath>


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

Matrix4x4f world2view(const Vec3f& camera_coord, const Vec3f& camera_direction, bool iscoord = true) {
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

Matrix4x4f view2cilp(const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far, bool isperspective = true) {
	Matrix4x4f result;
	result[0] = 2 / (right - left);
	result[5] = 2 / (top - bottom);
	result[10] = 2 / (near - far);
	result[3] = -(right + left) / (right - left);
	result[7] = -(top + bottom) / (top - bottom);
	result[11] = -(near + far) / (near - far);
	result[15] = 1;
	if (isperspective) {
		Matrix4x4f perspective;
		perspective[0] = near;
		perspective[5] = near;
		perspective[10] = near + far;
		perspective[11] = -(near * far);
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

#endif // !_MY_GL_H_

