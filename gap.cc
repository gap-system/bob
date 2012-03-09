#include "bob.h"

using namespace BOB;

static const char *GAP_dependencies[]
  = { NULL };

static Status GAP_prerequisites(string targetdir, Status depsresult)
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

static string GAP_archivename;

static Status GAP_getfunc(string targetdir)
{
    if (getindirectly(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/for/BOB/GAP.link",
           archivename)) {
        out(ERROR,"Could not download GAP archive.");
        return ERROR;
    } else 
        return OK;
}

static Status GAP_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (unpack("downloads"+archivename)) {
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

Component GAP("GAP",GAP_dependencies,GAP_prerequisites,
                    GAP_getfunc,GAP_buildfunc);


static const char *IO_dependencies[]
  = { "GAP", NULL };

static Status IO_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/io")) {
        out(ERROR,"Cannot change to the IO package's directory.");
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

Component IO_Pkg("IO_Pkg",IO_dependencies,NULL,NULL,IO_buildfunc);

