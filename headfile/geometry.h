#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include<iostream>
#include<array>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class t> struct Vec2 {
	union {
		struct { t u, v; };
		struct { t x, y; };
		t raw[2];
	};
	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u), v(_v) {}
	inline Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(u + V.u, v + V.v); }
	inline Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(u - V.u, v - V.v); }
	inline Vec2<t> operator *(float f)          const { return Vec2<t>(u * f, v * f); }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
	union {
		struct { t x, y, z; };
		struct { t ivert, iuv, inorm; };
		t raw[3];
	};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
	inline Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	inline Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
	inline Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }
	inline Vec3<t> operator /(float f)          const { return Vec3<t>(x / f, y / f, z / f); }
	inline t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
	float norm() const { return std::sqrt(x * x + y * y + z * z); }
	Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

template <class t> struct Vec4 {
	union {
		struct { t x, y, z, w; };          // 核心四维分量
		struct { t ivert, iuv, inorm, iw; }; // 备用命名（比如齐次坐标/颜色alpha）
		t raw[4];                          // 裸数组访问
	};

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(t _x, t _y, t _z, t _w) : x(_x), y(_y), z(_z), w(_w) {}

	// 注意：四维没有标准叉乘！这里保持三维叉乘逻辑（自动忽略w分量）
	inline Vec4<t> operator ^(const Vec4<t>& v) const {
		return Vec4<t>(y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x,
			0); 
	}
	inline Vec4<t> operator +(const Vec4<t>& v) const {return Vec4<t>(x + v.x, y + v.y, z + v.z, w + v.w);}
	inline Vec4<t> operator -(const Vec4<t>& v) const {return Vec4<t>(x - v.x, y - v.y, z - v.z, w - v.w);}
	inline Vec4<t> operator *(float f) const {return Vec4<t>(x * f, y * f, z * f, w * f);}
	inline t operator *(const Vec4<t>& v) const {return x * v.x + y * v.y + z * v.z + w * v.w;}
	float norm() const {return std::sqrt(x * x + y * y + z * z + w * w);}
	
	Vec4<t>& normalize(t l = 1) {
		*this = (*this) * (l / norm());
		return *this;
	}

	Vec3<t> change2vec3() {
		return Vec3<t>(x, y, z);
	}

	template <class > friend std::ostream& operator<<(std::ostream& s, Vec4<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
typedef Vec4<int> Vec4i;
typedef Vec4<float> Vec4f;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}
template <class t>std::ostream& operator<<(std::ostream& s, Vec4<t>& v) {
	s <<"(" << v.x << " " << v.y << " " << v.z << " " << v.w << ")\n";
	return s;
}

template<class t> struct Matrix4x4{
	std::array<t,16> m;                   //是行优先矩阵，按行储存

	Matrix4x4() : m{} {}
	Matrix4x4(std::vector<t> a) {
		if (a.size() != 16) {
			m.fill(0);
			std::cout << "初始化数组非法，矩阵默认全0" << std::endl;
		}
		for (int i = 0; i < 16; i++) {
			m[i] = a[i];
		}
	}
	void identity() {
		for (int i = 0; i < 16; i++) {
			if (i % 5 == 0) {
				m[i] = 1;
			}
			else {
				m[i] = 0;
			}
		}
	}
	inline Matrix4x4<t> operator * (const Matrix4x4<t>& a) {
		return Matrix4x4<t>({ m[0] * a.m[0] + m[1] * a.m[4] + m[2] * a.m[8] + m[3] * a.m[12],
											m[0] * a.m[1] + m[1] * a.m[5] + m[2] * a.m[9] + m[3] * a.m[13] ,
											m[0] * a.m[2] + m[1] * a.m[6] + m[2] * a.m[10] + m[3] * a.m[14],
											m[0] * a.m[3] + m[1] * a.m[7] + m[2] * a.m[11] + m[3] * a.m[15],

											m[4] * a.m[0] + m[5] * a.m[4] + m[6] * a.m[8] + m[7] * a.m[12],
											m[4] * a.m[1] + m[5] * a.m[5] + m[6] * a.m[9] + m[7] * a.m[13] ,
											m[4] * a.m[2] + m[5] * a.m[6] + m[6] * a.m[10] + m[7] * a.m[14],
											m[4] * a.m[3] + m[5] * a.m[7] + m[6] * a.m[11] + m[7] * a.m[15],

											m[8] * a.m[0] + m[9] * a.m[4] + m[10] * a.m[8] + m[11] * a.m[12],
											m[8] * a.m[1] + m[9] * a.m[5] + m[10] * a.m[9] + m[11] * a.m[13] ,
											m[8] * a.m[2] + m[9] * a.m[6] + m[10] * a.m[10] + m[11] * a.m[14],
											m[8] * a.m[3] + m[9] * a.m[7] + m[10] * a.m[11] + m[11] * a.m[15],

											m[12] * a.m[0] + m[13] * a.m[4] + m[14] * a.m[8] + m[15] * a.m[12],
											m[12] * a.m[1] + m[13] * a.m[5] + m[14] * a.m[9] + m[15] * a.m[13] ,
											m[12] * a.m[2] + m[13] * a.m[6] + m[14] * a.m[10] + m[15] * a.m[14],
											m[12] * a.m[3] + m[13] * a.m[7] + m[14] * a.m[11] + m[15] * a.m[15] });
	}
	inline t& operator [] (int idx) {
		return m[idx];
	}
	
	inline Matrix4x4<t> operator * (const float& a ) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] * a);
		}
		return temp;
	}
	inline Vec3<t> operator * (const Vec3<t>& a) {
		Vec3<t> result;
		for (int i = 0; i < 3; i++) {
			result.raw[i] = a.x * m[0 + 4 * i] + a.y * m[1 + 4 * i] + a.z * m[2 + 4 * i] + 1 * m[3 + 4 * i];
		}
		return result;
	}
	inline Matrix4x4<t> operator + (const float& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] + a);
		}
		return temp;
	}
	inline Matrix4x4<t> operator - (const float& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] - a);
		}
		return temp;
	}
	inline Matrix4x4<t> operator + (const Matrix4x4<t>& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] + a.m[i]);
		}
		return temp;
	}
	inline Matrix4x4<t> operator - (const Matrix4x4<t>& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] - a.m[i]);
		}
		return temp;
	}
	inline Matrix4x4<t> operator / (const Matrix4x4<t>& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp.m.push_back(m[i] / a.m[i]);
		}
		return temp;
	}
	inline Matrix4x4<t> operator / (const float& a) {
		Matrix4x4<t> temp;
		for (int i = 0; i < 16; i++) {
			temp[i] /= a;
		}
		return temp;
	}
};
typedef Matrix4x4<int> Matrix4x4i;
typedef Matrix4x4<float> Matrix4x4f;

#endif //__GEOMETRY_H__