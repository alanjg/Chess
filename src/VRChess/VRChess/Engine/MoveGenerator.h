#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "Board.h"

namespace SjelkjdChessEngine
{
	class MoveGenerator
	{		
	public:
		class CastleMove
		{
		public:
			CastleMove(){}
			CastleMove(int kingOrigin, int rookOrigin, int kingDestination, int rookDestination)
			{
				KingOrigin = kingOrigin;
				KingDestination = kingDestination;
				RookOrigin = rookOrigin;
				RookDestination = rookDestination;
			}
			int KingOrigin;
			int RookOrigin;
			int KingDestination;
			int RookDestination;
		};
		static CastleMove castleMoves[4];
	
		static int whiteKingOrigin;
		static int whiteKingsideRook;
		static int whiteKingsideKingCastle;
		static int whiteKingsideRookCastle;
		static int whiteQueensideRook;
		static int whiteQueensideKingCastle;
		static int whiteQueensideRookCastle;
		static int blackKingOrigin;
		static int blackKingsideRook;
		static int blackKingsideKingCastle;
		static int blackKingsideRookCastle;
		static int blackQueensideRook;
		static int blackQueensideKingCastle;
		static int blackQueensideRookCastle;

		static const int whiteKingsideCastleSquares = 5;
		static const int whiteQueensideCastleSquares = 3;
		static const int blackKingsideCastleSquares = 7*8+5;
		static const int blackQueensideCastleSquares = 7*8+3;
		static ulong whiteQueensideCastleBitBoard;
		static ulong whiteQueensideCastleKingBitBoard;
		static ulong whiteKingsideCastleBitBoard;
		static ulong whiteKingsideCastleKingBitBoard;
		static ulong blackQueensideCastleBitBoard;
		static ulong blackQueensideCastleKingBitBoard;
		static ulong blackKingsideCastleBitBoard;
		static ulong blackKingsideCastleKingBitBoard;

		static ulong whitePawnBitBoard;
		static ulong blackPawnBitBoard;

private:
		Board& m_board;
		
		static int bdr[];
		static int bdc[];

		static int rdr[];
		static int rdc[];

		static int qdr[];
		static int qdc[];

		static int ndr[];
		static int ndc[];

		static void GenerateRankAttackBitmasks();
		static void GenerateFileAttackBitmasks();
		static void GenerateDiagA1H8AttackBitmasks();
		static void GenerateDiagA8H1AttackBitmasks();
		static void GenerateKnightAttackBitmasks();
		static void GenerateKingAttackBitmasks();
		static void GeneratePawnAttackBitmasks();
		
		static void GenerateRankPinBitmasks();
		static void GenerateFilePinBitmasks();
		static void GenerateDiagA1H8PinBitmasks();
		static void GenerateDiagA8H1PinBitmasks();
		static void GenerateDiagonalMasks();
		static void GenerateIntermediateSquares();

		void AddMoves(int pieceType, int colr, ulong targets, std::vector<int>& currentMoves);
		void AddPawnMoves(bool queiscentMovesOnly, int toMove, ulong captureTargets, ulong moveTargets, std::vector<int>& currentMoves);
		void AddCastleMoves(int toMove, std::vector<int>& currentMoves);

	public:
		static ulong rankAttacks[64][64];
		static ulong fileAttacks[64][64];
		static ulong diagA1H8Attacks[64][64];
		static ulong diagA8H1Attacks[64][64];
		static ulong knightAttacks[64];
		static ulong kingAttacks[64];
		static ulong pawnAttacks[64][2];

		static ulong diagA1H8Masks[64];
		static ulong diagA8H1Masks[64];

		static ulong intermediateSquares[64][64];
		MoveGenerator(Board& board) : m_board(board)
		{
			static bool initialized = false;
			if(!initialized)
			{
				initialized = true;
				MoveGenerator::castleMoves[0] = CastleMove(whiteKingOrigin, whiteKingsideRook, whiteKingsideKingCastle, whiteKingsideRookCastle);
				MoveGenerator::castleMoves[1] = CastleMove(whiteKingOrigin, whiteQueensideRook, whiteQueensideKingCastle, whiteQueensideRookCastle);
				MoveGenerator::castleMoves[2] = CastleMove(blackKingOrigin, blackKingsideRook, blackKingsideKingCastle, blackKingsideRookCastle);
				MoveGenerator::castleMoves[3] = CastleMove(blackKingOrigin, blackQueensideRook, blackQueensideKingCastle, blackQueensideRookCastle);

				GenerateRankAttackBitmasks();
				GenerateFileAttackBitmasks();
				GenerateDiagA1H8AttackBitmasks();
				GenerateDiagA8H1AttackBitmasks();
				GenerateKnightAttackBitmasks();
				GenerateKingAttackBitmasks();
				GeneratePawnAttackBitmasks();

				GenerateDiagonalMasks();
				GenerateIntermediateSquares();
			}
		}

		void GenerateAllMoves(std::vector<int>& moves)
		{
			GenerateMoves(false, m_board.Turn(), moves);
		}

		void GenerateQuiescentMoves(std::vector<int>& moves)
		{
			GenerateMoves(true, m_board.Turn(), moves);
		}
		void GenerateCheckEscapes(bool quiescentMovesOnly, int toMove, std::vector<int>& moves);
		bool IsValidMove(int move);
	private:
		void GenerateMoves(bool quiescentMovesOnly, int currentMove, std::vector<int>& moves);
		void ProcessPawnCaptures(ulong captureMask, int toMove, ulong enemyPawns, std::vector<int>& currentMoves, bool quiescentMovesOnly, int direction);
	};

	
	void AddPawnPromotions(int move, std::vector<int>& moves, bool quiescentMovesOnly);
	ulong MakePawnMoveBitBoard(ulong position, int color);
	int MakePawnCaptureMove(int source, int color, int direction);
	ulong MakeBitboard(int row, int col);
	int MakePawnMove(int position, int color);
}

#endif