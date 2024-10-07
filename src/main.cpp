#include "database.h"

int
main()
{
    Database db("db", 10);
    db.Open();

    // put 10 values
    for (int i = 0; i < 10; i++)
    {
        db.Put(i, i * 10);
    }

    // put 6 more with the same keys but different values
    for (int i = 0; i < 6; i++)
    {
        db.Put(i, i * 100);
    }

    // get 10 values
    auto result = db.Scan(0, 9);
    for (const auto& r : result)
    {
        printf("key: %d, value: %d\n", r.first, r.second);
    }

    // the result are a mix of the two values, the hundreds being the in-memory values
    // and the tens being the first values put in, which are now in the sst files
}
