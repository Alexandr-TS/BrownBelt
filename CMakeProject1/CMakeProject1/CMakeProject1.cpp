#include "test_runner.h"
#include "profile.h"
#include <iostream>
#include <cstdio>

using namespace std;

pair<int, int> f() {
	return { 1, 2 };
}


int main() {
	auto [a, b] = f();
	cout << a << " " << b << endl;
}
