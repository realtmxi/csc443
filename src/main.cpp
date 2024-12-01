#include <filesystem>

#include "database.h"
#include "include/common/config.h"
int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE / 8);
    db.Open();

    for (int i = 0; i < 1000; i++)
    {
        db.Put(i, i * 10);
    }

    auto results = db.Scan(4, 1000);
    for (const auto& r : results)
    {
        printf("Key: %d, Value: %d\n", r.first, r.second);
    }
}
