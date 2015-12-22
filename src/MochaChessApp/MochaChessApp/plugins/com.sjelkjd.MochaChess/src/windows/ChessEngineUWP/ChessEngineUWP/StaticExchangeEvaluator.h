#ifndef STATICEXCHANGEEVALUATOR_H
#define STATICEXCHANGEEVALUATOR_H

#include "Types.h"
#include "Board.h"

namespace ChessEngineUWP
{

	class StaticExchangeEvaluator
	{
	private:
		Board& m_board;
		ulong attackerPieces[6];
		ulong defenderPieces[6];
		int scores[32];
		int pieces[32];
		int squares[32];
		int targetSquare;
		int attackerColor;
		int defenderColor;
		ulong knownPieces;
		ulong attackerBits;
		ulong defenderBits;
		
		void AddDiagonalAttacks();
		void AddSingleAttacks();
		void AddRankFileAttacks();
		void AddPiece(ulong attackBoard, ulong pieceArray[], int pieceType);

	public: 
		StaticExchangeEvaluator(Board& board) :
		  m_board(board)
		  {

		  }

		int Evaluate(int move);		
	};

}

#endif