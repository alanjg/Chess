#include "pch.h"
#include "ChessEngine.h"
#include "Types.h"

namespace ChessEngineUWP
{
	ChessEngine::ChessEngine()
	{
		Board* b = new Board();
		board.reset(b);
		search.reset(new Search(*b));
		moveGenerator.reset(new MoveGenerator(*b));
	}

	void ChessEngine::makeMove(int sr, int sc, int er, int ec)
	{
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		int foundMove = NullMove;
		for each(int move in moves)
		{
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

	Platform::String^ ChessEngine::getPiece(int row, int col)
	{
		int piece = board->GetPiece(Square(row, col));
		std::wstring p;
		switch (piece)
		{
		case Pieces::BlackBishop: p = L"b"; break;
		case Pieces::BlackKing: p = L"k"; break;
		case Pieces::BlackKnight: p = L"n"; break;
		case Pieces::BlackPawn: p = L"p"; break;
		case Pieces::BlackQueen: p = L"q"; break;
		case Pieces::BlackRook: p = L"r"; break;
		case Pieces::None: p = L""; break;
		case Pieces::WhiteBishop: p = L"B"; break;
		case Pieces::WhiteKing: p = L"K"; break;
		case Pieces::WhiteKnight: p = L"N"; break;
		case Pieces::WhitePawn: p = L"P"; break;
		case Pieces::WhiteQueen: p = L"Q"; break;
		case Pieces::WhiteRook: p = L"R"; break;
		}
		return ref new Platform::String(p.c_str());
	}
    
    Platform::String^ ChessEngine::getBoard()
	{
		std::string b = board->ToString();
		std::wstring pb(b.begin(), b.end());
		return ref new Platform::String(pb.c_str());
	}

	void ChessEngine::makeComputerMove()
	{
		int move = search->GetBestMove(1.0);
		board->MakeMove(move, true);
	}

	bool ChessEngine::isValidMoveStart(int row, int col)
	{
		vector<int> moves;
		moveGenerator->GenerateAllMoves(moves);
		for each(int move in moves)
		{
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
		for each(int move in moves)
		{
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
	
	void ChessEngine::setPosition(Platform::String^ fen)
	{
		std::wstring wf(fen->Data());
		std::string f(wf.begin(), wf.end());
		board->SetFEN(f);
	}
}