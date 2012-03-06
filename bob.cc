#include "bob.h"

#include <unistd.h>
#include <iostream>

using namespace BOB;

int HaveMakeTest()
{
    if (access("/usr/bin/make",X_OK))
        return 0;
    else
        return 1;
}
Test HaveMake("HaveMake",1,HaveMakeTest);

int main(int argc, const char *argv[])
{
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

