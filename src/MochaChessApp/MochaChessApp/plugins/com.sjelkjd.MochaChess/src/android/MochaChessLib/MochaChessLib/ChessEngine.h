#ifndef CHESSENGINE_H
#define CHESSENGINE_H
#include "Board.h"
#include "Search.h"
#include <memory>

namespace SjelkjdChessEngine
{
	class ChessEngine
	{
		std::auto_ptr<Board> board;
		std::auto_ptr<Search> search;
		std::auto_ptr<MoveGenerator> moveGenerator;
	public:
		ChessEngine();
		void MakeMove(int sr, int sc, int er, int ec);
		std::string GetPiece(int row, int col);
        std::string GetBoard();
		void MakeComputerMove();
		bool isValidMoveStart(int row, int col);
		bool isValidMove(int sr, int sc, int er, int ec);
		void setStartPosition();
		void setPosition(std::string& fen);
	};
}

#endif