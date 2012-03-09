#include "bob.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>

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
    if (which("make",st)) return 0;
    else {
        st = "";
        return -1;
    }
}
Test Have_make("Have_make",1,Have_make_Test);

int Which_C_Compiler_Test(string &st)
{
    string CC = getenvironment("CC");
    if (CC.size() > 0 && which(CC,st)) return 0;
    if (which("gcc",st)) return 0;
    if (which("cc",st)) return 0;
    if (which("CC",st)) return 0;
    st = "";
    return -1;
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
            if (access(absname.c_str(),X_OK) == 0) {
                res = absname;
                return true;
            }
            posold = pos+1;
            pos = path.find(':',pos+1);
        }
    } else {   // We are given a pathname
        if (access(name.c_str(),X_OK) == 0) {
            res = name;
            return true;
        } else {
            res = "";
            return false;
        }
    }
}

static CURL *curl = NULL;

void shutdowncurl()
{
    if (curl) {
        curl_easy_cleanup(curl);
        curl = NULL;
        curl_global_cleanup();
    }
}

Status get(string targetdir, string url, string &filename)
{
    CURLcode res;

    if (curl == NULL) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl == NULL) {
            out(ERROR,"Cannot initialize curl library.");
            return ERROR;
        }
        atexit(shutdowncurl);
    }
    size_t pos = url.rfind('/');
    if (pos == string::npos) {
        out(ERROR,"Given URL does not contain /.");
        return ERROR;
    }
    filename = targetdir+"download/"+url.substr(pos+1);
    if (access(filename.c_str(),R_OK) == 0) {
        out(OK,"Already have "+url);
        return OK;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    FILE *outs = fopen(filename.c_str(),"w");
    if (outs == NULL) {
        out(ERROR,"Cannot write to "+filename+" .");
        return ERROR;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outs); 
    if (verbose >= 3)
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    else
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

    res = curl_easy_perform(curl);
    if (res != 0) {
        stringstream msg(stringstream::out);
        msg << "Curl error " << res << ".";
        out(ERROR,msg.str());
        fclose(outs);
        return ERROR;
    }
    if (fclose(outs) != 0) {
        out(ERROR,"Cannot close "+filename+" .");
        return ERROR;
    }

    return OK;
}

Status getindirectly(string targetdir, string url, string &archivename)
{
    string filename;
    string url2;
    string hash;
    Status res;

    res = get(targetdir, url, filename);
    if (res == ERROR) return ERROR;
    fstream file;
    file.exceptions ( ifstream::failbit | ifstream::badbit );
    try {
        file.open(filename.c_str(),fstream::in);
    }
    catch (ifstream::failure e) {
        out(ERROR,"Could not write link file "+filename+" .");
        return ERROR;
    }
    try {
        getline(file,url2);
        if (url2 != "BOB") {
            file.close();
            out(ERROR,"Link file "+filename+" corrupt.");
            return ERROR;
        }
        getline(file,url2);
        getline(file,hash);
        file.close();
    } 
    catch (ifstream::failure e) {
        file.close();
        out(ERROR,"Could not read link file "+filename+" .");
        return ERROR;
    }
    return get(targetdir, url2, archivename);
}

static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF) return ARCHIVE_OK;
        if (r != ARCHIVE_OK) return r;
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            out(ERROR,string("Unpack write error: ")+
                      archive_error_string(aw));
            return r;
        }
    }
}

Status unpack(string archivename)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;
    const char *filename = archivename.c_str();
    Status res = ERROR;

    a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_format_zip(a);
    archive_read_support_compression_gzip(a);
    archive_read_support_compression_compress(a);
    if (filename != NULL && strcmp(filename, "-") == 0) filename = NULL;
    if ((r = archive_read_open_filename(a, filename, 10240))) {
        out(ERROR,"Cannot read archive "+archivename+"\nMessage: "+
            archive_error_string(a));
        archive_read_finish(a);
        return ERROR;
    }
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);
    while (true) {   // will be left by break
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) { res = OK; break; }
        if (r != ARCHIVE_OK) {
            out(ERROR,"Bad entry in archive "+archivename+"\nMessage: "+
                      archive_error_string(a));
            break;
        }
        if (verbose >= 4) out(OK,string("x ")+archive_entry_pathname(entry));
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            out(ERROR,"Could not write header for entry extracting "+
                      archivename+"\nMessage: "+
                      archive_error_string(ext));
            break;
        } else {
            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK) {
                out(ERROR,"Could not finish extraction of "+
                          archivename+"\nMessage: "+
                          archive_error_string(ext));
                break;
            }
        }
    }
    archive_read_close(a);
    archive_read_finish(a);
    archive_write_close(ext);
    archive_write_finish(ext);
    return res;
}

