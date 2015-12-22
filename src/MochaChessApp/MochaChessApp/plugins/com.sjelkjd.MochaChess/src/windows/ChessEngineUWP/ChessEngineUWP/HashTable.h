#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Types.h"

namespace ChessEngineUWP
{
	template<class Key, class Value> class HashTable
	{
		std::pair<Key, Value>* hashTable;
		int size;

		int hits;
		int collisions;
		int misses;
	public:

		int Hits() { return hits; }
		int Misses() { return misses; }
		int Collisions() { return collisions; }

		HashTable()
		{
			// Must call resize!!!
			hashTable = 0;
			size = 0;
			hits = 0;
			misses = 0;
			collisions = 0;
		}

		HashTable(int _size)
		{
			size = FindNextLowestPrime(_size);
			hashTable = new std::pair<Key, Value>[size];
			hits = 0;
			misses = 0;
			collisions = 0;
		}

		~HashTable()
		{
			delete[] hashTable;
		}

		// in bytes
		int EntrySize()
		{
			return sizeof(std::pair<Key, Value>);
		}

		void Resize(int _size)
		{
			delete[] hashTable;
			size = FindNextLowestPrime(_size);
			hashTable = new std::pair<Key, Value>[size];
			hits = 0;
			misses = 0;
			collisions = 0;
		}

		bool TryGetEntry(Key hashCode, Value& entry)
		{
			std::pair<Key, Value>* tableEntry = &hashTable[hashCode % size];
			if (hashCode != tableEntry->first)
			{
				if(tableEntry->first != 0)
				{
					collisions++;
				}
				else
				{
					misses++;
				}

				return false;
			}
			else
			{
				hits++;
				entry = tableEntry->second;
				return true;
			}
		}

		void SetEntry(Key hashCode, const Value& entry)
		{
			hashTable[hashCode % size] = std::pair<Key, Value>(hashCode, entry);
		}

	private:
		int FindNextLowestPrime(int size)
		{
			while(!IsPrime(size))
			{
				size--;
			}
			return size;
		}
		bool IsPrime(int num)
		{
			if(num % 2 == 0) return false;
			for(int i=3;i*i<=num;i++)
			{
				if( num % i == 0) return false;
			}
			return true;
		}
	};
}
#endif