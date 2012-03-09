#include "bob.h"

using namespace BOB;

static const char *dependencies[]
  = { NULL };

static Status prerequisites(string targetdir, Status depsresult)
{
    Status res = OK;
    string path;
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

static string archivename;

static Status getfunc(string targetdir)
{
    if (getindirectly(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/for/BOB/GAP.link",
           archivename)) {
        out(ERROR,"Could not download GAP archive.");
        return ERROR;
    } else 
        return OK;
}

static Status buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (untar("downloads"+archivename)) {
        out(ERROR,"A problem occurred when extracting the archive.");
        return ERROR;
    }
    if (chdir("gap4r5")) {
        out(ERROR,"Cannot change to GAP's root directory.");
        return ERROR;
    }
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}

Component GAP("GAP",dependencies,prerequisites,getfunc,buildfunc);

