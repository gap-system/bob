#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fstream>

#include "bob.h"

using namespace std;
using namespace BOB;

static const char *GAP_dependencies[]
  = { NULL };

static Status GAP_prerequisites(string, Status)
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

static Status GAP_buildfunc(string)
{
    // By convention, we are in the target directory.
    if (access("gap4r5",F_OK) == 0) {
        out(OK,"Removing old installation...");
        if (rmrf("gap4r5") == ERROR) 
            out(WARN,"Could not remove directory \"gap4r5\" recursively!");
    }
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


static Status BuildGAPPackage(string, string pkgname, bool withm32,
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
    return OK;
}

static const char *deps_onlyGAP[]
  = { "GAP", NULL };

static Status io_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "io", true, ERROR); }
Component io("io",deps_onlyGAP,NULL,NULL,io_buildfunc);

static Status orb_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "orb", true, WARN); }
Component orb("orb",deps_onlyGAP,NULL,NULL,orb_buildfunc);

static Status cvec_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "cvec", true, WARN); }
Component cvec("cvec",deps_onlyGAP,NULL,NULL,cvec_buildfunc);

static Status edim_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "edim", false, WARN); }
Component edim("edim",deps_onlyGAP,NULL,NULL,edim_buildfunc);

static Status Browse_prerequisites(string, Status depsresult)
{
    string path;
    if (depsresult != OK) return depsresult;
    if (Have_C_Library("-lncurses") != OK ||
        Have_C_Header("ncurses.h") != OK) {
        out(WARN,"Need ncurses library installed for component Browse.");
        return WARN;
    }
    if (Have_C_Library("-lpanel") != OK ||
        Have_C_Header("panel.h") != OK) {
        out(WARN,"Need panel library installed for component Browse.");
        return WARN;
    }
    return OK;
}

static Status Browse_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "Browse", false, WARN); }
Component Browse("Browse",deps_onlyGAP,Browse_prerequisites,NULL,
                 Browse_buildfunc);

static Status nq_prerequisites(string, Status depsresult)
{
    string path;
    if (depsresult != OK) return depsresult;
    if (!(which("gawk",path) || which("mawk",path) || which("nawk",path) ||
          which("awk",path))) {
        out(WARN,"Need awk for component nq-Pkg.");
        return WARN;
    }
    if (Have_C_Library("-lgmp") != OK ||
        Have_C_Header("gmp.h") != OK) {
        out(WARN,"Need gmp installed for component nq.");
        return WARN;
    }
    return OK;
}

static Status nq_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "nq-2.4", true, WARN); }
Component nq("nq",deps_onlyGAP,nq_prerequisites,NULL,nq_buildfunc);


static Status switch_sysinfo_link(string targetdir, int towhat)
{
    if (Double_Compile.num == 0) return OK;

    if (chdir(targetdir.c_str()) != 0 ||
        chdir("gap4r5") != 0 ||
        unlink("sysinfo.gap") != 0 ||
        unlink("Makefile") != 0) {
        out(ERROR,"Could not switch sysinfo.gap symbolic link.");
        return ERROR;
    }
    if (towhat == 0) {
        out(OK,"Switching to default32 configuration.");
        if (symlink("sysinfo.gap-default32","sysinfo.gap") != 0 ||
            symlink("Makefile-default32","Makefile") != 0) {
            out(ERROR,"Could not switch sysinfo.gap symbolic link.");
            return ERROR;
        }
    } else {
        out(OK,"Switching to default64 configuration.");
        if (symlink("sysinfo.gap-default64","sysinfo.gap") != 0 ||
            symlink("Makefile-default64","Makefile") != 0) {
            out(ERROR,"Could not switch sysinfo.gap symbolic link.");
            return ERROR;
        }
    }
    if (chdir(targetdir.c_str()) != 0) {
        out(ERROR,"Could not switch sysinfo.gap symbolic link.");
        return ERROR;
    }
    return OK;
}

