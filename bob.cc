#include "bob.h"

#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std;

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


Test::Test(string name, int phase, testfunc_t tester)
    :name(name),phase(phase),tester(tester),num(0),res(UNKNOWN)
{
    vector<Test *>::iterator pos 
        = lower_bound(alltests().begin(),alltests().end(),this,
                      compareTestPtrs);
    alltests().insert(pos,this);
}

Test *Test::find(string name)
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

vector<Component *> &allcomps(void)
{
    static vector<Component *> *allcomponents_ = new vector<Component *>;
    return *allcomponents_;
}

bool compareComponentPtrs(Component *a, Component *b)
{
    return a->name < b->name;
}

Component::Component(string name, const char *dependencies[],
          prereqfunc prerequisites, workerfunc getter, workerfunc builder)
         : name(name), prereq(prerequisites), prereqres(UNKNOWN),
           get(getter), getres(UNKNOWN), build(builder), buildres(UNKNOWN)
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

int Component::findnr(string name)
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
            return k;
    }
    return -1;
}

Component *Component::find(string name)
{
    int pos = Component::findnr(name);
    if (pos < 0) return NULL;
    else         return allcomps().at(pos);
}

// Some standard tests:

int Have_make_Test(string &st)
{
    if (!access("/usr/bin/make",X_OK))
        return 1;
    else
        return 0;
}
Test Have_make("Have_make",1,Have_make_Test);

int Which_C_Compiler_Test(string &st)
{
    st = "gcc";
    return 1;
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

int verbose = 3;
string boblogfilename;
string buildlogfilename;

void out(Status severity, string msg)
{
    fstream outs(boblogfilename.c_str(),fstream::out | fstream::app);
    outs << msg << "\n";
    outs.close();
    if (verbose >= 3 || 
        (verbose >= 2 && severity == WARN) ||
        (verbose >= 1 && severity == ERROR)) {
        cout << "BOB:";
        if (severity == WARN) cout << "Warning:";
        else if (severity == ERROR) cout << "Error:";
        cout << msg << "\n";
    }
}

bool which(string name, string &res)
{
    size_t pos,posold;
    string absname;

    pos = name.find('/');
    if (pos == string::npos) {
        string path = getenvironment("PATH");
        pos = path.find(':');
        posold = 0;
        while (pos != string::npos) {
            absname = path.substr(posold,pos-posold);
            if (absname[absname.size()-1] != '/') absname.push_back('/');
            absname += name;
            if (!access(absname.c_str(),X_OK)) {
                res = absname;
                return true;
            }
            posold = pos;
            pos = path.find(':',pos+1);
        }
    } else {   // We are given a pathname
        if (!access(name.c_str(),X_OK)) {
            res = name;
            return true;
        } else {
            res = "";
            return false;
        }
    }
}

int get(string targetdir, string url, string &filename)
{
    return 0;
}

int getindirectly(string targetdir, string url, string &archivename)
{
    return 0;
}

int untar(string archivename)
{
    return 0;
}

int sh(string cmd)
{
    return 0;
}

}   // namespace BOB

using namespace BOB;

// The main loop:

string origdir;
string targetdir;
bool nonetwork = false;

