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

    // get 8
    printf("Value for key 8: %d\n", db.Get(8));
    // the result are a mix of the two values, the hundreds being the in-memory
    // values and the tens being the first values put in, which are now in the
    // sst files
}
