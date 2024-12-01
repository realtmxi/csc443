#include <filesystem>

#include "database.h"
int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", 10);
    db.Open();

    for (int i = 0; i < 100; i++)
    {
        db.Put(i, i * 10);
    }

    auto results = db.Scan(4, 300);
    for (const auto& r : results)
    {
        printf("Key: %d, Value: %d\n", r.first, r.second);
    }
}
