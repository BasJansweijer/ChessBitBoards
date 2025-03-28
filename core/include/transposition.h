#pragma once

#include <cinttypes>
#include <optional>

#include "chess.h"

namespace chess::engine
{

    template <typename EntryData>
    class TranspositionTable
    {
        struct TTEntry
        {
            uint64_t hash;
            EntryData value;
            TTEntry() : hash(UINT64_MAX) {}

            inline bool occupied() { return hash == UINT64_MAX; }
        };

        // Initialize a transposition table with the specified mbs of storage.
        TranspositionTable(int mbSize)
        {
            size = (mbSize * 1024 * 1024) / sizeof(TTEntry);
            table = new TTEntry[size];
        }

        ~TranspositionTable()
        {
            delete table;
        }

        std::optional<EntryData &> get(BoardState b)
        {
            uint64_t boardHash = b.getHash();
            TTEntry &entry = getEntry(boardHash);
            // only return if it is occupied with the correct hash
            return entry.hash == boardHash ? entry.value : std::nullopt;
        }

        void set(const BoardState &b, const EntryData &value)
        {
            uint64_t hash = b.getHash();
            TTEntry &entry = getEntry(hash);
            entry.hash = hash;
            entry.value = value;
        }

    private:
        const int size;
        const TTEntry *table;

        inline TTEntry &getEntry(uint64_t hash) { return table[hash % size]; }
    };
}