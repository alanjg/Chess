#include "pch.h"
#include "TranspositionTable.h"

namespace ChessEngineUWP
{	
	ulong TranspositionTable::zobristPositionKeys[64][16];
	ulong TranspositionTable::zobristEnPassantKeys[65];
	ulong TranspositionTable::zobristCastleKeys[16];
	ulong TranspositionTable::zobristTurnKey;
	
	//HashTable<ulong, int> TranspositionTable::PawnScoreTable((1 << 16) - 1);
	HashTable<ulong, int> TranspositionTable::PawnScoreTable((1 << 10) - 1);
}