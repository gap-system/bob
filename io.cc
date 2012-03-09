#include "bob.h"

using namespace BOB;

static const char *dependencies[]
  = { "GAP", NULL };

static Status buildfunc(string targetdir)
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

Component IO_Pkg("IO_Pkg",dependencies,NULL,NULL,buildfunc);

