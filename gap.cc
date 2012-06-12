//
// gap.cc - part of the GAP installer BOB
//
// Copyright (C) by Max Neunhoeffer 2012
// This file is free software, see license information at the end.
//

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fstream>
#include <iostream>

#include "bob.h"

using namespace std;
using namespace BOB;

static const char *GAP_dependencies[]
  = { NULL };

static Status GAP_prerequisites(string, Status)
{
    Status res = OK;
    string path;
    bool hint = false;

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
    if (Have_C_Library("-lreadline -lncurses") != OK) {
        out(OK,"");
        out(WARN,"You do not have the readline library installed.");
        out(WARN,"GAP will be compiled without readline support.");
        hint = true;
    }
    if (Have_C_Header("readline/readline.h") != OK) {
        out(OK,"");
        out(WARN,"You do not have the headers for the readline library "
                 "installed.");
        out(WARN,"GAP will be compiled without readline support.");
        hint = true;
    }
    if (Double_Compile.str == "DoubleCompile") {
        if (Have_C_Library("-lreadline -lncurses",true) != OK) {
            out(OK,"");
            out(WARN,"You do not have the 32bit readline library installed.");
            out(WARN,"32bit-GAP will be compiled without readline support.");
            hint = true;
        }
        if (Have_C_Header("readline/readline.h",true) != OK) {
            out(OK,"");
            out(WARN,"You do not have the headers for the 32bit readline "
                     "library installed.");
            out(WARN,"32bit-GAP will be compiled without readline support.");
            hint = true;
        }
    }
    if (res != OK || hint) {
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "apt-get") {
          out(OK,"");
          out(ADVICE,"You can install the necessary tools by doing:");
          out(ADVICE,"  apt-get install gcc make m4 libc6-dev libreadline-dev");
          out(ADVICE,"with root privileges (using su or sudo).");
          if (Double_Compile.str == "DoubleCompile") {
            out(ADVICE,"For the 32-bit libraries do:");
            out(ADVICE,"  apt-get install lib32readline5-dev");
          }
          out(OK,"");
        }
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "rpm") {
          out(ADVICE,"You can install the necessary libraries from"
                     " the following rpm-packages:");
          out(ADVICE,"  gcc make m4 glibc-devel readline-devel");
          out(ADVICE,"Use");
          out(ADVICE,"  yum install gcc make m4 glibc-devel readline-devel");
          out(ADVICE,"with root privileges (using su or sudo).");
          if (Double_Compile.str == "DoubleCompile") {
            out(ADVICE,"For the 32-bit libraries do:");
            out(ADVICE,"  yum install glibc-devel.i686 readline-devel.i686");
          }
          out(OK,"");
        }
        if (Which_Architecture.str == "OSX") {
          out(OK,"");
          out(ADVICE,"First you need to install XCode with \"gcc\" \"make\""
                     " and \"m4\".");
          out(OK,"");
          if (which("port",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  sudo port install readline +universal");
            out(OK,"");
          }
          if (which("brew",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  brew install readline");
            out(OK,"");
          }
          if (which("fink",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  fink install readline5");
            out(OK,"");
          }
        }
    }
    return res;
}

static string GAP_archivename;

static Status GAP_getfunc(string targetdir)
{
    try {
        getind(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/Computer/Software/Gap/bob/GAP.link",
           GAP_archivename);
    }
    catch (Status e) {
        out(ERROR,"Could not download GAP archive.");
        return ERROR;
    }
    return OK;
}

vector<string> confignames;
vector<string> GAParchs;

static void DetermineGAParchs(void)
// Must be in targetdir.
{
    fstream f;
    int i;
    string line;

    if (GAParchs.size() > 0) return;
    for (i = 0;i <= Double_Compile.num;i++) {
        if (Double_Compile.num == 1 && i == 0)
            f.open("gap4r5/sysinfo.gap-default32",fstream::in);
        else if (Double_Compile.num == 1 && i == 1)
            f.open("gap4r5/sysinfo.gap-default64",fstream::in);
        else
            f.open("gap4r5/sysinfo.gap",fstream::in);
        if (f.fail()) {
            out(ERROR,"Cannot determine GAParchs.");
            return;
        }
        getline(f,line);
        f.close();
        GAParchs.push_back(line.substr(8));
    }
}

static string merkCFLAGS;
static string merkCPPFLAGS;
static string merkLDFLAGS;

static void GAP_sortoutenvironment(void)
{
    merkCFLAGS = getenvironment("CFLAGS");
    merkCPPFLAGS = getenvironment("CPPFLAGS");
    merkLDFLAGS = getenvironment("LDFLAGS");
    delenvironment("CFLAGS");
    delenvironment("CPPFLAGS");
    delenvironment("LDFLAGS");
    setenvironment("COPTS",merkCPPFLAGS + " " + merkCFLAGS);
    setenvironment("LOPTS",merkLDFLAGS);
    out(OK,"Using COPTS=\""+getenvironment("COPTS")+"\"");
    out(OK,"Using LOPTS=\""+getenvironment("LOPTS")+"\"");
}

static void GAP_restoreenvironment(void)
{
    if (merkCFLAGS != "") setenvironment("CFLAGS",merkCFLAGS);
    if (merkCPPFLAGS != "") setenvironment("CPPFLAGS",merkCPPFLAGS);
    if (merkLDFLAGS != "") setenvironment("LDFLAGS",merkLDFLAGS);
    delenvironment("COPTS");
    delenvironment("LOPTS");
}

static Status GAP_buildfunc(string)
{
    // By convention, we are in the target directory.
    if (access("gap4r5",F_OK) == 0) {
        if (interactive) {
            string answer;
            cout << "\nATTENTION!\n\n"
                 << "There seems to be an old installation of GAP 4.5 in the "
                 << "gap4r5 directory.\n\nRemove old installation?\n\n"
                 << "Answer \"yes\" to proceed or anything else to abort --> ";
            cin >> answer;
            if (answer != "yes") {
                out(ERROR,"Not removing old installation. Aborting.");
                exit(2);
            }
        }
        out(OK,"Removing old installation...");
        if (rmrf("gap4r5") == ERROR) 
            out(WARN,"Could not remove directory \"gap4r5\" recursively!");
    }
    out(OK,"Unpacking GAP archive...");
    try { unpack(GAP_archivename); }
    catch (Status e) {
        out(ERROR,"A problem occurred when extracting the archive.");
        return ERROR;
    }
    try { cd("gap4r5"); } catch (Status e) { return ERROR; }
    // Clean up environment due to GAP's funny behaviour:
    GAP_sortoutenvironment();
    string readlineopt = "";
    if (Which_Architecture.str == "OSX") {
        if (exists("/opt/local/lib/libreadline.dylib") &&
            exists("/opt/local/include/readline/readline.h")) {
            // MacPorts put the readline library there
            readlineopt = " --with-readline=/opt/local";
        } else if (exists("/opt/lib/libreadline.dylib") &&
                   exists("/opt/include/readline/readline.h")) {
            readlineopt = " --with-readline=/opt";
        } else if (isdir("/usr/local/Cellar/readline")) {
            // homebrew currently has it here.
            readlineopt = " --with-readline=/usr/local/Cellar/readline/6.2.2";
            // FIXME: Read off version number here
        } else if (finkpath != "" && 
                   exists(finkpath+"include/readline/readline.h") &&
                   exists(finkpath+"lib/libreadline.dylib")) {
            readlineopt = " --with-readline="+finkpath;
        }
    }
    if (Double_Compile.str == "DoubleCompile") {
        out(OK,"Compiling for both 32-bit and 64-bit...");
        out(OK,"Running ./configure ABI=32"+readlineopt+" for GAP...");
        try { sh("./configure ABI=32"+readlineopt); }
        catch (Status e) {
            out(ERROR,"Error in configure stage.");
            GAP_restoreenvironment();
            return ERROR;
        }
        out(OK,"Running make for GAP...");
        try { sh("make"); }
        catch (Status e) {
            out(ERROR,"Error in compilation stage.");
            GAP_restoreenvironment();
            return ERROR;
        }
        confignames.push_back("default32");
    }
    out(OK,"Running ./configure"+readlineopt+" for GAP...");
    try { sh("./configure"+readlineopt); }
    catch (Status e) {
        out(ERROR,"Error in configure stage.");
        GAP_restoreenvironment();
        return ERROR;
    }
    out(OK,"Running make for GAP...");
    try { sh("make"); }
    catch (Status e) {
        out(ERROR,"Error in compilation stage.");
        GAP_restoreenvironment();
        return ERROR;
    }
    GAP_restoreenvironment();
    if (Which_Wordsize.num == 32)
        confignames.push_back("default32");
    else
        confignames.push_back("default64");
    return OK;
}

Component GAP("GAP",GAP_dependencies,GAP_prerequisites,
                    GAP_getfunc,GAP_buildfunc);


static Status BuildGAPPackage(string, string pkgname, bool withm32,
                              Status err, bool withmakeclean = false,
                              bool withabi32 = false)
{
    string msg;
    string pkgdir = string("gap4r5/pkg/")+pkgname;
    string cmd;
    Status res = OK;
    try { cd(pkgdir); } catch (Status e) { return err; }
    if (Double_Compile.str == "DoubleCompile") {
        cmd = string("./configure CONFIGNAME=default32");
        if (withm32) cmd += " CFLAGS=-m32";
        if (withabi32) cmd += " ABI=32";
        msg = string("Running "+cmd+" for ")+pkgname+" package...";
        out(OK,msg);
        try { sh(cmd); }
        catch (Status e) {
            out(err,"Error in configure stage.");
            res = err;
            goto dosixtyfour;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make"); }
        catch (Status e) {
            out(err,"Error in compilation stage.");
            res = err;
            goto dosixtyfour;
        }
        if (withmakeclean) {
            msg = string("Running make clean for ")+pkgname+" package...";
            out(OK,msg);
            try { sh("make clean"); }
            catch (Status e) {
                out(err,"Error in make clean stage.");
                res = err;
            }
        }
      dosixtyfour:
        msg = string("Running ./configure CONFIGNAME=default64 for ")+pkgname+
                     " package...";
        out(OK,msg);
        try { sh("./configure CONFIGNAME=default64"); }
        catch (Status e) {
            out(err,"Error in configure stage.");
            return err;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make"); }
        catch (Status e) {
            out(err,"Error in compilation stage.");
            return err;
        }
        return res;
    } else {
        msg = string("Running ./configure for ")+pkgname+ " package...";
        out(OK,msg);
        try { sh("./configure"); }
        catch (Status e) {
            out(err,"Error in configure stage.");
            return err;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make"); }
        catch (Status e) {
            out(err,"Error in compilation stage.");
            return err;
        }
    }
    return OK;
}

static const char *deps_onlyGAP[]
  = { "!GAP", NULL };

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
    Status ret;
    bool hint = false;
    ret = OK;
    if (depsresult != OK) return depsresult;
    if (Have_C_Library("-lncurses") != OK ||
        Have_C_Header("ncurses.h") != OK) {
        out(OK,"");
        out(WARN,"Need ncurses library for component Browse.");
        ret = WARN;
    }
    if (Have_C_Library("-lpanel") != OK ||
        Have_C_Header("panel.h") != OK) {
        out(OK,"");
        out(WARN,"Need panel library for component Browse.");
        ret = WARN;
    }
    if (Double_Compile.str == "DoubleCompile") {
        if (Have_C_Library("-lncurses",true) != OK ||
            Have_C_Header("ncurses.h",true) != OK) {
            out(OK,"");
            out(WARN,"Need 32bit ncurses library for component "
                     "Browse to work in 32bit GAP.");
            hint = true;
        }
        if (Have_C_Library("-lpanel",true) != OK ||
            Have_C_Header("panel.h",true) != OK) {
            out(OK,"");
            out(WARN,"Need 32bit panel library for component "
                     "Browse to work in 32bit GAP.");
            hint = true;
        }
    }

    if (ret != OK || hint) {
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "apt-get") {
          out(OK,"");
          out(ADVICE,"You can install the necessary libraries by doing:");
          out(ADVICE,"  apt-get install libncurses-dev");
          out(ADVICE,"with root privileges (using su or sudo).");
          if (Double_Compile.str == "DoubleCompile") {
            out(ADVICE,"For the 32-bit libraries do:");
            out(ADVICE,"  apt-get install lib32ncurses5-dev");
          }
          out(OK,"");
        }
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "rpm") {
          out(ADVICE,"You can install the necessary libraries from"
                     " the following rpm-packages:");
          out(ADVICE,"  ncurses-devel");
          out(ADVICE,"Use");
          out(ADVICE,"  yum install ncurses-devel");
          out(ADVICE,"with root privileges (using su or sudo).");
          if (Double_Compile.str == "DoubleCompile") {
            out(ADVICE,"For the 32-bit libraries do:");
            out(ADVICE,"  yum install ncurses-devel.i686");
          }
          out(OK,"");
        }
        if (Which_Architecture.str == "OSX") {
          if (which("port",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  sudo port install ncurses +universal");
            out(OK,"");
          }
          if (which("brew",path)) {
            out(ADVICE,"Currently you cannot install ncurses via homebrew.");
            out(OK,"");
          }
          if (which("fink",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  fink install libncurses5");
            out(OK,"");
          }
        }
    }
    return ret;
}

static Status Browse_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "Browse", false, WARN); }
Component Browse("Browse",deps_onlyGAP,Browse_prerequisites,NULL,
                 Browse_buildfunc);

static Status nq_prerequisites(string, Status depsresult)
{
    string path;
    Status ret;
    if (depsresult != OK) return depsresult;
    ret = OK;
    if (!(which("gawk",path) || which("mawk",path) || which("nawk",path) ||
          which("awk",path))) {
        out(OK,"");
        out(WARN,"Need awk for component nq-Pkg.");
        ret = WARN;
    }
    if (ret != OK) {
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "apt-get") {
          out(OK,"");
          out(ADVICE,"You can install the necessary tools by doing:");
          out(ADVICE,"  apt-get install mawk");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "rpm") {
          out(ADVICE,"You can install the necessary libraries from"
                     " the following rpm-packages:");
          out(ADVICE,"  mawk");
          out(ADVICE,"Use");
          out(ADVICE,"  yum install mawk");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "OSX") {
          out(OK,"");
          if (which("port",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  sudo port install gawk");
            out(OK,"");
          }
          if (which("brew",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  brew install gawk");
            out(OK,"");
          }
          if (which("fink",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  fink install gawk");
            out(OK,"");
          }
        }
    }
    return ret;
}

static Status nq_buildfunc(string targetdir)
{ return BuildGAPPackage(targetdir, "nq-2.4", true, WARN, true, true); }
Component nq("nq",deps_onlyGAP,nq_prerequisites,NULL,nq_buildfunc);


static Status switch_sysinfo_link(string targetdir, int towhat)
{
    if (Double_Compile.str == "SingleCompile") return OK;

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
    try { cd(targetdir); } catch (Status e) {
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
            continue;
        }
        msg = string("Running ./configure ../.. for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("./configure ../.."); }
        catch (Status e) {
            out(err,"Error in configure stage.");
            ret = err;
            continue;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make"); }
        catch (Status e) {
            out(err,"Error in compilation stage.");
            ret = err;
            continue;
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
    try { cd("gap4r5/pkg/atlasrep"); } catch (Status e) { return WARN; }
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

static Status fplsa_patchfunc(string targetdir)
{
    string edscriptname;
    if (Which_Architecture.str != "OSX") return OK;
    out(OK,"Patching fplsa for Mac OSX...");
    try {
        getind(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/Computer/Software/Gap/bob/fplsa.edit.link",
           edscriptname);
    }
    catch (Status e) {
        out(ERROR,"Could not download patch for fplsa.");
        return WARN;
    }
    try { edit(edscriptname); } catch (Status e) { return WARN; }
    return OK;
}

static Status fplsa_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"fplsa", WARN); }
Component fplsa("fplsa",deps_onlyGAP,NULL,fplsa_patchfunc,fplsa_buildfunc);

static Status citrus_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"citrus-0.9", WARN); }
Component citrus("citrus",deps_onlyGAP,NULL,NULL,citrus_buildfunc);

static Status fr_prerequisites(string, Status depsresult)
{
    string path;
    if (depsresult != OK) return depsresult;
    if (!which("wget",path)) {
        out(OK,"");
        out(WARN,"Need wget utility installed for component fr.");
        out(OK,"");
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "apt-get") {
          out(ADVICE,"You can install the necessary tool by doing:");
          out(ADVICE,"  apt-get install wget");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "rpm") {
          out(ADVICE,"You can install the necessary tools from"
                     " the following rpm-packages:");
          out(ADVICE,"  wget");
          out(ADVICE,"Use");
          out(ADVICE,"  yum install wget");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "OSX") {
          out(OK,"");
          if (which("port",path)) {
            out(ADVICE,"You can install the necessary additional tools "
                       "by doing:");
            out(ADVICE,"  sudo port install wget");
            out(OK,"");
          }
          if (which("brew",path)) {
            out(ADVICE,"You can install the necessary additional tools "
                       "by doing:");
            out(ADVICE,"  brew install wget");
            out(OK,"");
          }
          if (which("fink",path)) {
            out(ADVICE,"You can install the necessary additional tools "
                       "by doing:");
            out(ADVICE,"  fink install wget");
            out(OK,"");
          }
        }
    }
    if (!which("appletviewer",path) ||
        !which("javac",path)) {
        out(WARN,"Need appletviewer and java compiler for component fr.");
    }
    return OK;
}
static Status fr_buildfunc(string targetdir)
{ 
    vector<string> PackageInfo;
    try { 
        readlines(targetdir+"gap4r5/pkg/fr/PackageInfo.g",PackageInfo); 
        if (PackageInfo.size() >= 10 &&
            PackageInfo[9] == "Version := \"1.2.6.4\",") {
            out(WARN,"This version 1.2.6.4 of fr has known compilation "
                     "problems.");
        }
    }
    catch (Status e) { 
        out(WARN,"Could not read gap4r5/pkg/fr/PackageInfo.g");
    }
    return BuildGAPPackage(targetdir,"fr",true,WARN); 
}
Component fr("fr",deps_onlyGAP,fr_prerequisites,NULL,fr_buildfunc);

static Status grape_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"grape", WARN); }
Component grape("grape",deps_onlyGAP,NULL,NULL,grape_buildfunc);

