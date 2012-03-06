#include <iostream>
#include <string>

using namespace std;

string x[3] = {"Max","Anja","Savina"};

string doer(string a, string b)
{
    string res = a;
    res += b;
    return res;
}

int main(void)
{
    string s = "Max";
    s = doer("abc","def");
    cout << s << "\n";
    return 0;
}

