#include "testsuite.h"

int main(int, const char*[])
{
	test::TestSuite::the().run();

	return 0;
}
