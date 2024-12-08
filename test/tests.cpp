#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <filesystem>

#include "../src/avl_tree.h"
#include "../src/b_tree/b_tree.h"
#include "../src/b_tree/b_tree_manager.h"
#include "../src/b_tree/b_tree_page.h"
#include "../src/buffer_pool/buffer_pool.h"
#include "../src/config.h"
#include "../src/database.h"

/*

    Helper Functions

 */
void
AssertEqual(int expected, int actual, const char *testName, int &testsPassed,
            int &testsFailed)
{
    if (expected == actual)
    {
        printf("    PASSED: %s\n", testName);
        testsPassed++;
    }
    else
    {
        printf("    %s: FAILED: expected %d, got %d\n", testName, expected,
               actual);
        testsFailed++;
    }
}

/*

    AVL Tree Tests

*/
void
TestAvlTreeInsert(int &totalPassed, int &totalFailed)
{
    printf("\n\nINSERT TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" INSERTING VALUES\n");
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);

    AssertEqual(100, tree.search(10), "  10 exists after insert", testsPassed,
                testsFailed);
    AssertEqual(200, tree.search(20), "  20 exists after insert", testsPassed,
                testsFailed);
    AssertEqual(300, tree.search(30), "  30 exists after insert", testsPassed,
                testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
TestAvlTreeGet(int &totalPassed, int &totalFailed)
{
    printf("\n\nGET TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" INSERTING VALUES\n");
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);

    AssertEqual(100, tree.search(10), "  Test get(10)", testsPassed,
                testsFailed);
    AssertEqual(200, tree.search(20), "  Test get(20)", testsPassed,
                testsFailed);
    AssertEqual(300, tree.search(30), "  Test get(30)", testsPassed,
                testsFailed);

    // Test non-existent key
    AssertEqual(-1, tree.search(40), "  return -1 for non-existent key",
                testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
TestAvlTreeScan(int &totalPassed, int &totalFailed)
{
    printf("\n\nSCAN TESTS\n");
    AVLTree tree;
    int testsPassed = 0;
    int testsFailed = 0;

    printf(" SCAN ON EMPTY TREE\n");
    auto result = tree.scan(1, 10);
    AssertEqual(0, result.size(), "  results size is 0", testsPassed,
                testsFailed);

    printf(" SCAN OUT OF RANGE\n");
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(30, 300);
    result = tree.scan(40, 50);
    AssertEqual(0, result.size(), "  results size is 0", testsPassed,
                testsFailed);

    printf(" SCAN FOR ALL RESULTS\n");
    result = tree.scan(5, 35);
    AssertEqual(3, result.size(), "  results size is 3", testsPassed,
                testsFailed);
    AssertEqual(100, result[0].second, "  key 10 exists", testsPassed,
                testsFailed);
    AssertEqual(200, result[1].second, "  key 20 exists", testsPassed,
                testsFailed);
    AssertEqual(300, result[2].second, "  key 30 exists", testsPassed,
                testsFailed);

    printf(" SCAN FOR PARTIAL RESULTS\n");
    result = tree.scan(15, 25);
    AssertEqual(1, result.size(), "  results size is 1", testsPassed,
                testsFailed);
    AssertEqual(200, result[0].second, "  key 20 exists", testsPassed,
                testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

void
TestAvlTree(int &overallPassed, int &overallFailed)
{
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;

    printf("AVL TREE TESTS:");
    TestAvlTreeInsert(totalTestsPassed, totalTestsFailed);
    TestAvlTreeGet(totalTestsPassed, totalTestsFailed);
    TestAvlTreeScan(totalTestsPassed, totalTestsFailed);

    printf("\n  SUMMARY\n");
    printf("    PASSED: %d\n", totalTestsPassed);
    printf("    FAILED: %d\n", totalTestsFailed);

    overallPassed += totalTestsPassed;
    overallFailed += totalTestsFailed;
}

/*

    Database Tests

*/

void
TestDatabaseOpenClose(int &totalPassed, int &totalFailed)
{
    printf("\n  OPEN & CLOSE\n");
    Database db("test_db", MEMTABLE_SIZE);
    int testsPassed = 0;
    int testsFailed = 0;

    // Test Open
    db.Open();
    struct stat buffer;
    AssertEqual(0, stat("test_db", &buffer), "Database directory exists",
                testsPassed, testsFailed);

    // Test Close without data in memtable
    db.Close();
    AssertEqual(1, db.Get(0) == -1, "Database closed and no data persists",
                testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
    std::filesystem::remove_all("test_db");
}

void
TestDatabasePutGet(int &totalPassed, int &totalFailed)
{
    printf("\n  PUT & GET\n");
    Database db("test_db", MEMTABLE_SIZE);
    db.Open();
    int testsPassed = 0;
    int testsFailed = 0;

    db.Put(1, 100);
    db.Put(2, 200);
    db.Put(3, 300);
    AssertEqual(100, db.Get(1), "Database get(1)", testsPassed, testsFailed);
    AssertEqual(200, db.Get(2), "Database get(2)", testsPassed, testsFailed);
    AssertEqual(300, db.Get(3), "Database get(3)", testsPassed, testsFailed);

    db.Close();
    totalPassed += testsPassed;
    totalFailed += testsFailed;

    std::filesystem::remove_all("test_db");
}

void
TestDatabaseScan(int &totalPassed, int &totalFailed)
{
    printf("\n  SCAN\n");
    Database db("test_db", MEMTABLE_SIZE);
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
    AssertEqual(4, scanResults.size(), "Scan results within range 2 to 5",
                testsPassed, testsFailed);

    db.Close();
    totalPassed += testsPassed;
    totalFailed += testsFailed;

    // Clean up
    std::filesystem::remove_all("test_db");
}

void
TestDatabase(int &overallPassed, int &overallFailed)
{
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;

    printf("\nDATABASE TESTS:");
    TestDatabaseOpenClose(totalTestsPassed, totalTestsFailed);
    TestDatabasePutGet(totalTestsPassed, totalTestsFailed);
    TestDatabaseScan(totalTestsPassed, totalTestsFailed);

    printf("\n  SUMMARY\n");
    printf("    PASSED: %d\n", totalTestsPassed);
    printf("    FAILED: %d\n", totalTestsFailed);

    overallPassed += totalTestsPassed;
    overallFailed += totalTestsFailed;
}

/*

    BTree Tests

*/
void
TestConvertMemtableToBTree(int &totalPassed, int &totalFailed)
{
    printf("\n  MEMTABLE TO BTREE\n");
    // the memtable turns into a sorted list of key-value pairs. So make a mock
    // one here
    std::vector<std::pair<int, int>> data;
    // add 100 key-value pairs
    for (int i = 0; i < 100; i++)
    {
        data.push_back({i, i * 10});
    }
    // the constructor will convert the data into a btree
    BTree btree(data);

    auto leaf_pages = btree.GetLeafPages();
    auto internal_pages = btree.GetInternalPages();

    // check that there is 1 of each
    AssertEqual(1, leaf_pages.size(), "Create 1 leaf page", totalPassed,
                totalFailed);
    AssertEqual(1, internal_pages.size(), "Create 1 internal page", totalPassed,
                totalFailed);

    // check that the leaf page has 100 key-value pairs and the internal page
    // has 1 key-value pair
    AssertEqual(100, leaf_pages[0].GetSize(),
                "Leaf page has 100 key-value pairs", totalPassed, totalFailed);
    AssertEqual(1, internal_pages[0].GetSize(),
                "Internal page has 1 key-value pair", totalPassed, totalFailed);

    // check that every key in leaf page is <= the key in the internal page
    int passed = 1;
    for (auto pair : leaf_pages[0].GetKeyValues())
    {
        if (pair.first > internal_pages[0].GetKeyValues()[0].first)
        {
            passed = 0;
            break;
        }
    }

    AssertEqual(1, passed, "Keys in leaf page <= Key in internal parent",
                totalPassed, totalFailed);
}

void
TestBTreeFiles(int &totalPassed, int &totalFailed)
{
    printf("\n  BTREE FILE\n");
    // make a test_db directory. Input 125k kv pairs into the db.
    // Input another 125k kv pairs into the db. this will trigger a merge.

    // input 125k kv pairs into the db
    Database db("test_db", MEMTABLE_SIZE);
    db.Open();
    for (int i = 0; i < 135000; i++)
    {
        db.Put(i, i * 10);
    }

    // read the test_db directory, store the list of files
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator("test_db"))
    {
        if (entry.path().extension() != ".filter")
        {
            files.push_back(entry.path().string());
        }
    }

    // check that there is one .sst file with _0000_ in the name
    int sst_files = 0;
    for (auto file : files)
    {
        if (file.find("sst_0000_") != std::string::npos)
        {
            sst_files++;
        }
    }
    AssertEqual(1, sst_files, "Create a BTree file upon filling Memtable",
                totalPassed, totalFailed);

    // test get and scan over btree files
    AssertEqual(30, db.Get(3), "Get a key from from within a BTree file",
                totalPassed, totalFailed);
    auto scanResults = db.Scan(2, 5);
    AssertEqual(4, scanResults.size(), "Scan a range within a BTree file",
                totalPassed, totalFailed);

    // update a key in the db
    db.Put(1, 100);
    // delete a key from the db
    db.Delete(2);

    // input another 135k kv pairs into the db
    for (int i = 135000; i < 270000; i++)
    {
        db.Put(i, i * 10);
    }

    // read the test_db directory, store the list of files
    files.clear();
    for (const auto &entry : std::filesystem::directory_iterator("test_db"))
    {
        if (entry.path().extension() != ".filter")
        {
            files.push_back(entry.path().string());
        }
    }

    // check that there is one .sst file with _0001_ in the name
    sst_files = 0;
    std::string filename;
    for (auto file : files)
    {
        if (file.find("sst_0001_") != std::string::npos)
        {
            sst_files++;
            filename = file;
        }
    }
    AssertEqual(1, sst_files, "Merge 2 BTree files into a new level",
                totalPassed, totalFailed);

    // load in the merged btree file and check for the updated value
    BufferPool bp(1);
    BTreeManager btm(filename, 1, bp);
    BTreePage page = btm.TraverseToKey(1);
    AssertEqual(100, page.Get(1), "Update a key", totalPassed, totalFailed);

    // check that the deleted key is not in the btree
    page = btm.TraverseToKey(2);
    AssertEqual(-1, page.Get(2),
                "Remove a tombstone when merging into a new level", totalPassed,
                totalFailed);

    // insert another 125k kv pairs into the db
    for (int i = 270000; i < 395000; i++)
    {
        db.Put(i, i * 10);
    }

    // ensure there are 2 files, one level 0000 and one level 0001
    files.clear();
    for (const auto &entry : std::filesystem::directory_iterator("test_db"))
    {
        if (entry.path().extension() != ".filter")
        {
            files.push_back(entry.path().string());
        }
    }

    sst_files = 0;
    for (auto file : files)
    {
        if (file.find("sst_0000_") != std::string::npos ||
            file.find("sst_0001_") != std::string::npos)
        {
            sst_files++;
        }
    }

    AssertEqual(2, sst_files, "Save 2 levels of BTree files", totalPassed,
                totalFailed);

    // scan across 2 files
    scanResults = db.Scan(0, 395000);
    AssertEqual(395000 - 1, scanResults.size(), "Scan across 2 BTree files",
                totalPassed, totalFailed);

    db.Close();
    std::filesystem::remove_all("test_db");
}

void
TestBTreeGetsCorrectness(int &totalPassed, int &totalFailed)
{
    // make test_db1 and test_db2 directories. Input 400k kv pairs into db1 and
    // db2. They each have the same data, but db1 uses a BTree and db2 uses a
    // BTree with binary search. get 100 random keys from each db and compare
    // the results.
    printf("\n  BTREE GET CORRECTNESS\n");

    // input 400k kv pairs into db1
    Database db1("test_db1", MEMTABLE_SIZE);
    Database db2("test_db2", MEMTABLE_SIZE, true);
    db1.Open();
    db2.Open();

    for (int i = 0; i < 400000; i++)
    {
        db1.Put(i, i * 10);
        db2.Put(i, i * 10);
    }

    // get 100 random keys from each db and compare the results
    int testsPassed = 0;
    int testsFailed = 0;

    int failed = 0;
    for (int i = 0; i < 300; i++)
    {
        int key = rand() % 400000;
        if (db1.Get(key) != db2.Get(key))
        {
            failed = 1;
            AssertEqual(db1.Get(key), db2.Get(key),
                        "Get key from BTree and BTree with binary search",
                        testsPassed, testsFailed);
            break;
        }
    }

    if (failed == 0)
    {
        AssertEqual(
            1, 1,
            "Get and compare keys from BTree and BTree with binary search",
            testsPassed, testsFailed);
    }

    totalPassed += testsPassed;
    totalFailed += testsFailed;

    db1.Close();

    std::filesystem::remove_all("test_db1");
    std::filesystem::remove_all("test_db2");
}

void
BTreeTests(int &overallPassed, int &overallFailed)
{
    int totalPassed = 0;
    int totalFailed = 0;

    printf("\n\nBTREE TESTS:");
    TestConvertMemtableToBTree(totalPassed, totalFailed);
    TestBTreeFiles(totalPassed, totalFailed);
    TestBTreeGetsCorrectness(totalPassed, totalFailed);

    printf("\n  SUMMARY\n");
    printf("    PASSED: %d\n", totalPassed);
    printf("    FAILED: %d\n", totalFailed);

    overallPassed += totalPassed;
    overallFailed += totalFailed;
}

/*
    Bloom Filter Tests
    author: Patrick
*/

/* Test basic insertion and membership functionality */
void
TestBloomFilterInsertAndMembership(int &totalPassed, int &totalFailed)
{
    printf("\n  INSERT AND MEMBERSHIP\n");
    int testsPassed = 0;
    int testsFailed = 0;

    BloomFilter filter(1024);

    filter.Insert(42);
    filter.Insert(84);
    filter.Insert(126);

    AssertEqual(true, filter.MayContain(42), "Key 42 exists", testsPassed,
                testsFailed);
    AssertEqual(true, filter.MayContain(84), "Key 84 exists", testsPassed,
                testsFailed);
    AssertEqual(true, filter.MayContain(126), "Key 126 exists", testsPassed,
                testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

/* Test Bloom filter union functionality */
void
TestBloomFilterUnion(int &totalPassed, int &totalFailed)
{
    printf("\n  UNION\n");
    int testsPassed = 0;
    int testsFailed = 0;

    BloomFilter filter1(1024);
    BloomFilter filter2(1024);

    filter1.Insert(1);
    filter1.Insert(2);
    filter2.Insert(3);
    filter2.Insert(4);
    filter1.Union(filter2);

    AssertEqual(true, filter1.MayContain(1), "Key 1 exists after union",
                testsPassed, testsFailed);
    AssertEqual(true, filter1.MayContain(2), "Key 2 exists after union",
                testsPassed, testsFailed);
    AssertEqual(true, filter1.MayContain(3), "Key 3 exists after union",
                testsPassed, testsFailed);
    AssertEqual(true, filter1.MayContain(4), "Key 4 exists after union",
                testsPassed, testsFailed);

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

/* Test serialization and deserialization of Bloom filter */
void
TestBloomFilterSerialization(int &totalPassed, int &totalFailed)
{
    printf("\n  SERIALIZATION\n");
    int testsPassed = 0;
    int testsFailed = 0;

    BloomFilter filter(1024);

    filter.Insert(10);
    filter.Insert(20);
    filter.Insert(30);

    filter.SerializeToDisk("bloom_filter_test.filter");
    BloomFilter deserializedFilter("bloom_filter_test.filter");

    AssertEqual(true, deserializedFilter.MayContain(10),
                "Key 10 exists after deserialization", testsPassed,
                testsFailed);
    AssertEqual(true, deserializedFilter.MayContain(20),
                "Key 20 exists after deserialization", testsPassed,
                testsFailed);
    AssertEqual(true, deserializedFilter.MayContain(30),
                "Key 30 exists after deserialization", testsPassed,
                testsFailed);

    std::filesystem::remove("bloom_filter_test.filter");

    totalPassed += testsPassed;
    totalFailed += testsFailed;
}

/* Top-level function to run all Bloom Filter tests */
void
TestBloomFilter(int &overallPassed, int &overallFailed)
{
    int totalTestsPassed = 0;
    int totalTestsFailed = 0;

    printf("\nBLOOM FILTER TESTS:");
    TestBloomFilterInsertAndMembership(totalTestsPassed, totalTestsFailed);
    TestBloomFilterUnion(totalTestsPassed, totalTestsFailed);
    TestBloomFilterSerialization(totalTestsPassed, totalTestsFailed);

    printf("\n  SUMMARY\n");
    printf("    PASSED: %d\n", totalTestsPassed);
    printf("    FAILED: %d\n", totalTestsFailed);

    overallPassed += totalTestsPassed;
    overallFailed += totalTestsFailed;
}

int
main()
{
    int overallPassed = 0;
    int overallFailed = 0;
    TestAvlTree(overallPassed, overallFailed);
    TestDatabase(overallPassed, overallFailed);
    BTreeTests(overallPassed, overallFailed);
    TestBloomFilter(overallPassed, overallFailed);

    printf("\n\nOVERALL\n");
    printf("  PASSED: %d\n", overallPassed);
    printf("  FAILED: %d\n", overallFailed);
    return 0;
}
