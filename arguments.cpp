#include <limits.h>  // PATH_MAX

#include <cstring>
// test for numerisch
#include <ctype.h>
#include <iostream>
// #include <iomanip>
// round
#include <cmath>
// 
// https://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html
// https://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
// https://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/stat.h.html
// https://www.man7.org/linux/man-pages/man0/sys_stat.h.0p.html
#include <sys/stat.h>  // stat
#include <unistd.h>    // getcwd
                       // #include <dirent.h>
// reading textfile
// https://cplusplus.com/reference/fstream/ifstream/
// https://en.cppreference.com/w/cpp/io/basic_ifstream
// https://cplusplus.com/reference/fstream/ifstream/ifstream/
#include <fstream>

//  https://en.cppreference.com/w/c/chrono
//  https://en.cppreference.com/w/cpp/chrono
#include <time.h>

#include "arguments.h"
#include "editDistance.h"

using namespace std;

/****************************************
 * Hilfsroutinen
 ****************************************/
time_t readTime(const char* par) {
    // read the time from a string
  // possible formats "yy.mm.dd" "yy.mm.dd hh:mm", no trailing zero needed
#define MAXNUMBERS 10
    int i = -1;
    int value = 0;
    bool hasDigit = false;
    int valueStored[MAXNUMBERS];
    int valueStoredj = 0;
    time_t result = 0;
    while (par[++i] != '\0') {
        if (isdigit(par[i])) {
            value = 10 * value + (par[i] - '0');
            hasDigit = true;
        }
        else {
            if (hasDigit && valueStoredj < MAXNUMBERS) valueStored[valueStoredj++] = value;
            value = 0;
            hasDigit = false;
        }
        // cout << "readTime\t" << par[i] << "\t" << isdigit(par[i]) << "\t" << valueStoredj << "\t" << value << endl;
    }
    if (hasDigit && valueStoredj < MAXNUMBERS) valueStored[valueStoredj++] = value;
    // cout << "readTime\t" << valueStoredj << endl;
    
    bool isDate = true;
    tm d = {};
    if (valueStoredj > 0) {
      if (valueStored[0] < 50) valueStored[0] += 100;
      if (valueStored[0] > 1900) valueStored[0] -= 1900;
      d.tm_year = valueStored[0];
    }
    if (valueStoredj > 1) {
      if (valueStored[1] > 0 && valueStored[1] < 13) d.tm_mon = valueStored[1] - 1;
      else isDate = false;
    }
    if (valueStoredj > 2) {
      if (valueStored[2] > 0 && valueStored[2] < 32) d.tm_mday = valueStored[2];
      else isDate = false;
    }
    if (valueStoredj > 3) {
      if (valueStored[3] < 25) d.tm_hour = valueStored[3];
      else isDate = false;
    }
    if (valueStoredj > 4) {
      if (valueStored[4] < 61) d.tm_min = valueStored[4];
      else isDate = false;
    }
    if (valueStoredj > 5) {
      if (valueStored[5] < 61) d.tm_sec = valueStored[5];
      else isDate = false;
    }
    d.tm_isdst = 0;
    if (isDate) {
      result = mktime(&d);
      // char buffer[80];
      // strftime (buffer,80,"%c",&d);
      // cout << buffer << endl;

      // tm* tt = localtime(&result);
      // strftime (buffer,80,"%c",tt);
      // cout << buffer << endl;
      // strftime (buffer,80,"%y.%m.%d %H:%M",tt);
      // cout << buffer << endl;
      
    }
    return result;
}

char* strTime(const time_t* par) {
  char* buffer = new char[80];
  tm* tt = localtime(par);
  strftime (buffer,80,"%y.%m.%d %H:%M",tt);
  return buffer;
}

/****************************************
 * FileOperation
 ****************************************/

char* getAbsFile(char* localName) {
    char* path = nullptr;
    int pathLength = strlen(localName);
    // cout << "getAbsFile:\t" << localName << "\t" << pathLength << endl;
    if (*localName == '/') {
        // abs path
        path = new char[pathLength + 1];
        strcpy(path, localName);
        path[pathLength] = '\0';
    } else {
        // rel path
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            path = new char[pathLength + strlen(cwd) + 2];
            strcpy(path, cwd);
            strcat(path, "/");
            strcat(path, localName);
        }
    }
    return path;
}

char* getFilename(char* path) {
    char* fileName = nullptr;
    int pathLength = strlen(path);
    int i = pathLength - 1;
    while (i > 0 && path[i] != '/') i--;
    if (i > 0) i++;
    fileName = new char[pathLength - i + 1];
    strcpy(fileName, &path[i]);
    fileName[pathLength - i] = '\0';
    // << fileName
    // cout << "getFilename:\t" << path << "\t" << i << "\t" << endl;
    return fileName;
}

