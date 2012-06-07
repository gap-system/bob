#include "bob.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>
#include "hl_sha1.h"

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

static int Have_make_Test(string &st)
{
    if (which("make",st)) return 0;
    else {
        st = "";
        return -1;
    }
}
Test Have_make("Have_make",1,Have_make_Test);

static int Which_C_Compiler_Test(string &st)
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

static int C_Compiler_Name_Test(string &st)
{
    if (Which_C_Compiler.num != 0) return -1;
    size_t pos = Which_C_Compiler.str.rfind('/');
    if (pos == string::npos)
        st = Which_C_Compiler.str;
    else
        st = Which_C_Compiler.str.substr(pos+1);
    return 0;
}

Test C_Compiler_Name("C_Compiler_Name",2,C_Compiler_Name_Test);

static int Which_Architecture_Test(string &st)
{
#if SYS_IS_WINDOWS
    st = "WINDOWS";
    return 0;
#endif
#if SYS_IS_LINUX
    st = "LINUX";
    return 0;
#endif
#if SYS_IS_OSX
    st = "OSX";
    return 0;
#endif
#if SYS_IS_BSD
    st = "BSD";
    return 0;
#endif
    st = "UNKNOWN";
    return 0;
}
Test Which_Architecture("Which_Architecture",1,Which_Architecture_Test);

static int Which_OS_Variant_Test(string &st)
{
    string path;
#if SYS_IS_LINUX
    if (which("apt-get",path)) {
        st = "apt-get";
        return 0;
    } else if (which("rpm",path)) {
        st = "rpm";
        return 0;
    } else if (which("emerge",path)) {
        st = "emerge";
        return 0;
    } 
#endif
#if SYS_IS_OSX
    if (which("apt-get",path)) {
        st = "fink";
        return 0;
    } else if (which("brew",path)) {
        st = "homebrew";
        return 0;
    } else if (which("ports",path)) {
        st = "macports";
        return 0;
    } 
    st = "none";
    return 0;
#endif
    st = "unknown";
    return 0;
}
Test Which_OS_Variant("Which_OS_Variant",2,Which_OS_Variant_Test);

static int Which_Wordsize_Test(string &st)
{
    if (Which_C_Compiler.num != 0) return 0;
    fstream testprog("/tmp/wordsize.c",fstream::out | fstream::trunc);
    testprog << "#include <stdio.h>\n";
    testprog << "int main(void) {\n";
    testprog << "  printf(\"%ld\\n\",sizeof(void *));\n";
    testprog << "  return 0;\n";
    testprog << "}\n";
    testprog.close();
    try { sh(Which_C_Compiler.str+" /tmp/wordsize.c -o /tmp/wordsize",0,true); }
    catch (Status e) { return 0; }
    FILE *prog = popen("/tmp/wordsize","r");
    if (prog == NULL) return 0;
    int size,ret;
    ret = fscanf(prog,"%d",&size);
    fclose(prog);
    unlink("/tmp/wordsize.c");
    unlink("/tmp/wordsize");
    size = ret == 1 ? size*8 : 0;
    stringstream output;
    output << "Word size is " << size << "bit.";
    out(OK,output.str());
    st = output.str();
    return size;
}
Test Which_Wordsize("Which_Wordsize",2,Which_Wordsize_Test);

Status Have_C_Header(string headername, bool m32)
{
    string m32opt = m32 ? " -m32" : "";
    if (Which_C_Compiler.num != 0) return ERROR;
    fstream testprog("/tmp/headertest.c",fstream::out | fstream::trunc);
    testprog << "#include <" << headername << ">\n";
    testprog << "int main(void) {\n";
    testprog << "  return 0;\n";
    testprog << "}\n";
    testprog.close();
    try { 
        sh(Which_C_Compiler.str+" "+getenvironment("CPPFLAGS")+" "+
           getenvironment("CFLAGS")+m32opt+
           " -E /tmp/headertest.c -o /tmp/headertest",0,true); 
    }
    catch (Status e) { return ERROR; }
    unlink("/tmp/headertest.c");
    unlink("/tmp/headertest");
    return OK;
}

