#pragma once
#include "Board.h"
#include "Search.h"
#include <memory>

namespace ChessEngineUWP
{
	public ref class ChessEngine sealed
	{
		std::auto_ptr<Board> board;
		std::auto_ptr<Search> search;
		std::auto_ptr<MoveGenerator> moveGenerator;
	public:
		ChessEngine();
		void makeMove(int sr, int sc, int er, int ec);
		Platform::String^ getPiece(int row, int col);
		void makeComputerMove();
		bool isValidMoveStart(int row, int col);
		bool isValidMove(int sr, int sc, int er, int ec);
		void setStartPosition();
		void setPosition(Platform::String^ fen);
	};
}
