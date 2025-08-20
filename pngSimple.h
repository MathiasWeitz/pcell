#ifndef PNGSIMPLE_H
#define PNGSIMPLE_H

#include <cstdlib>
#include <cstdio>
#include <tuple>
#include <png.h>

#include "pngIO.h"

/**
 * data for one pixel
 * ursprünglich RGB und berechneter RGB
 */
struct rgbf {
	// Vorgabewerte
    float r,g,b;
	// berechnete Werte
	float calc_r;
	float calc_g;
	float calc_b;
	// Zelle ist Teil der Betrachtung
	// das nur ist wichtig wenn man an den Rand eines Images kommt
    // alles ausserhalb des Bildes is invalid
	bool isValid;
	// Zelle ist Teil der Berechnung 
	// dieser Parameter wird von aussen durch die Maske vorgegeben
    // Vorraussetzung für isPart ist isValid
	bool isPart;
	// deprecated, war Teil der Clusterberechnung
	bool isActive;
	// 
	float error;
	// direction, bei linearer Regession
	float dx;
	float dy;
	// overall Minimal Error is needed for setting the Priorities for Floodfill
	float minError;
};

struct resultf {
  int targetR,targetG,targetB;
  float r,g,b;
  // linear
  float rx, ry, gx, gy, bx, by;
  // square
  float rxx, rxy, ryy, gxx, gxy, gyy, bxx, bxy, byy;
  // cubic
  float rxxx, rxxy, rxyy, ryyy, gxxx, gxxy, gxyy, gyyy, bxxx, bxxy, bxyy, byyy;
  float error;
  int iterationCount;
};

class PNGRead;

class CalcImg {
protected:
  rgbf *data = nullptr;
  // length in one side of the square,
  // which makes the whole dimension of the data = 2 * size + 1
  // 
  int sizeQ;
  // dim is 2 * size + 1
  int dim;
  void reset();
  int getIndex(int,int);
  int getAbsIndex(int, int);
  int getAbsLegalIndex(int, int);
  // internals
  int clusterCount;
  // values for constant (1 per channel)
  float offset_red;
  float offset_green;
  float offset_blue;
  // values for linear (2 per channel)
  float my_red;
  float my_green;
  float my_blue;
  float mx_red;
  float mx_green;
  float mx_blue;
  // values for square (3 per channel)
  float myy_red;
  float myy_green;
  float myy_blue;
  float mxy_red;
  float mxy_green;
  float mxy_blue;
  float mxx_red;
  float mxx_green;
  float mxx_blue;
  // values for cubic (4 per channel)
  float myyy_red;
  float myyy_green;
  float myyy_blue;
  float mxyy_red;
  float mxyy_green;
  float mxyy_blue;
  float mxxy_red;
  float mxxy_green;
  float mxxy_blue;
  float mxxx_red;
  float mxxx_green;
  float mxxx_blue;
  // ...
  
public:
 /**
  * test do say hello
  * @param maximal size of the matrix
   */
	CalcImg(int);
	~CalcImg();
	void initData(PNGRead*, int, int);
	const rgbf *getData(); 
	void resize(int);
	void print();
	void printc();
	void calc();
        resultf calcStep(int = 0);
        
	// get the centerValue
	std::tuple<int, int, int> getValue();
    std::tuple<float, float> getDir();

    
	void setMaskSquare(int);
	void setMaskRect(int,int,int);
    void setMaskOval(int,int,int);
	void setMaskCirc(int);
};


#endif