Status Have_C_Library(string lib, bool m32)
{
    string m32opt = m32 ? " -m32" : "";
    if (Which_C_Compiler.num != 0) return ERROR;
    fstream testprog("/tmp/libtest.c",fstream::out | fstream::trunc);
    testprog << "int main(void) {\n";
    testprog << "  return 0;\n";
    testprog << "}\n";
    testprog.close();
    try {
        sh(Which_C_Compiler.str+" "+getenvironment("CPPFLAGS")+" "+
           getenvironment("CFLAGS")+m32opt+" /tmp/libtest.c -o /tmp/libtest "+
           getenvironment("LDFLAGS")+" "+lib,0,true);
    }
    catch (Status e) { return ERROR; }
    unlink("/tmp/libtest.c");
    unlink("/tmp/libtest");
    return OK;
}

static int Can_Compile_32bit_Test(string &st)
{
    if (Which_C_Compiler.num != 0 ||
        Which_Wordsize.num != 64 || 
        (C_Compiler_Name.str != "gcc" && C_Compiler_Name.str != "clang")) {
        st = "no";
        return -1;
    }
    out(OK,"We are on a 64-bit system using gcc or clang.");
    out(OK,"Checking whether or not we can compile in 32-bit mode...");
    fstream testprog("/tmp/lib32test.c",fstream::out | fstream::trunc);
    testprog << "int main(void) {\n";
    testprog << "  return 0;\n";
    testprog << "}\n";
    testprog.close();
    try { 
        sh(Which_C_Compiler.str+" "+getenvironment("CPPFLAGS")+" "+
           getenvironment("CFLAGS")+" -m32 /tmp/lib32test.c -o /tmp/lib32test "+
           getenvironment("LDFLAGS"),0,true);
    }
    catch (Status e) {
        st = "no";
        out(OK,"No, we cannot compile 32-bit mode executables.");
        if (Which_Architecture.str == "LINUX") {
          out(ADVICE,"If you are running a debian-like Linux, you can "
                     "install the");
          out(ADVICE,"necessary tools by doing:");
          out(ADVICE,"  apt-get install ia32-libs libc6-dev-i386 "
                     "lib32readline5-dev");
          out(ADVICE,"with root privileges (using su or sudo).");
        }
        return -1;
    }
    unlink("/tmp/lib32test.c");
    unlink("/tmp/lib32test");
    st = "yes";
    out(OK,"Yes, we can compile 32-bit mode executables.");
    return 0;
}
Test Can_Compile_32bit("Can_Compile_32bit",3,Can_Compile_32bit_Test);

static int Double_Compile_Test(string &st)
{
    if (Which_Wordsize.num == 64 && 
        (C_Compiler_Name.str == "gcc" || C_Compiler_Name.str == "clang") &&
        Can_Compile_32bit.num == 0) {
        out(OK,"Performing 64-bit and 32-bit compilation.");
        st = "DoubleCompile";
        return 1;
    } else {
        st = "SingleCompile";
        out(OK,"Performing single compilation.");
        return 0;
    }
}
Test Double_Compile("Double_Compile",4,Double_Compile_Test);

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
    unsigned int i;
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
bool nonetwork = false;
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
        if (verbose >= 4) cout << "BOB:";
        if (severity == WARN) cout << "Warning:";
        else if (severity == ADVICE) cout << "Advice:";
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
        res = "";
        return false;
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

bool exists(string filename)
{
    struct stat stFileInfo;
    return (stat(filename.c_str(), &stFileInfo) == 0);
}

bool isdir(string dirname)
{
    struct stat stFileInfo;
    return stat(dirname.c_str(), &stFileInfo) == 0 &&
           S_ISDIR(stFileInfo.st_mode);
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

Status downloadname(string targetdir, string url, string &localname)
{
    size_t pos = url.rfind('/');
    if (pos == string::npos) {
        out(ERROR,"Given URL does not contain /.");
        return ERROR;
    }
    localname = targetdir+"bobdownloads/"+url.substr(pos+1);
    return OK;
}

Status download(string url, string localname)
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
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    FILE *outs = fopen(localname.c_str(),"w");
    if (outs == NULL) {
        out(ERROR,"Cannot write to "+localname+" .");
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
        out(ERROR,"Cannot close "+localname+" .");
        return ERROR;
    }

    return OK;
}

