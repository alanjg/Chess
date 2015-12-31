#include "pch.h"
#include "StaticExchangeEvaluator.h"
#include "Evaluation.h"
#include "MoveGenerator.h"
using namespace std;

namespace SjelkjdChessEngine
{		
	// is always positive for good captures, 0 for neutral captures, negative for bad captures.
	int StaticExchangeEvaluator::Evaluate(int move)
	{
		for (int i = 0; i < 6; i++)
		{
			attackerPieces[i] = 0;
			defenderPieces[i] = 0;
		}
		int source = m_board.GetPiece(GetSourceFromMove(move));
		int destPiece = m_board.GetPiece(GetDestFromMove(move));
		int attackingPiece = Pieces::GetPieceTypeFromPiece(source);
		int defendingPiece = Pieces::GetPieceTypeFromPiece(destPiece);
	//	if(attackingPiece == PieceTypes::Pawn && defendingPiece != PieceTypes::Pawn)
		{
	//		return Evaluation::PieceValue(defendingPiece) - 100;
		}
		attackerColor = Pieces::GetColorFromPiece(source);
		targetSquare = GetDestFromMove(move);
		defenderColor = OtherColor(attackerColor);

		attackerBits = m_board.colorBitBoards[attackerColor];
		defenderBits = m_board.colorBitBoards[defenderColor];
		
		knownPieces = 1ULL << GetSourceFromMove(move);
		int currentScore = Evaluation::PieceValue(Pieces::GetPieceTypeFromPiece(destPiece));

		m_board.RemovePiece(GetSourceFromMove(move), source);

		AddSingleAttacks();
		AddRankFileAttacks();
		AddDiagonalAttacks();

		pieces[0] = source;
		squares[0] = GetSourceFromMove(move);

		// trade off pieces
		int attackerIndex = 0;
		int defenderIndex = 0;
		int scoreIndex = 0;

		for (; ; )
		{
			scores[scoreIndex++] = currentScore;
			// break if we took the king
			if (currentScore > 25000) break;

			if (defenderIndex == 6) break;
			while (defenderPieces[defenderIndex] == 0)
			{
				defenderIndex++;
				if (defenderIndex == 6) break;
			}
			if (defenderIndex == 6) break;

			int nextSquare = PopLowestSetBit(defenderPieces[defenderIndex]);

			currentScore -= Evaluation::PieceValue(Pieces::GetPieceTypeFromPiece(attackingPiece));
			attackingPiece = defenderIndex;
			int piece = Pieces::GetPiece(attackingPiece, defenderColor);
			m_board.RemovePiece(nextSquare, piece);
			pieces[scoreIndex] = piece;
			squares[scoreIndex] = nextSquare;

			// handle revealed attacks
			if(defenderIndex == PieceTypes::Pawn || defenderIndex == PieceTypes::Bishop || defenderIndex == PieceTypes::Queen)
			{
				AddDiagonalAttacks();
				defenderIndex = min(defenderIndex, PieceTypes::Bishop);
			}
			if(defenderIndex == PieceTypes::Rook || defenderIndex == PieceTypes::Queen)
			{
				AddRankFileAttacks();
				defenderIndex = min(defenderIndex, PieceTypes::Rook);
			}

			scores[scoreIndex++] = currentScore;
			// break if we took the king
			if (currentScore < -25000) break;

			if (attackerIndex == 6) break;
			while (attackerPieces[attackerIndex] == 0)
			{
				attackerIndex++;
				if (attackerIndex == 6) break;
			}
			if (attackerIndex == 6) break;

			nextSquare = PopLowestSetBit(attackerPieces[attackerIndex]);
			currentScore += Evaluation::PieceValue(Pieces::GetPieceTypeFromPiece(attackingPiece));
			attackingPiece = attackerIndex;

			piece = Pieces::GetPiece(attackingPiece, attackerColor);
			m_board.RemovePiece(nextSquare, piece);
			pieces[scoreIndex] = piece;
			squares[scoreIndex] = nextSquare;

			// handle revealed attacks
			if (attackerIndex == PieceTypes::Pawn || attackerIndex == PieceTypes::Bishop || attackerIndex == PieceTypes::Queen)
			{
				AddDiagonalAttacks();
				attackerIndex = min(attackerIndex, PieceTypes::Bishop);
			}
			if (attackerIndex == PieceTypes::Rook || attackerIndex == PieceTypes::Queen)
			{
				AddRankFileAttacks();
				attackerIndex = min(attackerIndex, PieceTypes::Rook);
			}
		}

		int bestScore = scores[scoreIndex - 1];
		while (--scoreIndex >= 0)
		{
			m_board.AddPiece(squares[scoreIndex], pieces[scoreIndex]);
			if ((scoreIndex & 1) == 0)
			{
				// This is my move, opponent will try to minimize
				bestScore = min(bestScore, scores[scoreIndex]);
			}
			else
			{
				// Opponents move, try to maximize
				bestScore = max(bestScore, scores[scoreIndex]);
			}
		}
		return bestScore;
	}

