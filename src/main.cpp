#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <map>

#include "config.h"
#include "database.h"

void
Part2Experiment()
{
    // For this experiment, we will insert powers of 2 MB of data.
    // One will be inserted into the DB using the BTree and Buffer Pool,
    // the other will use the simple SSTable implementation, which stores
    // equal size files on disk.

    // We are only measuring Gets, so we will insert the data and then
    // measure the get throughput for each power of 2 MB of data. The
    // X-axis will be the size of the data in MB, and the Y-axis will be
    // the throughput in MB/s.

    std::filesystem::remove_all("btree_db");
    std::filesystem::remove_all("btree_binary_search_db");
    Database btree_db("btree_db", MEMTABLE_SIZE);
    Database btree_binary_search_db("btree_binary_search_db", MEMTABLE_SIZE,
                                    true);
    btree_db.Open();
    btree_binary_search_db.Open();

    std::vector<double> btree_throughputs;
    std::vector<double> binary_search_throughputs;

    int current_data_mb = 0;
    int next_record_mb = 1;
    int total_size_mb = 2048;

    printf("Beginning part 2 experiment.\n");
    auto start = std::chrono::high_resolution_clock::now();

    while (current_data_mb < total_size_mb)
    {
        // Calculate the number of key-value pairs needed to reach the
        // next_record_mb
        int num_pairs = (next_record_mb - current_data_mb) * 1000000 / 8;

        // Insert data up to next_record_mb
        for (int j = 0; j < num_pairs; j++)
        {
            int random_key = rand() % (next_record_mb * 1000000 / 8);
            int random_value = rand();
            btree_db.Put(random_key, random_value);
            btree_binary_search_db.Put(random_key, random_value);
            // print every mb
            if (j % 100000 == 0)
            {
                printf("\n%f%%. ",
                       static_cast<double>(current_data_mb + j * 8 / 1000000) /
                           total_size_mb * 100);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = end - start;
                int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
                int seconds = static_cast<int>(elapsed_seconds.count()) % 60;
                printf("Time elapsed: %02d:%02d. ", minutes, seconds);
                printf("MB inserted: %d", current_data_mb + j * 8 / 1000000);
            }
        }

        current_data_mb = next_record_mb;
        next_record_mb *= 2;

        // get throughput
        double btree_total_time = 0;
        double binary_total_time = 0;

        for (int j = 0; j < 100; j++)
        {
            int get_key = rand() % (next_record_mb * 1000000 / 8);
            auto start_btree = std::chrono::high_resolution_clock::now();
            btree_db.Get(get_key);
            auto end_btree = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_btree =
                end_btree - start_btree;
            btree_total_time += elapsed_btree.count();

            auto start_binary = std::chrono::high_resolution_clock::now();
            btree_binary_search_db.Get(get_key);
            auto end_sst = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_sst = end_sst - start_binary;
            binary_total_time += elapsed_sst.count();
        }

        // get throughputs in MB/s
        // ((8bytes/pair * 100 kv pairs) / 1MB) / time
        double btree_throughput = ((8.0 * 100) / 1000000) / btree_total_time;
        double binary_throughput = ((8.0 * 100) / 1000000) / binary_total_time;

        printf("\nBTree GET Throughput: %f MB/s", btree_throughput);
        printf("\nBTree BinarySearchGet Throughput: %f MB/s",
               binary_throughput);

        btree_throughputs.push_back(btree_throughput);
        binary_search_throughputs.push_back(binary_throughput);
    }

    // Write throughput results to a CSV
    std::ofstream file("part2_results.csv");
    file << "data_size_mb,btree_throughput,binary_throughput\n";
    for (int i = 0; i < btree_throughputs.size(); i++)
    {
        int size_mb = 1 << i;  // Powers of 2 (1MB, 2MB, 4MB, ...)
        file << size_mb << "," << btree_throughputs[i] << ","
             << binary_search_throughputs[i] << "\n";
    }

    file.close();
    btree_db.Close();
    btree_binary_search_db.Close();

    printf(
        "\nPart 2 experiment complete. Results written to part2_results.csv\n");
}

