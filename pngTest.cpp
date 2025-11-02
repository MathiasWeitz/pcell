/*
 * 
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
// #include <iomanip>

#include <sstream>
#include <string>
#include <cstring>
#include <vector>


#include <png.h>
#include <math.h>
#include <thread>
// #include <mutex>
#include <atomic>

#include "pngSimple.h"
#include "pngIO.h"
// #include "matrix.h"
#include "editDistance.h"
#include "arguments.h"

using namespace std;

std::atomic<int> counterAtomic;

struct ThreadParameter {
    PNGRead *image = nullptr;
    int height = 0;
    int width = 0;
    resultf* data = nullptr;
    int maskRadius = 5;
    int searchRadius = 5;
    int polynomOrder = 0;
    float distanceDemerit = 0;
};

void getCell1(CalcImg *c, const ThreadParameter &p) {
  bool b = true;
  int x;
  int y;
  while (b) {
    b = false;
    int ii = counterAtomic++;
    if (ii < p.height * p.width) {
      b = true;
      x = ii % p.width;
      y = ii / p.width;
      c->initData(p.image, y, x);
      c->setMaskCirc(p.maskRadius);
      // c->calcStep(linear);
      p.data[ii] = c->calcStep(p.polynomOrder);
    }
  }
}

void getCell2(const ThreadParameter &p) {
  bool b = true;
  int x;
  int y;
  while (b) {
    b = false;
    int ii = counterAtomic++;
    if (ii < p.height * p.width) {
      b = true;
      x = ii % p.width;
      y = ii / p.width;

      int bestIndex = y * p.width + x;
      int bestError = 9999;
      int bestX = 0;
      int bestY = 0;
      for (int ky = -p.searchRadius; ky <= p.searchRadius; ky++) {
        for (int kx = -p.searchRadius; kx <= p.searchRadius; kx++) {
          float r = sqrt(kx * kx + ky * ky);
          if (r < p.searchRadius) {
            int kkx = x + kx;
            int kky = y + ky;
            if (kkx >= 0 && kky >= 0 && kkx < p.width && kky < p.height) {
              int ik = kky * p.width + kkx;
              float distFactor = 1 + p.distanceDemerit * r;
              if (p.data[ik].error * distFactor < bestError) {
                // cout << "\tnew";
                bestError = p.data[ik].error * distFactor;
                bestIndex = ik;
                bestX = kx;
                bestY = ky;
              }
            }
          }
        }
      }
      resultf *d = &p.data[bestIndex];
      int colR = static_cast<int>(round(d->r - bestX * d->rx - bestY * d->ry - bestX * bestX * d->rxx - bestX * bestY * d->rxy - bestY * bestY * d->ryy - bestX * bestX * bestX * d->rxxx - bestX * bestX * bestY * d->rxxy - bestX * bestY * bestY * d->rxyy - bestY * bestY * bestY * d->ryyy));
      int colG = static_cast<int>(round(d->g - bestX * d->gx - bestY * d->gy - bestX * bestX * d->gxx - bestX * bestY * d->gxy - bestY * bestY * d->gyy - bestX * bestX * bestX * d->gxxx - bestX * bestX * bestY * d->gxxy - bestX * bestY * bestY * d->gxyy - bestY * bestY * bestY * d->gyyy ));
      int colB = static_cast<int>(round(d->b - bestX * d->bx - bestY * d->by - bestX * bestX * d->bxx - bestX * bestY * d->bxy - bestY * bestY * d->byy - bestX * bestX * bestX * d->bxxx - bestX * bestX * bestY * d->bxxy - bestX * bestY * bestY * d->bxyy - bestY * bestY * bestY * d->byyy ));

      if (colR < 0) colR = 0;
      if (colR > 255) colR = 255;
      if (colG < 0) colG = 0;
      if (colG > 255) colG = 255;
      if (colB < 0) colB = 0;
      if (colB > 255) colB = 255;
      p.data[ii].targetR = colR;
      p.data[ii].targetG = colG;
      p.data[ii].targetB = colB;
      // pw->set(y, x, colR, colG, colB);
    }
  }
}

void getCellError(const ThreadParameter &p) {
  bool b = true;
  while (b) {
    b = false;
    int ii = counterAtomic++;
    if (ii < p.height * p.width) {
      b = true;
      int c = static_cast<int>(round(p.data[ii].error * 5 - 5));
      if (c < 0) c = 0;
      if (c > 255) c = 255;
      p.data[ii].targetR = c;
      p.data[ii].targetG = c;
      p.data[ii].targetB = c;
      // pw->set(y, x, colR, colG, colB);
    }
  }
}


int main(int argc, char **argv) {

  #if 0
  // Test Colorcodes
  for (int i = 0; i < 256; i++) {
    cout << "\033[38;5;" << i << "m\t" << i << " Color \033[0m";
    if (i % 16 == 15) {
       cout << endl;
    }
  }
  cout << endl << endl;
  
  for (int i = 0; i < 256; i++) {
    cout << "\033[48;5;" << i << "m\t" << i << " Color \033[0m";
    if (i % 16 == 15) {
       cout << endl;
    }
  }
  cout << endl << endl;
#endif

#if 0
    // pcell Test for one Pixel of the image
    // 
    // calculates on Point in the Matrix
  
    PNGRead *p = new PNGRead("/home/mathias/tmp/test/svgCreatedImg/svg_02.png");
    
    cout << "imgDim:\t" << p->getHeight() << "\t"  << p->getWidth() << "\t" << endl;
    
    // 114,145,135
    // std::tuple<int, int, int> m = p->get(245, 194);
    // std::cout << "R: " << get<0>(m) << ", G: " << get<1>(m) << ", B: " << get<2>(m) << std::endl;

    int radius = 8;
    CalcImg *c = new CalcImg(radius);
    c->setMaskCirc(radius);
    // c->initData(p,336,179);
    // c->initData(p,357,161);
    // c->initData(p,0,0);
    // c->initData(p,399,599);
    // c->initData(p,34,36);
    
    c->initData(p,31,31);
    c->setMaskCirc(radius);
    // c->print();
    c->calcStep(0);
    // c->print();
    c->printc();
    
    delete c;
    delete p;
#endif

#if 0
    // Test count cpu
    unsigned int cpu_count = std::thread::hardware_concurrency();
    if (cpu_count == 0)
        std::cout << "Could not detect number of CPU cores.\n";
    else
        std::cout << "Number of CPU cores: " << cpu_count << std::endl;

#endif

#if 0
  // test cli-parameters
  // pcell main with circle trheads and cli-parameter
  // pcell -r 5 -p 0 -s 4 "/home/mathias/tmp/test/testVideos/t12.png"  "/home/mathias/tmp/test/tmpFilter15/result_%.png"
  
  
   CliArgs cliArgs = CliArgs(argc, argv);
   
   int radius = 3;
   int polynom = 1;
   int radiusSearch = 2;
   float distanceFactor = 0.0;
   char* sourceFile = nullptr;
   char* targetFile = nullptr;
   
    int processorUsedParallel = std::thread::hardware_concurrency() - 1;
   
    //cout << "test 1: " << endl;
    
   // read all the cli parameters
    bool b = true;
    bool argOpen = true;
    while (b) {
        b = false;
        argOpen = true;
        if (argOpen && cliArgs.has("-*r*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                radius = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*p*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                polynom = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*s*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                radiusSearch = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*d*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isFloat()) {
                distanceFactor = atof(cliArgs.get()) / 250;
            }
        }
        // cout << "int: " << cliArgs.isInt() << "\tfloat: " << cliArgs.isFloat() << endl;
        if (argOpen) {
            if (cliArgs.isExistingFile("*.png")) {
              argOpen = false;
              sourceFile = cliArgs.get();
            }
        }
        if (argOpen) {
            if (cliArgs.isFile("*.png")) {
              argOpen = false;
              targetFile = cliArgs.get();
            }
        }
        b = cliArgs.next();
    }
    
    cout << "radius: " << radius << "\tradiusSearch: " << radiusSearch << "\tpolynom: " << polynom << "\t" << distanceFactor << "\t" << sourceFile << "\t" << targetFile << endl;
    if (sourceFile != nullptr && targetFile != nullptr) {
      cout << "radius: " << radius << "\tradiusSearch: " << radiusSearch << "\tpolynom: " << polynom << "\t" << distanceFactor << "\t" << sourceFile << "\t" << targetFile << "\t" << strlen(targetFile) << endl;
      
      bool hasSearchradiusWildcard = false;
      for (size_t i = 0; i < strlen(targetFile); i++) {
        if (targetFile[i] == '%') hasSearchradiusWildcard = true;
      }
      
      char* targetFileProcessed = new char[strlen(targetFile) + 50];
      int iSource = 0;
      int iTarget = 0;
      while (targetFile[iSource] != '\0') {
        // cout << targetFile[iSource] << endl;
        targetFileProcessed[iTarget] = targetFile[iSource];
        if (targetFile[iSource] == '%') {
          char formattedInteger[10];
          int iRepl = 0;
          snprintf(formattedInteger, sizeof(formattedInteger), "%0*d", 6, radiusSearch);
          while (formattedInteger[iRepl] != '\0') {
            targetFileProcessed[iTarget] = formattedInteger[iRepl];
            iRepl++;
            iTarget++;
          }
          iTarget--;
        }
        
        iSource++;
        iTarget++;
      }
      targetFileProcessed[iTarget] = targetFile[iSource];
      cout << targetFileProcessed << endl;
      
      delete[] targetFileProcessed;
      
      
      
    }
  
  
#endif
  
#if 1
  // pcell main with circle trheads and cli-parameter
  //
  // /home/mathias/Projects/cpp/07_png/pngTest -r 5 -p 0 -s 4 /home/mathias/Projects/cpp/07_png/motiv_01a.png /home/mathias/Projects/cpp/07_png/parTest.png
  
   CliArgs cliArgs = CliArgs(argc, argv);
   
   int radius = 3;
   int polynom = 1;
   int radiusSearch = 2;
   float distanceFactor = 0.0;
   char* sourceFile = nullptr;
   char* targetFile = nullptr;
   
   bool hasSearchradiusWildcard = false;
   int searchBatchSize = 1;
      
    int processorUsedParallel = std::thread::hardware_concurrency() - 1;
   
   // read all the cli parameters
    bool b = true;
    bool argOpen = true;
    while (b) {
        b = false;
        argOpen = true;
        if (argOpen && cliArgs.has("-*r*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                radius = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*p*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                polynom = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*s*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                radiusSearch = atoi(cliArgs.get());
            }
        }
        if (argOpen && cliArgs.has("-*d*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isFloat()) {
                distanceFactor = atof(cliArgs.get()) / 250;
            }
        }
        if (argOpen && cliArgs.has("-*b*")) {
            argOpen = false;
            cliArgs.next();
            if (cliArgs.isInt()) {
                searchBatchSize = atoi(cliArgs.get());
            }
        }
        // cout << "int: " << cliArgs.isInt() << "\tfloat: " << cliArgs.isFloat() << endl;
        if (argOpen) {
            if (cliArgs.isExistingFile("*.png")) {
              argOpen = false;
              sourceFile = cliArgs.get();
            }
        }
        if (argOpen) {
            if (cliArgs.isFile("*.png")) {
              argOpen = false;
              targetFile = cliArgs.get();
            }
        }
        b = cliArgs.next();
    }

    if (sourceFile != nullptr && targetFile != nullptr) {
      // cout << "radius: " << radius << "\tradiusSearch: " << radiusSearch << "\tpolynom: " << polynom << "\t" << distanceFactor << "\t" << sourceFile << "\t" << targetFile << endl;
      for (size_t i = 0; i < strlen(targetFile); i++) {
        if (targetFile[i] == '%') hasSearchradiusWildcard = true;
      }
      
      PNGRead *imageRead = new PNGRead(sourceFile);
      int h = imageRead->getHeight();
      int w = imageRead->getWidth();

      // the program have to steps,
      // the result of the first is saved here
      resultf *dataOfCalculation = (resultf *)malloc(sizeof(resultf) * h * w);
      
      ThreadParameter threadParameter;
      threadParameter.image = imageRead;
      threadParameter.width = w;
      threadParameter.height = h;
      threadParameter.data = dataOfCalculation;
      threadParameter.maskRadius = radius;
      threadParameter.searchRadius = radiusSearch;
      threadParameter.polynomOrder = polynom;
      threadParameter.distanceDemerit = distanceFactor;
      
      // for every thread it's own CalcImg Instance
      CalcImg *segment[processorUsedParallel];
      for (int i = 0; i < processorUsedParallel; ++i) {
        segment[i] = new CalcImg(radius);
      }
      // ********************************* Step 1
      counterAtomic = 0;
      std::thread *threads1 = new std::thread[processorUsedParallel];
      for (int i = 0; i < processorUsedParallel; ++i) {
        threads1[i] = std::thread(getCell1, segment[i], threadParameter);
      }
      for (int i = 0; i < processorUsedParallel; ++i) {
        threads1[i].join();
        delete segment[i];
        segment[i] = nullptr;
      }
      delete[] threads1;
      delete imageRead;
      
      // ********************************* Step 2
      if (!hasSearchradiusWildcard) searchBatchSize = 1;
      if (searchBatchSize < 1) searchBatchSize = 1;
      for (int radiusSearchOffset = 0; radiusSearchOffset < searchBatchSize; radiusSearchOffset++) {
        counterAtomic = 0;
        threadParameter.searchRadius = radiusSearch + radiusSearchOffset;
        
        // replace wildcards for targetFile with parameters
        char* targetFileProcessed = new char[strlen(targetFile) + 50];
        int iSource = 0;
        int iTarget = 0;
        while (targetFile[iSource] != '\0') {
          // cout << targetFile[iSource] << endl;
          targetFileProcessed[iTarget] = targetFile[iSource];
          if (targetFile[iSource] == '%') {
            char formattedInteger[10];
            int iRepl = 0;
            snprintf(formattedInteger, sizeof(formattedInteger), "%0*d", 2, threadParameter.searchRadius);
            while (formattedInteger[iRepl] != '\0') {
              targetFileProcessed[iTarget] = formattedInteger[iRepl];
              iRepl++;
              iTarget++;
            }
            iTarget--;
          }
          iSource++;
          iTarget++;
        }
        targetFileProcessed[iTarget] = targetFile[iSource];
        bool result = false;
        if (getPathStatus(targetFileProcessed) != 0) result = true;
        // cout << "isFile\t" << targetFileProcessed << "\t" << result << "\t" << getPathStatus(targetFileProcessed) << "\thasSearchwildcard:" << hasSearchradiusWildcard << endl;
        if (result) {
          std::thread *threads2 = new std::thread[processorUsedParallel];

          for (int i = 0; i < processorUsedParallel; ++i) {
            threads2[i] = std::thread(getCell2, threadParameter); 
            // erzeugt die Fehlerwerte
            // threads2[i] = std::thread(getCellError, threadParameter);
          }
          for (int i = 0; i < processorUsedParallel; ++i) {
            threads2[i].join();
          }
          delete[] threads2;
          
          PNGWrite *imageWrite = new PNGWrite(targetFileProcessed);
          imageWrite->setHeight(h);
          imageWrite->setWidth(w);
          for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
              int ii = y * w + x;
              imageWrite->set(y, x, dataOfCalculation[ii].targetR, dataOfCalculation[ii].targetG, dataOfCalculation[ii].targetB);
            }
          }
          imageWrite->write();
          delete imageWrite;
        }
        delete[] targetFileProcessed;
      }
      
      free(dataOfCalculation);
      // delete[] sourceFile;
      // delete[] targetFile;
    }

#endif 
  
   #if 0
  // kuwahara 2 with circle trheads
  //
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_01.png"); // a
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_03.png"); // aa
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_20.png"); // ab
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_26c.png"); // ac
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_26e.png"); // ad
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_31a.png"); // ae
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32a1.png"); // ba
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32e1.png"); // bb
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32f.png"); // bc
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32h.png"); // bd
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32r.png"); // ca
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32s.png"); // cb
    
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/drawing_01b.png"); // da
  PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/drawing_01c.png"); // da
  // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_01b.png"); // da
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32e1.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32d.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32r.png"); // => v
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32s.png"); // => w
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32t.png"); // => x
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32u.png"); => y
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32w.png"); => z
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32x.png"); => aa
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32y.png"); => ab
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32z.png"); => ac
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_33a.png"); => ad
    int h = p->getHeight();
    int w = p->getWidth();

    cout << "imgDim:\t" << h << "\t" << w << "\t" << endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    // 2,3,5,8,13,21
    std::vector<int> rradius = {2,3,5,8,13,21};
    for (int radius : rradius) {
      //int radius = 21;
      int krMax = radius+4;
      int linearMax = 4;
      if (radius < 3) linearMax = 3;
      for (int tlinear = 0; tlinear < linearMax; tlinear++) {
        int n = 7;
        
        // for every thread it's own CalcImg Instance
        CalcImg *c[n]; 
        for (int i = 0; i < n; ++i) {
          c[i] = new CalcImg(radius);
        }
        
        int maskCirc = radius;
        resultf *cR = (resultf *)malloc(sizeof(resultf) * h * w);
        
        // parallel start 1
        counterAtomic = 0;
        std::thread* threads1 = new std::thread[n];
        for (int i = 0; i < n; ++i) {
          threads1[i] = std::thread(getCell1, p, h, w, cR, c[i], radius, tlinear);
        }
        for (int i = 0; i < n; ++i) {
          threads1[i].join();
        }
        delete[] threads1;

        // cout << "maxError\t" << maxError << endl;
        // cout << "result\t" << d->r << " " << d->rx
        
        for (int kr = 1; kr < krMax; kr++) {
          std::ostringstream oss;
          oss.clear();
          oss << "/home/mathias/Projects/cpp/07_png/test_db_";
          
          oss.fill('0');
          oss.width(2);
          oss << maskCirc << "_";

          oss.fill('0');
          oss.width(2);
          oss << kr << "_";

          oss.fill('0');
          oss.width(1);
          oss << tlinear << "_";
          
          oss << ".png";

          string str = oss.str();
          char *cstr = strdup(str.c_str());
          PNGWrite *pw = new PNGWrite(cstr);
          pw->setHeight(h);
          pw->setWidth(w);

          // int bestIndex;
          // int bestX;
          // int bestY;
          // float bestError;
          counterAtomic = 0;
          std::thread* threads2 = new std::thread[n];

#if 0
          for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
              bestIndex = y * w + x;
              bestError = 9999;
              bestX = 0;
              bestY = 0;
              for (int ky = -kr; ky <= kr; ky++) {
                for (int kx = -kr; kx <= kr; kx++) {
                  float r = sqrt(kx * kx + ky * ky);
                  if (r < kr) {
                    int kkx = x + kx;
                    int kky = y + ky;
                    if (kkx >= 0 && kky >= 0 && kkx < w && kky < h) {
                      int ik = kky * w + kkx;
                      // cout << "result\t" << y << "," << x << "\t" << kky << "," << kkx << "\t" << cR[ik].error;
                      if (cR[ik].error < bestError) {
                        // cout << "\tnew";
                        bestError = cR[ik].error;
                        bestIndex = ik;
                        bestX = kx;
                        bestY = ky;
                      }
                      // cout << endl;
                    }
                  }
                }
              }
              resultf *d = &cR[bestIndex];
              int colR = static_cast<int>(round(d->r - bestX * d->rx - bestY * d->ry));
              int colG = static_cast<int>(round(d->g - bestX * d->gx - bestY * d->gy));
              int colB = static_cast<int>(round(d->b - bestX * d->bx - bestY * d->by));

              if (colR < 0) colR = 0;
              if (colR > 255) colR = 255;
              if (colG < 0) colG = 0;
              if (colG > 255) colG = 255;
              if (colB < 0) colB = 0;
              if (colB > 255) colB = 255;

              pw->set(y, x, colR, colG, colB);
            }
          }
#endif
          for (int i = 0; i < n; ++i) {
            threads2[i] = std::thread(getCell2, cR, h, w, kr);
          }
          for (int i = 0; i < n; ++i) {
            threads2[i].join();
          }
          delete[] threads2;
          
          for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
              int ii  = y * w + x;
              pw->set(y, x, cR[ii].targetR, cR[ii].targetG, cR[ii].targetB);
            }
          }
          pw->write();
          delete pw;
          free(cstr);
        }
        
        // delete d;
        // c->print();

        // std::tuple<int, int, int> result = c->getValue();
        // std::cout << "R: " << get<0>(result) << ", G: " << get<1>(result) << ", B: " << get<2>(result) << std::endl;

        // pw->set(y, x, get<0>(result), get<1>(result), get<2>(result));
        // delete[] *c;
        for (int i = 0; i < n; ++i) {
          delete c[i];
        }
        /*
        for (int y = 0; y < h; y++) {
          for (int x = 0; x < w; x++) {
            int ii = y * w + x;
            // delete cR[ii];
          }
        }
        */
        free(cR);
      }
    }

    delete p;
    
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate elapsed time in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Elapsed time: " << duration.count() << " ms" << std::endl;

