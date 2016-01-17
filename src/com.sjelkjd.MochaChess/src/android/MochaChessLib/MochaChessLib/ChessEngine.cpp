#include "pch.h"
#include "ChessEngine.h"
#include "Types.h"

namespace SjelkjdChessEngine
{
	ChessEngine::ChessEngine()
	{
		Board* b = new Board();
		board.reset(b);
		search.reset(new Search(*b));
		board->SetFEN(Board::startPosition);
		moveGenerator.reset(new MoveGenerator(*b));
	}

	void ChessEngine::MakeMove(int sr, int sc, int er, int ec)
	{
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		int foundMove = NullMove;
		for (int i = 0; i < moves.size();i++)
		{
			int move = moves[i];
			int source = GetSourceFromMove(move);
			int dest = GetDestFromMove(move);
			if (sr == GetRow(source) && sc == GetCol(source) && er == GetRow(dest) && ec == GetCol(dest))
			{
				foundMove = move;
				break;
			}
		}
		if (foundMove != NullMove)
		{
			board->MakeMove(foundMove, true);
		}
	}

	std::string ChessEngine::GetPiece(int row, int col)
	{
		int piece = board->GetPiece(Square(row, col));
		std::string p;
		switch (piece)
		{
		case Pieces::BlackBishop: p = "b"; break;
		case Pieces::BlackKing: p = "k"; break;
		case Pieces::BlackKnight: p = "n"; break;
		case Pieces::BlackPawn: p = "p"; break;
		case Pieces::BlackQueen: p = "q"; break;
		case Pieces::BlackRook: p = "r"; break;
		case Pieces::None: p = ""; break;
		case Pieces::WhiteBishop: p = "B"; break;
		case Pieces::WhiteKing: p = "K"; break;
		case Pieces::WhiteKnight: p = "N"; break;
		case Pieces::WhitePawn: p = "P"; break;
		case Pieces::WhiteQueen: p = "Q"; break;
		case Pieces::WhiteRook: p = "R"; break;
		}
		return p;
	}
    
    std::string ChessEngine::GetBoard()
	{
		return board->ToString();
	}

	void ChessEngine::MakeComputerMove()
	{
		int move = search->GetBestMove(1.0);
		board->MakeMove(move, true);
	}

	bool ChessEngine::isValidMoveStart(int row, int col)
	{
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		for (int i = 0; i < moves.size(); i++)
		{
			int move = moves[i];
			int source = GetSourceFromMove(move);
			int dest = GetDestFromMove(move);
			if (row == GetRow(source) && col == GetCol(source))
			{
				return true;
			}
		}
		return false;
	}

	bool ChessEngine::isValidMove(int sr, int sc, int er, int ec)
	{
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		for (int i = 0; i < moves.size(); i++)
		{
			int move = moves[i];
			int source = GetSourceFromMove(move);
			int dest = GetDestFromMove(move);
			if (sr == GetRow(source) && sc == GetCol(source) && er == GetRow(dest) && ec == GetCol(dest))
			{
				return true;
			}
		}
		return false;
	}

	void ChessEngine::setStartPosition()
	{
		board->SetFEN(Board::startPosition);
	}
	
	void ChessEngine::setPosition(std::string& fen)
	{
		board->SetFEN(fen);
	}
}