void get(string targetdir, string url, string &filename, bool alwaysget)
{
    Status res;
    if (downloadname(targetdir,url,filename) == ERROR) throw ERROR;
    if (!alwaysget && access(filename.c_str(),R_OK) == 0) {
        out(OK,"Already have "+url);
        return;
    }
    res = download(url,filename);
    if (res != OK) throw res;
}

unsigned char *sha1(FILE *f)
{
    HL_SHA1_CTX c;
    SHA1 sha;
    static hl_uint8 md[SHA1HashSize];
    int i;
    unsigned char buf[4096];

    sha.SHA1Reset(&c);
    while (true) {
        i = fread(buf,1,4096,f);
        if (i <= 0) break;
        sha.SHA1Input(&c,(const hl_uint8 *) buf,(unsigned int)i);
    }
    sha.SHA1Result(&c,md);
    return (unsigned char *)md;
}

static inline int hexdig(char c)
{
    if (c >= '0' && c <= '9') return (int) (c-'0');
    else if (c >= 'a' && c <= 'f') return (int) (c-'a'+10);
    else if (c >= 'A' && c <= 'F') return (int) (c-'A'+19);
    return 0;
}

Status checksha1(string filename, string hash)
{
    if (hash.size() < 40) return OK;
    FILE *f = fopen(filename.c_str(),"r");
    if (f == NULL) return ERROR;
    unsigned char *md = sha1(f);
    fclose(f);
    int i;
    for (i = 0;i < 20;i++)
        if (md[i] != hexdig(hash[2*i])*16+hexdig(hash[2*i+1])) 
            return ERROR;
    return OK;
}

void getind(string targetdir, string url, string &archivename)
{
    string filename;
    string url2;
    string hash;
    Status res;
    size_t pos;

    out(OK,"Getting link file...");
    if (nonetwork)
        get(targetdir, url, filename, false);
        // WARN or ERROR exception is handed through
    else
        get(targetdir, url, filename, true);
        // WARN or ERROR exception is handed through
    fstream file;
    file.exceptions ( ifstream::failbit | ifstream::badbit );
    try {
        file.open(filename.c_str(),fstream::in);
    }
    catch (ifstream::failure e) {
        out(ERROR,"Could not write link file "+filename+" .");
        throw ERROR;
    }
    try {
        getline(file,url2);
        if (url2 != "BOB") {
            file.close();
            out(ERROR,"Link file "+filename+" corrupt.");
            throw ERROR;
        }
        getline(file,url2);
        getline(file,hash);
        file.close();
    } 
    catch (ifstream::failure e) {
        file.close();
        out(ERROR,"Could not read link file "+filename+" .");
        throw ERROR;
    }
    // Now check if it is already there:
    if (downloadname(targetdir,url2,archivename) == ERROR) throw ERROR;
    pos = archivename.rfind('/');
    if (pos != string::npos)
        out(OK,string("Archive name: ")+archivename.substr(pos+1));

    // Is it already there?
    if (access(archivename.c_str(),R_OK) == 0) {
        out(OK,"Checking sha1 checksum of file that was found...");
        if (checksha1(archivename,hash) == OK) return;
        out(OK,"Checksum of archive found is wrong, downloading it again...");
    }

    // Now get it:
    res = download(url2,archivename);
    if (res == ERROR) throw ERROR;

    // Now check it (again):
    out(OK,"Checking sha1 checksum of downloaded file...");
    res = checksha1(archivename,hash);
    if (res == ERROR) {
        out(ERROR,"SHA1 checksum of downloaded file "+archivename+
                  " was wrong!");
        throw ERROR;
    }
}

static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    int64_t offset;

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

void unpack(string archivename)
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
    archive_read_support_compression_bzip2(a);
    archive_read_support_compression_xz(a);
    archive_read_support_compression_compress(a);
    if (filename != NULL && strcmp(filename, "-") == 0) filename = NULL;
    if ((r = archive_read_open_filename(a, filename, 10240))) {
        out(ERROR,"Cannot read archive "+archivename+"\nMessage: "+
            archive_error_string(a));
        archive_read_finish(a);
        throw ERROR;
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
    if (res != OK) throw res;
}

static string massagearg(string cmd)
{
    string res;
    unsigned long i;
    res.reserve(cmd.size());
    for (i = 0;i < cmd.size();i++) {
        if (cmd[i] == '~') {
            if (i+1 < cmd.size() && cmd[i+1] == '~') {
                res.push_back('~');
                i++;
            } else
                res.push_back(' ');
        } else
            res.push_back(cmd[i]);
    }
    return res;
}

