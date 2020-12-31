// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util.h"
// TODO: Verify the requirement of below link
// #include "base58.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

// Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//
// Main network
//
class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x1a;
        pchMessageStart[1] = 0x2a;
        pchMessageStart[2] = 0x34;
        pchMessageStart[3] = 0x3f;
        vAlertPubKey = ParseHex("01b88735a489f996be6b659c91a56897ebeb5d517698712acdbef78945c2f81f85d131aadfef3be6145678454852a2d08c6314bba5ca3cbe5616262da3b1a6afed");
        nDefaultPort = 19427;
        
        nRPCPort = 18695;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 14);
        bnProofOfStakeLimit = CBigNum(~uint256(0) >> 16);

        const char* pszTimestamp = "The COVID-19 Economy in Isolation | Brian Wallace | May 14, 2020 | Infographics | TheMerkle";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].nValue = 1 * COIN;
        vout[0].SetEmpty();
        CTransaction txNew(1, timeGenesisBlock, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = timeGenesisBlock; // Sat, December 15, 2018 8:00:00 PM
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 14018;

        /** Genesis Block MainNet */
        /*
        Hashed MainNet Genesis Block Output
        block.hashMerkleRoot == 89bc5744c5f6133ff2146ab982fafdcc9298a8d766e3437f0b4fa655bd4c0c77
        block.nTime = 1589457600
        block.nNonce = 14018
        block.GetHash = 0001b3bc9d451f3fddcef4276f52d98ea17fb48cc7ff4d9151e00beaeadaf0dc
        */

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x0001b3bc9d451f3fddcef4276f52d98ea17fb48cc7ff4d9151e00beaeadaf0dc"));
        assert(genesis.hashMerkleRoot == uint256("0x89bc5744c5f6133ff2146ab982fafdcc9298a8d766e3437f0b4fa655bd4c0c77"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,28);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,29);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,88);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,87);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // vSeeds.push_back(CDNSSeedData("node0",  "000.000.000.000"));


        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        nPoolMaxTransactions = 9;
        strMNenginePoolDummyAddress = "CcABDmWkcSZPw8rMtoobShVFuudhf1svZu"; // CVGQAbKX5MvmsSN4x1GeCNqNsxzkPJuWEW devopsaddress is dHy3LZvqX5B2rAAoLiA7Y7rpvkLXKTkD18
        strDevOpsAddress = "Ce1XyENjUHHPBt8mxy2LupkH2PnequevMM"; // CaKwWL6e84WeKGsqBPYPXckkgp6jUa2ARr
        nEndPoWBlock = 15010;
        nEndPoWBlock_v2 = 525600; // Changed to ensure One year of mining
        nStartPoSBlock = 1;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xa2;
        pchMessageStart[1] = 0xb5;
        pchMessageStart[2] = 0x16;
        pchMessageStart[3] = 0x98;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 12);
        bnProofOfStakeLimit = CBigNum(~uint256(0) >> 14);
        vAlertPubKey = ParseHex("00f88456af9f1996be6b659c91a94fbfebeb5d517648afbacdbef262f7c2f81f85d131a669df3be6113afd454852a2d08c6314bba5ca3cbe5616262da3b1a6afed");
        nDefaultPort = 20201;
        nRPCPort = 20202;
        strDataDir = "testnet";

        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime  = timeTestNetGenesis;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 8599;

        /** Genesis Block TestNet */
        /*
            Hashed TestNet Genesis Block Output
        Hashed TestNet Genesis Block Output
        block.hashMerkleRoot == 89bc5744c5f6133ff2146ab982fafdcc9298a8d766e3437f0b4fa655bd4c0c77
        block.nTime = 1589457630
        block.nNonce = 8599
        block.GetHash = 000ea64a9182c361f48ee8fd7d0bc0578091ad3defc35a53ecb97627d76f9f7b
        */

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x000ea64a9182c361f48ee8fd7d0bc0578091ad3defc35a53ecb97627d76f9f7b"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,86);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,89);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,65);
        base58Prefixes[STEALTH_ADDRESS] = std::vector<unsigned char>(1,53);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        nEndPoWBlock = 0x7fffffff;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;

//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        pchMessageStart[0] = 0x54;
        pchMessageStart[1] = 0xf5;
        pchMessageStart[2] = 0x03;
        pchMessageStart[3] = 0x8d;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 1);
        genesis.nTime = timeRegNetGenesis;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 8;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 78787;
        strDataDir = "regtest";

        /** Genesis Block RegNet */
        /*
        Hashed RegNet Genesis Block Output
        block.hashMerkleRoot == 89bc5744c5f6133ff2146ab982fafdcc9298a8d766e3437f0b4fa655bd4c0c77
        block.nTime = 1589457690
        block.nNonce = 8
        block.GetHash = f34d463c24568f5c1c272a96a8f9cb9dd71db2d7aef42068a6f863bca5377b99
        */


        assert(hashGenesisBlock == uint256("0xf34d463c24568f5c1c272a96a8f9cb9dd71db2d7aef42068a6f863bca5377b99"));

        vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
