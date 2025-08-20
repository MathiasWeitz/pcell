// #include <cstdio>
#include <iostream>
#include <iomanip>
#include <math.h>
#include "matrix.h"

using namespace std;

Vector::Vector(int n) : n(n) { values = (float *)malloc(n * sizeof(float)); }
Vector::~Vector() { free(values); }
int Vector::lenght() { return n; }
void Vector::set(int pos, float value) { values[pos] = value; }
float Vector::get(int pos) { return values[pos]; }
void Vector::print() {
  cout << std::fixed << std::setprecision(6);
  std::cout << std::setw(12);
  for (int i = 0; i < n; i++) {
    cout << std::setw(12) << values[i] << '\t';
  }
  cout << endl;
}

Matrix::Matrix(int y, int x) : height(y), width(x) {
  field = (float *)malloc(x * y * sizeof(float));
  for (int yi = 0; yi < height; yi++) {
    for (int xi = 0; xi < width; xi++) {
      field[yi * width + xi] = 0.0;
    }
  }
}
Matrix::~Matrix() { free(field); }

void Matrix::set(int y, int x, float v) { field[y * width + x] = v; }
float Matrix::get(int y, int x) { return field[y * width + x]; }

Matrix *Matrix::transpose() {
  Matrix *m = new Matrix(width, height);
  for (int yi = 0; yi < height; yi++) {
    for (int xi = 0; xi < width; xi++) {
      m->set(xi, yi, field[yi * width + xi]);
    }
  }
  return m;
}

Matrix *Matrix::mul(Matrix *m) {
  Matrix *r = new Matrix(height, height);
  for (int w1 = 0; w1 < height; w1++) {
    for (int w2 = 0; w2 < height; w2++) {
      float sum = 0;
      for (int i = 0; i < width; i++) {
        sum += field[w1 * width + i] * m->get(i, w2);
      }
      r->set(w1, w2, sum);
    }
  }
  return r;
}

Vector *Matrix::mulV(Vector *v) {
  Vector *erg = new Vector(height);
  for (int i = 0; i < height; i++) {
    float s = 0;
    for (int k = 0; k < width; k++) {
      s += get(i, k) * v->get(k);
    }
    erg->set(i, s);
  }
  return erg;
}

Vector *Matrix::cholesky(Vector *e) {
  Matrix *l = new Matrix(height, height);
  // cout << "start cholesky" << endl;
  for (int k = 0; k < height; k++) {
    float pivot = get(k, k);
    if (0.0 < pivot) {
      l->set(k, k, sqrt(pivot));
      // cout << "###\t" << k << endl;
      // l->print();
      for (int i = k + 1; i < height; i++) {
        float l1 = get(i, k) / l->get(k, k);
        l->set(i, k, l1);
        // cout << "###\t" << k << "\t" << i << endl;
        // l->print();
        for (int j = k + 1; j < i + 1; j++) {
          set(i, j, get(i, j) - l->get(i, k) * l->get(j, k));
        }
      }
    }
  }
  // cout << "Matrix 1:" << endl;
  // l->print();
  // cout << endl;
  // cout << "Matrix 2:" << endl;
  // print();
  // cout << endl;
  Vector *c = new Vector(height);
  for (int i = 0; i < height; i++) {
    float s = e->get(i);
    // cout << "s1==\t" << s << endl;
    for (int j = 0; j < i; j++) {
      s -= l->get(i, j) * c->get(j);
    }
    // cout << "s2==\t" << s << endl;
    if (abs(l->get(i, i)) < 1e-12) {
      c->set(i, 0);
    } else {
      c->set(i, s / l->get(i, i));
    }
    // cout << "c==\t";
    // c->print();
  }
  // cout << "Vector 1:" << endl;
  // c->print();
  // cout << endl;S
  Vector *x = new Vector(height);
  for (int i = height - 1; i >= 0; i--) {
    float s = c->get(i);
    for (int k = i + 1; k < height; k++) {
      s += l->get(k, i) * x->get(k);
    }
    float sp = l->get(i, i);
    if (1e-12 < abs(sp)) {
      x->set(i, -s / sp);
    }
  }
  // cout << "Vector 2:" << endl;
  // x->print();
  // cout << endl;
  delete c;
  delete l;
  return x;
}

void Matrix::print() {
  cout << std::fixed << std::setprecision(6);
  std::cout << std::setw(12);
  for (int yi = 0; yi < height; yi++) {
    for (int xi = 0; xi < width; xi++) {
      cout << std::setw(12) << field[yi * width + xi] << '\t';
    }
    cout << endl;
  }
}
