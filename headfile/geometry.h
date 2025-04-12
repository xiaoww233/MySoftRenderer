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

template <typename t> struct Vec4 {
	union {
		struct { t x, y, z, w; };          // 核心四维分量
		struct { t ivert, iuv, inorm, iw; }; // 备用命名（比如齐次坐标/颜色alpha）
		t raw[4];                          // 裸数组访问
	};

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(t _x, t _y, t _z, t _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vec4(const Vec3<t>& a, t _w) :x(a.x), y(a.y), z(a.z), w(_w) {}

	inline Vec4<t> operator +(const Vec4<t>& v) const {return Vec4<t>(x + v.x, y + v.y, z + v.z, w + v.w);}
	inline Vec4<t> operator -(const Vec4<t>& v) const {return Vec4<t>(x - v.x, y - v.y, z - v.z, w - v.w);}
	inline Vec4<t> operator *(float f) const {return Vec4<t>(x * f, y * f, z * f, w * f);}
	inline Vec4<t> operator /(float f) const { return Vec4<t>(x / f, y / f, z / f, w / f); }
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

	Matrix4x4<t> get_normalM() {
		Matrix4x4<t> normalM;  // 最终的法线矩阵
		// 提取左上角 3x3 部分
		// 根据行优先存储，索引如下：
		// 第一行： m[0],  m[1],  m[2]
		// 第二行： m[4],  m[5],  m[6]
		// 第三行： m[8],  m[9],  m[10]
		t a = m[0], b = m[1], c = m[2];
		t d = m[4], e = m[5], f = m[6];
		t g = m[8], h = m[9], i = m[10];

		// 计算行列式： det = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g)
		t det = a * (e * i - f * h) -
			b * (d * i - f * g) +
			c * (d * h - e * g);

		if (std::abs(det) < 1e-6) {
			std::cerr << "Warning: Singular matrix in get_normalM(). 返回单位矩阵作为法线矩阵。" << std::endl;
			normalM.identity();
			return normalM;
		}
		t invDet = 1 / det;

		// 下面计算 (M_3x3)^-1 后转置，即 (M^-1)^t
		// 先写出 M-1 的元素（按行主序）：
		// inverse M_3x3:
		//   inv[0][0] = (e*i - f*h) * invDet
		//   inv[0][1] = - (b*i - c*h) * invDet
		//   inv[0][2] = (b*f - c*e) * invDet
		//   inv[1][0] = - (d*i - f*g) * invDet
		//   inv[1][1] = (a*i - c*g) * invDet
		//   inv[1][2] = - (a*f - c*d) * invDet
		//   inv[2][0] = (d*h - e*g) * invDet
		//   inv[2][1] = - (a*h - b*g) * invDet
		//   inv[2][2] = (a*e - b*d) * invDet
		//
		// 然后取转置：(M-1)t的元素为：
		//   N[0][0] = inv[0][0] = (e*i - f*h) * invDet
		//   N[0][1] = inv[1][0] = - (d*i - f*g) * invDet
		//   N[0][2] = inv[2][0] = (d*h - e*g) * invDet
		//
		//   N[1][0] = inv[0][1] = - (b*i - c*h) * invDet
		//   N[1][1] = inv[1][1] = (a*i - c*g) * invDet
		//   N[1][2] = inv[2][1] = - (a*h - b*g) * invDet
		//
		//   N[2][0] = inv[0][2] = (b*f - c*e) * invDet
		//   N[2][1] = inv[1][2] = - (a*f - c*d) * invDet
		//   N[2][2] = inv[2][2] = (a*e - b*d) * invDet

		// 将计算结果写入 normalM 的左上角 3x3 部分：
		normalM.m[0] = (e * i - f * h) * invDet;  // (0,0)
		normalM.m[1] = -(d * i - f * g) * invDet;  // (0,1)
		normalM.m[2] = (d * h - e * g) * invDet;      // (0,2)

		normalM.m[4] = -(b * i - c * h) * invDet;    // (1,0)
		normalM.m[5] = (a * i - c * g) * invDet;        // (1,1)
		normalM.m[6] = -(a * h - b * g) * invDet;       // (1,2)

		normalM.m[8] = (b * f - c * e) * invDet;         // (2,0)
		normalM.m[9] = -(a * f - c * d) * invDet;        // (2,1)
		normalM.m[10] = (a * e - b * d) * invDet;         // (2,2)

		// 其余部分填为单位矩阵
		normalM.m[3] = normalM.m[7] = normalM.m[11] = 0;
		normalM.m[12] = normalM.m[13] = normalM.m[14] = 0;
		normalM.m[15] = 1;

		return normalM;
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
	inline Vec4<t> operator * (const Vec4<t>& a) {
		Vec4<t> result;
		for (int i = 0; i < 4; i++) {
			result.raw[i] = a.x * m[0 + 4 * i] + a.y * m[1 + 4 * i] + a.z * m[2 + 4 * i] + a.w * m[3 + 4 * i];
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