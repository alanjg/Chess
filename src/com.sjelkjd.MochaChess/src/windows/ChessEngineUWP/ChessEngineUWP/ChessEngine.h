#pragma once
#include "Board.h"
#include "Search.h"
#include <memory>

namespace SjelkjdChessEngine
{
	ref class ChessEngine sealed
	{
		std::auto_ptr<Board> board;
		std::auto_ptr<Search> search;
		std::auto_ptr<MoveGenerator> moveGenerator;
	public:
		ChessEngine();
		void MakeMove(int sr, int sc, int er, int ec);
		Platform::String^ GetPiece(int row, int col);
		void MakeComputerMove();
		bool isValidMoveStart(int row, int col);
		bool isValidMove(int sr, int sc, int er, int ec);
		void initializeBoard();
	};
}
