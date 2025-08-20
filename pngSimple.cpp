

// #include <cstdlib>
// #include <cstdio>
#include <math.h>

#include <cmath>
#include <iomanip>
#include <iostream>

#include "matrix.h"
#include "pngSimple.h"

using namespace std;

/**
 * do say hello
 */ 
CalcImg::CalcImg(int parSize): sizeQ(parSize) {
  dim = 2 * sizeQ + 1;
  data = (rgbf *)malloc(sizeof(rgbf) * dim * dim);
  }

CalcImg::~CalcImg() {
  free(data);
  data = nullptr;
}

int CalcImg::getIndex(int y, int x) {
  // index relativ zum Zentrum (-dim bis dim)
  int result = -1;
  if (y >= -sizeQ && y <= sizeQ && x >= -sizeQ && x <= sizeQ) result = dim * (y + sizeQ) + x + sizeQ;
  return result;
}

int CalcImg::getAbsIndex(int y, int x) {
  int result = -1;
  if (y >= 0 && y < dim && x >= 0 && x < dim) result = dim * y + x;
  return result;
}

int CalcImg::getAbsLegalIndex(int y, int x) {
  int result = -1;
  if (y >= 0 && y < dim && x >= 0 && x < dim) {
    int index = dim * y + x;
    if (data[index].isPart) result = index;
  }
  return result;
}

const rgbf *CalcImg::getData() { return data; }

void CalcImg::reset() {
  for (int y = 0; y < dim; y++) {
    for (int x = 0; x < dim; x++) {
      data[dim * y + x].r = 0;
      data[dim * y + x].g = 0;
      data[dim * y + x].b = 0;

      data[dim * y + x].calc_r = 0;
      data[dim * y + x].calc_g = 0;
      data[dim * y + x].calc_b = 0;
      // berechnete Werte
      // float calc_r,calc_g,calc_b;
      // Zelle ist Teil der Betrachtung
      data[dim * y + x].isValid = false;
      // Zelle ist Teil der Berechnung
      data[dim * y + x].isPart = false;
      // Zelle ist Teil der Berechnung
      data[dim * y + x].isActive = false;
      // float error;
      data[dim * y + x].error = 0;
    }
  }
  // parameter for regression
  offset_red = 0.0;
  offset_green = 0.0;
  offset_blue = 0.0;
  // linear
  my_red = 0.0;
  my_green = 0.0;
  my_blue = 0.0;
  mx_red = 0.0;
  mx_green = 0.0;
  mx_blue = 0.0;
  // square
  myy_red = 0.0;
  myy_green = 0.0;
  myy_blue = 0.0;
  mxy_red = 0.0;
  mxy_green = 0.0;
  mxy_blue = 0.0;
  mxx_red = 0.0;
  mxx_green = 0.0;
  mxx_blue = 0.0;
  // cubic
  myyy_red = 0.0;
  myyy_green = 0.0;
  myyy_blue = 0.0;
  mxyy_red = 0.0;
  mxyy_green = 0.0;
  mxyy_blue = 0.0;
  mxxy_red = 0.0;
  mxxy_green = 0.0;
  mxxy_blue = 0.0;
  mxxx_red = 0.0;
  mxxx_green = 0.0;
  mxxx_blue = 0.0;
}

