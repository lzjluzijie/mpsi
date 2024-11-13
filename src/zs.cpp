#include "mpsi.hpp"

vector<block> zeroShare(u64 n, u64 numParties, u64 id) {
    vector<block> ret(n);
    PRNG prng;
    for (u64 j = 0; j < id; j++) {
        prng.SetSeed(block(j, id), n);
        for (u64 i = 0; i < n; i++) {
            ret[i] = ret[i] ^ prng.mBuffer[i];
        }
    }
    for (u64 j = id + 1; j < numParties; j++) {
        prng.SetSeed(block(id, j), n);
        for (u64 i = 0; i < n; i++) {
            ret[i] = ret[i] ^ prng.mBuffer[i];
        }
    }
    return ret;
}

macoro::task<> recv(u64 numParties, u64 id, u64 j, block &seed) {
    // send from id to j use port 7000 + id * numParties + j
    uint port = 7000 + id * numParties + j;
    std::string address("localhost:" + std::to_string(port));
    auto chl = co_await coproto::AsioAcceptor(address, coproto::global_io_context(), 1);
    co_await chl.recv(seed);
    co_await chl.flush();
}

macoro::task<> send(u64 numParties, u64 id, u64 j, block seed) {
    // send from id to j use port 7000 + j * numParties + id
    uint port = 7000 + j * numParties + id;
    std::string address("localhost:" + std::to_string(port));
    auto chl = co_await coproto::AsioConnect(address, coproto::global_io_context());
    co_await chl.send(seed);
    co_await chl.flush();
}

vector<block> realZeroShare(u64 n, u64 numParties, u64 id) {
    oc::Timer timer;
    timer.setTimePoint("start");
    vector<block> ret(n);
    vector<block> seeds(numParties);
    PRNG prng0(oc::sysRandomSeed());
    prng0.get(seeds.data(), numParties);
    PRNG prng;

    timer.setTimePoint("init");
    vector<macoro::eager_task<>> tasks(numParties - 1);
    for (u64 j = 0; j < id; j++) {
        tasks[j] = recv(numParties, id, j, seeds[j]) | macoro::make_eager();
    }
    for (u64 j = id + 1; j < numParties; j++) {
        tasks[j - 1] = send(numParties, id, j, seeds[j]) | macoro::make_eager();
    }

    timer.setTimePoint("send/recv");
    for (u64 j = id + 1; j < numParties; j++) {
        prng.SetSeed(block(id, j), n);
        for (u64 i = 0; i < n; i++) {
            ret[i] = ret[i] ^ prng.mBuffer[i];
        }
    }

    timer.setTimePoint("wait");
    for (u64 j = 0; j < numParties - 1; j++) {
        macoro::sync_wait(std::move(tasks[j]));
    }

    timer.setTimePoint("wait done");
    for (u64 j = 0; j < id; j++) {
        prng.SetSeed(block(j, id), n);
        for (u64 i = 0; i < n; i++) {
            ret[i] = ret[i] ^ prng.mBuffer[i];
        }
    }
    timer.setTimePoint("done");
//    cout << timer << endl;

    return ret;
}
