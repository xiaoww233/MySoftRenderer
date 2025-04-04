#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push,1)
struct TGA_Header {
	char idlength;					// ID�ֶγ��ȣ�0��ʾ�ޣ�
	char colormaptype;			// 0=����ɫ��1=����ɫ��
	char datatypecode;			// 2=δѹ��RGB��10=RLEѹ��RGB
	short colormaporigin;		// ��ɫ����ʼ������ͨ��Ϊ0��
	short colormaplength;		// ��ɫ����Ŀ��
	char colormapdepth;		// ��ɫ��ÿ��Ŀλ������15/16/24/32��
	short x_origin;					// ͼ��ԭ�����꣨ͨ��Ϊ0��
	short y_origin;
	short width;						// ͼ����
	short height;
	char  bitsperpixel;			// ÿ����λ����16/24/32��
	char  imagedescriptor;	// ��4λ��Alphaͨ��λ������2λ������0x20=���Ͻ�ԭ�㣩
};
#pragma pack(pop)



struct TGAColor {
	union {
		struct {
			unsigned char b, g, r, a;
		};//rgbaֵ
		unsigned char raw[4];
		unsigned int val;
	};//unionʵ�����ֶ�ȡ��ʽ
	int bytespp;//ÿ�����ص��ֽ���

	TGAColor() : val(0), bytespp(1) {
	}

	TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}

	TGAColor(int v, int bpp) : val(v), bytespp(bpp) {
	}//���������µĹ��캯��

	TGAColor(const TGAColor &c) : val(c.val), bytespp(c.bytespp) {
	}//�������캯��

	TGAColor(const unsigned char *p, int bpp) : val(0), bytespp(bpp) {
		for (int i=0; i<bpp; i++) {
			raw[i] = p[i];
		}
	}//ͨ���ַ���������Ĺ��캯��

	TGAColor & operator =(const TGAColor &c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}//����=��ʹ��TGAColor����Ա���ֵ
		return *this;
	}
};//����һ��rgb��ɫ���ࡣ


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
