#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <filesystem>

#include "../src/avl_tree.h"
#include "../src/database.h"

void
assertEqual(int expected, int actual, const char *testName, int &testsPassed,
            int &testsFailed)
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

    assertEqual(100, tree.get(10), "  10 exists after insert", testsPassed,
                testsFailed);
    assertEqual(200, tree.get(20), "  20 exists after insert", testsPassed,
                testsFailed);
    assertEqual(300, tree.get(30), "  30 exists after insert", testsPassed,
                testsFailed);

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
    assertEqual(-1, tree.get(40), "  return -1 for non-existent key",
                testsPassed, testsFailed);

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
    assertEqual(0, result.size(), "  results size is 0", testsPassed,
                testsFailed);

    printf(" SCAN OUT OF RANGE\n");
    tree.put(10, 100);
    tree.put(20, 200);
    tree.put(30, 300);
    result = tree.scan(40, 50);
    assertEqual(0, result.size(), "  results size is 0", testsPassed,
                testsFailed);

    printf(" SCAN FOR ALL RESULTS\n");
    result = tree.scan(5, 35);
    assertEqual(3, result.size(), "  results size is 3", testsPassed,
                testsFailed);
    assertEqual(100, result[0].second, "  key 10 exists", testsPassed,
                testsFailed);
    assertEqual(200, result[1].second, "  key 20 exists", testsPassed,
                testsFailed);
    assertEqual(300, result[2].second, "  key 30 exists", testsPassed,
                testsFailed);

    printf(" SCAN FOR PARTIAL RESULTS\n");
    result = tree.scan(15, 25);
    assertEqual(1, result.size(), "  results size is 1", testsPassed,
                testsFailed);
    assertEqual(200, result[0].second, "  key 20 exists", testsPassed,
                testsFailed);

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

// Test Open and Close
void
testDatabaseOpenClose(int &totalPassed, int &totalFailed)
{
    printf("\n\nDATABASE OPEN & CLOSE TESTS\n");
    Database db("test_db", 5);
    int testsPassed = 0;
    int testsFailed = 0;

    // Test Open
    db.Open();
    assertEqual(1, directoryExists("test_db"), "Database directory created",
                testsPassed, testsFailed);

    // Test Close without data in memtable
    db.Close();
    assertEqual(1, db.Get(0) == -1, "Database closed and no data persists",
                testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

// Test StoreMemtable functionality
void
testDatabaseStoreMemtable(int &totalPassed, int &totalFailed)
{
    printf("\n\nDATABASE STORE MEMTABLE TESTS\n");
    Database db("test_db", 2);  // Low memtable size for quick flush
    db.Open();
    int testsPassed = 0;
    int testsFailed = 0;

    db.Put(10, 1000);
    db.Put(20, 2000);

    // Fill memtable to trigger store
    db.Put(30, 3000);
    assertEqual(3000, db.Get(30), "Data from new Memtable after flush",
                testsPassed, testsFailed);

    db.Close();
    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

// Test Put and Get functionality
void
testDatabasePutGet(int &totalPassed, int &totalFailed)
{
    printf("\n\nDATABASE PUT & GET TESTS\n");
    Database db("test_db", 5);
    db.Open();
    int testsPassed = 0;
    int testsFailed = 0;

    db.Put(1, 100);
    db.Put(2, 200);
    db.Put(3, 300);
    assertEqual(100, db.Get(1), "Database get(1)", testsPassed, testsFailed);
    assertEqual(200, db.Get(2), "Database get(2)", testsPassed, testsFailed);
    assertEqual(300, db.Get(3), "Database get(3)", testsPassed, testsFailed);

    db.Close();
    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

// Test Scan functionality
void
testDatabaseScan(int &totalPassed, int &totalFailed)
{
    printf("\n\nDATABASE SCAN TESTS\n");
    Database db("test_db", 3);  // Memtable threshold 3
    db.Open();
    int testsPassed = 0;
    int testsFailed = 0;

    // Triggers memtable flush to SST
    db.Put(1, 100);
    db.Put(2, 200);
    db.Put(3, 300);
    db.Put(4, 400);
    db.Put(5, 500);

    auto scanResults = db.Scan(2, 5);
    assertEqual(4, scanResults.size(), "Scan results within range 2 to 5",
                testsPassed, testsFailed);

    db.Close();
    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

// New master test function for Database
void
testDatabase()
{
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;

    printf("DATABASE TESTS:\n");
    testDatabaseOpenClose(totalTestsPassed, totalTestsFailed);
    testDatabaseStoreMemtable(totalTestsPassed, totalTestsFailed);
    testDatabasePutGet(totalTestsPassed, totalTestsFailed);
    testDatabaseScan(totalTestsPassed, totalTestsFailed);

    printf("\n\nDATABASE TEST SUMMARY\n");
    printf("Passed: %d\n", totalTestsPassed);
    printf("Failed: %d\n", totalTestsFailed);
}

int
main()
{
    testAVLTree();
    testDatabase();
    return 0;
}
