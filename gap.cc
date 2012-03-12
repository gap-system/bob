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
        if (sh("./configure ABI=32")) {
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
    if (sh("./configure")) {
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

static Status IO_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/io")) {
        out(ERROR,"Cannot change to the IO package's directory.");
        return ERROR;
    }
    out(OK,"Running ./configure for IO_Pkg...");
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for IO_Pkg...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}
Component IO_Pkg("IO_Pkg",dependencies_onlyGAP,NULL,NULL,IO_buildfunc);

static Status Orb_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/orb")) {
        out(ERROR,"Cannot change to the Orb package's directory.");
        return ERROR;
    }
    out(OK,"Running ./configure for Orb_Pkg...");
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for Orb_Pkg...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}
Component Orb_Pkg("Orb_Pkg",dependencies_onlyGAP,NULL,NULL,Orb_buildfunc);

static Status CVec_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/cvec")) {
        out(ERROR,"Cannot change to the CVec package's directory.");
        return ERROR;
    }
    out(OK,"Running ./configure for CVec_Pkg...");
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for CVec_Pkg...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}
Component CVec_Pkg("CVec_Pkg",dependencies_onlyGAP,NULL,NULL,CVec_buildfunc);

static Status Edim_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/edim")) {
        out(ERROR,"Cannot change to the Edim package's directory.");
        return ERROR;
    }
    out(OK,"Running ./configure for Edim_Pkg...");
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for Edim_Pkg...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}
Component Edim_Pkg("Edim_Pkg",dependencies_onlyGAP,NULL,NULL,Edim_buildfunc);

static Status Browse_buildfunc(string targetdir)
{
    // By convention, we are in the target directory.
    if (chdir("gap4r5/pkg/Browse")) {
        out(ERROR,"Cannot change to the Browse package's directory.");
        return ERROR;
    }
    out(OK,"Running ./configure for Browse_Pkg...");
    if (sh("./configure")) {
        out(ERROR,"Error in configure stage.");
        return ERROR;
    }
    out(OK,"Running make for Browse_Pkg...");
    if (sh("make")) {
        out(ERROR,"Error in compilation stage.");
        return ERROR;
    }
    return OK;
}
Component Browse("Browse_Pkg",dependencies_onlyGAP,NULL,NULL,Browse_buildfunc);