void CalcImg::initData(PNGRead *p, int y, int x) {
  reset();
  int height = p->getHeight();
  int width = p->getWidth();
  for (int y1 = 0; y1 < dim; y1++) {
    for (int x1 = 0; x1 < dim; x1++) {
      data[dim * y1 + x1].isValid = false;
      data[dim * y1 + x1].isPart = false;
      data[dim * y1 + x1].isActive = false;
      int ry = y - sizeQ + y1;
      int rx = x - sizeQ + x1;
      if (rx >= 0 && rx < width && ry >= 0 && ry < height) {
        std::tuple<int, int, int> m = p->get(ry, rx);
        data[dim * y1 + x1].r = get<0>(m);
        data[dim * y1 + x1].g = get<1>(m);
        data[dim * y1 + x1].b = get<2>(m);
        data[dim * y1 + x1].isValid = true;
      }
    }
  }

  int c0 = getIndex(0, 0);
  if (-1 < c0) {
    data[c0].isPart = true;
  }
  int c1 = getIndex(1, 0);
  if (-1 < c1) {
    data[c1].isPart = data[c1].isValid;
  }
  int c2 = getIndex(-1, 0);
  if (-1 < c2) {
    data[c2].isPart = data[c2].isValid;
  }
  int c3 = getIndex(0, 1);
  if (-1 < c3) {
    data[c3].isPart = data[c3].isValid;
  }
  int c4 = getIndex(0, -1);
  if (-1 < c4) {
    data[c4].isPart = data[c4].isValid;
  }
}

/**
 * print the center pixel and it's neighbours
 */
void CalcImg::printc() {
  /*
  int index = sizeQ * dim + sizeQ;
  cout << " " << std::fixed << std::setprecision(4) << std::setw(5) << data[index].calc_r << ',' << std::setw(5) << data[index].calc_g << ',' << std::setw(5) << data[index].calc_b << "\033[0m\t";
  cout << " " << std::fixed << std::setprecision(4) << std::setw(5) << data[index - dim].calc_r << ',' << std::setw(5) << data[index - dim].calc_g << ',' << std::setw(5) << data[index - dim].calc_b
       << "\033[0m\t";
  cout << " " << std::fixed << std::setprecision(4) << std::setw(5) << data[index - 1].calc_r << ',' << std::setw(5) << data[index - 1].calc_g << ',' << std::setw(5) << data[index - 1].calc_b
       << "\033[0m\t";
  cout << " " << std::fixed << std::setprecision(4) << std::setw(5) << data[index + 1].calc_r << ',' << std::setw(5) << data[index + 1].calc_g << ',' << std::setw(5) << data[index + 1].calc_b
       << "\033[0m\t";
  cout << " " << std::fixed << std::setprecision(4) << std::setw(5) << data[index + dim].calc_r << ',' << std::setw(5) << data[index + dim].calc_g << ',' << std::setw(5) << data[index + dim].calc_b
       << "\033[0m\t";
  cout << endl;
  */
  cout << "[" << std::fixed << std::setprecision(3) << std::setw(7) << offset_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << offset_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << offset_blue << "] ";
  cout << "[[" << std::fixed << std::setprecision(3) << std::setw(7) << my_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mx_red << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << my_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mx_green << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << my_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mx_blue << "]] ";
  cout << "[[" << std::fixed << std::setprecision(3) << std::setw(7) << myy_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxy_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxx_red << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << myy_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxy_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxx_green << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << myy_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxy_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxx_blue << "]] ";
  cout << "[[" << std::fixed << std::setprecision(3) << std::setw(7) << myyy_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxyy_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxy_red << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxx_red << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << myyy_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxyy_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxy_green << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxx_green << "],[";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << myyy_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxyy_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxy_blue << ",";
  cout << std::fixed << std::setprecision(3) << std::setw(7) << mxxx_blue << "]] ";
  cout << endl;
}

