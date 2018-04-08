//////////////////////////////////////////////////////////////////////////////////////
//
// (C) Daniel Strano 2018. All rights reserved.
//
// This is an abstraction on "CoherentUnit" per https://arxiv.org/abs/1710.05867
//
// "SeparatedUnit" keeps representation of qubit states separated until explicitly
// entangled. This makes for large gains in memory and speed optimization in the
// best case scenario. "CoherentUnit" has been optimized for the worst case scenario.
//
// Licensed under the GNU General Public License V3.
// See LICENSE.md in the project root or https://www.gnu.org/licenses/gpl-3.0.en.html
// for details.

#pragma once

#include "qregister.hpp"
#include <memory>
#include <vector>

#define bitLenInt uint8_t
#define bitCapInt uint64_t
#define bitsInByte 8

namespace Qrack {

struct QbLookup {
    bitLenInt cu;
    bitLenInt qb;
};

struct QbListEntry {
    bitLenInt cu;
    bitLenInt start;
    bitLenInt length;
};

class SeparatedUnit;

class SeparatedUnit : public CoherentUnit {
public:
    /** Initialize a coherent unit with qBitCount number of bits, all to |0> state. */
    SeparatedUnit(bitLenInt qBitCount);
    /** Initialize a coherent unit with qBitCount number of bits, to initState unsigned integer permutation state */
    SeparatedUnit(bitLenInt qBitCount, bitCapInt initState);
    /**
     * Initialize a cloned register with same exact quantum state as pqs
     *
     * \warning PSEUDO-QUANTUM
     */
    SeparatedUnit(const SeparatedUnit& pqs);

    void CloneRawState(Complex16* output);
    void SetQuantumState(Complex16* inputState);
    void Cohere(CoherentUnit& toCopy);
    void Cohere(SeparatedUnit& toCopy);
    void Decohere(bitLenInt start, bitLenInt length, CoherentUnit& destination);
    void Dispose(bitLenInt start, bitLenInt length);

    double Prob(bitLenInt qubitIndex);
    bool M(bitLenInt qubitIndex);
    bitCapInt MReg(bitLenInt start, bitLenInt length);
    void SetBit(bitLenInt qubitIndex1, bool value);
    void SetReg(bitLenInt start, bitLenInt length, bitCapInt value);
    void SetPermutation(bitCapInt value);

    unsigned char SuperposeReg8(bitLenInt inputStart, bitLenInt outputStart, unsigned char* values);
    unsigned char AdcSuperposeReg8(
        bitLenInt inputStart, bitLenInt outputStart, bitLenInt carryIndex, unsigned char* values);
    unsigned char SbcSuperposeReg8(
        bitLenInt inputStart, bitLenInt outputStart, bitLenInt carryIndex, unsigned char* values);

    void Swap(bitLenInt qubitIndex1, bitLenInt qubitIndex2);
    void Swap(bitLenInt qubitIndex1, bitLenInt qubitIndex2, bitLenInt length);

    void AND(bitLenInt inputBit1, bitLenInt inputBit2, bitLenInt outputBit);
    void OR(bitLenInt inputBit1, bitLenInt inputBit2, bitLenInt outputBit);
    void XOR(bitLenInt inputBit1, bitLenInt inputBit2, bitLenInt outputBit);
    void CLAND(bitLenInt inputQBit, bool inputClassicalBit, bitLenInt outputBit);
    void CLOR(bitLenInt inputQBit, bool inputClassicalBit, bitLenInt outputBit);
    void CLXOR(bitLenInt inputQBit, bool inputClassicalBit, bitLenInt outputBit);

    void CCNOT(bitLenInt inputBit1, bitLenInt inputBit2, bitLenInt outputBit);
    void AntiCCNOT(bitLenInt inputBit1, bitLenInt inputBit2, bitLenInt outputBit);

    void H(bitLenInt qubitIndex);
    void X(bitLenInt qubitIndex);
    void Y(bitLenInt qubitIndex);
    void Z(bitLenInt qubitIndex);

    void X(bitLenInt start, bitLenInt length);

    void CY(bitLenInt control, bitLenInt target);
    void CZ(bitLenInt control, bitLenInt target);

    void RT(double radians, bitLenInt qubitIndex);
    void RTDyad(int numerator, int denominator, bitLenInt qubitIndex);
    void RX(double radians, bitLenInt qubitIndex);
    void RXDyad(int numerator, int denominator, bitLenInt qubitIndex);
    void RY(double radians, bitLenInt qubitIndex);
    void RYDyad(int numerator, int denominator, bitLenInt qubitIndex);
    void RZ(double radians, bitLenInt qubitIndex);
    void RZDyad(int numerator, int denominator, bitLenInt qubitIndex);

    void CRT(double radians, bitLenInt control, bitLenInt target);
    void CRTDyad(int numerator, int denominator, bitLenInt control, bitLenInt target);
    void CRY(double radians, bitLenInt control, bitLenInt target);
    void CRYDyad(int numerator, int denominator, bitLenInt control, bitLenInt target);
    void CRZ(double radians, bitLenInt control, bitLenInt target);
    void CRZDyad(int numerator, int denominator, bitLenInt control, bitLenInt target);

protected:
    std::unique_ptr<QbLookup[]> qubitLookup;
    std::unique_ptr<bitLenInt[]> qubitInverseLookup;
    std::vector<std::shared_ptr<CoherentUnit>> coherentUnits;

    /**
     * Compile an order-preserving list of CoherentUnit bit strings for applying an register-wise operation
     *
     * This operation optimizes compiling a list out of qubit pile when bit order is important. We apply register-wise
     * operations over a pile of arbitrarily entangled and separated qubits. Entangled qubits are stored together in
     * single CoherentUnit objects, but their mapping to SeparatedUnit bit indices can be generally random. Sometimes,
     * we must preserve bit order to correctly carry out the operation, whereas sometimes our operation is bitwise
     * parallel and does not depend on the ordering of bits in the list.
     */
    void GetOrderedBitList(bitLenInt start, bitLenInt length, std::vector<QbListEntry>& qbList);
    /**
     * Compile a list of CoherentUnit bit strings for applying a bitwise-parallel operation
     *
     * This operation optimizes compiling a list out of qubit pile when bit order is not important. We apply
     * register-wise operations over a pile of arbitrarily entangled and separated qubits. Entangled qubits are stored
     * together in single CoherentUnit objects, but their mapping to SeparatedUnit bit indices can be generally random.
     * Sometimes, we must preserve bit order to correctly carry out the operation, whereas sometimes our operation is
     * bitwise parallel and does not depend on the ordering of bits in the list.
     */
    void GetParallelBitList(bitLenInt start, bitLenInt length, std::vector<QbListEntry>& qbList);
    /** Optimizes combined lists returned by GetParallelBitList() by the same logic as that algorithm */
    void OptimizeParallelBitList(std::vector<QbListEntry>& qbList);
    /** Entangle and sort the indices of a list of CoherentUnit objects */
    void EntangleBitList(std::vector<QbListEntry> qbList);
    /** Quicksort entangled bits */
    void QuickSortQubits(bitLenInt* arr, bitLenInt low, bitLenInt high, std::weak_ptr<CoherentUnit> cu);
    /** Convenience method for three bit gate */
    void EntangleIndices(std::vector<bitLenInt> indices);
};
} // namespace Qrack
