#ifndef ENDGAMETABLE_H
#define ENDGAMETABLE_H

#include "Types.h"
#include "Board.h"

namespace ChessEngineUWP
{
	enum EndgameTableEntryResult
	{
		Unknown=0,
		Loss=1,
		Draw=2,
		Win=3,
		BeingSearched=4 // only used while building the table
	};

	struct EndgameTableEntry
	{
		EndgameTableEntryResult result; // -1 loss, 0 draw, 1 win
		int bestMoveIndex;
		int checkmateDistance; // how many moves to checkmate

		uint16 Compress();
		EndgameTableEntry();
		EndgameTableEntry(uint16 compressedRepresentation);
		EndgameTableEntry Reverse() const;
	};

	class EndgameTableDatabase
	{
	public:
		EndgameTableDatabase();
		~EndgameTableDatabase();
		EndgameTableEntry GetSetTableEntry(Board& board, EndgameTableEntry* entryToSet);
		void GenerateTables();
		void LoadTables(string file);
		void SaveTables(string file);
	private:
		uint16** endgameTables21;
		uint16*** endgameTables22;
		uint16*** endgameTables31;

		void GenerateTables21();
		EndgameTableEntry MiniMax(Board& board, int depth);
		std::vector<std::vector<int>*> moveLists;
		void EnsureMoveListDepth(uint depth);
	};

}

#endif