void sh(string cmd, int stdinfd, bool quiet)
{
    string prog;
    string path;
    vector<string> args;
    vector<char const *> argv;
    char **envp;
    unsigned int i;
    size_t pos,oldpos;
    pid_t pid;

    // First log command if not quiet:
    if (!quiet) {
        fstream logfile(buildlogfilename.c_str(),fstream::out | fstream::app);
        logfile << "COMMAND:" << cmd << "\n";
        logfile.close();
    }

    // Split string into command and arguments:
    pos = cmd.find(' ');
    if (pos != string::npos) {
        prog = cmd.substr(0,pos);
        while (true) {
            oldpos = pos+1;
            pos = cmd.find(' ',oldpos);
            if (pos == string::npos) {
                if (oldpos < cmd.size()-1)
                    args.push_back(massagearg(cmd.substr(oldpos)));
                break;
            }
            if (pos > oldpos) 
                args.push_back(massagearg(cmd.substr(oldpos,pos-oldpos)));
        }
    } else prog = cmd;
    argv.push_back(prog.c_str());   // Give the prog as 0-th argument
    for (i = 0;i < args.size();i++)
        argv.push_back(args[i].c_str());
    argv.push_back(NULL);

    if (!which(prog,path)) {
        if (!quiet) out(ERROR,"Could not execute command:\n  "+cmd);
        throw ERROR;
    }

    int fds[2];
    if (pipe(fds) != 0) {
        if (!quiet) out(ERROR,"Could not create pipes for:\n  "+cmd);
        throw ERROR;
    }

    pid = fork();

    if (pid == 0) {   // the child
        // Open file build.log, dup to stdout and stderr
        // Close all other file descriptors
        if (stdinfd != 0) dup2(stdinfd,0);
        dup2(fds[1],1);
        dup2(fds[1],2);
        for (int fd = 3;fd < 64;fd++) close(fd);

        envp = prepareenvironment();
        if (execve(path.c_str(), (char *const *) &(argv[0]),envp) == -1) {
            freepreparedenvironment(envp);
            cerr << "Errno: " << errno << "\n";
            if (!quiet) out(ERROR,"Cannot execve.");
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
        if (!quiet) out(ERROR,"Subprocess "+cmd+" returned with an error.");
        throw ERROR;
    }
}

int shbg(string cmd, int stdinfd, bool quiet)
{
    string prog;
    string path;
    vector<string> args;
    vector<char const *> argv;
    char **envp;
    unsigned int i;
    size_t pos,oldpos;
    pid_t pid;

    // First log command if not quiet:
    if (!quiet) {
        fstream logfile(buildlogfilename.c_str(),fstream::out | fstream::app);
        logfile << "COMMAND:" << cmd << "\n";
        logfile.close();
    }

    // Split string into command and arguments:
    pos = cmd.find(' ');
    if (pos != string::npos) {
        prog = cmd.substr(0,pos);
        while (true) {
            oldpos = pos+1;
            pos = cmd.find(' ',oldpos);
            if (pos == string::npos) {
                if (oldpos < cmd.size()-1)
                    args.push_back(massagearg(cmd.substr(oldpos)));
                break;
            }
            if (pos > oldpos) 
                args.push_back(massagearg(cmd.substr(oldpos,pos-oldpos)));
        }
    } else prog = cmd;
    argv.push_back(prog.c_str());   // Give the prog as 0-th argument
    for (i = 0;i < args.size();i++)
        argv.push_back(args[i].c_str());
    argv.push_back(NULL);

    if (!which(prog,path)) {
        if (!quiet) out(ERROR,"Could not execute command:\n  "+cmd);
        return -1;
    }

    pid = fork();

    if (pid == 0) {   // the child
        // Open file build.log, dup to stdout and stderr
        if (stdinfd != 0) dup2(stdinfd,0);
        int logfile = open(buildlogfilename.c_str(),O_WRONLY | O_APPEND);
        dup2(logfile,1);
        dup2(logfile,2);
        // Close all other file descriptors
        for (int fd = 3;fd < 64;fd++) close(fd);

        envp = prepareenvironment();
        if (execve(path.c_str(), (char *const *) &(argv[0]),envp) == -1) {
            freepreparedenvironment(envp);
            cerr << "Errno: " << errno << "\n";
            if (!quiet) out(ERROR,"Cannot execve.");
            exit(17);
        }
    }

    return pid;
}

Status rmrf(string dirname)
{
  DIR *dp;
  struct dirent *ep;
  string abs_filename;
  struct stat stFileInfo;

  dp = opendir (dirname.c_str());
  if (dp != NULL) {
      while ((ep = readdir(dp)) != NULL) {
          if(strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
              abs_filename = dirname + "/" + ep->d_name;
              if (lstat(abs_filename.c_str(), &stFileInfo) == 0) {
                  if(S_ISDIR(stFileInfo.st_mode))
                      rmrf(abs_filename);
                  else
                      unlink(abs_filename.c_str());
              }
          }
      }
      (void) closedir (dp);
  }

  if (rmdir(dirname.c_str()) == 0) return OK;
  else return ERROR;
}

Status cp(string from, string to)
{
    fstream fin,fout;
    char buf[1024];
    size_t len;

    fin.exceptions(ifstream::badbit);
    fout.exceptions(ofstream::badbit);
    try {
        fin.open(from.c_str(),fstream::in);
        fout.open(to.c_str(),fstream::out | fstream::trunc);
        while (!fin.eof()) {
            fin.read(buf,1024);
            len = fin.gcount();
            if (len > 0) fout.write(buf,len);
        }
        fout.close();
        fin.close();
    }
    catch (ifstream::failure e) {
        out(ERROR,"I/O error copying "+from+" to "+to);
        return ERROR;
    }
    return OK;
}

void cd(string dir)
{
    fstream logfile(buildlogfilename.c_str(),fstream::out | fstream::app);
    logfile << "COMMAND:cd " << dir << "\n";
    if (chdir(dir.c_str()) != 0) {
        out(ERROR,"Could not change to directory "+dir);
        logfile << "ERROR: Could not change to directory "+dir << "\n";
        logfile.close();
        throw ERROR;
    }
    logfile.close();
}

bool interactive = true;
bool osxaddpaths = true;
string origCFLAGS = "";
string origLDFLAGS = "";

void readlines(string filename, vector<string> &v)
// Throws ERROR if anything goes wrong.
{
    string line;
    fstream f;
    f.exceptions( ifstream::badbit );
    try { f.open(filename.c_str(),fstream::in); }
    catch (ifstream::failure e) {
        out(ERROR,"Could not open file \""+filename+"\" for reading");
        throw ERROR;
    }
    v.clear();
    try {
        while (getline(f,line)) v.push_back(line);
        f.close();
    }
    catch (ifstream::failure e) {
        out(ERROR,"Could not read file \""+filename+"\"");
        throw ERROR;
    }
}

void writelines(string filename, vector<string> &v)
// Throws ERROR if anything goes wrong.
{
    string line;
    fstream f;
    f.exceptions( ifstream::failbit | ifstream::badbit );
    try { f.open(filename.c_str(),fstream::out | fstream::trunc); }
    catch (ifstream::failure e) {
        out(ERROR,"Could not open file \""+filename+"\" for writing");
        throw ERROR;
    }
    try {
        for (size_t i = 0;i < v.size();i++) f << v[i] << "\n";
        f.close();
    }
    catch (ifstream::failure e) {
        out(ERROR,"Could not write file \""+filename+"\"");
        throw ERROR;
    }
}


void edit(string edscriptpath)
// Throws ERROR if anything goes wrong.
{
    vector<string> edscript;
    vector<string> file;
    size_t start,stop;
    string find,replace;

    readlines(edscriptpath,edscript);
    readlines(edscript[0],file);
    for (size_t i = 1;i+3 < edscript.size();i += 4) {
        start = atoi(edscript[i].c_str());
        stop = atoi(edscript[i+1].c_str());
        if (start < 1 || start > file.size() || 
            stop < start || stop > file.size()) break;
        find = edscript[i+2];
        replace = edscript[i+3];
        for (size_t j = start-1;j <= stop-1;j++) {
            size_t pos = file[j].find(find);
            if (pos != string::npos) {
                file[j] = file[j].substr(0,pos) + replace + 
                          file[j].substr(pos+find.size());
            }
        }
    }
    writelines(edscript[0],file);
}

}   // namespace BOB

