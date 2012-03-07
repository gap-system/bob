#include "bob.h"

#include <unistd.h>
#include <iostream>

using namespace BOB;

// Some standard tests:

int HaveMakeTest()
{
    if (access("/usr/bin/make",X_OK))
        return 0;
    else
        return 1;
}
Test HaveMake("HaveMake",1,HaveMakeTest);


string targetdir;
int verbose = 1;

int main(int argc, char * const argv[])
{
    int opt;

    // Initialise targetdir with the realpath of the current dir:
    char *cwd = getcwd(NULL,1024);
    targetdir = cwd;
    free(cwd);

    while ((opt = getopt(argc, argv, "vqt:")) != -1) {
        switch (opt) {
          case 'v':
              verbose = 2;
              break;
          case 'q':
              verbose = 0;
              break;
          case 't':
              targetdir = optarg;
              break;
          default: /* '?' */
              cerr << "Usage: " << argv[0] << "[-v] [-q] [-t TARGETDIR]\n";
              return -1;
        }
    }
    // At this stage we ignore further arguments.

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
    t = Test::find("HaveMake");
    cout << "Do we have make? Answer: " << t->intres << "\n";
    return 0;
}