void CalcImg::print() {
  int color;
  int fcolor;
  for (int y = 0; y < dim; y++) {
    cout << std::fixed << std::setprecision(1);
    for (int x = 0; x < dim; x++) {
      fcolor = 7;
      if (data[dim * y + x].isPart) {
        fcolor = 1;
        if (data[dim * y + x].isActive) {
          fcolor = 15;
        }
        color = 8;
        cout << "\033[48;5;" << color << "m";
      }
      cout << "\033[38;5;" << fcolor << "m";
      if (data[dim * y + x].isValid) {
        cout << " " << std::setw(5) << data[dim * y + x].r << ',' << std::setw(5) << data[dim * y + x].g << ',' << std::setw(5) << data[dim * y + x].b << "\033[0m\t";
      } else {
        cout << ".................." << '\t';
      }
      // Zelle ist Teil der Berechnung
      // data[dim * y + x].isPart = false;
      // float error;
      // data[dim * y + x].error = 0;
    }
    cout << endl;

    for (int x = 0; x < dim; x++) {
      if (data[dim * y + x].isValid) {
        fcolor = 7;
        if (data[dim * y + x].isPart) {
          fcolor = 10;
          if (data[dim * y + x].isActive) {
            fcolor = 15;
          }
          color = 8;
          cout << "\033[48;5;" << color << "m";
        }
        cout << "\033[38;5;" << fcolor << "m";
        cout << std::setw(5) << data[dim * y + x].calc_r << ',' << std::setw(5) << data[dim * y + x].calc_g << ',' << std::setw(5) << data[dim * y + x].calc_b << "\033[0m\t";
      } else {
        cout << ".................." << '\t';
      }
    }
    cout << endl;
    for (int i = 0; i <= clusterCount; i++) {
      cout << std::fixed << std::setprecision(4);
      for (int x = 0; x < dim; x++) {
        fcolor = 7;
        if (data[dim * y + x].isValid) {
          fcolor = 7;
          if (data[dim * y + x].isActive) {
            fcolor = 15;
          }
          cout << "\033[38;5;" << fcolor << "\033[0m\t";
        } else {
          cout << "\033[38;5;" << fcolor << "\033[0m\t";
        }
      }
      cout << endl;
    }
    cout << endl;
    
  }
}

/**
 * get the recalculated Value
 */
std::tuple<int, int, int> CalcImg::getValue() {
  // count the values in a certain diameter
  int index = dim * sizeQ + sizeQ;
  // cout << "getValue\t" << index << "\t" << data[index].calc_r  << "\t" <<
  // data[index].calc_g << "\t" << data[index].calc_b << endl;
  // int dim = 2 * MAXSIZECALC + 1;
  int r = 0, g = 0, b = 0;  
    
  r = static_cast<int>(round(data[index].calc_r));
  g = static_cast<int>(round(data[index].calc_g));
  b = static_cast<int>(round(data[index].calc_b));
    
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
  return std::make_tuple(r, g, b);
}

std::tuple<float, float> CalcImg::getDir() {
  int index = dim * sizeQ + sizeQ;
  // cout << "getValue\t" << index << "\t" << data[index].calc_r  << "\t" <<
  // data[index].calc_g << "\t" << data[index].calc_b << endl;
  int dx = static_cast<int>(round(data[index].dx));
  int dy = static_cast<int>(round(data[index].dy));
  return std::make_tuple(dy, dx);
}

/**
 * linear regression all channels independent
 * @param linear  =true linear regression
 */
