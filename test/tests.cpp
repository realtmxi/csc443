#include <cstdio>

#include "../src/avl_tree.h"

void
assertEqual(int expected, int actual, const char *testName,
            int &testsPassed, int &testsFailed)
{
    if (expected == actual)
    {
        printf("%s: PASSED\n", testName);
        testsPassed++;
    }
    else
    {
        printf("%s: FAILED: expected %d, got %d\n", testName, expected, actual);
        testsFailed++;
    }
}

void
testAVLTreeInsert(int &totalPassed, int &totalFailed)
{
    printf("\n\nINSERT TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" INSERTING VALUES\n");
    tree.put(10, 100);
    tree.put(20, 200);
    tree.put(30, 300);

    assertEqual(100, tree.get(10), "  10 exists after insert", testsPassed, testsFailed);
    assertEqual(200, tree.get(20), "  20 exists after insert", testsPassed, testsFailed);
    assertEqual(300, tree.get(30), "  30 exists after insert", testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
testAVLTreeGet(int &totalPassed, int &totalFailed)
{
    printf("\n\nGET TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" INSERTING VALUES\n");
    tree.put(10, 100);
    tree.put(20, 200);
    tree.put(30, 300);

    assertEqual(100, tree.get(10), "  Test get(10)", testsPassed, testsFailed);
    assertEqual(200, tree.get(20), "  Test get(20)", testsPassed, testsFailed);
    assertEqual(300, tree.get(30), "  Test get(30)", testsPassed, testsFailed);

    // Test non-existent key
    assertEqual(-1, tree.get(40), "  return -1 for non-existent key", testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
testAVLTreeScan(int &totalPassed, int &totalFailed)
{
    printf("\n\nSCAN TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" SCAN ON EMPTY TREE\n");
    auto result = tree.scan(1, 10);
    assertEqual(0, result.size(), "  results size is 0", testsPassed, testsFailed);

    printf(" SCAN OUT OF RANGE\n");
    tree.put(10, 100);
    tree.put(20, 200);
    tree.put(30, 300);
    result = tree.scan(40, 50);
    assertEqual(0, result.size(), "  results size is 0", testsPassed, testsFailed);

    printf(" SCAN FOR ALL RESULTS\n");
    result = tree.scan(5, 35);
    assertEqual(3, result.size(), "  results size is 3", testsPassed, testsFailed);
    assertEqual(100, result[0].second, "  key 10 exists", testsPassed, testsFailed);
    assertEqual(200, result[1].second, "  key 20 exists", testsPassed, testsFailed);
    assertEqual(300, result[2].second, "  key 30 exists", testsPassed, testsFailed);

    printf(" SCAN FOR PARTIAL RESULTS\n");
    result = tree.scan(15, 25);
    assertEqual(1, result.size(), "  results size is 1", testsPassed, testsFailed);
    assertEqual(200, result[0].second, "  key 20 exists", testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
testAVLTree()
{
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;

    printf("AVL TREE TESTS:\n");
    testAVLTreeInsert(totalTestsPassed, totalTestsFailed);
    testAVLTreeGet(totalTestsPassed, totalTestsFailed);
    testAVLTreeScan(totalTestsPassed, totalTestsFailed);

    printf("\n\nTEST SUMMARY\n");
    printf("Passed: %d\n", totalTestsPassed);
    printf("Failed: %d\n", totalTestsFailed);
}

int
main()
{
    testAVLTree();
    return 0;
}