static Status BuildOldGAPPackage(string targetdir, string pkgname, Status err)
{
    string msg;
    int i;
    Status ret = OK;
    for (i = 0;i <= Double_Compile.num;i++) {
        if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
        string pkgdir = string("gap4r5/pkg/")+pkgname;
        string cmd;
        if (chdir(pkgdir.c_str())) {
            msg = string("Cannot change to the ")+pkgname+
                         " package's directory.";
            out(err,msg);
            ret = err;
            break;
        }
        msg = string("Running ./configure ../.. for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("./configure ../..")) {
            out(err,"Error in configure stage.");
            ret = err;
            break;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make")) {
            out(err,"Error in compilation stage.");
            ret = err;
            break;
        }
    }
    if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
    return ret;
}

static Status example_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"example", WARN); }
Component example("example",deps_onlyGAP,NULL,NULL,example_buildfunc);

static Status ace_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"ace", WARN); }
Component ace("ace",deps_onlyGAP,NULL,NULL,ace_buildfunc);

static Status atlasrep_buildfunc(string)
{
    if (chdir("gap4r5/pkg/atlasrep") < 0) {
        out(WARN,"Cannot change directory to \"gap4r5/pkg/atlasrep\".");
        return WARN;
    }
    if (chmod("datagens",01777) < 0 ||
        chmod("dataword",01777) < 0) {
        out(WARN,"Cannot set permissions for \"datagens\" and \"dataword\"."); 
        return WARN;
    }
    return OK;
}
Component atlasrep("atlasrep",deps_onlyGAP,NULL,NULL,atlasrep_buildfunc);

static Status cohomolo_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"cohomolo", WARN); }
Component cohomolo("cohomolo",deps_onlyGAP,NULL,NULL,cohomolo_buildfunc);

static Status fplsa_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"fplsa", WARN); }
Component fplsa("fplsa",deps_onlyGAP,NULL,NULL,fplsa_buildfunc);

static Status fr_prerequisites(string, Status depsresult)
{
    string path;
    if (depsresult != OK) return depsresult;
    if (Have_C_Header("gsl/gsl_vector.h") != OK) {
        out(WARN,"Need gsl library installed for component fr.");
        return WARN;
    }
    if (!which("appletviewer",path) ||
        !which("javac",path)) {
        out(WARN,"Need appletviewer and java compiler for component fr.");
        return WARN;
    }
    return OK;
}
static Status fr_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir,"fr",true,WARN); }
Component fr("fr",deps_onlyGAP,fr_prerequisites,NULL,fr_buildfunc);

static Status grape_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"grape", WARN); }
Component grape("grape",deps_onlyGAP,NULL,NULL,grape_buildfunc);

static Status guava_buildfunc(string targetdir)
{ 
    string msg;
    string pkgname = "guava3.11";
    int i;
    Status ret = OK;
    for (i = 0;i <= Double_Compile.num;i++) {
        if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
        string pkgdir = string("gap4r5/pkg/")+pkgname;
        string cmd;
        if (chdir(pkgdir.c_str())) {
            msg = string("Cannot change to the ")+pkgname+
                         " package's directory.";
            out(WARN,msg);
            ret = WARN;
            break;
        }
        msg = string("Running ./configure ../.. for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("./configure ../..")) {
            out(WARN,"Error in configure stage.");
            ret = WARN;
            break;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make")) {
            out(WARN,"Error in compilation stage.");
            ret = WARN;
            break;
        }
        msg = string("Running make install for ")+pkgname+" package...";
        out(OK,msg);
        if (sh("make install")) {
            out(WARN,"Error in installation stage.");
            ret = WARN;
            break;
        }
    }
    if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
    return ret;
}
Component guava("guava",deps_onlyGAP,NULL,NULL,guava_buildfunc);

static Status kbmag_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"kbmag", WARN); }
Component kbmag("kbmag",deps_onlyGAP,NULL,NULL,kbmag_buildfunc);

// Finishing off the installation:

const char *AllPkgs[] =
  { "io", "orb", "edim", "example", "Browse", "cvec", "ace", "atlasrep",
    "cohomolo", "fplsa", "fr", "grape", "guava", "kbmag",NULL };

