#pragma once

#include "volePSI/Paxos.h"
#include <coproto/Socket/AsioSocket.h>
#include <cryptoTools/cryptoTools/Crypto/AES.h>

using oc::u8;
using oc::u64;
using oc::u32;
using oc::u64;
using oc::block;
using oc::ZeroBlock;
using oc::PRNG;
using oc::AES;
using namespace volePSI;
using namespace std;

vector<block> zeroShare(u64 n, u64 numParties, u64 id);
vector<block> realZeroShare(u64 n, u64 numParties, u64 id);

class Mpsi {
public:
    u64 n;
    u64 numParties;

    u64 w = 3;
    u64 ssp = 40;
    block seed = block(227, 229);

    u64 p1Data = 0;
    u64 p2Data = 0;
    u64 p3Data = 0;
    u64 p4Data = 0;

    void p1(const vector<block> &data);

    void p2(const vector<block> &data);

    void p3(u64 id, const vector<block> &data);

    block seed1 = block(5, 13);
    block seed2 = block(17, 36);

    void ca1(const vector<block> &data);

    void ca2(const vector<block> &data);

    void ca3();
};

void test();
