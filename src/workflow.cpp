#include <iostream>
#include <filesystem>
#include <chrono>
#include "database.h"

// Helper function to measure time for a task
template <typename Func>
void MeasureTime(const std::string &taskName, Func &&task) {
    auto start = std::chrono::high_resolution_clock::now();
    task();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << taskName << " completed in " << elapsed.count() << " seconds.\n";
}

void DatabaseWorkflow() {
    std::string db_name = "example_db";
    size_t memtable_size = 1024 * 1024;  // 1 MB memtable size

    // Step 1: Open the database
    std::cout << "Opening the database..." << std::endl;
    Database db(db_name, memtable_size);
    db.Open();

    // Step 2: Insert 1 million key-value pairs into the database
    size_t numKeys = 1000000;  // 1 million keys
    MeasureTime("Inserting 1 million keys", [&]() {
        for (size_t i = 0; i < numKeys; ++i) {
            db.Put(i, i * 10);  // Key: i, Value: i * 10
        }
    });

    // Step 3: Retrieve random keys using Get
    std::cout << "Retrieving some keys using Get..." << std::endl;
    MeasureTime("Retrieving keys", [&]() {
        for (size_t i = 0; i < 10; ++i) {
            int key = i * (numKeys / 10);  // Sample evenly spaced keys
            int value = db.Get(key);
            std::cout << "Key: " << key << ", Value: " << value << std::endl;
        }
    });

    // Step 4: Delete some keys
    std::cout << "Deleting some keys..." << std::endl;
    MeasureTime("Deleting keys", [&]() {
        for (size_t i = 0; i < 10; ++i) {
            db.Delete(i * (numKeys / 10));  // Sample evenly spaced keys
        }
    });

    // Step 5: Scan a range of keys
    size_t scanStart = numKeys / 2, scanEnd = scanStart + 100;  // Range in the middle of the keys
    std::cout << "Scanning keys from " << scanStart << " to " << scanEnd << "..." << std::endl;
    MeasureTime("Scanning keys", [&]() {
        auto scanResults = db.Scan(scanStart, scanEnd);
        for (const auto &kv : scanResults) {
            std::cout << "Key: " << kv.first << ", Value: " << kv.second << std::endl;
        }
    });

    // Step 6: Trigger compaction
    std::cout << "Triggering compaction by adding more keys..." << std::endl;
    MeasureTime("Inserting additional keys to trigger compaction", [&]() {
        for (size_t i = numKeys; i < numKeys + 500000; ++i) {  // Add 500,000 more keys
            db.Put(i, i * 10);
        }
    });

    // Verify a key after compaction
    int testKey = numKeys + 250000;
    int value = db.Get(testKey);
    std::cout << "After compaction, Key: " << testKey << ", Value: " << value << std::endl;

    // Step 7: Close and reopen the database to test persistence
    std::cout << "Closing the database..." << std::endl;
    db.Close();
    std::cout << "Reopening the database and verifying data to test persistence" << std::endl;
    db.Open();

    MeasureTime("Verifying data after reopening", [&]() {
        for (size_t i = 0; i < 10; ++i) {
            int key = i * (numKeys / 10);
            int value = db.Get(key);
            std::cout << "Key: " << key << ", Value: " << (value != -1 ? std::to_string(value) : "Not found") << std::endl;
        }
    });

    // Step 8: Clean up
    std::cout << "Cleaning up database files..." << std::endl;
    db.Close();
    std::filesystem::remove_all(db_name);
    std::cout << "Database files cleaned up." << std::endl;
}

int main() {
    DatabaseWorkflow();
    return 0;
}
