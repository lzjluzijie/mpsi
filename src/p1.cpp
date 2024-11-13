#include "mpsi.hpp"

macoro::task<> recv(u64 id, vector<block> &data, u64 &transfer) {
    string address = "localhost:" + std::to_string(7700 + id);
    auto chl = co_await coproto::AsioAcceptor(address, coproto::global_io_context(), 1);

    co_await chl.recv(data);
    transfer += chl.bytesReceived() + chl.bytesSent();
    co_await chl.flush();
}

void Mpsi::p1(const vector<block> &data) {
    u64 id = 1;

    Paxos<u32> okvs;
    okvs.init(n, w, ssp, PaxosParam::Binary, seed);
    u64 kSize = okvs.size();

    vector<block> values(n);
    vector<block> kData(kSize, ZeroBlock);

    u64 transfer = 0;
    vector<vector<block>> shares(numParties - 1, vector<block>(kSize));
    vector<macoro::eager_task<>> tasks(numParties - 1);
    for (u64 i = 2; i <= numParties; i++) {
        tasks[i - 2] = recv(i, shares[i - 2], transfer) | macoro::make_eager();
    }

    for (u64 i = 2; i <= numParties; i++) {
        macoro::sync_wait(std::move(tasks[i - 2]));
    }

    for (u64 i = 0; i < numParties - 1; i++) {
        for (u64 j = 0; j < kSize; j++) {
            kData[j] = kData[j] ^ shares[i][j];
        }
    }
    okvs.decode<block>(data, values, kData);

    p1Data += transfer;
    ca1(values);
}

void Mpsi::ca1(const vector<block> &data) {
    auto connectTask = macoro::make_task(coproto::AsioConnect("localhost:7698", coproto::global_io_context())) |
                       macoro::make_eager();

    u64 size = n;
    AES aes;
    aes.setKey(seed1);
    vector<block> my(size);
    aes.ecbEncBlocks(data.data(), size, my.data());
    aes.setKey(seed2);
    aes.ecbEncBlocks(my.data(), size, my.data());

    set<block> mySet(my.begin(), my.end());

    auto chl = macoro::sync_wait(std::move(connectTask));

    vector<block> data2(n);
    macoro::sync_wait(chl.recv(data2));

    u64 count = 0;
    for (u64 i = 0; i < n; i++) {
        if (mySet.count(data2[i])) {
            count++;
        }
    }
    cout << count << endl;

    p1Data += chl.bytesReceived() + chl.bytesSent();
    macoro::sync_wait(chl.flush());
}