	void StaticExchangeEvaluator::AddDiagonalAttacks()
	{
		ulong diagAttackMask = MoveGenerator::diagA1H8Attacks[targetSquare][m_board.GetDiagA1H8Status(targetSquare)] | 
			MoveGenerator::diagA8H1Attacks[targetSquare][m_board.GetDiagA8H1Status(targetSquare)];
		
		ulong bishops = m_board.pieceBitBoards[PieceTypes::Bishop];
		AddPiece((diagAttackMask & bishops & attackerBits), attackerPieces, PieceTypes::Bishop);
		AddPiece((diagAttackMask & bishops & defenderBits), defenderPieces, PieceTypes::Bishop);

		ulong queens = m_board.pieceBitBoards[PieceTypes::Queen];
		AddPiece((diagAttackMask & queens & attackerBits), attackerPieces, PieceTypes::Queen);
		AddPiece((diagAttackMask & queens & defenderBits), defenderPieces, PieceTypes::Queen);
	}

	void StaticExchangeEvaluator::AddSingleAttacks()
	{
		ulong pawns = m_board.pieceBitBoards[PieceTypes::Pawn];
		AddPiece(MoveGenerator::pawnAttacks[targetSquare][attackerColor] & pawns & attackerBits, attackerPieces, PieceTypes::Pawn);
		AddPiece(MoveGenerator::pawnAttacks[targetSquare][defenderColor] & pawns & defenderBits, defenderPieces, PieceTypes::Pawn);

		ulong knights = m_board.pieceBitBoards[PieceTypes::Knight];
		AddPiece(MoveGenerator::knightAttacks[targetSquare] & knights & attackerBits, attackerPieces, PieceTypes::Knight);
		AddPiece(MoveGenerator::knightAttacks[targetSquare] & knights & defenderBits, defenderPieces, PieceTypes::Knight);

		ulong kings = m_board.pieceBitBoards[PieceTypes::King];
		AddPiece(MoveGenerator::kingAttacks[targetSquare] & kings & attackerBits, attackerPieces, PieceTypes::King);
		AddPiece(MoveGenerator::kingAttacks[targetSquare] & kings & defenderBits, defenderPieces, PieceTypes::King);
	}

	void StaticExchangeEvaluator::AddRankFileAttacks()
	{
		ulong rowFileAttackMask = MoveGenerator::rankAttacks[targetSquare][m_board.GetRankStatus(targetSquare)] | 
			MoveGenerator::fileAttacks[targetSquare][m_board.GetFileStatus(targetSquare)];

		ulong rooks = m_board.pieceBitBoards[PieceTypes::Rook];
		AddPiece((rowFileAttackMask & rooks & attackerBits), attackerPieces, PieceTypes::Rook);
		AddPiece((rowFileAttackMask & rooks & defenderBits), defenderPieces, PieceTypes::Rook);

		ulong queens = m_board.pieceBitBoards[PieceTypes::Queen];
		AddPiece((rowFileAttackMask & queens & attackerBits), attackerPieces, PieceTypes::Queen);
		AddPiece((rowFileAttackMask & queens & defenderBits), defenderPieces, PieceTypes::Queen);
	}

	void StaticExchangeEvaluator::AddPiece(ulong attackBoard, ulong pieceArray[], int pieceType)
	{
		ulong newPieces = attackBoard & ~knownPieces;
		knownPieces |= newPieces;
		pieceArray[pieceType] |= newPieces;
	}
}