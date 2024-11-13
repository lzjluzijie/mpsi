#include "mpsi.hpp"

void Mpsi::p2(const vector<block> &data) {
    u64 id = 2;
    PRNG prng(oc::sysRandomSeed());

    Paxos<u32> okvs;
    okvs.init(n, w, ssp, PaxosParam::Binary, seed);
    u64 kSize = okvs.size();

    vector<block> in(n);
    prng.get(in.data(), in.size());
    vector<block> kData(kSize);

    string address = std::to_string(7700 + id);
    auto connectTask =
            macoro::make_task(coproto::AsioConnect(address, coproto::global_io_context())) | macoro::make_eager();

    vector<block> zs = realZeroShare(kSize, numParties - 1, id - 2);
    okvs.solve<block>(data, in, kData, &prng);
    for (u64 i = 0; i < kSize; i++) {
        kData[i] = kData[i] ^ zs[i];
    }

    auto chl = macoro::sync_wait(std::move(connectTask));
    macoro::sync_wait(chl.send(kData));

    macoro::sync_wait(chl.flush());

    p2Data += chl.bytesReceived() + chl.bytesSent();
    ca2(in);
}

void Mpsi::ca2(const vector<block> &data) {
    u64 size = data.size();
    if (size != n) {
        throw RTE_LOC;
    }

    auto connectTask = macoro::make_task(coproto::AsioConnect("localhost:7699", coproto::global_io_context())) |
                       macoro::make_eager();

    vector<block> data1(size);
    AES aes;
    aes.setKey(seed1);
    aes.ecbEncBlocks(data.data(), size, data1.data());

    auto chl = macoro::sync_wait(std::move(connectTask));
    macoro::sync_wait(chl.send(data1));

    p2Data += chl.bytesReceived() + chl.bytesSent();
    macoro::sync_wait(chl.flush());
}
