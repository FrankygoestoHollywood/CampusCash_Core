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

    //printf(" -- CMasternodePayments CalculateScore() n2 = %d \n", n2.Get64());
    //printf(" -- CMasternodePayments CalculateScore() n3 = %d \n", n3.Get64());
    //printf(" -- CMasternodePayments CalculateScore() n4 = %d \n", n4.Get64());

    return n4.Get64();
}

bool CMasternodePayments::GetWinningMasternode(int nBlockHeight, CTxIn& vin)
{
    BOOST_FOREACH(CMasternodePaymentWinner& winner, vWinning){
        if(winner.nBlockHeight == nBlockHeight) {
            vin = winner.vin;
            return true;
        }
    }

    return false;

    // Ensure exclusion of pointless looping
    //
    // TODO: Move this to block params after verification
 /*   if(nMNpayBlockHeight == nBlockHeight){
        if(fMnWnr){
            return true;
        } else {
            return false;
        }
    }
    // Set loop logging
    nMNpayBlockHeight = nBlockHeight;
    fMnWnr = false;
    // If initial sync or we can't find a masternode in our list
    if(IsInitialBlockDownload() || !ProcessBlock(nBlockHeight)){
        // Return false (for sanity, we have no masternode to pay)
        LogPrintf(" GetWinningMasternode()) - ProcessBlock failed (OR) Still syncing... \n");
        return false;
    }
    // Set masternode winner to pay
    BOOST_FOREACH(CMasternodePaymentWinner& winner, vWinning){
        vin = winner.vin;
    }
    // Check Tier level of MasterNode winner
    fMnT2 = mnEngineSigner.IsVinTier2(vin);
    // Return true if previous checks pass
    return true;*/
}

bool CMasternodePayments::AddWinningMasternode(CMasternodePaymentWinner& winnerIn)
{
    BOOST_FOREACH(CMasternodePaymentWinner& winner, vWinning)
    {
        if(winner.nBlockHeight == winnerIn.nBlockHeight) 
        {
            return true;
        }
    }

    vWinning.push_back(winnerIn);
    return true;
}

void CMasternodePayments::CleanPaymentList()
{
    LOCK(cs_masternodepayments);

    if(pindexBest == NULL) return;

    int nLimit = std::max(((int)mnodeman.size())*((int)1.25), 1000);

    vector<CMasternodePaymentWinner>::iterator it;
    for(it=vWinning.begin();it<vWinning.end();it++){
        if(pindexBest->nHeight - (*it).nBlockHeight > nLimit){
            if(fDebug) LogPrintf("CMasternodePayments::CleanPaymentList - Removing old Masternode payment - block %d\n", (*it).nBlockHeight);
            vWinning.erase(it);
            break;
        }
    }
}

bool CMasternodePayments::NodeisCapable()
{
    bool fNodeisCapable;
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes){
        if(pnode->nVersion >= MIN_MASTERNODE_BSC_RELAY) {
            LogPrintf(" NodeisCapable() - Capable peers found (Masternode relay) \n");
            fNodeisCapable = true;
            break;
        } else {
            LogPrintf(" NodeisCapable() - Cannot relay with peers (Masternode relay) \n");
            fNodeisCapable = false;
            break;
        }
    }
    return fNodeisCapable;
}

bool CMasternodePayments::ProcessBlock(int nBlockHeight)
{
    LOCK(cs_masternodepayments);

    if(IsInitialBlockDownload()) return false;
    if(nBlockHeight <= nLastBlockHeight)
    {
        LogPrintf("Masternode-Payments::ProcessBlock - FAILED - height too low - current = %d VS last = %d\n", nBlockHeight, nLastBlockHeight);
        return false;
    }

    CMasternodePaymentWinner newWinner;

    CBitcoinAddress cDevopsPayee;
    cDevopsPayee = CBitcoinAddress("Ce1XyENjUHHPBt8mxy2LupkH2PnequevMM");// devops address
    cMNpayee = GetScriptForDestination(cDevopsPayee.Get());

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

    cMNpayee = GetScriptForDestination(pmn->pubkey.GetID());
 
    if(AddWinningMasternode(newWinner))
    {
        nLastBlockHeight = nBlockHeight;
        LogPrintf("Masternode-Payments::ProcessBlock - SUCCESS - height = %d  winner : %s...\n", nBlockHeight, pmn->pubkey.GetID());
        return true;
    }

    return false;
}


void CMasternodePayments::Relay(CMasternodePaymentWinner& winner)
{
    CInv inv(MSG_MASTERNODE_WINNER, winner.GetHash());

    LogPrintf("Relayed winning masternode. \n");

    vector<CInv> vInv;
    vInv.push_back(inv);
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes){
        if(NodeisCapable()) { // Only send to capable nodes
            // TODO: Fix Dash's old implementation, this is just sad.
            // First figure out why RelayMasternode bans MNs and then get the
            // Relay properly functional. None of this half ass shit which seems
            // to be the damn standard.
            //
            //pnode->PushMessage("inv", vInv);
        }
    }
}

void CMasternodePayments::Sync(CNode* node)
{
    LOCK(cs_masternodepayments);

    BOOST_FOREACH(CMasternodePaymentWinner& winner, vWinning)
        if(winner.nBlockHeight >= pindexBest->nHeight-10 && winner.nBlockHeight <= pindexBest->nHeight + 20)
            node->PushMessage("mnw", winner);
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
