#include "avl_tree.h"
#include <iostream>

void assertEqual(int expected, int actual, const std::string &testName,
                 int &testsPassed, int &testsFailed) {
  if (expected == actual) {
    std::cout << testName << ": PASSED" << std::endl;
    testsPassed++;
  } else {
    std::cout << testName << ": FAILED: expected " << expected << ", got "
              << actual << std::endl;
    testsFailed++;
  }
}

void testAVLTreeInsert(int &totalPassed, int &totalFailed) {
  std::cout << "\n\nINSERT TESTS" << std::endl;
  AVLTree tree;
  int testsPassed = 0;
  int testsFailed = 0;

  tree.put(10, 100);
  tree.put(20, 200);
  tree.put(30, 300);

  assertEqual(100, tree.get(10), "Test get(10) after insert", testsPassed,
              testsFailed);
  assertEqual(200, tree.get(20), "Test get(20) after insert", testsPassed,
              testsFailed);
  assertEqual(300, tree.get(30), "Test get(30) after insert", testsPassed,
              testsFailed);

  totalPassed += testsPassed;
  totalFailed += testsFailed;
}

void testAVLTreeGet(int &totalPassed, int &totalFailed) {
  std::cout << "\n\nINSERT TESTS" << std::endl;
  AVLTree tree;
  int testsPassed = 0;
  int testsFailed = 0;

  tree.put(10, 100);
  tree.put(20, 200);
  tree.put(30, 300);

  assertEqual(100, tree.get(10), "Test get(10)", testsPassed, testsFailed);
  assertEqual(200, tree.get(20), "Test get(20)", testsPassed, testsFailed);
  assertEqual(300, tree.get(30), "Test get(30)", testsPassed, testsFailed);

  // Test non-existent key
  assertEqual(-1, tree.get(40), "Test get(40) - non-existent key", testsPassed,
              testsFailed);

  totalPassed += testsPassed;
  totalFailed += testsFailed;
}

void testAVLTreeScan(int &totalPassed, int &totalFailed) {
  std::cout << "\n\nINSERT TESTS" << std::endl;
  AVLTree tree;
  int testsPassed = 0;
  int testsFailed = 0;

  tree.put(10, 100);
  tree.put(20, 200);
  tree.put(30, 300);

  auto result = tree.scan();
  assertEqual(100, result[0].second, "Test scan - key 10", testsPassed,
              testsFailed);
  assertEqual(200, result[1].second, "Test scan - key 20", testsPassed,
              testsFailed);
  assertEqual(300, result[2].second, "Test scan - key 30", testsPassed,
              testsFailed);

  totalPassed += testsPassed;
  totalFailed += testsFailed;
}

void testAVLTree() {
  int totalTestsPassed = 0;
  int totalTestsFailed = 0;

  std::cout << "Running AVL Tree Tests...";
  testAVLTreeInsert(totalTestsPassed, totalTestsFailed);
  testAVLTreeGet(totalTestsPassed, totalTestsFailed);
  testAVLTreeScan(totalTestsPassed, totalTestsFailed);

  std::cout << "\n\nTEST SUMMARY" << std::endl;
  std::cout << "Passed: " << totalTestsPassed << std::endl;
  std::cout << "Failed: " << totalTestsFailed << std::endl;
}

int main() {
  testAVLTree();
  return 0;
}