Status sh(string cmd)
{
    string prog;
    string path;
    vector<string> args;
    vector<char const *> argv;
    char **envp;
    int i;
    int pos,oldpos;
    pid_t pid;

    // Split string into command and arguments:
    pos = cmd.find(' ');
    if (pos != string::npos) {
        prog = cmd.substr(0,pos);
        while (true) {
            oldpos = pos+1;
            pos = cmd.find(' ',oldpos);
            if (pos == string::npos) {
                if (oldpos < cmd.size()-1)
                    args.push_back(cmd.substr(oldpos));
                break;
            }
            if (pos > oldpos) args.push_back(cmd.substr(oldpos,pos-oldpos));
        }
    } else prog = cmd;
    argv.push_back(prog.c_str());   // Give the prog as 0-th argument
    for (i = 0;i < args.size();i++)
        argv.push_back(args[i].c_str());
    argv.push_back(NULL);

    if (!which(prog,path)) {
        out(ERROR,"Could not execute command:\n  "+cmd);
        return ERROR;
    }

    int fds[2];
    if (pipe(fds) != 0) {
        out(ERROR,"Could not create pipes for:\n  "+cmd);
        return ERROR;
    }

    pid = fork();

    if (pid == 0) {   // the child
        // Open file build.log, dup to stdout and stderr
        // Close all other file descriptors
        close(1);
        close(2);
        dup2(fds[1],1);
        dup2(fds[1],2);
        for (int fd = 3;fd < 64;fd++) close(fd);

        envp = prepareenvironment();
        if (execve(path.c_str(), (char *const *) &(argv[0]),envp) == -1) {
            freepreparedenvironment(envp);
            cerr << "Errno: " << errno << "\n";
            out(ERROR,"Cannot execve.");
            exit(17);
        }
    }
    close(fds[1]);
    fstream logfile(buildlogfilename.c_str(),fstream::out | fstream::app);
    FILE *output = fdopen(fds[0],"r");
    setlinebuf(output);
    char buf[1024];
    char *p;

    while (true) {
        p = fgets(buf,1024,output);
        if (p != NULL) {
            if (verbose >= 4) cout << buf;
            logfile << buf;
            logfile.flush();
        }
        else break;
    }
    fclose(output);
    logfile.close();

    int status;
    waitpid(pid,&status,0);
    if (WEXITSTATUS(status) != 0) {
        out(ERROR,"Subprocess "+cmd+" returned with an error.");
        return ERROR;
    }
    return OK;
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
              if (chdir(optarg) != 0) {
                  if (chdir(origdir.c_str()) != 0) {
                      cerr << "Cannot chdir to original dir. Stopping.\n";
                      return 16;
                  }
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
    if (chdir(targetdir.c_str()) != 0) {
        // has worked before, we assume it does again
        cerr << "Cannot chdir to target dir. Stopping.\n";
        return 15;
    }
    struct stat statbuf;
    if (stat("download",&statbuf) == -1) {
        if (mkdir("download",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0 &&
            errno != EEXIST) {
            out(ERROR,"Cannot create \"download\" directory in "+
                      targetdir+" .");
            return 7;
        }
    } else if (!S_ISDIR(statbuf.st_mode)) {
        out(ERROR,"Cannot create \"download\" directory in "+targetdir+" .");
        return ERROR;
    }

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
        if (chdir(targetdir.c_str()) != 0) {
            out(ERROR,"Cannot chdir to target directory. Stopping.");
            return 7;
        }
        res = OK;
        if (c->get != NULL) {
            out(OK,"Getting component "+c->name);
            res = c->get(targetdir);
        }
        if (res == OK) {
            if (chdir(targetdir.c_str()) != 0) {
                out(ERROR,"Cannot chdir to target directory. Stopping.");
                return 8;
            }
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

