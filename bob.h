// bob.h - Copyright 2012 by Max Neunhoeffer

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

namespace BOB {

enum Status { OK, WARN, ERROR };

typedef int (*inttestfunc_t)(void);
typedef string (*strtestfunc_t)(void);

class Test;

vector<Test *> &alltests(void);

bool compareTestPtrs(Test *a, Test *b);

class Test {
  public:
    string name;
    int phase;
    inttestfunc_t inttest;
    strtestfunc_t strtest;
    int intres;
    string strres;

    Test(string name, int phase, inttestfunc_t tester)
        :name(name),phase(phase),inttest(tester),strtest(NULL)
    {
        vector<Test *>::iterator pos 
            = lower_bound(alltests().begin(),alltests().end(),this,
                          compareTestPtrs);
        alltests().insert(pos,this);
    }

    Test(string name, int phase, strtestfunc_t tester)
        :name(name),phase(phase),inttest(NULL),strtest(tester)
    {
        vector<Test *>::iterator pos 
            = lower_bound(alltests().begin(),alltests().end(),this,
                          compareTestPtrs);
        alltests().insert(pos,this);
    }

    void run(void)
    {
        if (inttest) intres = inttest();
        else         strres = strtest();
    }

    static Test *find(string name)
    {
        static vector<Test *> &tests = alltests();
        int i = 0;
        int j = tests.size();
        int k;
        int res;
        // invariant: the right position is >= i and < j
        while (j-i >= 1) {
            k = (i+j)/2;   // we always have i <= k < j
            res = name.compare(tests[k]->name);
            if (res < 0)        // name < tests[k]->name
                j = k;
            else if (res > 0)   // name > tests[k]->name
                i = k+1;
            else                // we found it
                return tests[k];
        }
        return NULL;
    }
};

// Some standard tests:

extern Test Have_make;
extern Test Which_C_Compiler;

// Build components:

class Component;

vector<Component *> &allcomps(void);

bool compareComponentPtrs(Component *a, Component *b);

class Component {
  public:
    typedef Status (*workerfunc)(string targetdir);

    string name;
    int phase;
    vector<string> depends;
    workerfunc prereq;
    workerfunc get;
    workerfunc build;

    Component(string name, int phase, const char *dependencies[],
              workerfunc prerequisites, workerfunc getter, workerfunc builder)
             : name(name), phase(phase), prereq(prerequisites),
               get(getter), build(builder)
    {
        int i = 0;
        const char *p;
        while (true) {
            p = dependencies[i++];
            if (!p) break;
            depends.push_back(string(p));
        }
        vector<Component *>::iterator pos 
            = lower_bound(allcomps().begin(),allcomps().end(),this,
                          compareComponentPtrs);
        allcomps().insert(pos,this);
    }

    static Component *find(string name)
    {
        static vector<Component *> &comps = allcomps();
        int i = 0;
        int j = comps.size();
        int k;
        int res;
        // invariant: the right position is >= i and < j
        while (j-i >= 1) {
            k = (i+j)/2;   // we always have i <= k < j
            res = name.compare(comps[k]->name);
            if (res < 0)        // name < tests[k]->name
                j = k;
            else if (res > 0)   // name > tests[k]->name
                i = k+1;
            else                // we found it
                return comps[k];
        }
        return NULL;
    }
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
int untar(string archivename);
int sh(string cmd);
int get(string targetdir, string url, string filename);
int getindirectly(string targetdir, string url, string archivename);
bool which(string name, string &res);

}  // namespace BOB

