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
    if (Double_Compile.num == 1) {
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


static Status BuildGAPPackage(string targetdir, string pkgname, bool withm32,
                              Status err)
{
    string msg;
    string pkgdir = string("gap4r5/pkg/")+pkgname;
    string cmd;
    if (chdir(pkgdir.c_str())) {
        msg = string("Cannot change to the ")+pkgname+" package's directory.";
        out(err,msg);
        return err;
    }
    if (Double_Compile.num == 1) {
        msg = string("Running ./configure CONFIGNAME=default32 for ")+
                     pkgname+" package...";
        out(OK,msg);
        cmd = string("./configure CONFIGNAME=default32");
        if (withm32) cmd += " CFLAGS=-m32";
        if (sh(cmd)) {
            out(err,"Error in configure stage.");
            return err;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make")) {
            out(err,"Error in compilation stage.");
            return err;
        }
        msg = string("Running ./configure CONFIGNAME=default64 for ")+pkgname+
                     " package...";
        out(OK,msg);
        if (sh("./configure CONFIGNAME=default64")) {
            out(err,"Error in configure stage.");
            return err;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make")) {
            out(err,"Error in compilation stage.");
            return err;
        }
    } else {
        msg = string("Running ./configure for ")+pkgname+ " package...";
        out(OK,msg);
        if (sh("./configure")) {
            out(err,"Error in configure stage.");
            return err;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make")) {
            out(err,"Error in compilation stage.");
            return err;
        }
    }
}

static const char *deps_onlyGAP[]
  = { "GAP", NULL };

static Status io_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "io", true, ERROR); }
Component io_Pkg("io_PKG",deps_onlyGAP,NULL,NULL,io_buildfunc);

static Status orb_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "orb", true, WARN); }
Component orb_Pkg("orb_PKG",deps_onlyGAP,NULL,NULL,orb_buildfunc);

static Status cvec_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "cvec", true, WARN); }
Component cvec_Pkg("cvec_PKG",deps_onlyGAP,NULL,NULL,cvec_buildfunc);

static Status edim_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "edim", false, WARN); }
Component edim_Pkg("edim_PKG",deps_onlyGAP,NULL,NULL,edim_buildfunc);

static Status Browse_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "Browse", false, WARN); }
Component Browse_Pkg("Browse_PKG",deps_onlyGAP,NULL,NULL,Browse_buildfunc);

static Status nq_prerequisites(string targetdir, Status depsresult)
{
    string path;
    if (depsresult != OK) return depsresult;
    if (!(which("gawk",path) || which("mawk",path) || which("nawk",path) ||
          which("awk",path))) {
        out(WARN,"Need awk for component nq-Pkg.");
        return WARN;
    }
    if (access("/usr/include/gmp.h",R_OK) != 0) {
        out(WARN,"Need gmp installed for component nq-Pkg.");
        return WARN;
    }
}

static Status nq_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "nq-2.4", true, WARN); }
// Component nq_Pkg("nq_PKG",deps_onlyGAP,nq_prerequisites,NULL,nq_buildfunc);

