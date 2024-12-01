#include <filesystem>

#include "database.h"
#include "include/common/config.h"
int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE);
    db.Open();

    for (int i = 0; i < 5000; i++)
    {
        db.Put(i, i);
    }

    // auto results = db.Scan(200, 12000);
    // for (const auto& r : results)
    // {
    //     printf("Key: %d, Value: %d\n", r.first, r.second);
    // }
}
