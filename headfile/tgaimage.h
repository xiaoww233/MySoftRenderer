#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push,1)
struct TGA_Header {
	char idlength;					// ID字段长度（0表示无）
	char colormaptype;			// 0=无颜色表，1=有颜色表
	char datatypecode;			// 2=未压缩RGB，10=RLE压缩RGB
	short colormaporigin;		// 颜色表起始索引（通常为0）
	short colormaplength;		// 颜色表条目数
	char colormapdepth;		// 颜色表每条目位数（如15/16/24/32）
	short x_origin;					// 图像原点坐标（通常为0）
	short y_origin;
	short width;						// 图像宽高
	short height;
	char  bitsperpixel;			// 每像素位数（16/24/32）
	char  imagedescriptor;	// 低4位：Alpha通道位数，高2位：方向（0x20=左上角原点）
};
#pragma pack(pop)



struct TGAColor {
	union {
		struct {
			unsigned char b, g, r, a;
		};//rgba值
		unsigned char raw[4];
		unsigned int val;
	};//union实现三种读取方式
	int bytespp;//每个像素的字节数

	TGAColor() : val(0), bytespp(1) {
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}

	TGAColor(int v, int bpp) : val(v), bytespp(bpp) {
	}//各种情形下的构造函数

	TGAColor(const TGAColor &c) : val(c.val), bytespp(c.bytespp) {
	}//拷贝构造函数

	TGAColor(const unsigned char *p, int bpp) : val(0), bytespp(bpp) {
		for (int i=0; i<bpp; i++) {
			raw[i] = p[i];
		}
	}//通过字符数组输入的构造函数

	TGAColor & operator =(const TGAColor &c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}//重载=，使得TGAColor类可以被赋值
		return *this;
	}
};//储存一个rgb颜色的类。


class TGAImage {
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;

	bool   load_rle_data(std::ifstream &in);
	bool unload_rle_data(std::ofstream &out);
public:
	enum Format {
		GRAYSCALE=1, RGB=3, RGBA=4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage &img);
	bool read_tga_file(const char *filename);
	bool write_tga_file(const char *filename, bool rle=true);
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y);
	bool set(int x, int y, TGAColor c);
	~TGAImage();
	TGAImage & operator =(const TGAImage &img);
	int get_width();
	int get_height();
	int get_bytespp();
	unsigned char *buffer();
	void clear();
};

#endif //__IMAGE_H__