using namespace BOB;

// The main loop:

string origdir;
string targetdir;

void usage()
{
    cout << "============\n";
    cout << " Bob-Manual \n";
    cout << "============\n\n";
    cout << "This program will download and compile GAP on your machine.\n";
    cout << "It needs a working C-Compiler and some other utilities.\n";
    cout << "It will first check whether everything that is needed is there.\n";
    cout << "and alert you about missing tools.\n\n";
    cout << "Usage: bob [-h] [-i] [-v] [-q] [-t TARGETDIR]\n";
    cout << "    -h : Show this help\n";
    cout << "    -v : Increase verbosity by 1, default is 3\n";
    cout << "    -q : Decrease verbosity by 1\n";
    cout << "         Verbosity levels:\n";
    cout << "           0 : do not show anything, completely quiet\n";
    cout << "           1 : only show errors\n";
    cout << "           2 : show errors and warnings\n";
    cout << "           3 : show errors, warnings and what is going on\n";
    cout << "           4 : show additionally output of building process\n";
    cout << "    -f : Run non-interactively, that is, do not ask if warnings\n";
    cout << "         are found (without this options, the program asks the\n";
    cout << "         user if it should continue if warnings are found,\n";
    cout << "         when errors are found, it stops always)\n";
    cout << "    -n : Do not use network access\n";
    cout << "         (This means that all link and archive files must\n";
    cout << "          already be present and that Bob does not check for\n";
    cout << "          updates.)\n";
    cout << "    -t : Specify a different target directory than the current\n";
    cout << "         directory. Bob needs write access to that directory.\n";
    cout << "    -c : Specify one component name and only the build routine\n";
    cout << "         for that component is run without any dependencies.\n";
    cout << "    -z : Do not add various paths for additional libraries\n";
    cout << "         to CFLAGS and LDFLAGS on Mac OSX.\n";
    cout << "For any questions or complaints please contact:\n";
    cout << "  Max Neunhoeffer <neunhoef@mcs.st-and.ac.uk>\n\n";
}

