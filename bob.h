//
// bob.h - part of the GAP installer BOB
//
// Copyright (C) by Max Neunhoeffer 2012
// This file is free software, see license information at the end.
//

#define BOBVERSION 13

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

#include "config.h"

using std::vector;
using std::string;

namespace BOB {

enum Status { UNKNOWN = -1, OK = 0, ADVICE = 1, WARN = 2, ERROR = 3 };

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
extern Test Which_Architecture;
extern Test Which_OS_Variant;
extern Test Which_Wordsize;
extern Test Can_Compile_32bit;
extern Test Double_Compile;

Status Have_C_Header(string headername, bool m32 = false);
Status Have_C_Library(string lib, bool m32 = false);

extern bool interactive;
extern bool osxaddpaths;
extern string origCFLAGS;
extern string origLDFLAGS;

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

bool exists(string filename);

bool isdir(string dirname);

Status downloadname(string targetname, string url, string &localname);

Status download(string url, string localname);

Status checksha1(string filename, string hash);

void get(string targetdir, string url, string &filename, bool alwaysget);
// Throws an exception of type Status in case of an error

void getind(string targetdir, string url, string &archivename);
// Throws an exception of type Status in case of an error

void unpack(string archivename);
// Throws an exception of type Status in case of an error

void sh(string cmd, int stdinfd = 0, bool quiet = false);
// Throws an exception of type Status in case of an error

int shbg(string cmd, int stdinfd = 0, bool quiet = false);

Status rmrf(string dirname);

Status cp(string from, string to);

void cd(string dir);
// Throws an exception of type Status in case of an error

void cdprefix(string dir, string &dirfound);
// dir is a prefix of a directory name in the current directory
// If this uniquely determines the directory or there is a perfect 
// match, the current directory is changed and dirfound is changed
// accordingly. Otherwise an ERROR exception is thrown.

void readlines(string filename, vector<string> &v);
// Throws ERROR if anything goes wrong.
void writelines(string filename, vector<string> &v);
// Throws ERROR if anything goes wrong.
void edit(vector<string> &edscript);
// Throws ERROR if anything goes wrong.
void edit(string edscriptpath);
// Throws ERROR if anything goes wrong.

void listdir(string dirname, vector<string> &names);
// Throws ERROR if anything goes wrong.

extern string finkpath;

}  // namespace BOB

//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

