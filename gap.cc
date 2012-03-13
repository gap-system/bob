#include "bob.h"

using namespace BOB;

static const char *GAP_dependencies[]
  = { NULL };

static Status GAP_prerequisites(string targetdir, Status depsresult)
{
    Status res = OK;
    string path;
    if (!which("/bin/sh",path)) {
        out(ERROR,"Need a (bash-like) shell in /bin/sh, please install one.");
        res = ERROR;
    }
    if (Which_C_Compiler.num != 0) {
        out(ERROR,"Need a C-compiler, preferably gcc, please install one.");
        res = ERROR;
    }
    if (Have_make.num != 0) {
        out(ERROR,"Need the 'make' utility, please install it.");
        res = ERROR;
    }
    if (!which("m4",path)) {
        out(ERROR,"Need the 'm4' utility, please install it.");
        res = ERROR;
    }
    return res;
}

static string GAP_archivename;

static Status GAP_getfunc(string targetdir)
{
    if (getind(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/for/BOB/GAP.link",
           GAP_archivename)) {
        out(ERROR,"Could not download GAP archive.");
        return ERROR;
    } else 
        return OK;
}

vector<string> confignames;

static Status GAP_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    out(OK,"Unpacking GAP archive...");
    if (unpack(GAP_archivename)) {
        out(ERROR,"A problem occurred when extracting the archive.");
        return ERROR;
    }
    if (chdir("gap4r5") != 0) {
        out(ERROR,"Cannot change to GAP's root directory.");
        return ERROR;
    }
    if (Which_Wordsize.num == 64 && 
        (C_Compiler_Name.str == "gcc" || C_Compiler_Name.str == "clang")) {
        out(OK,"Compiling for both 32-bit and 64-bit...");
        out(OK,"Running ./configure ABI=32 for GAP...");
        if (sh("./configure ABI=32 --with-gmp=no")) {
            out(ERROR,"Error in configure stage.");
            return ERROR;
        }
        out(OK,"Running make for GAP...");
        if (sh("make")) {
            out(ERROR,"Error in compilation stage.");
            return ERROR;
        }
        out(OK,"Running make clean for GAP...");
        if (sh("make")) {
            out(ERROR,"Error in compilation stage.");
            return ERROR;
        }
        confignames.push_back("default32");
    }
    out(OK,"Running ./configure for GAP...");
    if (sh("./configure --with-gmp=no")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for GAP...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    confignames.push_back("default32");
    return OK;
}

Component GAP("GAP",GAP_dependencies,GAP_prerequisites,
                    GAP_getfunc,GAP_buildfunc);


static const char *dependencies_onlyGAP[]
  = { "GAP", NULL };

static const char *stdpackages[]
  = { "io", "orb", "cvec", "edim", "Browse", NULL };

static const int cflagslimit = 3;

static Status StdPkgs_buildfunc(string targetdir)
{
    const char *name;
    int i;
    string pkgdir;
    string msg;
    string cmd;

    i = 0;
    while (true) {   // will be left by break
        name = stdpackages[i];
        if (name == NULL) break;

        if (chdir(targetdir.c_str()) != 0) {
            out(ERROR,"Cannot change to target directory.");
            return ERROR;
        }
 
        msg = string("Compiling ")+name+" package...";
        out(OK,msg);
        pkgdir = string("gap4r5/pkg/")+name;
        if (chdir(pkgdir.c_str())) {
            msg = string("Cannot change to the ")+name+" package's directory.";
            out(ERROR,msg);
            return ERROR;
        }
        if (Which_Wordsize.num == 64 && 
            (C_Compiler_Name.str == "gcc" || C_Compiler_Name.str == "clang")) {
            msg = string("Running ./configure CONFIGNAME=default32 for ")+
                         name+" package...";
            out(OK,msg);
            cmd = string("./configure CONFIGNAME=default32");
            if (i < cflagslimit) cmd += " CFLAGS=-m32";
            if (sh(cmd)) {
                out(ERROR,"Error in configure stage.");
                return ERROR;
            }
            msg = string("Running make for ")+name+" package...";
            out(OK,msg);
            if (sh("make")) {
                out(ERROR,"Error in compilation stage.");
                return ERROR;
            }
            msg = string("Running ./configure CONFIGNAME=default64 for ")+name+
                         " package...";
            out(OK,msg);
            if (sh("./configure CONFIGNAME=default64")) {
                out(ERROR,"Error in configure stage.");
                return ERROR;
            }
            msg = string("Running make for ")+name+" package...";
            out(OK,msg);
            if (sh("make")) {
                out(ERROR,"Error in compilation stage.");
                return ERROR;
            }
        } else {
            msg = string("Running ./configure for ")+name+ " package...";
            out(OK,msg);
            if (sh("./configure")) {
                out(ERROR,"Error in configure stage.");
                return ERROR;
            }
            msg = string("Running make for ")+name+" package...";
            out(OK,msg);
            if (sh("make")) {
                out(ERROR,"Error in compilation stage.");
                return ERROR;
            }
        }
        i++;
    }
    return OK;
}
Component StdPkgs("StdPkgs",dependencies_onlyGAP,NULL,NULL,StdPkgs_buildfunc);

