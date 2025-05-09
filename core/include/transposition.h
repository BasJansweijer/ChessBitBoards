#pragma once

#include <cinttypes>
#include <iostream>
#include <algorithm>

#include <string>

#include "chess.h"
#include "types.h"
#include "eval.h"

namespace chess
{
    /*
     * Layout:
     * 2 bytes: eval
     * 1 byte: generation
     * 1 byte: flags (occupied, bound type etc)
     * 4 bytes: move
     * 4 bytes: hash (only first 32 bits)
     *
     * In total this is 2+1+1+4+4 = 12 bytes
     * (12 bytes with natural allignment)
     *
     */
    struct TTEntry
    {
    public:
        TTEntry() : flags(0) {}
        TTEntry(score normalizedEval, uint8_t depth, EvalBound bound, Move bestMove)
            : flags(depth | 0b100000 | bound), // depth (bits 1-5) occupied (bit 6) bound (bit 78)
              eval(normalizedEval),
              move(bestMove)
        {
        }

        score eval; // 16 bits

        inline bool occupied() const
        {
            // check first bit
            return flags & 0b100000;
        }

        inline bool containsHash(key boardHash) const
        {
            return occupied() && int32_t(boardHash) == partialHash;
        }

        inline EvalBound bound() const
        {
            // The eval bound options have been chosen to match the 0bxx0
            return (EvalBound)(flags & 0b11000000);
        }

        inline bool evalUsable(int curDepth, int remainingDepth, score curAlpha, score curBeta) const
        {
            bool goodDepth = depth() >= remainingDepth;
            if (!goodDepth)
                return false; // need deeper search

            score rootEval = scoreForRootNode(eval, curDepth);

            /*
             * Eval is usable if:
             *  - it is exact
             *  - if a cutoff can be produced from this
             */
            switch (bound())
            {
            case EvalBound::Exact:
                return true; // always usable
            case EvalBound::Lower:
                return curBeta < rootEval; // true if produces cutoff
            case EvalBound::Upper:
                return curAlpha > rootEval; // true if produces cutoff
            default:
                throw std::runtime_error("Invalid bound type");
            }
        }

        // The depth of the search that was used to generate this entry
        uint8_t depth() const
        {
            return flags & 0b11111; // first 5 bits
        }

    private:
        // Returns true if the newEntry should replace the current (it is more relevant)
        bool shouldOverwrite(const TTEntry &newEntry)
        {
            return !occupied() ||                        // unoccupied
                   depth() <= newEntry.depth() ||        // better depth
                   newEntry.generation - generation > 5; // stale current entry
        }

    private:
        friend class TranspositionTable;
        uint8_t generation; // at what search this was added

        /*
         * bit 1-5: depth (assumes depth < 32)
         * bit 6: occupied
         * bit 7: is lower bound
         * bit 8: is upper bound
         * (Exact bound is bit 1 and 2 set)
         */
        uint8_t flags;

        // we could use int16_t cast of full key (to save more space)
        // (need additional validation checks to prevent collisions)
        int32_t partialHash;

    public:
        // Placed here to avoid padding in TTEntry due to natural allignment
        Move move; // the best move found
    };

    class TranspositionTable
    {
    public:
        // Initialize a transposition table with the specified mbs of storage.
        TranspositionTable(int mbSize)
            : size((mbSize * 1024 * 1024) / sizeof(TTEntry))
        {
            static_assert(sizeof(TTEntry) == 12);
            table = new TTEntry[size];
        }

        ~TranspositionTable()
        {
            delete table;
        }

        // Note: the TTEntry * might not contain the correct hash
        TTEntry *get(key boardHash)
        {
            return &table[boardHash % size];
        }

        void set(key boardHash, TTEntry newEntry)
        {
            TTEntry *entry = &table[boardHash % size];

            // Check if we should overwrite our current entry
            if (!entry->shouldOverwrite(newEntry))
                return;

            // Overwrite
            *entry = newEntry;
            entry->generation = m_curSearchGeneration;
            entry->partialHash = int32_t(boardHash);
        }

        // Used to determine if entries are old enough to remove
        void startNewSearch() { m_curSearchGeneration++; }

        void clear()
        {
            // Set back to default entries
            std::fill(table, &table[size], TTEntry());
        }

        void purgeStaleEntries(int maxGenerationDiff = 5)
        {
            for (int i = 0; i < size; i++)
            {
                if (table[i].occupied() && table[i].generation - m_curSearchGeneration > maxGenerationDiff)
                    table[i] = TTEntry();
            }
        }

        // Estimates how full the table is
        double fullNess()
        {
            int searchedEntries = std::min(10000, size);

            int fullEntries = 0;
            for (int i = 0; i < searchedEntries; i++)
            {
                if (table[i].occupied() && table[i].generation - m_curSearchGeneration <= 5)
                    fullEntries++;
            }

            return fullEntries / (double)searchedEntries;
        }

    private:
        const int size;
        TTEntry *table;

        // only 8 bits since the TTEntries need to be efficient
        uint8_t m_curSearchGeneration;
    };
}