resultf CalcImg::calcStep(int polGrad) {
  int matrixLength = 0;
  for (int y = 0; y < dim; y++) {
    for (int x = 0; x < dim; x++) {
      if (data[dim * y + x].isPart) {
        matrixLength++;
      }
    }
  }
  float totalError = 0;
  float lastError = 9999;
  bool b = true;
  int matrixSizeRed;
  int whileCounter = 0;
  while (b) {
    b = false;
    int matrixSize = 1;
    if (polGrad == 1) matrixSize = 3;
    if (polGrad == 2) matrixSize = 6;
    if (polGrad == 3) matrixSize = 10;
    matrixSizeRed = matrixSize;
    if (whileCounter < 1 && matrixSize > 3) matrixSizeRed = 3;
    
    Matrix *mr = new Matrix(matrixLength, matrixSizeRed);
    Vector *tr = new Vector(matrixLength);
    Matrix *mg = new Matrix(matrixLength, matrixSizeRed);
    Vector *tg = new Vector(matrixLength);
    Matrix *mb = new Matrix(matrixLength, matrixSizeRed);
    Vector *tb = new Vector(matrixLength);

    int i = 0;
    for (int y = 0; y < dim; y++) {
      for (int x = 0; x < dim; x++) {
        if (data[dim * y + x].isPart) {
          int rx = x - sizeQ;
          int ry = y - sizeQ;
          int ii = dim * y + x;
          mr->set(i, 0, 1);
          mg->set(i, 0, 1);
          mb->set(i, 0, 1);
          tr->set(i, offset_red   + ry*my_red   + rx*mx_red   + ry*ry*myy_red   + rx*ry*mxy_red   + rx*rx*mxx_red   + ry*ry*ry*myyy_red   + rx*ry*ry*mxyy_red   + rx*rx*ry*mxxy_red   + rx*rx*rx*mxxx_red   - data[ii].r);
          tg->set(i, offset_green + ry*my_green + rx*mx_green + ry*ry*myy_green + rx*ry*mxy_green + rx*rx*mxx_green + ry*ry*ry*myyy_green + rx*ry*ry*mxyy_green + rx*rx*ry*mxxy_green + rx*rx*rx*mxxx_green - data[ii].g);
          tb->set(i, offset_blue  + ry*my_blue  + rx*mx_blue  + ry*ry*myy_blue  + rx*ry*mxy_blue  + rx*rx*mxx_blue  + ry*ry*ry*myyy_blue  + rx*ry*ry*mxyy_blue  + rx*rx*ry*mxxy_blue  + rx*rx*rx*mxxx_blue  - data[ii].b);
          if (matrixSizeRed > 2) {
            mr->set(i, 1, ry);
            mr->set(i, 2, rx);
            mg->set(i, 1, ry);
            mg->set(i, 2, rx);
            mb->set(i, 1, ry);
            mb->set(i, 2, rx);
          }
          if (matrixSizeRed > 5) {
            mr->set(i, 3, ry*ry);
            mr->set(i, 4, rx*ry);
            mr->set(i, 5, rx*rx);
            mg->set(i, 3, ry*ry);
            mg->set(i, 4, rx*ry);
            mg->set(i, 5, rx*rx);
            mb->set(i, 3, ry*ry);
            mb->set(i, 4, rx*ry);
            mb->set(i, 5, rx*rx);
          }
          if (matrixSizeRed > 9) {
            mr->set(i, 6, ry*ry*ry);
            mr->set(i, 7, rx*ry*ry);
            mr->set(i, 8, rx*rx*ry);
            mr->set(i, 9, rx*rx*rx);
            mg->set(i, 6, ry*ry*ry);
            mg->set(i, 7, rx*ry*ry);
            mg->set(i, 8, rx*rx*ry);
            mg->set(i, 9, rx*rx*rx);
            mb->set(i, 6, ry*ry*ry);
            mb->set(i, 7, rx*ry*ry);
            mb->set(i, 8, rx*rx*ry);
            mb->set(i, 9, rx*rx*rx);
          }
          i++;
        }
      }
    }

    // mr->print();
    // mg->print();
    // mb->print();

    // channel red
    Matrix *mrt = mr->transpose();
    Matrix *mrm = mrt->mul(mr);
    Vector *br = mrt->mulV(tr);
    Vector *er = mrm->cholesky(br);
    // cout << "Result:\t";
    // er->print();
    // cout << endl;

    // channel green
    Matrix *mgt = mg->transpose();
    Matrix *mgm = mgt->mul(mg);
    Vector *bg = mgt->mulV(tg);
    Vector *eg = mgm->cholesky(bg);

    // channel blue
    Matrix *mbt = mb->transpose();
    Matrix *mbm = mbt->mul(mb);
    Vector *bb = mbt->mulV(tb);
    Vector *eb = mbm->cholesky(bb);

    offset_red += er->get(0);
    offset_green += eg->get(0);
    offset_blue += eb->get(0);

    if (matrixSizeRed > 2) {
      my_red += er->get(1);
      mx_red += er->get(2);
      my_green += eg->get(1);
      mx_green += eg->get(2);
      my_blue += eb->get(1);
      mx_blue += eb->get(2);
    }
    if (matrixSizeRed > 5) {
      myy_red += er->get(3);
      mxy_red += er->get(4);
      mxx_red += er->get(5);
      myy_green += eg->get(3);
      mxy_green += eg->get(4);
      mxx_green += eg->get(5);
      myy_blue += eb->get(3);
      mxy_blue += eb->get(4);
      mxx_blue += eb->get(5);
    }
    if (matrixSizeRed > 9) {
      myyy_red += er->get(6);
      mxyy_red += er->get(7);
      mxxy_red += er->get(8);
      mxxx_red += er->get(9);
      myyy_green += eg->get(6);
      mxyy_green += eg->get(7);
      mxxy_green += eg->get(8);
      mxxx_green += eg->get(9);
      myyy_blue += eb->get(6);
      mxyy_blue += eb->get(7);
      mxxy_blue += eb->get(8);
      mxxx_blue += eb->get(9);
    }

    delete mrt;
    delete mrm;
    delete br;
    delete er;
    delete mr;
    delete tr;
    delete mgt;
    delete mgm;
    delete bg;
    delete eg;
    delete mg;
    delete tg;
    delete mbt;
    delete mbm;
    delete bb;
    delete eb;
    delete mb;
    delete tb;

    // update of the calulated values
    totalError = 0;
    int count = 0;
    for (int y = 0; y < dim; y++) {
      for (int x = 0; x < dim; x++) {
        int ii = dim * y + x;
        if (data[ii].isPart) {
          
          int rx = x - sizeQ;
          int ry = y - sizeQ;
          
          data[ii].calc_r = offset_red + ry * my_red + rx * mx_red + ry * ry * myy_red + rx * ry * mxy_red + rx * rx * mxx_red + ry * ry * ry * myyy_red + rx * ry * ry * mxyy_red + rx * rx * ry * mxxy_red + rx * rx * rx * mxxx_red;
          float error_r = data[ii].calc_r - data[ii].r;
          data[ii].calc_g = offset_green + ry * my_green + rx * mx_green + ry * ry * myy_green + rx * ry * mxy_green + rx * rx * mxx_green + ry * ry * ry * myyy_green + rx * ry * ry * mxyy_green + rx * rx * ry * mxxy_green + rx * rx * rx * mxxx_green;
          float error_g = data[ii].calc_g - data[ii].g;
          data[ii].calc_b = offset_blue + ry * my_blue + rx * mx_blue + ry * ry * myy_blue + rx * ry * mxy_blue + rx * rx * mxx_blue + ry * ry * ry * myyy_blue + rx * ry * ry * mxyy_blue + rx * rx * ry * mxxy_blue + rx * rx * rx * mxxx_blue;
          float error_b = data[ii].calc_b - data[ii].b;

          data[ii].error = sqrt(error_r * error_r + error_g * error_g + error_b * error_b);
          data[ii].dx = mx_red + mx_green + mx_blue;
          data[ii].dy = my_red + my_green + my_blue;
          count++;
          totalError += data[ii].error;
        }
      }
    }
    // printc();
    totalError /= count;
    // totalError *= static_cast<float>(dim) / static_cast<float>( count * count);
    // cout << "totalError\t" << totalError << endl;
    if (1 < polGrad && ++whileCounter < 15 && abs(totalError - lastError) > 0.1) {
      b = true;
      lastError = totalError;
    }
    // if (8 > whileCounter) b = true;
  } /* while (b) */
  // print();
  // resultf result = new resultf();
  resultf result;
  result.error = totalError;
  result.r = offset_red;
  result.g = offset_green;
  result.b = offset_blue;
  result.rx = mx_red;
  result.ry = my_red;
  result.rxx = mxx_red;
  result.rxy = mxy_red;
  result.ryy = myy_red;
  result.rxxx = mxxx_red;
  result.rxxy = mxxy_red;
  result.rxyy = mxyy_red;
  result.ryyy = myyy_red;
  result.gx = mx_green;
  result.gy = my_green;
  result.gxx = mxx_green;
  result.gxy = mxy_green;
  result.gyy = myy_green;
  result.gxxx = mxxx_green;
  result.gxxy = mxxy_green;
  result.gxyy = mxyy_green;
  result.gyyy = myyy_green;
  result.bx = mx_blue;
  result.by = my_blue;
  result.bxx = mxx_blue;
  result.bxy = mxy_blue;
  result.byy = myy_blue;
  result.bxxx = mxxx_blue;
  result.bxxy = mxxy_blue;
  result.bxyy = mxyy_blue;
  result.byyy = myyy_blue;

  return result;
}

