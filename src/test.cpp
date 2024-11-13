#include "mpsi.hpp"

void paxos() {
    u64 n = 1 << 15;
    u64 w = 3;

    PRNG prng(oc::sysRandomSeed());
    block seed1 = prng.get();

    Paxos<u32> paxos;
    Paxos<u32> paxos2;
    paxos.init(n, w, 40, PaxosParam::Binary, seed1);
    paxos2.init(n, w, 40, PaxosParam::Binary, seed1);

    std::vector<block> items(n), values(n), values2(n), p(paxos.size());

    prng.get(items.data(), items.size());
    prng.get(values.data(), values.size());

    paxos.solve<block>(items, values, p);

    u64 lim = 227;
    for (u64 i = 0; i < lim; i++) {
        items[i] = prng.get();
    }
    paxos2.decode<block>(items, values2, p);

    for (u64 i = 0; i < n; i++) {
        if (i < lim) {
            if (values2[i] == values[i]) {
                throw RTE_LOC;
            }
        } else {
            if (values[i] != values2[i]) {
                std::printf("%lu\n", i);
                throw RTE_LOC;
            }
        }
    }


    std::printf("paxos ok\n");
}

void zs(u64 numParties) {
    u64 n = 1 << 10;

    vector<vector<block>> shares(numParties);
    for (u64 i = 0; i < numParties; i++) {
        shares[i] = zeroShare(n, numParties, i);
    }

    vector<block> sum(n, ZeroBlock);
    for (u64 i = 0; i < numParties; i++) {
        for (u64 j = 0; j < n; j++) {
            sum[j] = sum[j] ^ shares[i][j];
        }
    }

    for (u64 i = 0; i < n; i++) {
        if (sum[i] != ZeroBlock) {
            throw RTE_LOC;
        }
    }
    std::printf("zero share ok\n");
}

void rzs(u64 numParties) {
    u64 n = 1 << 10;

    vector<vector<block>> shares(numParties);
    vector<std::thread> threads(numParties);
    std::mutex lock;

    for (u64 i = 0; i < numParties; i++) {
        threads[i] = std::thread([&, i]() {
            auto res = realZeroShare(n, numParties, i);
            lock.lock();
            shares[i] = res;
            lock.unlock();
        });
    }
    for (u64 i = 0; i < numParties; i++) {
        threads[i].join();
    }

    vector<block> sum(n, ZeroBlock);
    for (u64 i = 0; i < numParties; i++) {
        for (u64 j = 0; j < n; j++) {
            sum[j] = sum[j] ^ shares[i][j];
        }
    }

    for (u64 i = 0; i < n; i++) {
        if (sum[i] != ZeroBlock) {
            cout << i << " " << sum[i] << endl;
            throw RTE_LOC;
        }
    }
    std::printf("real zero share ok\n");
}

void mpsi3(u64 n) {
    Mpsi mpsi;
    mpsi.n = n;
    mpsi.numParties = 3;

    thread t1 = thread([&] {
        auto start = std::chrono::high_resolution_clock::now();
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i);
        }
        mpsi.p1(data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p1Data / 1024.0 / 1024.0;
        std::printf("p1 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    thread t2 = thread([&] {
        auto start = std::chrono::high_resolution_clock::now();
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i + 1);
        }
        mpsi.p2(data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p2Data / 1024.0 / 1024.0;
        std::printf("p2 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    thread t3 = thread([&] {
        auto start = std::chrono::high_resolution_clock::now();
        vector<block> data(n);
        for (u64 i = 0; i < n; i++) {
            data[i] = block(0, i + 2);
        }
        mpsi.p3(3, data);
        auto end = std::chrono::high_resolution_clock::now();
        u64 time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        double transfer = mpsi.p3Data / 1024.0 / 1024.0;
        std::printf("p3 time: %lu ms, transfer: %0.2f MiB \n", time, transfer);
    });

    t1.join();
    t2.join();
    t3.join();
}

void test() {
//    paxos();
//    zs(3);
//    zs(10);
//    rzs(2);
//    rzs(3);
    rzs(16);

//    mpsi3(1 << 10);
}