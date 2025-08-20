#ifndef MATRIXMY_H
#define MATRIXMY_H

class Vector {
protected:
    int n;
    float* values;
public:
    Vector(int);
    ~Vector();
    int lenght();
    void set(int,float);
    float get(int);
    void print();
};

class Matrix {
protected:
	int height, width;
	float* field;
public:
    Matrix(int, int);
    ~Matrix();
    void set(int,int,float);
    float get(int,int);
    Matrix* transpose();
    Matrix* mul(Matrix*);
    Vector* mulV(Vector*);
    Vector* cholesky(Vector*);
    void print();
};

#endif
