#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <map>

#include "database.h"
#include "include/common/config.h"
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

    printf("Beginning part 3 experiment.\n");
    auto start = std::chrono::high_resolution_clock::now();

    while (current_data_mb < 2048)
    {
        // Calculate the number of key-value pairs needed to reach the
        // next_record_mb
        int num_pairs = (next_record_mb - current_data_mb) * 1000000 / 8;

        // Insert data up to next_record_mb
        auto start_put = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < num_pairs; j++)
        {
            db.Put(rand() % (next_record_mb * 1000000 / 8), rand());
            // print every mb
            if (j % 100000 == 0)
            {
                printf("\n%f%%. ",
                       static_cast<double>(current_data_mb + j * 8 / 1000000) /
                           2048 * 100);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = end - start;
                int minutes = static_cast<int>(elapsed_seconds.count()) / 60;
                int seconds = static_cast<int>(elapsed_seconds.count()) % 60;
                printf("Time elapsed: %02d:%02d. ", minutes, seconds);
                printf("MB inserted: %d", current_data_mb + j * 8 / 1000000);
            }
        }
        auto end_put = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_put = end_put - start_put;

        // put throughput in MB/s
        double put_throughput = (num_pairs * 8.0 / 1000000) /
                                elapsed_put.count();  // Convert 8B pairs to MB
        put_throughputs.push_back(put_throughput);

        current_data_mb = next_record_mb;

        // get and scan throughput
        double get_total_time = 0;
        double scan_total_time = 0;

        for (int j = 0; j < 100; j++)
        {
            int get_key = rand() % (current_data_mb * 1000000 /
                                    8);  // Key range up to current data
            auto start_get = std::chrono::high_resolution_clock::now();
            db.Get(get_key);
            auto end_get = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_get = end_get - start_get;
            get_total_time += elapsed_get.count();

            int scan_key = rand() % (current_data_mb * 1000000 / 8);
            int scan_end = scan_key + 100;
            auto start_scan = std::chrono::high_resolution_clock::now();
            db.Scan(scan_key, scan_end);
            auto end_scan = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_scan = end_scan - start_scan;
            scan_total_time += elapsed_scan.count();
        }

        // get and scan throughputs in MB/s
        double get_throughput = (100 * 8.0 / 1000000) / get_total_time;
        double scan_throughput = (100 * 8.0 / 1000000) / scan_total_time;

        get_throughputs[current_data_mb] = get_throughput;
        scan_throughputs[current_data_mb] = scan_throughput;
        next_record_mb *= 2;
    }

    // Write throughput results to a CSV
    std::ofstream file("part3_results.csv");
    file << "data_size_mb,put_throughput,get_throughput,scan_throughput\n";
    for (int i = 0; i < put_throughputs.size(); i++)
    {
        int size_mb = 1 << i;  // Powers of 2 (1MB, 2MB, 4MB, ...)
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
    Part3Experiment();
    return 0;
}
