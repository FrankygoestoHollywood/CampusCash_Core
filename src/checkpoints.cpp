// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 5000;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (0,        Params().HashGenesisBlock() )
        (1,        uint256("0x0003071185b88b3b7ac32ce3a04c9e4bc5b485aee7f9d4be0cd906b8158ee2b4"))
        (10,       uint256("0x0001d465f5ba77a70cda9f5149619567edd0afe9ca40415709354ae534b66e89"))
        (100,      uint256("0x542a8101388444870dde573f2c126dcf9666bfe1664efa3dae647892e22cf916"))
        (150,      uint256("0xe18d2f78673c4503fc078eb9aeded8fe33d6db9f46a5d49ffd648d666607499a"))
        (175,      uint256("0x1bc95bf898bf5ad5868f079717158bd3a1314a534bbaea965fb36c9a26843f2d"))
        (500,      uint256("0xee09e49dc639a43f10f06ccbb54e53e77b05933f0923e9e4e8d499358a0f6d0a"))
        (1000,     uint256("0xfd7b4850ebe601810c03f46b8d11c38de7c5ed59253b373187b263ee5e0a32eb"))
        (2500,     uint256("0x00022741d81f82864f64b13489736a64f68898fe6662d7e24e85744fecf82e10"))
        (5000,     uint256("0x00001a9cae6e86f313e38481025b5f304bdf905dc1b963186edd0c2bd60ecebb"))
        (19000,    uint256("0x0000000000019619935fdce0d4468e31443ed4fd6b969419fc9e14b2f1c25894"))
        (20000,    uint256("0x00000000009c6ccc3ea60c6d593443be571c797bdc4cfa766ea6107843c93129"))
        (40000,    uint256("0x8d6ec3a75d44073ff08558c0a6152e18f53582e797e78335a47437a177219351"))
        (43500,    uint256("0x85bad7c9d35fe4b8af008ddab416f3df56d1287f360d5d4c76abe19dc5c3eafa"))
        (75000,    uint256("0x00000000003aba90a7e55ce85b9e6fa9d5190c771e7bbc31bca2862506023bd9"))
        (93152,    uint256("0x1eb5327e1a7e2a8671670832748deede920d8c302c9f148299b95f68b1ebda03"))
        (93160,    uint256("0x0002957c2e616c3a3965af78924b1c29ff70c81caa86b24b9c53baad96d3f3e3")) // Never Remove
        (93162,    uint256("0x00032b5ce45d53b67c1cd9b371b75ec054e0bf7542663e8122677d87ea356dca"))
        (93175,    uint256("0x7ba9de6ba9c0493ad08ffa5e677da710f9b9dd9ac3c1e837e8c98509f301be79"))
        (100000,   uint256("0x000000000027a8aeae0cad7a2bde09c9cd073affb3c5e070a67e508b990dca40"))
        (125000,   uint256("0x8cce829d75931cab2bfab72258afc2ffc9afa454e65597de8f92ff928010df6f"))
        (180750,   uint256("0x00000000001ef025e017c3fb10f4c3f62f5230b6ce2f022ac19b73ca4a0e01f7"))
        (198719,   uint256("0x75dcfb61d78bc665f7594782812b931777985563bdf64e06f84e95d1ff1c022c")) // Never Remove
        (200000,   uint256("0x9803fa4132bc11cdb465ff16bf0f8ff0f30bc9a2fb63e3985398235565f266f4"))
        (202884,   uint256("0x944ede48259183bf3e0867d90f1ae90fc76b18f990ee42e9048ec57741fce6d8"))
        (226933,   uint256("0x9957578726e950382d499e55af8086a5b55ad869334947b51adc4681e41d55da")) // Never Remove
        (253375,   uint256("0x4bac34cf01f45424daa5b19ccf2aa3be62ba7aa96e97a0c699572af2bb5eaa77")) // Never Remove
        (262168,   uint256("0x00000000002fb4bc686a0a6c9019971e60fbfc956b100f7e2fa86bda483fd536"))
        (348258,   uint256("0x72463265801c162f0ca96e1ea3226a66103ffd64ea4eec657311882b7ac9c9aa"))
        (348259,   uint256("0x722e6fb7a6821af7eb030366d44f7f44ca566722c5172c8631cf5931d4cbad3d"))
        (406094,   uint256("0xf3740130d3d7dc1d39dcb7721006e69f59e787eba785473c40a409baa061e92e"))
        (406095,   uint256("0xbf7665e1ca4c6402db9b65e6be281bccbe0874953de827d77e971c5df4271ff6"))
    ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        if (checkpoints.empty())
            return 0;
        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // Automatically select a suitable sync-checkpoint
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && pindex->nHeight + nCheckpointSpan > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
    bool CheckSync(int nHeight)
    {
        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();
        if (nHeight <= pindexSync->nHeight){
            return false;
        }
        return true;
    }
}
