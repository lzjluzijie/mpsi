#include "mpsi.hpp"

void mpsi(u64 numParties, u64 n) {
    if (numParties < 3) {
        printf("numParties must be at least 3\n");
        return;
    }

    Mpsi mpsi;
    mpsi.n = n;
    mpsi.numParties = numParties;

    printf("numParties: %lu, n: %lu\n", numParties, n);

    auto start = std::chrono::high_resolution_clock::now();
    thread t1 = thread([&] {
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i);
        }
        auto start = std::chrono::high_resolution_clock::now();
        mpsi.p1(data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p1Data / 1024.0 / 1024.0;
        std::printf("p1 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    thread t2 = thread([&] {
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i + 1);
        }
        auto start = std::chrono::high_resolution_clock::now();
        mpsi.p2(data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p2Data / 1024.0 / 1024.0;
        std::printf("p2 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    thread t3 = thread([&] {
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i + 2);
        }
        auto start = std::chrono::high_resolution_clock::now();
        mpsi.p3(3, data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p3Data / 1024.0 / 1024.0;
        std::printf("p3 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    vector<thread> threads(numParties - 3);
    for (u64 id = 4; id <= numParties; id++) {
        threads[id - 4] = thread([&, id] {
            vector<block> data(n);
            for (u64 i = 0; i < n; i++) {
                data[i] = block(0, i + id - 1);
            }
            auto start = std::chrono::high_resolution_clock::now();
            mpsi.p3(id, data);
            auto end = std::chrono::high_resolution_clock::now();
            u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (id == 4) {
                double transfer = mpsi.p4Data / 1024.0 / 1024.0;
                std::printf("p4 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
            }
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::printf("start time: %lu ms\n", time);

    t1.join();
    t2.join();
    t3.join();
    for (u64 id = 4; id <= numParties; id++) {
        threads[id - 4].join();
    }
    printf("\n");
}

void bench() {
    mpsi(3, 1 << 12);
    mpsi(4, 1 << 12);
    mpsi(8, 1 << 12);
    mpsi(16, 1 << 12);
    mpsi(3, 1 << 16);
    mpsi(4, 1 << 16);
    mpsi(8, 1 << 16);
    mpsi(16, 1 << 16);
    mpsi(3, 1 << 20);
    mpsi(4, 1 << 20);
    mpsi(8, 1 << 20);
    mpsi(16, 1 << 20);
}

int main(int argc, char **argv) {
    if (argc == 4) {
        u64 numParties, id, n;
        numParties = std::stoul(argv[1]);
        id = std::stoul(argv[2]);
        n = std::stoul(argv[3]);

//        printf("n = %lu, numParties = %lu, id = %lu\n", n, numParties, id);

        Mpsi mpsi;
        mpsi.n = n;
        mpsi.numParties = numParties;
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i + id - 1);
        }
        auto start = std::chrono::high_resolution_clock::now();
        if (id == 1) {
            mpsi.p1(data);
        } else if (id == 2) {
            mpsi.p2(data);
        } else {
            mpsi.p3(id, data);
        }
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = 0;
        if (id == 1) {
            transfer = mpsi.p1Data / 1024.0 / 1024.0;
            std::printf("p1 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
        } else if (id == 2) {
            transfer = mpsi.p2Data / 1024.0 / 1024.0;
            std::printf("p2 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
        } else if (id == 3) {
            transfer = mpsi.p3Data / 1024.0 / 1024.0;
            std::printf("p3 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
        } else if (id == 4) {
            transfer = mpsi.p4Data / 1024.0 / 1024.0;
            std::printf("p4 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
        }
    } else {
        bench();
//        test();
    }
}