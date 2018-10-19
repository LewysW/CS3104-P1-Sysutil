//Defines red and green character codes for test output
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define WHITE   "\033[39m"

//Defines number of tests to be run by test suite
#define NUM_TESTS 10

//Functions for unit tests
int runTests(bool (*testFunctions[]) (), int numTests);
void initTests(bool (*testFunctions[]) ());
bool myitoaTest1();
bool myitoaTest2();
bool myitoaTest3();
bool myitoaTest4();
bool myStrCpyTest1();
bool myStrCpyTest2();
bool myStrCpyTest3();
bool myStrLenTest1();
bool myStrLenTest2();
bool myStrLenTest3();

/**
Runs a given list of unit tests.
Iterates over each unit test in the list of test functions. If calling the
current function returns true then the test has passed, otherwise it has failed.
@testFunctions - list of unit tests to run
@numTests - num tests to run
@returns - number of passing unit tests
*/
int runTests(bool (*testFunctions[]) (), int numTests) {
    //Number of tests that have passed
    int numPassingTests = 0;
    char printBuf[MAX_INT_DIGITS];

    //Lists result of each unit test
    int i;
    for (i = 0; i < numTests; i++) {
        if ((*testFunctions[i]) ()) {
            numPassingTests += 1;
            myWrite(GREEN);
            myWrite("\n***TEST ");
            myitoa(i + 1, printBuf);
            myWrite(printBuf);
            myWrite(" PASSED***\n");
        } else {
            myWrite(RED);
            myWrite("\n***TEST ");
            myitoa(i + 1, printBuf);
            myWrite(printBuf);
            myWrite(" FAILED***\n");
        }
    }

    //Displays total number of unit tests which have passed
    myWrite("\n***");
    (numPassingTests > 0) ? myitoa(numPassingTests, printBuf) : myStrCpy(printBuf, "0", 1);
    myWrite(printBuf);
    myWrite("/");
    myitoa(i, printBuf);
    myWrite(printBuf);
    myWrite(" TESTS PASSED***\n");

    myWrite(WHITE);
    return numPassingTests;
}

/**
Initialises the array of tests to be run with the address of each unit test function.
@testFunctions - list of test functions to be called
**/
void initTests(bool (*testFunctions[]) ()) {
    testFunctions[0] = myitoaTest1;
    testFunctions[1] = myitoaTest2;
    testFunctions[2] = myitoaTest3;
    testFunctions[3] = myitoaTest4;
    testFunctions[4] = myStrCpyTest1;
    testFunctions[5] = myStrCpyTest2;
    testFunctions[6] = myStrCpyTest3;
    testFunctions[7] = myStrLenTest1;
    testFunctions[8] = myStrLenTest2;
    testFunctions[9] = myStrLenTest3;
}

//Tests that myitoa returns string representation of 0
bool myitoaTest1() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(0, buf);
    return (strEqual(buf, "0"));
}

//Tests that myitoa returns string representation of 1
bool myitoaTest2() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(1, buf);
    return (strEqual(buf, "1"));
}

//Tests that myitoa does not return a negative string as an unsigned int is used
bool myitoaTest3() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(-1, buf);
    return (!strEqual(buf, "-1"));
}

//Tests that myitoa converts normal data to string
bool myitoaTest4() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(1024, buf);
    return (strEqual(buf, "1024"));
}

//Tests that myStrCpy can copy a normal string from one buffer to another
bool myStrCpyTest1() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "Hello";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, "Hello"));
}

//Tests that myStrCpy can copy an empty string
bool myStrCpyTest2() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, ""));
}

//Tests that myStrCpy can copy a pathname
bool myStrCpyTest3() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "~/Documents/CS3104/practicals/CS3104-P1-Sysutil";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, "~/Documents/CS3104/practicals/CS3104-P1-Sysutil"));
}

//Test myStrCpy with some normal test data
bool myStrLenTest1() {
    char buf[6] = "Hello";
    return (myStrLen(buf) == 5);
}

//Tests myStrLen with an empty string
bool myStrLenTest2() {
    char buf[10] = "";
    return (myStrLen(buf) == 0);
}

//Tests myStrLen with NULL
bool myStrLenTest3() {
    char* buf = NULL;
    return (myStrLen(buf) == -1);
}