static Status GAP_cp_scripts_func(string targetdir)
{
    if (Double_Compile.num == 1) {
        if (cp(targetdir+"gap4r5/bin/gap-default64.sh",targetdir+"gap64") 
                 == ERROR) return WARN;
        if (chmod((targetdir+"gap64").c_str(),0755) != 0)
            out(WARN,"Cannot make gap64 script executable.");
        if (cp(targetdir+"gap4r5/bin/gap-default64.sh",targetdir+"gap") 
                 == ERROR) return WARN;
        if (chmod((targetdir+"gap").c_str(),0755) != 0)
            out(WARN,"Cannot make gap script executable.");
        if (cp(targetdir+"gap4r5/bin/gap-default32.sh",targetdir+"gap32") 
                 == ERROR) return WARN;
        if (chmod((targetdir+"gap32").c_str(),0755) != 0)
            out(WARN,"Cannot make gap32 script executable.");
        return OK;
    } else {
        if (cp(targetdir+"gap4r5/bin/gap.sh",targetdir+"gap") 
                 == ERROR) return WARN;
        if (chmod((targetdir+"gap").c_str(),0755) != 0)
            out(WARN,"Cannot make gap32 script executable.");
        return OK;
    }
}
Component GAP_cp_scripts("GAP_cp_scripts",AllPkgs,NULL,NULL,
                         GAP_cp_scripts_func);

// Create a saved workspace:

static const char *GAP_workspace_deps[]
  = { "GAP_cp_scripts", NULL };

static Status GAP_workspace_func(string targetdir)
{
    int i;
    Status ret = OK;
    int fds[2];
    int pid;
    string cmd;
    string bits;

    for (i = 0;i <= Double_Compile.num;i++) {
        if (i == 0 && Double_Compile.num == 0) {
            out(OK,"Creating workspace for faster startup...");
            bits = "";
        } else if (i == 0 && Double_Compile.num == 1) {
            out(OK,"Creating workspace for faster startup for 32-bit...");
            bits = "32";
        } else {
            out(OK,"Creating workspace for faster startup for 64-bit...");
            bits = "64";
        }
        if (pipe(fds) != 0) {
            out(ERROR,"Cannot create pipe for workspace creation.");
            ret = WARN;
            break;
        }
        cmd = string("./gap")+bits+" -r";
        pid = shbg(cmd,fds[0]);
        if (pid < 0) {
            out(ERROR,"Cannot start process for workspace creation.");
            close(fds[0]);
            close(fds[1]);
            ret = WARN;
            break;
        }
        close(fds[0]);
        FILE *stdin = fdopen(fds[1],"w");
        setlinebuf(stdin);
        //fputs("??blablabla",stdin);  // To load help books
        cmd = string("SaveWorkspace(\"")+targetdir+
              "gap4r5/bin/ws"+bits+".ws\");\n";
        fputs(cmd.c_str(),stdin);
        fputs("quit;\n",stdin);
        fclose(stdin);
        int status,res;
        res = waitpid(pid,&status,0);
        if (WEXITSTATUS(status) != 0) {
            out(ERROR,"Creation of workspace did not work.");
            ret = WARN;
            break;
        }
        fstream outs;
        cmd = targetdir+"gap"+bits+"L";
        outs.open(cmd.c_str(),fstream::out | fstream::trunc);
        if (outs.fail()) {
            out(ERROR,"Cannot create script "+cmd);
            ret = WARN;
            break;
        }
        outs << "#!/bin/sh\n";
        outs << targetdir << "gap4r5/bin/gap";
        if (bits != "") 
            outs << "-default" << bits;
        outs << ".sh -L " << targetdir << "gap4r5/bin/ws"
             << bits << ".ws $*\n";
        outs.close();
        if (outs.fail()) {
            out(ERROR,"Cannot write script "+cmd);
            ret = WARN;
            break;
        }
        if (chmod(cmd.c_str(),0755) != 0) {
            out(ERROR,"Cannot make script "+cmd+" executable.");
            ret = WARN;
            break;
        }
    }
    if (ret == OK && Double_Compile.num == 1) {
        if (cp(targetdir+"gap64L",targetdir+"gapL") == OK &&
            chmod((targetdir+"gapL").c_str(),0755) == 0) 
            return OK;
        else {
            out(ERROR,"Could not copy script gap64L to gapL.");
            return WARN;
        }
    }
    return ret;
}

Component GAP_workspace("GAP_workspace",GAP_workspace_deps,NULL,NULL,
                        GAP_workspace_func);

