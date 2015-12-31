#ifndef EVALUATION_H
#define EVALUATION_H

#include "Types.h"
namespace SjelkjdChessEngine
{
	class Board;
	class Evaluation
	{
		friend void TestPawnValues();
	private:
		static ulong qpawn[2];
		static ulong kpawn[2];
		static ulong qking[2];
		static ulong kking[2];

		static ulong pawnShield[2][64];
		static ulong blockers[2][64];
		static ulong rearDefense[2][64];
		
		static int kingSafetyFunction[];
		static int maxKingSafetyFunction;

		static int pieceValues[];
		static int endgamePawnSquareValues[2][64];
		
		static int pieceSquareValues[16][64];		
		static int endgamePieceSquareValues[16][64];

		static const int passedPawnScore = 15;
		static const int outpostScore = 4;
		static const int undefendedPawnScore = -5;
		static const int doublePawnScore = -8;

		// per square
		static const int advanceScore = 2;

		static ulong passedPawnMasks[64][2];
		static ulong outpostPawnMasks[64][2][2];
		static ulong defendedPawnMasks[64][2];
		static ulong doubledPawnMasks[64][2];

		static void InitializePawnMasks();
		static void InitializeBlockers();
		static void GeneratePieceSquareValues();
		
		static void InitializePassedMask(int color);
		
		static void InitializeOutpostMask(int color);
		static void InitializeDefendedPawnMasks(int color);
		static void InitializeDoubledPawnMasks(int color);
		
		static void GeneratePawnValues(int color);
		static void GenerateKnightValues(int color);
		static void GenerateBishopValues(int color);
		static void GenerateRookValues(int color);
		static void GenerateQueenValues(int color);
		static void GenerateKingValues(int color);

		static int pawnRankValue[];
		static int mobilityMultiplier[];
		static int attackCountMultiplier[]; 
	 
	
		Board& m_board;
		int kingSafety;
		
		ulong whiteMobility, blackMobility;
		int pieceSquareScore, materialScore;
	
		ulong whiteNonKingMobility;
		ulong blackNonKingMobility;
		int whiteKingAttackCount;
		int blackKingAttackCount;

		int TotalKingMobility(int color);
		int CalculateMobility();
		int CalculateMobility(int color);
		
		int CalculatePawnScore();
		int CalculatePawnScore(int color);

		
		int GetPawnScore();
		int CalculateKingSafety();
		int CalculateKingSafety(int color);
	public:
		Evaluation(Board& board):
		  m_board(board)
		  {
		  }
		
		static void Initialize()
		{
			GeneratePieceSquareValues();
			InitializePawnMasks();
			InitializeBlockers();
		}

		static int PieceValue(int piece)
		{
			return pieceValues[piece];
		}
	
		static int PieceSquareValue(int piece, int square)
		{
			return pieceSquareValues[piece][square];
		}

		static int EndgamePieceSquareValue(int piece, int square)
		{
			return endgamePieceSquareValues[piece][square];
		}

		int GetScore();
		int GetMoveAttackValue(int move);
		bool IsDrawByMaterial();
		static bool IsDrawByMaterial(const Board& board);
	};
}
#endif