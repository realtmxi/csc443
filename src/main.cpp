#include <filesystem>

#include "database.h"
#include "include/common/config.h"
int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE);
    db.Open();

    for (int i = 1; i < 270001; i++)
    {
        db.Put(i, i);
    }

    int key1 = 1;
    int key2 = 270000;

    auto results = db.Scan(key1, key2);
    // print size of results
    printf("Results size: %lu\n", results.size());
    // sort and print the min and max keys
    std::sort(results.begin(), results.end());
    printf("Min key: %d\n", results[0].first);
    printf("Max key: %d\n", results[results.size() - 1].first);
}