int main(int argc, char * const argv[], char *envp[])
{
    int opt;

    // Initialise targetdir with the realpath of the current dir:
    char *cwd = getcwd(NULL,1024);
    origdir = cwd;
    free(cwd);
    if (origdir[origdir.size()-1] != '/') origdir.push_back('/');
    targetdir = origdir;

    // Initialise our environment business:
    initenvironment(envp);

    while ((opt = getopt(argc, argv, "vqnt:")) != -1) {
        switch (opt) {
          case 'v':
              verbose++;
              break;
          case 'q':
              verbose--;
              break;
          case 't':
              if (chdir(optarg)) {
                  chdir(origdir.c_str());
              } else {
                  targetdir = optarg;
                  if (targetdir[targetdir.size()-1] != '/')
                      targetdir.push_back('/');
              }
              break;
          case 'n':
              nonetwork = true;
              break;
          default: /* '?' */
              cerr << "Usage: " << argv[0] << "[-v] [-q] [-n] [-t TARGETDIR]\n";
              return -1;
        }
    }
    // At this stage we ignore further arguments.

    // Create the necessary infrastructure for logging:
    boblogfilename = targetdir;
    boblogfilename += "bob.log";
    buildlogfilename = targetdir;
    buildlogfilename += "build.log";

    fstream outs(boblogfilename.c_str(),fstream::out | fstream::trunc);
    if (outs.fail()) {
        cout << "Cannot create log file \"bob.log\" in target directory. "
             << "Stopping.\n";
        return 3;
    }
    outs.close();
    outs.open(buildlogfilename.c_str(),fstream::out | fstream::trunc);
    if (outs.fail()) {
        cout << "Cannot create log file \"build.log\" in target directory. "
             << "Stopping.\n";
        return 4;
    }
    outs.close();

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

    // Change to target directory:
    chdir(targetdir.c_str());  // has worked before, we assume it does again

    static vector<Component *> &comps = allcomps();
    Component *c,*cc;

    // First compute a total order that is compatible with the
    // dependencies:
    vector<Component *> deporder;
    vector<bool> done(comps.size(),false);

    bool somenew,cando;
    int j,pos;

    somenew = true;
    while (deporder.size() < comps.size()) {
        if (!somenew) {
            out(ERROR,"Cyclic dependencies detected. Stopping.");
            return 5;
        }
        somenew = false;
        for (i = 0;i < comps.size();i++) {
            c = comps[i];
            if (!done[i]) {
                cando = true;
                for (j = 0;j < c->depends.size();j++) {
                    pos = Component::findnr(c->depends[j]);
                    if (pos >= 0 && !done[pos]) {
                        cando = false;
                        break;
                    }
                }
                if (cando) {
                    deporder.push_back(c);
                    done[i] = true;
                    somenew = true;
                }
            }
        }
    }

    bool goterror;
    bool gotwarning;

    // Now check all the prerequisites:
    goterror = false;
    gotwarning = false;
    Status res;
    for (i = 0;i < deporder.size();i++) {
        c = deporder[i];
        res = UNKNOWN;
        // First check whether all our dependencies are happy:
        for (j = 0;j < c->depends.size();j++) {
            cc = Component::find(c->depends[j]);
            if (cc != NULL) {
                if (res < cc->prereqres) res = cc->prereqres;
            }
        }
        if (c->prereq != NULL) {
            c->prereqres = c->prereq(targetdir,res);
            if (c->prereqres == ERROR) goterror = true;
            else if (c->prereqres == WARN) gotwarning = true;
        } else if (res > OK) {
            if (res == WARN) {
                out(WARN,string("Warnings detected in dependencies of"
                                " component ")+c->name);
            } else {
                out(ERROR,string("Errors detected in dependencies of"
                                 " component ")+c->name);
            }
            c->prereqres = res;
        } else c->prereqres = OK;
    }
    if (goterror) {
        out(ERROR,"Stopping because of errors.");
        return 1;
    }
    string answer;
    if (gotwarning) {
        cout << "There have been warnings, go on regardless? ";
        cout.flush();
        cin >> answer;
        if (answer.size() == 0 || 
            (answer[0] != 'y' && answer[0] != 'Y')) {
            out(WARN,"Stopping because of warnings.");
            return 2;
        }
    }

    // Now do the actual work of getting and building:
    goterror = false;
    gotwarning = false;
    for (i = 0;i < deporder.size();i++) {
        if (goterror) {
            out(ERROR,"Stopping.");
            return 6;
        }
        c = deporder[i];
        chdir(targetdir.c_str());
        res = OK;
        if (c->get != NULL) {
            out(OK,"Getting component "+c->name);
            res = c->get(targetdir);
        }
        if (res == OK) {
            chdir(targetdir.c_str());
            out(OK,"Building component "+c->name);
            res = c->build(targetdir);
        }
        if (res == ERROR) goterror = true;
        else if (res == WARN) gotwarning = true;
        else {
            out(OK,"Successfully built component "+c->name);
        }
    }

    return 0;
}

