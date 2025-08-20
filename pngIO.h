#ifndef PNGIO_H
#define PNGIO_H

// #include <cstdlib>
// #include <cstdio>
#include <tuple>
#include <png.h>

class PNGRead {
protected:
	const char* filename;
	FILE *fp = nullptr;
	png_structp png = nullptr;
    png_infop info = nullptr;
	png_bytep *row_pointers = nullptr;
	void read();
public:
	PNGRead(const char*);
	~PNGRead();
	int getHeight();
	int getWidth();
	std::tuple<int, int, int> get(int,int);
};

class PNGWrite {
	protected:
	const char* filename;
	int height = 0;
	int width = 0;
	FILE *fp = nullptr;
	png_structp png = nullptr;
    png_infop info = nullptr;
    png_bytep *row_pointers = nullptr;
	png_bytep image_data = nullptr;
public:
	PNGWrite(char*);
	~PNGWrite();
	void setHeight(int);
	void setWidth(int);
	void set(int,int,int,int,int);
	void write();
	
	void init();
};

#endif