static Status Gauss_buildfunc(string targetdir)
{ return BuildOldGAPPackage(targetdir,"Gauss", WARN); }
Component Gauss("Gauss",deps_onlyGAP,NULL,NULL,Gauss_buildfunc);

static Status guava_buildfunc(string targetdir)
{ 
    string msg;
    string pkgname = "guava-3.12";
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
        mkdir("bin",0755);  // intentionally ignore errors here
        msg = string("Running ./configure ../.. for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("./configure ../.."); }
        catch (Status e) {
            out(WARN,"Error in configure stage.");
            ret = WARN;
            break;
        }
        msg = string("Running make for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make"); }
        catch (Status e) {
            out(WARN,"Error in compilation stage.");
            ret = WARN;
            break;
        }
        msg = string("Running make install for ")+pkgname+" package...";
        out(OK,msg);
        try { sh("make install"); }
        catch (Status e) {
            out(WARN,"Error in installation stage.");
            ret = WARN;
            break;
        }
    }
    if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
    return ret;
}
Component guava("guava",deps_onlyGAP,NULL,NULL,guava_buildfunc);

static Status kbmag_prerequisites(string, Status depsresult)
{
    if (depsresult != OK) return depsresult;
    if (Which_Wordsize.num == 64 &&
        Can_Compile_32bit.num != 0) {
        out(WARN,"Need to compile in 32-bit mode using -m32 for component "
                 "kbmag.");
        out(WARN,"Please use gcc or clang and install the necessary 32-bit "
                 "libraries.");
        return WARN;
    }
    return OK;
}

