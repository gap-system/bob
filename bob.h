// bob.h - Copyright 2012 by Max Neunhoeffer

#include <string.h>
#include <string>
#include <vector>

using namespace std;

typedef int (*inttestfunc_t)(void);
typedef string (*strtestfunc_t)(void);

class Test;

vector<Test *> &alltests(void)
{
    static vector<Test *> *alltests_ = new vector<Test *>;
    return *alltests_;
}

class Test {
  public:
    const char *name;
    int priority;
    inttestfunc_t inttest;
    strtestfunc_t strtest;
    int intres;
    string strres;

    Test(const char *name, int priority, inttestfunc_t tester)
        :name(name),priority(priority),inttest(tester),strtest(NULL)
    {
        alltests().push_back(this);
    }

    Test(const char *name, int priority, strtestfunc_t tester)
        :name(name),priority(priority),inttest(NULL),strtest(tester)
    {
        alltests().push_back(this);
    }

    void run(void)
    {
        if (inttest) intres = inttest();
        else         strres = strtest();
    }

    static Test *find(const char *name)
    {
        static vector<Test *> &tests = alltests();
        int i;
        Test *t;
        for (i = 0;i < tests.size();i++) {
            t = tests[i];
            if (!strcmp(name,t->name)) return t;
        }
        return NULL;
    }
};

class Component {
  public:
    const char *name;
    const char *version;

};
