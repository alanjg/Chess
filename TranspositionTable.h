#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "Types.h"
#include "HashTable.h"

namespace SjelkjdChessEngine
{
	class TranspositionTable
	{
	public:
		static HashTable<ulong, int> PawnScoreTable;
		std::vector<int> killer1Score;
		std::vector<int> killer2Score;
		std::vector<int> killerMove1;
		std::vector<int> killerMove2;

		void EnsureKillerDepth(int depth)
		{
			depth++;
			if((uint)depth > killer1Score.size())
			{
				killer1Score.resize(depth);
				killer2Score.resize(depth);
				killerMove1.resize(depth);
				killerMove2.resize(depth);
			}
		}

		enum EntryType
		{
			Exact = 0,
			AlphaCutoff = 1,
			BetaCutoff = 2
		};
		
		struct Entry
		{
			Entry()
			{
			}
			int move;
			int evaluation;
			int plyDepth;
			EntryType entryType;
		};

	private:
		ulong CompressEntry(const Entry& entry)
		{
			ulong val = ENTRY_SET_MASK;

			unsigned int entryValue = (unsigned int)entry.evaluation;
			ulong add = (ulong)entryValue;
			add <<= 31;
			val += add;
			add = (ulong)std::min(entry.plyDepth, (1 << 6)-1);
			add <<= 25;
			val += add;
			add = (ulong)entry.entryType;
			add <<= 23;
			val += add;
			val += (ulong)entry.move;

			return val;
		}

		Entry DecompressEntry(ulong val)
		{
			// turn off the bit we stored to indicate that this entry was set.
			val ^= ENTRY_SET_MASK;

			Entry entry;
			ulong mask = 1ULL << 23;
			entry.move = int(val % mask);
			val >>= 23;
			mask = 1 << 2;
			entry.entryType = (EntryType)(val % mask);
			val >>= 2;
			mask = 1 << 6;
			entry.plyDepth = (int)(val % mask);
			val >>= 6;
			unsigned int intval = (unsigned int)val;
			entry.evaluation = (int)intval;
			return entry;
		}		
		
		static ulong zobristPositionKeys[64][16];
		static ulong zobristEnPassantKeys[65];
		static ulong zobristCastleKeys[16];
		static ulong zobristTurnKey;

		

	public:
		HashTable<ulong, ulong> hashTable;
		static const int minHashSize = 1;
		static const int maxHashSize = 1024;

		TranspositionTable()
		{
			static bool initialized = false;
			if(!initialized)
			{
				for (int i = 0; i < 64; i++)
				{
					for (int j = 0; j < 16; j++)
					{
						zobristPositionKeys[i][j] = GenerateKey();
					}
				}

				for (int i = 0; i < 65; i++)
				{
					zobristEnPassantKeys[i] = GenerateKey();
				}

				for (int i = 0; i < 16; i++)
				{
					zobristCastleKeys[i] = GenerateKey();
				}

				zobristTurnKey = GenerateKey();
				initialized = true;
			}
		}

		// Invalidates any current entries.
		void SetSize(int megabytes)
		{
			long size = megabytes;
			size *= 1000000 / hashTable.EntrySize();
			hashTable.Resize(size);
		}

	private:
		static ulong GenerateKey()
		{
			ulong need = std::numeric_limits<ulong>::max();
			ulong key = 0;
			while(need > 0)
			{
				key *= RAND_MAX + 1;
				key += (ulong)rand();

				need /= RAND_MAX + 1;
			}
			return key;
		}
		static const ulong ENTRY_SET_MASK = 1ULL << 63;
	public:
		bool TryGetEntry(ulong zobristKey, Entry& entry)
		{
			ulong value;
			if(hashTable.TryGetEntry(zobristKey, value) && (value & ENTRY_SET_MASK) != 0)
			{
				entry = DecompressEntry(value);
				return true;
			}
			return false;
		}

		void SetEntry(ulong zobristKey, int bestMove, int evaluation, int plyDepth, EntryType entryType)
		{
			Entry entry;
			entry.evaluation = evaluation;
			entry.move = bestMove;
			entry.plyDepth = plyDepth;
			entry.entryType = entryType;

			hashTable.SetEntry(zobristKey, CompressEntry(entry));
		}

		static ulong GetPositionKey(int square, int piece)
		{
			return zobristPositionKeys[square][piece];
		}

		static ulong GetEnPassantKey(EnPassantRights enPassantRights)
		{
			return zobristEnPassantKeys[enPassantRights.GetRights()];
		}

		static ulong GetCastleKey(CastleRights castleRights)
		{
			return zobristCastleKeys[castleRights.GetRights()];
		}

		static ulong GetTurnKey()
		{
			return zobristTurnKey;
		}
	};
}
#endif