static Status kbmag_buildfunc(string targetdir)
{
    string msg;
    int i;
    Status ret = OK;
    for (i = 0;i <= Double_Compile.num;i++) {
        if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
        string pkgdir = "gap4r5/pkg/kbmag";
        string cmd;
        if (chdir(pkgdir.c_str())) {
            msg = "Cannot change to the kbmag package's directory.";
            out(WARN,msg);
            ret = WARN;
            break;
        }
        msg = "Running ./configure ../.. for kbmag package...";
        out(OK,msg);
        try { sh("./configure ../.."); }
        catch (Status e) {
            out(WARN,"Error in configure stage.");
            ret = WARN;
            break;
        }
        out(OK,"Running make for kbmag package...");

        if (Which_Wordsize.num == 64)
            msg = "make COPTS=-O2~-m32";
        else
            msg = "make COPTS=-O2";
        try { sh(msg); }
        catch (Status e) {
            out(WARN,"Error in compilation stage.");
            ret = WARN;
            break;
        }
    }
    if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
    return ret;
}
Component kbmag("kbmag",deps_onlyGAP,kbmag_prerequisites,NULL,kbmag_buildfunc);

static Status carat_buildfunc(string targetdir)
{
    string topdir;
    DetermineGAParchs();
    try { cd("gap4r5/pkg/carat"); } catch (Status e) { return WARN; }
    try {unpack("carat-2.1b1.tgz"); }
    catch (Status e) {
        out(ERROR,"Cannot unpack carat archive.");
        return WARN;
    }
    try { sh("ln -sfn carat-2.1b1/bin bin"); }
    catch (Status e) {
        out(ERROR,"Cannot create bin link for carat package.");
        return WARN;
    }
    try { cd("carat-2.1b1"); } catch (Status e) { return WARN; }

    // Now patch the Makefile for Mac OSX:
    string edscriptname;
    if (Which_Architecture.str == "OSX") {
        out(OK,"Patching carat for Mac OSX...");
        try {
            getind(targetdir,
               "http://www-groups.mcs.st-and.ac.uk/~neunhoef/Computer/Software/Gap/bob/carat.edit.link",
               edscriptname);
        }
        catch (Status e) {
            out(ERROR,"Could not download patch for carat.");
            return WARN;
        }
        vector<string> edscript;
        try { 
            readlines(edscriptname,edscript);
            edscript[edscript.size()-1] 
              += " -I"+targetdir+"gap4r5/bin/"+GAParchs[Double_Compile.num]+
                 "/extern/gmp/include"+
                 " -L"+targetdir+"gap4r5/bin/"+GAParchs[Double_Compile.num]+
                 "/extern/gmp/lib";
            edit(edscript);
        }
        catch (Status e) { 
            out(ERROR,"Failed to patch carat.");
            return WARN; 
        }
    }

    topdir = targetdir + "gap4r5/pkg/carat/carat-2.1b1";
    try { sh("make TOPDIR="+topdir); }
    catch (Status e) {
        out(ERROR,"Failed to build standalone carat program.");
        return WARN;
    }
    try { cd("bin"); } catch (Status e) { return WARN; }
    vector<string> filenames;
    try { listdir(".",filenames); }
    catch (Status e) {
        out(ERROR,"Could not read off architecture string for carat.");
        return WARN;
    }
    size_t i;
    for (i = 0;i < filenames.size();i++) {
        if (filenames[i] != "Makefile" && filenames[i] != "config.guess") break;
    }
    if (i >= filenames.size()) {
        out(ERROR,"Could not find architecture string for carat.");
        return WARN;
    }
    string targetbin = filenames[i];
    for (i = 0;i < GAParchs.size();i++)
        try { sh(string("ln -sfn ")+targetbin+" "+GAParchs[i]); }
        catch (Status e) {
            out(ERROR,"Cannot create symbolic link to carat.");
            return WARN;
        }
    return OK;
}
Component carat("carat",deps_onlyGAP,NULL,NULL,carat_buildfunc);