void
Part3Experiment()
{
    std::filesystem::remove_all("db");
    Database db("db", MEMTABLE_SIZE);
    db.Open();

    std::vector<double> put_throughputs;
    std::map<int, double> get_throughputs;
    std::map<int, double> scan_throughputs;

    int current_data_mb = 0;
    int next_record_mb = 1;
    int total_size_mb = 2048;

    printf("Beginning part 3 experiment.\n");
    auto start = std::chrono::high_resolution_clock::now();

    while (current_data_mb < total_size_mb)
    {
        // Calculate the number of key-value pairs needed to reach the
        // next_record_mb
        int num_pairs = (next_record_mb - current_data_mb) * 1000000 / 8;

        std::chrono::duration<double> elapsed_put;
        // Insert data up to next_record_mb
        for (int j = 0; j < num_pairs; j++)
        {
            // uniformly distributed random key from 0 to total data size
            int random_key = rand() % (next_record_mb * 1000000 / 8);
            int random_value = rand();
            auto start_put = std::chrono::high_resolution_clock::now();
            db.Put(random_key, random_value);
            auto end_put = std::chrono::high_resolution_clock::now();
            elapsed_put += end_put - start_put;
            // print every mb
            if (j % 100000 == 0)
            {
                printf("\n%f%%. ",
                       static_cast<double>(current_data_mb + j * 8 / 1000000) /
                           total_size_mb * 100);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = end - start;
                int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
                int seconds = static_cast<int>(elapsed_seconds.count()) % 60;
                printf("Time elapsed: %02d:%02d. ", minutes, seconds);
                printf("MB inserted: %d", current_data_mb + j * 8 / 1000000);
            }
        }

        // put throughput in MB/s
        double put_throughput =
            (num_pairs * 8.0 / 1000000) / elapsed_put.count();
        put_throughputs.push_back(put_throughput);

        current_data_mb = next_record_mb;

        // get and scan throughput
        double get_total_time = 0;
        double scan_total_time = 0;

        for (int j = 0; j < 100; j++)
        {
            int get_key = rand() % (next_record_mb * 1000000 /
                                    8);  // Key range up to current data
            auto start_get = std::chrono::high_resolution_clock::now();
            db.Get(get_key);
            auto end_get = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_get = end_get - start_get;
            get_total_time += elapsed_get.count();

            int scan_key = rand() % (next_record_mb * 1000000 / 8);
            int scan_end = scan_key + 100;
            auto start_scan = std::chrono::high_resolution_clock::now();
            db.Scan(scan_key, scan_end);
            auto end_scan = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_scan = end_scan - start_scan;
            scan_total_time += elapsed_scan.count();
        }

        // get and scan throughputs in MB/s
        // ((8bytes/pair * 100 kv pairs) / 1MB) / time
        double get_throughput = (100 * 8.0 / 1000000) / get_total_time;
        // ((8bytes/pair * 100 scans * 100 kv pairs) / 1MB) / time
        double scan_throughput = (100 * 100 * 8.0 / 1000000) / scan_total_time;

        printf("\nGet Throughput: %f MB/s", get_throughput);
        printf("\nScan Throughput: %f MB/s", scan_throughput);
        printf("\nPut Throughput: %f MB/s", put_throughput);

        get_throughputs[current_data_mb] = get_throughput;
        scan_throughputs[current_data_mb] = scan_throughput;
        next_record_mb *= 2;
    }

    // Write throughput results to a CSV
    std::ofstream file("part3_results.csv");
    file << "data_size_mb,put_throughput,get_throughput,scan_throughput\n";
    for (std::vector<double>::size_type i = 0; i < put_throughputs.size(); i++)
    {
        int size_mb = 1 << i;
        file << size_mb << "," << put_throughputs[i] << ","
             << (get_throughputs.count(size_mb) ? get_throughputs[size_mb] : 0)
             << ","
             << (scan_throughputs.count(size_mb) ? scan_throughputs[size_mb]
                                                 : 0)
             << "\n";
    }
    file.close();
    db.Close();

    printf(
        "\nPart 3 experiment complete. Results written to part3_results.csv\n");
}

int
main()
{
    // Part2Experiment();
    Part3Experiment();

    // std::filesystem::remove_all("db");

    // Database db("db", MEMTABLE_SIZE);

    // db.Open();

    // // add 300k key-value pairs
    // printf("Adding 300k key-value pairs to the database...\n");
    // for (int i = 0; i < 700000; i++)
    // {
    //     db.Put(i, i);
    // }

    // // get 1000 random keys
    // printf("Getting 1000 random keys\n");
    // for (int i = 0; i < 1000; i++)
    // {
    //     db.Get(rand() % 300000);
    // }

    // return 0;
}
