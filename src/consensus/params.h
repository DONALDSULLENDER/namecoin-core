// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <amount.h>
#include <uint256.h>
#include <limits>
#include <map>
#include <string>

#include <memory>

namespace Consensus {

/**
 * Interface for classes that define consensus behaviour in more
 * complex ways than just by a set of constants.
 */
class ConsensusRules
{
public:

    /* Provide a virtual destructor since we have virtual methods.  */
    virtual ~ConsensusRules() = default;

    /* Return the expiration depth for names at the given height.  */
    virtual unsigned NameExpirationDepth(unsigned nHeight) const = 0;

    /* Return minimum locked amount in a name.  */
    virtual CAmount MinNameCoinAmount(unsigned nHeight) const = 0;

};

class MainNetConsensus : public ConsensusRules
{
public:

    unsigned NameExpirationDepth(unsigned nHeight) const
    {
        /* Important:  It is assumed (in ExpireNames) that
           "n - expirationDepth(n)" is increasing!  (This is
           the update height up to which names expire at height n.)  */

        if (nHeight < 24000)
            return 12000;
        if (nHeight < 48000)
            return nHeight - 12000;

        return 36000;
    }

    CAmount MinNameCoinAmount(unsigned nHeight) const
    {
        if (nHeight < 212500)
            return 0;

        return COIN / 100;
    }

};

class TestNetConsensus : public MainNetConsensus
{
public:

    CAmount MinNameCoinAmount(unsigned) const
    {
        return COIN / 100;
    }

};

class RegTestConsensus : public TestNetConsensus
{
public:

    unsigned NameExpirationDepth (unsigned nHeight) const
    {
        return 30;
    }

};

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;

    /** Constant for nTimeout very far in the future. */
    static constexpr int64_t NO_TIMEOUT = std::numeric_limits<int64_t>::max();

    /** Special value for nStartTime indicating that the deployment is always active.
     *  This is useful for testing, as it means tests don't need to deal with the activation
     *  process (which takes at least 3 BIP9 intervals). Only tests that specifically test the
     *  behaviour during activation cannot use this. */
    static constexpr int64_t ALWAYS_ACTIVE = -1;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    /** Block height at with BIP16 becomes active */
    int BIP16Height;
    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;
    /** Block height at which BIP65 becomes active */
    int BIP65Height;
    /** Block height at which BIP66 becomes active */
    int BIP66Height;
    /**
     * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargeting period,
     * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testchains.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    int64_t nMinDifficultySince;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetTimespan;
    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** Auxpow parameters */
    int32_t nAuxpowChainId;
    int nAuxpowStartHeight;
    bool fStrictChainId;
    int nLegacyBlocksBefore; // -1 for "always allow"

    /** Consensus rule interface.  */
    std::unique_ptr<ConsensusRules> rules;

    /**
     * Check whether or not minimum difficulty blocks are allowed
     * with the given time stamp.
     * @param nBlockTime Time of the block with minimum difficulty.
     * @return True if it is allowed to have minimum difficulty.
     */
    bool AllowMinDifficultyBlocks(int64_t nBlockTime) const
    {
        if (!fPowAllowMinDifficultyBlocks)
            return false;
        return nBlockTime > nMinDifficultySince;
    }

    /**
     * Check whether or not to allow legacy blocks at the given height.
     * @param nHeight Height of the block to check.
     * @return True if it is allowed to have a legacy version.
     */
    bool AllowLegacyBlocks(unsigned nHeight) const
    {
        if (nLegacyBlocksBefore < 0)
            return true;
        return static_cast<int> (nHeight) < nLegacyBlocksBefore;
    }
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