#endif
    
  #if 0
    // kuwahara 2 with circle
    // 
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_01a.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_11.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/img_03c.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32d.png");
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32r.png"); // => v
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32s.png"); // => w
    PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32t.png"); // => x
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32u.png"); => y
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32w.png"); => z
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32x.png"); => aa
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32y.png"); => ab
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_32z.png"); => ac
    // PNGRead *p = new PNGRead("/home/mathias/Projects/cpp/07_png/motiv_33a.png"); => ad
    int h = p->getHeight();
    int w = p->getWidth();

    cout << "imgDim:\t" << h << "\t" << w << "\t" << endl;

    // 114,145,135
    // std::tuple<int, int, int> m = p->get(245, 194);
    // std::cout << "R: " << get<0>(m) << ", G: " << get<1>(m) << ", B: " << get<2>(m) << std::endl;

    // 13,8,5,3,2
    std::vector<int> rradius = {13,8,5,3,2};
    for (int radius : rradius) {
      //int radius = 21;
      int krMax = radius+4;
      
      for (int tlinear = 0; tlinear < 2; tlinear++) {
        CalcImg *c = new CalcImg(radius);
        int maskCirc = radius;
        bool linear = tlinear == 1;
        resultf *cR = (resultf *)malloc(sizeof(resultf) * h * w);

        // c->initData(p,336,179);
        // c->initData(p,357,161);
        // c->initData(p,0,0);
        // c->initData(p,399,599);
        // c->initData(p,testY,testX);
        // c->initData(p,19,63);
        // c->setMaskCirc(5);
        // c->print();
        // c->calcStep(false);
        // c->print();
        int ii;
        float maxError = 0;
        for (int y = 0; y < h; y++) {
          for (int x = 0; x < w; x++) {
            c->initData(p, y, x);
            c->setMaskCirc(maskCirc);
            ii = y * w + x;
            cR[ii] = c->calcStep(linear);
            if (cR[ii].error > maxError) {
              maxError = cR[ii].error;
            }
          }
        }

        // cout << "maxError\t" << maxError << endl;
        // cout << "result\t" << d->r << " " << d->rx

        for (int kr = 1; kr < krMax; kr++) {
          std::ostringstream oss;
          oss.clear();
          oss << "/home/mathias/Projects/cpp/07_png/test_x3_";

          oss.fill('0');
          oss.width(1);
          oss << (linear ? "1" : "0") << "_";

          oss.fill('0');
          oss.width(2);
          oss << maskCirc << "_";

          oss.fill('0');
          oss.width(2);
          oss << kr << "_";

          oss << ".png";

          string str = oss.str();
          char *cstr = strdup(str.c_str());
          PNGWrite *pw = new PNGWrite(cstr);
          pw->setHeight(h);
          pw->setWidth(w);

          int bestIndex;
          int bestX;
          int bestY;
          float bestError;
          for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
              bestIndex = y * w + x;
              bestError = 9999;
              bestX = 0;
              bestY = 0;
              for (int ky = -kr; ky <= kr; ky++) {
                for (int kx = -kr; kx <= kr; kx++) {
                  float r = sqrt(kx * kx + ky * ky);
                  if (r < kr) {
                    int kkx = x + kx;
                    int kky = y + ky;
                    if (kkx >= 0 && kky >= 0 && kkx < w && kky < h) {
                      int ik = kky * w + kkx;
                      // cout << "result\t" << y << "," << x << "\t" << kky << "," << kkx << "\t" << cR[ik].error;
                      if (cR[ik].error < bestError) {
                        // cout << "\tnew";
                        bestError = cR[ik].error;
                        bestIndex = ik;
                        bestX = kx;
                        bestY = ky;
                      }
                      // cout << endl;
                    }
                  }
                }
              }
              // int index = h * y + x;
              // float r = sqrt((y-testY)*(y-testY) + (x-testX)*(x-testX));
              // ii = y * w + x;
              // cout << "result\t" << ii << "\t" << bestIndex << endl;
              ii = bestIndex;
              resultf *d = &cR[ii];
              int colR = static_cast<int>(round(d->r - bestX * d->rx - bestY * d->ry));
              int colG = static_cast<int>(round(d->g - bestX * d->gx - bestY * d->gy));
              int colB = static_cast<int>(round(d->b - bestX * d->bx - bestY * d->by));

              // int colR = static_cast<int>(round(d->r));
              // int colG = static_cast<int>(round(d->g));
              // int colB = static_cast<int>(round(d->b));

              // int colR = static_cast<int>(round(d->error * 255 / maxError));
              // int colG = static_cast<int>(round(d->error * 255 / maxError));
              // int colB = static_cast<int>(round(d->error * 255 / maxError));

              if (colR < 0) colR = 0;
              if (colR > 255) colR = 255;
              if (colG < 0) colG = 0;
              if (colG > 255) colG = 255;
              if (colB < 0) colB = 0;
              if (colB > 255) colB = 255;

              pw->set(y, x, colR, colG, colB);
            }
          }
          
          pw->write();
          delete pw;
          free(cstr);
        }
        // delete d;
        // c->print();

        // std::tuple<int, int, int> result = c->getValue();
        // std::cout << "R: " << get<0>(result) << ", G: " << get<1>(result) << ", B: " << get<2>(result) << std::endl;

        // pw->set(y, x, get<0>(result), get<1>(result), get<2>(result));
        delete c;
        for (int y = 0; y < h; y++) {
          for (int x = 0; x < w; x++) {
            ii = y * w + x;
            // delete cR[ii];
          }
        }
        free(cR);
      }
    }

    delete p;

#endif
    
    return 0;
}