static Status xgap_prerequisites(string, Status)
{
    Status res = OK;
    string path;
    if (Have_C_Library("-lXaw -lXmu -lXt -lXext -lX11  -lSM -lICE") != OK) {
        out(WARN,"You have not enough X11 libraries installed, thus "
                 "XGAP cannot run.");
        res = WARN;
    }
    if (Have_C_Header("X11/X.h") != OK ||
        Have_C_Header("X11/Xlib.h") != OK ||
        Have_C_Header("X11/Intrinsic.h") != OK ||
        Have_C_Header("X11/Xaw/XawInit.h") != OK ||
        Have_C_Header("X11/keysym.h") != OK) {
        out(WARN,"You have not enough X11 headers installed, thus "
                  "XGAP cannot be compiled.");
        res = WARN;
    }
    if (res != OK) {
        out(WARN,"The following libraries with headers are required "
                  "for XGAP:");
        out(WARN,"  libXaw libXmu libXt libXext libX11 libSM libICE");
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "apt-get") {
          out(OK,"");
          out(ADVICE,"You can install the necessary libraries by doing:");
          out(ADVICE,"  apt-get install libx11-dev libxt-dev libxaw7-dev");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "LINUX" &&
            Which_OS_Variant.str == "rpm") {
          out(ADVICE,"You can install the necessary libraries from"
                     " the following rpm-packages:");
          out(ADVICE,"  libX11-devel libXaw-devel libXmu-devel libXt-devel"
                     "  libXext-devel libSM-devel libICE-devel");
          out(ADVICE,"Use");
          out(ADVICE,"  yum install libX11-devel libXaw-devel libXmu-devel "
                     "libXt-devel libXext-devel \\ ");
          out(ADVICE,"              libSM-devel libICE-devel");
          out(ADVICE,"with root privileges (using su or sudo).");
          out(OK,"");
        }
        if (Which_Architecture.str == "OSX") {
          out(OK,"");
          if (which("port",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  port install xorg-libX11 xorg-libXaw");
            out(ADVICE,"with root privileges (using su or sudo).");
            out(OK,"");
          }
          if (which("brew",path)) {
            out(ADVICE,"You can currently not install X11 using homebrew.");
            out(OK,"");
          }
          if (which("fink",path)) {
            out(ADVICE,"You can install the necessary additional libraries "
                       "by doing:");
            out(ADVICE,"  fink install x11-dev");
            out(OK,"");
          }
        }
    }
    return res;
}

