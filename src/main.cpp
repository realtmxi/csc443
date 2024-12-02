#include <filesystem>

#include "database.h"
#include "include/common/config.h"

int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE);
    db.Open();

    for (int i = 0; i < 1000000; i++)
    {
        int key = rand() % 10000000;
        int value = rand() % 1000000;
        db.Put(key, value);
    }

    for (int i = 0; i < 1000; i++)
    {
        int key = rand() % 10000000;
        int value = db.Get(key);
        if (value != -1)
        {
            printf("Key: %d, Value: %d\n", key, value);
        }
    }
}
