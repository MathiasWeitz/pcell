#ifndef CLIARGUMENTS_H
#define CLIARGUMENTS_H

#include <iostream>
#include <cstring>

char* getAbsFile(char*);
char* getFilename(char*);
char* getPath(char*);
int getPathStatus(char*);

/**
 * single Parameter
 * Parameter can be 
 *  Argument like '-n'
 *  multiple Arguments like -np
 *  Arguments with value
 * 
 */
class CliArg {
  protected:
    int decimalPoints = 0;
    CliArg* value;

  public:
    char* arg;
    bool isArg = true;
    bool isBoolean = false;
    bool isHex = true;
    bool isInt = true;
    bool isFloat = true;
    bool isFile = false;
    // int fileSiz = 0;
    CliArg(char*);
    ~CliArg();
    int len();
    bool has(const std::string);
};

/**
 * handle all Arguments
 * including Files
 * should also make wildcard associations like aaa*.jpg => bbb*.jpg 
 *    where all files with the first wildcard are transferred to the second textfile with the wildcard
 */
class CliArgs {
 protected:
    // ArgFile* argFiles = nullptr;
    int index;
    int argsLen;
    char** args;

  public:
    CliArgs(int argc, char** argv);
    ~CliArgs();
    bool next();
    char* get();
    bool has(const char*);
    bool isFile(const char*);
    bool isExistingFile(const char*);
    bool isInt();
    bool isFloat();
    // int filesCount();
    // ArgFile* fileGet(int);
};

#endif