static string XGAP_archivename;

static Status xgap_getfunc(string)
{
#if 0
    try {
        getind(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/Computer/Software/Gap/bob/XGAP.link",
           XGAP_archivename);
    }
    catch (Status e) {
        out(ERROR,"Could not download XGAP archive.");
        return ERROR;
    }
#endif
    return OK;
}

static Status xgap_buildfunc(string targetdir)
{ 
    Status res;
#if 0
    if (chdir("gap4r5/pkg") != 0) {
        out(ERROR,"Cannot change to GAP's pkg directory.");
        return WARN;
    }
    out(OK,"Unpacking XGAP archive...");
    try { unpack(XGAP_archivename); }
    catch (Status e) {
        out(ERROR,"A problem occurred when extracting the archive.");
        return WARN;
    }
    if (chdir(targetdir.c_str()) != 0) {
        out(ERROR,"Cannot change to target directory.");
        return WARN;
    }
#endif
    res = BuildGAPPackage(targetdir, "xgap", false, WARN); 
    if (res != OK) return res;
    res = cp(targetdir+"gap4r5/pkg/xgap/bin/xgap.sh",targetdir+"xgap");
    if (res != OK) {
        out(WARN,"Could not copy startup script for xgap.");
        return WARN;
    }
    if (chmod((targetdir+"xgap").c_str(),0755) != 0)
        out(WARN,"Cannot make xgap script executable.");
    return OK;
}
Component xgap("xgap",deps_onlyGAP,
               xgap_prerequisites,xgap_getfunc,xgap_buildfunc);

