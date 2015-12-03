#include "syscall.h"


int main() {

	Exec("../test/ApplicationClerk", sizeof("../test/ApplicationClerk"));
	Exec("../test/PictureClerk", sizeof("../test/PictureClerk"));
	Exec("../test/PassportClerk", sizeof("../test/PassportClerk"));
	Exec("../test/Cashier", sizeof("../test/Cashier"));
	Exec("../test/Customer", sizeof("../test/Customer"));
	Exec("../test/Manager", sizeof("../test/Manager"));
	Exec("../test/Senator", sizeof("../test/Senator"));



}