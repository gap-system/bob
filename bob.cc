#include "bob.h"

#include <unistd.h>
#include <iostream>

namespace BOB {

vector<Test *> &alltests(void)
{
    static vector<Test *> *alltests_ = new vector<Test *>;
    return *alltests_;
}

bool compareTestPtrs(Test *a, Test *b)
{
    return a->name < b->name;
}

vector<Component *> &allcomps(void)
{
    static vector<Component *> *allcomponents_ = new vector<Component *>;
    return *allcomponents_;
}

bool compareComponentPtrs(Component *a, Component *b)
{
    return a->name < b->name;
}

// Some standard tests:

int Have_make_Test()
{
    if (access("/usr/bin/make",X_OK))
        return 0;
    else
        return 1;
}
Test Have_make("Have_make",1,Have_make_Test);

string Which_C_Compiler_Test()
{
    return "gcc";
}
Test Which_C_Compiler("Which_C_Compiler",1,Which_C_Compiler_Test);

// Access to the environment:

vector<string> envkeys;
vector<string> envvals;

void initenvironment(char *environ[])
{
    char *p,*q;
    int i;
    string key;
    string val;
    vector<string>::iterator pos;
    for (i = 0,p = environ[0];p;p = environ[++i]) {
        q = strchr(p,'=');
        key = string(p,0,q-p);
        val = string(q+1);
        pos = lower_bound(envkeys.begin(),envkeys.end(),key);
        envvals.insert(envvals.begin() + (pos - envkeys.begin()),val);
        envkeys.insert(pos,key);
    }
}

string getenvironment(string key)
{
    vector<string>::iterator pos;
    pos = lower_bound(envkeys.begin(),envkeys.end(),key);
    if (pos >= envkeys.end() || *pos != key) {
        return "";
    } else
        return envvals[pos - envkeys.begin()];
}

void setenvironment(string key, string val)
{
    vector<string>::iterator pos;
    pos = lower_bound(envkeys.begin(),envkeys.end(),key);
    if (pos >= envkeys.end() || *pos != key) {
        envvals.insert(envvals.begin() + (pos-envkeys.begin()),val);
        envkeys.insert(pos,key);
    } else
        envvals[pos-envkeys.begin()] = val;
}

void delenvironment(string key)
{
    vector<string>::iterator pos;
    pos = lower_bound(envkeys.begin(),envkeys.end(),key);
    if (pos < envkeys.end() && *pos == key) {
        envvals.erase(envvals.begin() + (pos - envkeys.begin()));
        envkeys.erase(pos);
    }
}

char **prepareenvironment()
{
    char **p = new char * [envkeys.size()+1];
    char **pp = p;
    char *q;
    int i;
    for (i = 0;i < envkeys.size();i++) {
        q = (char *) malloc(envkeys[i].size()+2+envvals[i].size());
        strcpy(q,envkeys[i].c_str());
        q[envkeys[i].size()] = '=';
        strcpy(q+envkeys[i].size()+1,envvals[i].c_str());
        *pp++ = q;
    }
    *pp++ = NULL;
    return p;
}

void freepreparedenvironment(char **p)
{
    int i = 0;
    char *q;
    while (true) {
        q = p[i++];
        if (q == NULL) break;
        free(q);
    }
    delete[] p;
}

// Utility functions:

int verbose = 2;

void out(Status severity, string msg)
{
}

bool which(string name, string &res)
{
    res = "";
    return false;
}

int sh(string cmd)
{
    return 0;
}

int get(string targetdir, string url, string filename)
{
    return 0;
}

int getindirectly(string targetdir, string url, string archivename)
{
    return 0;
}

int untar(string archivename)
{
    return 0;
}

}   // namespace BOB

using namespace BOB;

// The main loop:

string targetdir;

int main(int argc, char * const argv[], char *envp[])
{
    int opt;

    // Initialise targetdir with the realpath of the current dir:
    char *cwd = getcwd(NULL,1024);
    targetdir = cwd;
    free(cwd);

    // Initialise our environment business:
    initenvironment(envp);

    while ((opt = getopt(argc, argv, "vqt:")) != -1) {
        switch (opt) {
          case 'v':
              verbose++;
              break;
          case 'q':
              verbose--;
              break;
          case 't':
              targetdir = optarg;
              break;
          default: /* '?' */
              cerr << "Usage: " << argv[0] << "[-v] [-q] [-t TARGETDIR]\n";
              return -1;
        }
    }
    // At this stage we ignore further arguments.

    static vector<Test *> &tests = alltests();
    Test *t;
    int p,i;
    for (p = 1;p < 9;p++) {
        for (i = 0;i < tests.size();i++) {
            t = tests[i];
            if (t->phase == p) {
                t->run();
            }
        }
    }
    t = Test::find("Have_make");
    cout << "Do we have make? Answer: " << t->intres << "\n";
    return 0;
}

