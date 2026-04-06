---
title: Unit tests
---

All unit tests are implemented as a part of a @b3d::TestSuite class. You can create your own test suites, or add tests to the existing ones. 

To register new tests call @B3D_ADD_TEST in the test suite's constructor. The test method must not accept any parameters or return any values. To report test failure call @B3D_TEST_ASSERT or @B3D_TEST_ASSERT_MSG. If neither of those trigger, test is assumed to be successful.

~~~~~~~~~~~~~{.cpp}
class MyTestSuite : TestSuite
{
public:
	MyTestSuite()
	{
		B3D_ADD_TEST(MyTestSuite::myTest);
	}
	
private:
	void myTest()
	{
		B3D_TEST_ASSERT_MSG(2 + 2 == 4, "Something really bad is going on.");
	}
};
~~~~~~~~~~~~~

Tests are executed through the **UnitTestRunner** executable, which dynamically loads test suite plugins (`FrameworkTests.dll`, `EditorTests.dll`). Each test suite is organized into layers:

 - **Utility** - Tests that don't require application initialization
 - **Core** - Tests requiring **Application** to be running
 - **Editor** - Tests requiring **EditorApplication** to be running

You can control test execution via command-line arguments:

 - `--test-layer` - Run only tests from a specific layer
 - `--test-output-format` - Output format: `console` (default) or `json`
 - `--test-output-path` - Path for test output file

The runner also supports snapshot testing for visual validation:

 - `--enable-test-snapshot` - Enable snapshot capture
 - `--test-name` - Name of the test to snapshot
 - `--capture-frame` - Frame number to capture