static Status anupq_prerequisites(string, Status depsresult)
{
    if (depsresult != OK) return depsresult;
    if (Which_Wordsize.num == 64 &&
        Can_Compile_32bit.num != 0) {
        out(WARN,"Need to compile in 32-bit mode using -m32 for component "
                 "anupq.");
        out(WARN,"Please use gcc or clang and install the necessary 32-bit "
                 "libraries.");
        return WARN;
    }
    return OK;
}

static Status anupq_buildfunc(string targetdir)
{
    string msg;
    int i;
    Status ret = OK;
    for (i = 0;i <= Double_Compile.num;i++) {
        if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
        string pkgdir = "gap4r5/pkg/anupq";
        string cmd;
        if (chdir(pkgdir.c_str())) {
            msg = "Cannot change to the anupq package's directory.";
            out(WARN,msg);
            ret = WARN;
            break;
        }
        msg = "Running ./configure ../.. for anupq package...";
        out(OK,msg);
        try { sh("./configure ../.."); }
        catch (Status e) {
            out(WARN,"Error in configure stage.");
            ret = WARN;
            break;
        }
        out(OK,"Running make for anupq package...");

        if (Which_Wordsize.num == 64)
            msg = "make linux-iX86-gcc2 COPTS=-m32~-g LOPTS=-m32~-g";
        else
            msg = "make linux-iX86-gcc2";
        try { sh(msg); }
        catch (Status e) {
            out(WARN,"Error in compilation stage.");
            ret = WARN;
            break;
        }
    }
    if (switch_sysinfo_link(targetdir,i) == ERROR) return ERROR;
    return ret;
}
Component anupq("anupq",deps_onlyGAP,anupq_prerequisites,NULL,anupq_buildfunc);

// Finishing off the installation:

const char *AllPkgs[] =
  { " io", " orb", " edim", " example", " Browse", " cvec", " ace", " atlasrep",
    " cohomolo", " fplsa", " fr", " grape", " guava", " kbmag", " carat", 
    " xgap", " Gauss", " anupq", NULL };

static Status GAP_cp_scripts_func(string targetdir)
{
    if (Double_Compile.str == "DoubleCompile") {
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
  = { "!GAP_cp_scripts", NULL };

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

//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

