// Copyright (c) 2014-2015 The Dash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode-payments.h"
#include "masternodeman.h"
#include "mnengine.h"
#include "util.h"
#include "sync.h"
#include "spork.h"
#include "addrman.h"
#include <boost/lexical_cast.hpp>

CCriticalSection cs_masternodepayments;

/** Object for who's going to get paid on which blocks */
CMasternodePayments masternodePayments;
// keep track of Masternode votes I've seen
map<uint256, CMasternodePaymentWinner> mapSeenMasternodeVotes;

int CMasternodePayments::GetMinMasternodePaymentsProto() {
    return IsSporkActive(SPORK_10_MASTERNODE_PAY_UPDATED_NODES)
            ? MIN_MASTERNODE_PAYMENT_PROTO_VERSION_2
            : MIN_MASTERNODE_PAYMENT_PROTO_VERSION_1;
}


void ProcessMessageMasternodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if(!mnEnginePool.IsBlockchainSynced()) return;

    if (strCommand == "mnget") { //Masternode Payments Request Sync

        if(pfrom->HasFulfilledRequest("mnget")) {
            LogPrintf("mnget - peer already asked me for the list\n");
            Misbehaving(pfrom->GetId(), 20);
            return;
        }

        pfrom->FulfilledRequest("mnget");
        masternodePayments.Sync(pfrom);
        LogPrintf("mnget - Sent Masternode winners to %s\n", pfrom->addr.ToString().c_str());
    }
    else if (strCommand == "mnw") { //Masternode Payments Declare Winner

        LOCK(cs_masternodepayments);

        //this is required in litemode
        CMasternodePaymentWinner winner;
        vRecv >> winner;

        if(pindexBest == NULL) return;

        CTxDestination address1;
        ExtractDestination(winner.payee, address1);
        CCampusCashAddress address2(address1);

        uint256 hash = winner.GetHash();
        if(mapSeenMasternodeVotes.count(hash)) {
            if(fDebug) LogPrintf("mnw - seen vote %s Addr %s Height %d bestHeight %d\n", hash.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);
            return;
        }

        if(winner.nBlockHeight < pindexBest->nHeight - 10 || winner.nBlockHeight > pindexBest->nHeight+20){
            LogPrintf("mnw - winner out of range %s Addr %s Height %d bestHeight %d\n", winner.vin.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);
            return;
        }

        if(winner.vin.nSequence != std::numeric_limits<unsigned int>::max()){
            LogPrintf("mnw - invalid nSequence\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if(pfrom->nVersion < MIN_MASTERNODE_BSC_RELAY){
            LogPrintf("mnw - WARNING - Skipping winner relay, peer: %d is unable to handle such requests!\n", pfrom->nVersion);
            return;
        }

        LogPrintf("mnw - winning vote - Vin %s Addr %s Height %d bestHeight %d\n", winner.vin.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);

        if(!masternodePayments.CheckSignature(winner)){
            LogPrintf("mnw - invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        mapSeenMasternodeVotes.insert(make_pair(hash, winner));
    }
}


bool CMasternodePayments::CheckSignature(CMasternodePaymentWinner& winner)
{
    //note: need to investigate why this is failing
    std::string strMessage = winner.vin.ToString().c_str() + boost::lexical_cast<std::string>(winner.nBlockHeight) + winner.payee.ToString();
    std::string strPubKey = strMainPubKey ;
    CPubKey pubkey(ParseHex(strPubKey));

    std::string errorMessage = "";
    if(!mnEngineSigner.VerifyMessage(pubkey, winner.vchSig, strMessage, errorMessage)){
        return false;
    }

    return true;
}


bool CMasternodePayments::Sign(CMasternodePaymentWinner& winner)
{
    std::string strMessage = winner.vin.ToString().c_str() + boost::lexical_cast<std::string>(winner.nBlockHeight) + winner.payee.ToString();

    CKey key2;
    CPubKey pubkey2;
    std::string errorMessage = "";

    if(!mnEngineSigner.SetKey(strMasterPrivKey, errorMessage, key2, pubkey2))
    {
        LogPrintf("CMasternodePayments::Sign - ERROR: Invalid Masternodeprivkey: '%s'\n", errorMessage.c_str());
        LogPrintf("CMasternodePayments::Sign - FORCE BYPASS - SetKey checks!!!\n");
        //return false;
    }

    if(!mnEngineSigner.SignMessage(strMessage, errorMessage, winner.vchSig, key2)) {
        LogPrintf("CMasternodePayments::Sign - Sign message failed\n");
        LogPrintf("CMasternodePayments::Sign - FORCE BYPASS - Sign message checks!!!\n");
        //return false;
    }

    if(!mnEngineSigner.VerifyMessage(pubkey2, winner.vchSig, strMessage, errorMessage)) {
        LogPrintf("CMasternodePayments::Sign - Verify message failed\n");
        LogPrintf("CMasternodePayments::Sign - FORCE BYPASS - Verify message checks!!!\n");
        //return false;
    }

    return true;
}


uint64_t CMasternodePayments::CalculateScore(uint256 blockHash, CTxIn& vin)
{
    uint256 n1 = blockHash;
    uint256 n2 = Hash(BEGIN(n1), END(n1));
    uint256 n3 = Hash(BEGIN(vin.prevout.hash), END(vin.prevout.hash));
    uint256 n4 = n3 > n2 ? (n3 - n2) : (n2 - n3);

    return n4.Get64();
}


bool CMasternodePayments::GetWinningMasternode(CBlockIndex* pindexLast, CTxIn& vin, CScript& payee)
{
    if(IsInitialBlockDownload() || !mnEnginePool.IsMasternodeListSynced()) return false;

    CMasternode* winningNode = mnodeman.GetMasterNodeWinner(pindexLast);

    if(winningNode)
    {
        payee = GetScriptForDestination(winningNode->pubkey.GetID());
        vin = winningNode->vin;
        return true; 
    }

    return false;
}


bool CMasternodePayments::AddWinningMasternode(CMasternodePaymentWinner& winnerIn)
{
    if(mWinning.count(winnerIn.nBlockHeight) != 0)
    {
        if(mWinning[winnerIn.nBlockHeight].vin != winnerIn.vin)
        {
            mWinning[winnerIn.nBlockHeight] = winnerIn;
            return true;
        }

        return false;
    }
    
    mWinning[winnerIn.nBlockHeight] = winnerIn;

    return true;
}


void CMasternodePayments::CleanPaymentList()
{
    LOCK(cs_masternodepayments);

    if(pindexBest == NULL) return;

    int nLimit = std::max((int)((int)mnodeman.size()*1.25), 1000);
    
    std::map<int,CMasternodePaymentWinner>::iterator it = mWinning.lower_bound(pindexBest->nHeight - nLimit);
    
    mWinning.erase(mWinning.begin(),it);
}


bool CMasternodePayments::ProcessBlock(int nBlockHeight)
{
    LOCK(cs_masternodepayments);

    if(IsInitialBlockDownload()) return false;

    CMasternodePaymentWinner newWinner;

    CMasternode *pmn = mnodeman.GetCurrentMasterNode(1);
    if(pmn == NULL) 
    {
        LogPrintf("Masternode-Payments::ProcessBlock - FAILED - No masternodes detected...\n");
        return false;
    }

    newWinner.score = 0;
    newWinner.nBlockHeight = nBlockHeight;
    newWinner.vin = pmn->vin;
    newWinner.payee = GetScriptForDestination(pmn->pubkey.GetID());
 
    if(AddWinningMasternode(newWinner))
    {
        CTxDestination address1;
        ExtractDestination(newWinner.payee, address1);
        CCampusCashAddress address2(address1);

        int tier = 1;
        if(mnEngineSigner.IsVinTier2(newWinner.vin)) tier = 2; 

        LogPrintf("MASTERNODE WINNER - SUCCESS - height = %d  winner : %s  tier : %d\n", nBlockHeight, address2.ToString().c_str(), tier);
        return true;
    }

    return false;
}


void CMasternodePayments::Relay(CMasternodePaymentWinner& winner)
{
    CInv inv(MSG_MASTERNODE_WINNER, winner.GetHash());
    vector<CInv> vInv;
    vInv.push_back(inv);

    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if(pnode->nVersion >= MIN_MASTERNODE_BSC_RELAY) 
        { 
            pnode->PushMessage("inv", vInv);
        }
    }
}


void CMasternodePayments::Sync(CNode* node)
{
    LOCK(cs_masternodepayments);

    std::map<int,CMasternodePaymentWinner>::iterator it = mWinning.lower_bound(pindexBest->nHeight - 10);

    while(it != mWinning.end() && it->first <= pindexBest->nHeight + 20)
    {
        node->PushMessage("mnw", it->second);
        it++;
    }
}


bool CMasternodePayments::SetPrivKey(const std::string strPrivKey)
{
    CMasternodePaymentWinner winner;

    // Test signing successful, proceed
    strMasterPrivKey = strPrivKey;

    Sign(winner);

    if(CheckSignature(winner)){
        LogPrintf("CMasternodePayments::SetPrivKey - Successfully initialized as Masternode payments master\n");
        enabled = true;
        return true;
    } else {
        return false;
    }
}
