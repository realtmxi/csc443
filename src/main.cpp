#include <unistd.h>

#include <filesystem>

#include "database.h"
#include "include/common/config.h"

int
main()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE);
    db.Open();

    // add key 1
    db.Put(1, 10);
    printf("Put 1: 10\n");

    db.Put(2, 20);
    // add 300000 keys
    for (int i = 2; i < 7000000; i++)
    {
        db.Put(i, i * 10);
    }

    printf("Get 1: %d\n", db.Get(1));

    // delete 1
    db.Delete(1);

    printf("Get 1: %d\n", db.Get(1));

    // add 300000 more keys
    for (int i = 300002; i < 600002; i++)
    {
        db.Put(i, i * 10);
    }

    // get 1
    int result = db.Get(1);
    printf("Get 1: %d\n", result);
    
    // Test retrieval of an existing key
    printf("Get 2: %d\n", db.Get(2));
}
