// bob.h - Copyright 2012 by Max Neunhoeffer

#define BOBVERSION 1

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

#include "config.h"

using std::vector;
using std::string;

namespace BOB {

enum Status { UNKNOWN = -1, OK = 0, WARN = 1, ERROR = 2 };

typedef int (*testfunc_t)(string &st);

class Test;

vector<Test *> &alltests(void);

bool compareTestPtrs(Test *a, Test *b);

class Test {
  public:
    string name;
    int phase;
    testfunc_t tester;
    int num;
    string str;
    Status res;

    Test(string name, int phase, testfunc_t tester);

    void run(void)
    {
        num = tester(str);
        res = OK;
    }

    static Test *find(string name);
};

// Some standard tests:

extern Test Have_make;
extern Test Which_C_Compiler;
extern Test C_Compiler_Name;
extern Test Which_Wordsize;

// Build components:

class Component;

vector<Component *> &allcomps(void);

bool compareComponentPtrs(Component *a, Component *b);

class Component {
  public:
    typedef Status (*workerfunc)(string targetdir);
    typedef Status (*prereqfunc)(string targetdir, Status depsresult);

    string name;
    vector<string> depends;
    prereqfunc prereq;
    Status prereqres;
    workerfunc get;
    Status getres;
    workerfunc build;
    Status buildres;

    Component(string name, const char *dependencies[],
              prereqfunc prerequisites, workerfunc getter, workerfunc builder);

    static int findnr(string name);
    static Component *find(string name);
};

// Access to the environment:

extern vector<string> envkeys;
extern vector<string> envvals;

void initenvironment();
string getenvironment(string key);
void setenvironment(string key, string val);
void delenvironment(string key);

// Some utility functions:

void out(Status severity, string msg);
bool which(string name, string &res);
Status downloadname(string targetname, string url, string &localname);
Status download(string url, string localname);
Status checksha1(string filename, string hash);
Status get(string targetdir, string url, string &filename, bool alwaysget);
Status getind(string targetdir, string url, string &archivename);
Status unpack(string archivename);
Status sh(string cmd);

}  // namespace BOB

