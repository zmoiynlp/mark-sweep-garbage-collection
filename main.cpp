#include <iostream>
#include <string>
#include <vector>
#include <list>
#include<unordered_map>
#include <unordered_set>
#include "sbt.h"
#include "gc.h"

using namespace std;

#define dump(x) do{cout<<#x<<"="<<(x)<<"\n";}while(0)


int *g() {
    return New<int>(6);
}

int *f() {
    enter();
    int ff, *a(nullptr), *b(nullptr), *c(nullptr), *arr(nullptr);
    setRoot(a, b, c, arr);
    a = New<int>(1);
    arr = New<int>(8);
    b = g();
    c = New<int>(3);
    cout << "\n";

    return finish(a);
}
void h(){}

int main() {
    enter();
    SBT<int> t;
    setRoot(t);
    for (int i = 1; i <= 10; ++i)t.insert(i);
    collect();
    cout<<t<<"\n\n";

    for (int i = 1; i < 10; ++i)t.remove(i);
    collect();
    cout<<t<<"\n\n";

    f();
    cout<<t<<"\n\n";
    collect();
    finish();

    collect();

    return 0;
}