char* getPath(char* localName) {
    char* path = nullptr;
    int localNameLength = strlen(localName);
    // cout << "getPath.1:\t" << localName << "\t" << localNameLength << endl;
    if (*localName == '/') {
        int i = localNameLength - 1;
        while (i > 0 && localName[i] != '/') i--;
        // if (i > 0) i++;
        // cout << "getPath.2:\t" << localName << "\t" << i << "/" << localNameLength << endl;
        path = new char[i + 1];
        strncpy(path, localName, i);
        path[i] = '\0';
    } else {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            path = new char[strlen(cwd) + 1];
            strcpy(path, cwd);
            path[strlen(cwd)] = '\0';
        }
    }
    // cout << "getPath.3:\t" << localName << "\t" << localNameLength << "\t" << path << endl;
    return path;
}

int getPathStatus(char* localName) {
    struct stat file_stats;
    int status = stat(localName, &file_stats);
    // perror(localName);
    // cout << "getPathStatus.1:\t" << localName << "\t" << status << "\t" << file_stats.st_size << "\t" << file_stats.st_mtime << "\t" << file_stats.st_ctime << endl;
    return status;
}

/****************************************
 * CliArg
 ****************************************/ 
CliArg::CliArg(char *p) : arg(p) {
  size_t length = std::strlen(arg);
  if (0 < length && arg[0] != '-') isArg = false;
  int j = -1;
  while (arg[++j] != '\0') {
    if (!std::isxdigit(arg[j])) isHex = false;
    if (!std::isdigit(arg[j])) isInt = false;
    if (!std::isdigit(arg[j]) && arg[j] != '.') isFloat = false;
    if (arg[j] == '.') decimalPoints++;
  }
  if (1 < decimalPoints) isFloat = false;
};

CliArg::~CliArg() {
  // if (value != nullptr) delete value;
}

int CliArg::len() { return strlen(arg); };

bool CliArg::has(const std::string c) {
  bool result = false;
  return result;
}

/****************************************
 * CliArgs
 ****************************************/ 
CliArgs::CliArgs(int argc, char **argv):index(0) {
  argsLen = argc - 1;
  args = argv;
}

CliArgs::~CliArgs() {
  // if (argFiles != nullptr) delete argFiles;
}

bool CliArgs::next() { 
  // cout << "next:\t" << index << ":" << argsLen << "\t" << args[index+1] << endl;
  index++;
  return index < argsLen; 
}

char* CliArgs::get() { 
  return args[index+1];
}

bool CliArgs::has(const char* pattern) {
  int e = editDistanceGlob(pattern, args[index+1]);
  // cout << "has:\t" << pattern << "\t" << args[index+1] << "\t" << e << endl;
  return e == 0;
}

bool CliArgs::isFile(const char* pattern) { 
  // path to fileName exists
  bool result = true;
  char* path = getPath(args[index+1]);
  char* fileName = getFilename(args[index+1]);
  if (getPathStatus(path) != 0) result = false;
  if (editDistanceGlob(pattern, fileName) != 0) result = false;
  // cout << "isFile\t" << path << "\t" << fileName << "\t" << getPathStatus(path) << "\t" << editDistanceGlob(pattern, fileName) << "\t:" << result << endl;
  delete[] path;
  delete[] fileName;
  return result; }


bool CliArgs::isExistingFile(const char* pattern) {
  // like isFile but the file must exist
  bool result = true;
  char* absFile = getAbsFile(args[index+1]);
  char* fileName = getFilename(args[index+1]);
  if (getPathStatus(absFile) != 0) result = false;
  if (editDistanceGlob(pattern, fileName) != 0) result = false;
  // cout << "isExistingFile\t" << absFile << "\t" << fileName << "\t" << getPathStatus(absFile) << "\t" << editDistanceGlob(pattern, fileName) << "\t:" << result << endl;
  delete[] absFile;
  delete[] fileName;
  return result;
}

bool CliArgs::isInt() { 
  bool result = true;
  for (size_t i = 0; i < strlen(args[index+1]); i++) {
    if (!std::isdigit(args[index+1][i])) result = false;
  }
  return result; 
}

bool CliArgs::isFloat() {
  bool result = true;
  int floatPoints = 0;
  for (size_t i = 0; i < strlen(args[index+1]); i++) {
    if (!std::isdigit(args[index+1][i]) && args[index+1][i] != '.') result = false;
    if (args[index+1][i] == '.') floatPoints++;
  }
  if (floatPoints > 1) result = false;
  return result;
}