void CalcImg::setMaskSquare(int s) {
  for (int y = -sizeQ; y <= sizeQ; y++) {
    for (int x = -sizeQ; x <= sizeQ; x++) {
      int i = getIndex(y, x);
      if (i >= 0) {
        data[i].isPart = false;
          data[i].isActive = false;
      }
    }
  }
  for (int y = -s; y <= s; y++) {
    for (int x = -s; x <= s; x++) {
      int i = getIndex(y, x);
      if (i >= 0 && data[i].isValid) {
        data[i].isPart = true;
          data[i].isActive = true;
      }
    }
  }
}

void CalcImg::setMaskRect(int width, int height, int angle) {
  float fw = static_cast<float>(width);
  float fh = static_cast<float>(height);
  float fa = static_cast<float>(angle) / 180 * M_PI;
  
  for (int y = -sizeQ; y <= sizeQ; y++) {
    for (int x = -sizeQ; x <= sizeQ; x++) {
      int i = getIndex(y, x);
      if (i >= 0) {
        data[i].isPart = false;
        data[i].isActive = false;
        
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        float tfx = cos(fa) * fx - sin(fa) * fy;
        float tfy = sin(fa) * fx + cos(fa) * fy;
        if (abs(tfx) < fw && abs(tfy) < fh) {
          data[i].isPart = true;
          data[i].isActive = true;
        }
        
      }
    }
  }
}