int main(int argc, char * const argv[], char *envp[])
{
    int opt;
    string onlycomp;

    // Keep the original current directory:
    char *cwd = getcwd(NULL,1024);
    origdir = cwd;
    free(cwd);
    if (origdir[origdir.size()-1] != '/') origdir.push_back('/');

    // Initialise targetdir with our position if given:
    string ourname = argv[0];
    size_t po = ourname.rfind('/');
    if (po == string::npos) {
        targetdir = origdir;
    } else {
        targetdir = ourname.substr(0,po+1);
        cwd = realpath(targetdir.c_str(),NULL);
        targetdir = cwd;
        free(cwd);
        if (targetdir[targetdir.size()-1] != '/') 
            targetdir.push_back('/');
    }

    // Initialise our environment business:
    initenvironment(envp);

    while ((opt = getopt(argc, argv, "zhvqnft:c:")) != -1) {
        switch (opt) {
          case 'h':
              usage();
              return 0;
          case 'f':
              interactive = false;
              break;
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
          case 'c':
              onlycomp = optarg;
              break;
          case 'n':
              nonetwork = true;
              break;
          case 'z':
              osxaddpaths = false;
              break;
          default: /* '?' */
              cerr << "Usage: " << argv[0] << "[-v] [-q] [-n] [-t TARGETDIR]\n";
              return -1;
        }
    }
    // At this stage we ignore further arguments.

    // Create the necessary infrastructure for logging:
    boblogfilename = targetdir + "bob.log";
    buildlogfilename = targetdir + "build.log";

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

    // Change to target directory:
    if (chdir(targetdir.c_str()) != 0) {
        // has worked before, we assume it does again
        cerr << "Cannot chdir to target dir. Stopping.\n";
        return 15;
    }
    stringstream msg;
    msg << "This is BOB version " << BOBVERSION << ".\n";
    out(OK,msg.str());
    out(OK,"Target directory is: "+targetdir);
    struct stat statbuf;
    if (stat("bobdownloads",&statbuf) == -1) {
        if (mkdir("bobdownloads",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)!=0 &&
            errno != EEXIST) {
            out(ERROR,"Cannot create \"bobdownloads\" directory in "+
                      targetdir+" .");
            return 7;
        }
    } else if (!S_ISDIR(statbuf.st_mode)) {
        out(ERROR,"Cannot create \"bobdownloads\" directory in "+
            targetdir+" .");
        return ERROR;
    }

    // Check whether we are current:
    if (!nonetwork) {
        out(OK,"Checking for updates...");
        int merkverbose = verbose;
        verbose = 2;
        string versionfile;
        string version;
        try {
            get(targetdir,"http://www-groups.mcs.st-and.ac.uk/~neunhoef/Computer/Software/Gap/bob/BOBVERSION",versionfile,true);
            verbose = merkverbose;
            fstream file(versionfile.c_str(),fstream::in);
            getline(file,version);
            file.close();
            if (atoi(version.c_str()) > BOBVERSION) {
                out(WARN,"There is a newer version of bob available.");
            } else {
                out(OK,"I am the latest version of myself, good. :-)");
            }
        }
        catch (Status e) {
            verbose = merkverbose;
            out(OK,"Could not get latest version number, does not matter.");
        }
    }

    origCFLAGS = getenvironment("CFLAGS");
    origLDFLAGS = getenvironment("LDFLAGS");
    if (C_Compiler_Name.str == "gcc")
        setenvironment("CFLAGS","-march=native "+getenvironment("CFLAGS"));
#if SYS_IS_OSX
    // For Mac OSX, add things to CFLAGS and LDFLAGS:
    if (osxaddpaths) {
        if (isdir("/usr/local/include")) {
            setenvironment("CFLAGS",getenvironment("CFLAGS") +
                           " -I/usr/local/include");
            out(OK,"Adding \"-I/usr/local/include\" to CFLAGS.");
        }
        if (isdir("/sw/include")) {
            setenvironment("CFLAGS",getenvironment("CFLAGS") +
                           " -I/sw/include");
            out(OK,"Adding \"-I/sw/include\" to CFLAGS.");
        }
        if (isdir("/opt/local/include")) {
            setenvironment("CFLAGS",getenvironment("CFLAGS") +
                           " -I/opt/local/include");
            out(OK,"Adding \"-I/opt/local/include\" to CFLAGS.");
        }
        if (isdir("/opt/include")) {
            setenvironment("CFLAGS",getenvironment("CFLAGS") +
                           " -I/opt/include");
            out(OK,"Adding \"-I/opt/include\" to CFLAGS.");
        }
        if (isdir("/usr/local/lib")) {
            setenvironment("LDFLAGS",getenvironment("LDFLAGS") +
                           " -L/usr/local/lib");
            out(OK,"Adding \"-L/usr/local/lib\" to LDFLAGS.");
        }
        if (isdir("/sw/lib")) {
            setenvironment("LDFLAGS",getenvironment("LDFLAGS") +
                           " -L/sw/lib");
            out(OK,"Adding \"-L/sw/lib\" to LDFLAGS.");
        }
        if (isdir("/opt/local/lib")) {
            setenvironment("LDFLAGS",getenvironment("LDFLAGS") +
                           " -L/opt/local/lib");
            out(OK,"Adding \"-L/opt/local/lib\" to LDFLAGS.");
        }
        if (isdir("/opt/lib")) {
            setenvironment("LDFLAGS",getenvironment("LDFLAGS") +
                           " -L/opt/lib");
            out(OK,"Adding \"-L/opt/lib\" to LDFLAGS.");
        }
    }
#endif
    
    out(OK,"");
    out(OK,"Performing tests...");
    static vector<Test *> &tests = alltests();
    Test *t;
    int p;
    unsigned int i;
    for (p = 1;p < 9;p++) {
        for (i = 0;i < tests.size();i++) {
            t = tests[i];
            if (t->phase == p) {
                t->run();
            }
        }
    }

    if (onlycomp != "") {
        Component *oc = Component::find(onlycomp);
        if (oc == NULL) {
            out(ERROR,"Did not find component "+onlycomp+" ...");
            return 8;
        }
        if (oc->prereq != NULL) {
            out(OK,"Checking prerequisites for component "+onlycomp+" ...");
            oc->prereqres = oc->prereq(targetdir,OK);
            if (oc->prereqres != OK) {
                out(ERROR,"Prerequisites not fulfilled, stopping.");
                return 9;
            }
        }
        if (chdir(targetdir.c_str()) != 0) {
            out(ERROR,"Cannot chdir to target directory. Stopping.");
            return 10;
        }
        if (oc->get != NULL) {
            out(OK,"Getting component "+onlycomp+" ...");
            oc->getres = oc->get(targetdir);
            if (oc->getres != OK) return 11;
        }
        if (chdir(targetdir.c_str()) != 0) {
            out(ERROR,"Cannot chdir to target directory. Stopping.");
            return 12;
        }
        out(OK,"Building component "+onlycomp+" ...");
        if (oc->build(targetdir) != OK) {
            out(ERROR,"Building did not work.");
            return 13;
        }
        out(OK,"Done.");
        return 0;
    }

    static vector<Component *> &comps = allcomps();
    Component *c,*cc;

    // First compute a total order that is compatible with the
    // dependencies:
    vector<Component *> deporder;
    vector<bool> done(comps.size(),false);

    bool somenew,cando;
    size_t j;
    int pos;

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
                    pos = Component::findnr(c->depends[j].substr(1));
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
    out(OK,"");
    out(OK,"Checking prerequisites...");
    goterror = false;
    gotwarning = false;
    Status res;
    for (i = 0;i < deporder.size();i++) {
        c = deporder[i];
        res = UNKNOWN;
        // First check whether all necessary dependencies are happy:
        for (j = 0;j < c->depends.size();j++) {
            if (c->depends[j][0] == '!') {
                cc = Component::find(c->depends[j].substr(1));
                if (cc != NULL) {
                    if (res < cc->prereqres) res = cc->prereqres;
                }
            }
        }
        if (c->prereq != NULL) {
            out(OK,"Checking prerequisites for component "+c->name+" ...");
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
    if (interactive && gotwarning) {
        cout << "\nThere have been warnings, go on regardless? ";
        cout.flush();
        cin >> answer;
        if (answer.size() == 0 || 
            (answer[0] != 'y' && answer[0] != 'Y')) {
            out(WARN,"Stopping because of warnings.");
            return 2;
        }
    }

    // Now do the actual work of getting and building:
    int nrerrors = 0;
    int nrwarnings = 0;
    for (i = 0;i < deporder.size();i++) {
        if (nrerrors > 0) {
            out(ERROR,"Stopping.");
            break;
        }
        c = deporder[i];
        if (c->prereqres != OK) {
            out(OK,"");
            out(OK,string("Not working on component ")+c->name+" due to"+
                   " failed prerequisites check.");
            c->getres = WARN;
            c->buildres = WARN;
            continue;
        }
        out(OK,"");
        out(OK,string("Working on component ")+c->name+" ...");
        if (chdir(targetdir.c_str()) != 0) {
            out(ERROR,"Cannot chdir to target directory. Stopping.");
            nrerrors++;
            break;
        }
        res = OK;
        if (c->get != NULL) {
            out(OK,"Getting component "+c->name);
            res = c->get(targetdir);
        }
        c->getres = res;
        if (res == OK) {
            if (chdir(targetdir.c_str()) != 0) {
                out(ERROR,"Cannot chdir to target directory. Stopping.");
                nrerrors++;
                break;
            }
            out(OK,"Building component "+c->name);
            outs.open(buildlogfilename.c_str(),fstream::out | fstream::app);
            if (!outs.fail()) {
                outs << "\nBOB: Building component " << c->name << "...\n\n";
                outs.close();
            }
            res = c->build(targetdir);
        }
        if (res == ERROR) nrerrors++;
        else if (res == WARN) nrwarnings++;
        else {
            out(OK,"Successfully built component "+c->name);
        }
        c->buildres = res;
    }

    out(OK,"Summary:");
    if (nrerrors > 0) {
        out(OK,"");
        out(OK,"Components with errors:");
        for (i = 0; i < deporder.size();i++) {
            c = comps[i];
            if (c->buildres == ERROR) out(OK,string("  ")+c->name);
        }
    }
    if (nrwarnings > 0) {
        out(OK,"");
        out(OK,"Components with warnings:");
        for (i = 0; i < deporder.size();i++) {
            c = comps[i];
            if (c->buildres == WARN) out(OK,string("  ")+c->name);
        }
    }
    if (nrerrors == 0 && nrwarnings == 0) 
        out(OK,"All went smoothly.");
    return 0;
}

