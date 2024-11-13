#include "mpsi.hpp"

void Mpsi::p3(u64 id, const vector<block> &data) {
    PRNG prng(oc::sysRandomSeed());

    Paxos<u32> okvs;
    okvs.init(n, w, ssp, PaxosParam::Binary, seed);
    u64 kSize = okvs.size();

    vector<block> in(n, ZeroBlock);
    vector<block> kData(kSize);

    oc::Timer timer;
    timer.setTimePoint("start");
    string address = std::to_string(7700 + id);
    auto connectTask =
            macoro::make_task(coproto::AsioConnect(address, coproto::global_io_context())) | macoro::make_eager();

    vector<block> zs = realZeroShare(kSize, numParties - 1, id - 2);
    timer.setTimePoint("zero share");
    okvs.solve<block>(data, in, kData, &prng);
    timer.setTimePoint("solve");
    for (u64 i = 0; i < kSize; i++) {
        kData[i] = kData[i] ^ zs[i];
    }

    auto chl = macoro::sync_wait(std::move(connectTask));
    timer.setTimePoint("connect");
//    cout << timer << endl;
    macoro::sync_wait(chl.send(kData));

    if (id == 3) {
        p3Data += chl.bytesReceived() + chl.bytesSent();
    } else if (id == 4) {
        p4Data += chl.bytesReceived() + chl.bytesSent();
    }
    macoro::sync_wait(chl.flush());

    if (id == 3) {
        ca3();
    }
}

void Mpsi::ca3() {
    auto connectTask1 = macoro::make_task(coproto::AsioAcceptor("localhost:7699", coproto::global_io_context(), 1)) |
                        macoro::make_eager();
    auto connectTask2 = macoro::make_task(coproto::AsioAcceptor("localhost:7698", coproto::global_io_context(), 1)) |
                        macoro::make_eager();

    u64 size = n;
    vector<block> data1(n);

    auto chl1 = macoro::sync_wait(std::move(connectTask1));
    macoro::sync_wait(chl1.recv(data1));
    AES aes;
    aes.setKey(seed2);
    aes.ecbEncBlocks(data1.data(), size, data1.data());
    std::random_device rd;
    std::mt19937 rng(rd());
    shuffle(data1.begin(), data1.end(), rng);
    auto chl2 = macoro::sync_wait(std::move(connectTask2));
    macoro::sync_wait(chl2.send(data1));

    p3Data += chl1.bytesReceived() + chl1.bytesSent();
    p3Data += chl2.bytesReceived() + chl2.bytesSent();
    macoro::sync_wait(chl1.flush());
    macoro::sync_wait(chl2.flush());
}