void CalcImg::setMaskOval(int width, int height, int angle) {
  float fw = static_cast<float>(width);
  float fh = static_cast<float>(height);
  float fa = static_cast<float>(angle) / 180 * M_PI;
  
  for (int y = -sizeQ; y <= sizeQ; y++) {
    for (int x = -sizeQ; x <= sizeQ; x++) {
      int i = getIndex(y, x);
      if (i >= 0) {
        data[i].isPart = false;
        data[i].isActive = false;
        
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        float tfx = (cos(fa) * fx - sin(fa) * fy) / fw;
        float tfy = (sin(fa) * fx + cos(fa) * fy) / fh;
        if (tfx*tfx + tfy*tfy <= 1.0) {
          data[i].isPart = true;
          data[i].isActive = true;
        }
        
      }
    }
  }
}

void CalcImg::setMaskCirc(int s) {
  for (int y = -sizeQ; y <= sizeQ; y++) {
    for (int x = -sizeQ; x <= sizeQ; x++) {
      int i = getIndex(y, x);
      if (i >= 0) {
        data[i].isPart = false;
        data[i].isActive = false;
      }
    }
  }
  for (int y = -s; y <= s; y++) {
    for (int x = -s; x <= s; x++) {
      int i = getIndex(y, x);
      // isValid wurde bei initData gesetzt
      if (i >= 0 && data[i].isValid) {
        float r = sqrt(x * x + y * y);
        if (r <= s) {
          data[i].isPart = true;
          data[i].isActive = true;
        }
      }
    }
  }
}
