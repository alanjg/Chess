#include "StdAfx.h"
#include "Evaluation.h"
#include "Types.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "TranspositionTable.h"

using namespace std;
using namespace stdext;
namespace SjelkjdChessEngine
{
	int Evaluation::pieceValues[] = { 100, 300, 300, 500, 900, 50000, -100, -300, -300, -500, -900, -50000, 0 };
	int Evaluation::pawnRankValue[] = { 0, 0, 2, 4, 5, 8, 10, 0 };
	int	Evaluation::kingSafetyFunction[] = 
			{   0,  2,  3,  6, 12, 18, 25, 37, 50, 75,
              100,125,150,175,200,225,250,275,300,325,
              350,375,400,425,450,475,500,525,550,575, 
              600,600,600,600,600 };

	ulong Evaluation::qpawn[] = {1ULL << (1*8+0) | 1ULL << (1*8+1) | 1ULL << (1*8+2) | 1ULL << (2*8+0) | 1ULL << (2*8+1) | 1ULL << (2*8+2),
		1ULL << (6*8+0) | 1ULL << (6*8+1) | 1ULL << (6*8+2) | 1ULL << (5*8+0) | 1ULL << (5*8+1) | 1ULL << (5*8+2)};
	ulong Evaluation::kpawn[] = {1ULL << (1*8+5) | 1ULL << (1*8+6) | 1ULL << (1*8+7) | 1ULL << (2*8+5) | 1ULL << (2*8+6) | 1ULL << (2*8+7),
		1ULL << (6*8+5) | 1ULL << (6*8+6) | 1ULL << (6*8+7) | 1ULL << (5*8+5) | 1ULL << (5*8+6) | 1ULL << (5*8+7)};
	ulong Evaluation::qking[] = {1ULL << (0*8+0) | 1ULL << (0*8+1) | 1ULL << (0*8+2) | 1ULL << (1*8+0) | 1ULL << (1*8+1) | 1ULL << (1*8+2),
		1ULL << (7*8+0) | 1ULL << (7*8+1) | 1ULL << (7*8+2) | 1ULL << (6*8+0) | 1ULL << (6*8+1) | 1ULL << (6*8+2)};
	ulong Evaluation::kking[] = {1ULL << (0*8+6) | 1ULL << (0*8+7) | 1ULL << (1*8+6) | 1ULL << (1*8+7),
		1ULL << (7*8+6) | 1ULL << (7*8+7) | 1ULL << (6*8+6) | 1ULL << (6*8+7)};

	int Evaluation::maxKingSafetyFunction = sizeof(kingSafetyFunction) / sizeof(int);
	ulong Evaluation::passedPawnMasks[64][2];
	ulong Evaluation::outpostPawnMasks[64][2][2];
	ulong Evaluation::defendedPawnMasks[64][2];
	ulong Evaluation::doubledPawnMasks[64][2];
	int Evaluation::pieceSquareValues[16][64];
	int Evaluation::endgamePieceSquareValues[16][64];

	ulong Evaluation::pawnShield[2][64];
	
	ulong Evaluation::blockers[2][64];
	
	ulong Evaluation::rearDefense[2][64];
	
	void Evaluation::GeneratePieceSquareValues()
	{
		GeneratePawnValues(Colors::White);
		GeneratePawnValues(Colors::Black);
	
		GenerateBishopValues(Colors::White);
		GenerateBishopValues(Colors::Black);
		GenerateKnightValues(Colors::White);
		GenerateKnightValues(Colors::Black);
		GenerateRookValues(Colors::White);
		GenerateRookValues(Colors::Black);
		GenerateQueenValues(Colors::White);
		GenerateQueenValues(Colors::Black);
		GenerateKingValues(Colors::White);
		GenerateKingValues(Colors::Black);
	}

	void Evaluation::InitializePawnMasks()
	{
		InitializePassedMask(Colors::White);
		InitializePassedMask(Colors::Black);

		InitializeOutpostMask(Colors::White);
		InitializeOutpostMask(Colors::Black);

		InitializeDoubledPawnMasks(Colors::White);
		InitializeDoubledPawnMasks(Colors::Black);

		InitializeDefendedPawnMasks(Colors::White);
		InitializeDefendedPawnMasks(Colors::Black);
	}

	void Evaluation::InitializePassedMask(int color)
	{
		int dRow = color == Colors::White ? 1 : -1;
		int homeRow = color == Colors::White ? 1 : 6;
		int lastRow = color == Colors::White ? 7 : 0;

		int colorIndex = color == Colors::White ? 0 : 1;
		for (int i = 0; i < 64; i++)
		{
			int square(i);
			int srow = GetRow(square);
			int scol = GetCol(square);
			if (srow != 0 && srow != 7)
			{
				// is this a passed pawn?
				ulong passMask = 0;
				for (int row = srow + dRow; row != lastRow; row += dRow)
				{
					for (int col = max(0, scol - 1); col <= min(7, scol + 1); col++)
					{
						passMask |= 1ULL << Square(row, col);
					}
				}
				passedPawnMasks[i][colorIndex] = passMask;
			}
		}
	}

	void Evaluation::InitializeOutpostMask(int color)
	{
		int dRow = color == Colors::White ? 1 : -1;
		int homeRow = color == Colors::White ? 1 : 6;
		int lastRow = color == Colors::White ? 7 : 0;

		int colorIndex = color == Colors::White ? 0 : 1;
		
		for (int i = 0; i < 64; i++)
		{
			ulong leftMask = 0;
			ulong rightMask = 0;
			int square(i);
			int srow = GetRow(square);
			int scol = GetCol(square);
			if (srow != 0 && srow != 7 && srow + dRow != lastRow)
			{
				for (int row = srow + 2 * dRow; row != lastRow; row += dRow)
				{
					int col = scol - 2;
					if (col >= 0)
					{
						leftMask |= 1ULL << Square(row, col);
					}
					int endcol = scol + 2;
					if (endcol <= 7)
					{
						rightMask |= 1ULL << Square(row, endcol);
					}
					
					leftMask |= 1ULL << Square(row, scol);
					rightMask |= 1ULL << Square(row, scol);
				}
			}
			outpostPawnMasks[i][colorIndex][0] = leftMask;
			outpostPawnMasks[i][colorIndex][1] = rightMask;
		}
	}

	void Evaluation::InitializeDefendedPawnMasks(int color)
	{
		int dRow = color == Colors::White ? 1 : -1;
		int homeRow = color == Colors::White ? 1 : 6;
		int lastRow = color == Colors::White ? 7 : 0;

		int colorIndex = color == Colors::White ? 0 : 1;
		
		for (int i = 0; i < 64; i++)
		{
			int square(i);
			int srow = GetRow(square);
			int scol = GetCol(square);
			if (srow != 0 && srow != 7)
			{
				if (srow != homeRow)
				{
					ulong defenseMask = 0;
					if (scol > 0)
					{
						defenseMask |= 1ULL << Square(srow - dRow, scol - 1);
					}
					if (scol < 7)
					{
						defenseMask |= 1ULL << Square(srow - dRow, scol + 1);
					}

					defendedPawnMasks[i][colorIndex] = defenseMask;
				}
			}
		}
	}

	void Evaluation::InitializeDoubledPawnMasks(int color)
	{
		int dRow = color == Colors::White ? 1 : -1;
		int homeRow = color == Colors::White ? 1 : 6;
		int lastRow = color == Colors::White ? 7 : 0;

		int colorIndex = color == Colors::White ? 0 : 1;

		for (int i = 0; i < 64; i++)
		{
			int square(i);
			int srow = GetRow(square);
			int scol = GetCol(square);
			if (srow != 0 && srow != 7)
			{
				ulong doubledMask = 0;
				for (int row = homeRow; row != lastRow; row += dRow)
				{
					doubledMask |= 1ULL << Square(row, scol);
				}
				doubledPawnMasks[i][colorIndex] = doubledMask;
			}
		}
	}

	int Evaluation::CalculatePawnScore()
	{
		return CalculatePawnScore(Colors::White) - CalculatePawnScore(Colors::Black);
	}

	int Evaluation::CalculatePawnScore(int color)
	{
		int colorIndex = color == Colors::White ? 0 : 1;
		
		int otherColor = OtherColor(color);
		int dRow = color == Colors::White ? 1 : -1;
		int homeRow = color == Colors::White ? 1 : 6;
		int lastRow = color == Colors::White ? 7 : 0;

		int score = 0;

		int pawn = Pieces::GetPiece(PieceTypes::Pawn, color);	
		int otherPawn = Pieces::GetPiece(PieceTypes::Pawn, otherColor);

		ulong pawns = m_board.GetPieceBitBoard(pawn);
		ulong otherPawns = m_board.GetPieceBitBoard(otherPawn);

		ulong pawnIteration = pawns;
		while (pawnIteration != 0)
		{
			int i = PopLowestSetBit(pawnIteration);

			int square(i);
			int srow = GetRow(square);
			int scol = GetCol(square);

			// is this an outpost?
			if (srow + dRow != lastRow)
			{
				// left outpost
				if (scol > 0 && (outpostPawnMasks[i][colorIndex][0] & otherPawns) == 0)
				{
					score += outpostScore;
				}
				// right outpost
				if(scol < 7 && (outpostPawnMasks[i][colorIndex][1] & otherPawns) == 0)
				{
					score += outpostScore;
				}
			}

			// is this undefended and off the home row?
			if (srow != homeRow)
			{
				if ((defendedPawnMasks[i][colorIndex] & pawns) == 0)
				{
					score += undefendedPawnScore;
				}
			}

			// is this pawn doubled?
			int pawnCount = BitCount(doubledPawnMasks[i][colorIndex] & pawns);
			if (pawnCount > 1)
			{
				score += (pawnCount - 1) * doublePawnScore;
			}

			// score the advance
			int distance = abs(srow - homeRow);
			score += distance * advanceScore;

			// is this a passed pawn?
			if ((passedPawnMasks[i][colorIndex] & otherPawns) == 0)
			{
				score += passedPawnScore;
				score += distance * passedPawnAdvanceScore;
			}
		}			

		return score;
	}

	int Evaluation::CalculatePawnPushScore()
	{
		return CalculatePawnPushScore(Colors::White) - CalculatePawnPushScore(Colors::Black);
	}

	int Evaluation::CalculatePawnPushScore(int color)
	{
		ulong me = m_board.GetColorBitBoard(color);
		int currentMaterial = 3 * BitCount(me & (m_board.GetPieceTypeBitBoard(PieceTypes::Knight) | m_board.GetPieceTypeBitBoard(PieceTypes::Bishop))) +
			5 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Rook) & me) +
			9 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Queen) & me);
		if (me > 3)
		{
			return 0;
		}
		
		int homeRow = color == Colors::White ? 1 : 6;

		int score = 0;

		int pawn = Pieces::GetPiece(PieceTypes::Pawn, color);

		ulong pawns = m_board.GetPieceBitBoard(pawn);

		ulong pawnIteration = pawns;
		while (pawnIteration != 0)
		{
			int i = PopLowestSetBit(pawnIteration);

			int square(i);
			int srow = GetRow(square);

			// score the advance
			int distance = abs(srow - homeRow);
			score += distance * 50;
		}

		return score;
	}

	void Evaluation::GeneratePawnValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			int advance = color == Colors::White ? row : 7 - row;
			if (col > 3)
			{
				col = 7 - col;
			}
			
			// give more value to center pawns
			int center = col;
			value = advance + advance * center;
			value += pawnRankValue[advance];
			if (color == Colors::Black)
			{
				value *= -1;	
			}
			pieceSquareValues[6 * color + PieceTypes::Pawn][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::Pawn][i] = value;			
		}
		pieceSquareValues[Pieces::WhitePawn][27] = 20;
		pieceSquareValues[Pieces::WhitePawn][28] = 20;
		pieceSquareValues[Pieces::WhitePawn][35] = 25;
		pieceSquareValues[Pieces::WhitePawn][36] = 25;

		pieceSquareValues[Pieces::BlackPawn][27] = -25;
		pieceSquareValues[Pieces::BlackPawn][28] = -25;
		pieceSquareValues[Pieces::BlackPawn][35] = -20;
		pieceSquareValues[Pieces::BlackPawn][36] = -20;
	}

	void Evaluation::GenerateKnightValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			if(row > 3) row = 7-row;
			if(col > 3) col = 7-col;

			// 0 to 6, higher is closer to center
			int distance = 6 - ((3 - row) + (3 - col));

			// [0,12]
			value = distance * 2;
			if (color == Colors::Black)
			{
				value *= -1;
			}
			pieceSquareValues[6 * color + PieceTypes::Knight][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::Knight][i] = value;
		}

		// discourage camping
		pieceSquareValues[Pieces::WhiteKnight][1] = -40;
		endgamePieceSquareValues[Pieces::WhiteKnight][1] = -40;
		pieceSquareValues[Pieces::WhiteKnight][6] = -40;
		endgamePieceSquareValues[Pieces::WhiteKnight][6] = -40;

		pieceSquareValues[Pieces::BlackKnight][57] = 40;
		endgamePieceSquareValues[Pieces::BlackKnight][57] = 40;
		pieceSquareValues[Pieces::BlackKnight][62] = 40;
		endgamePieceSquareValues[Pieces::BlackKnight][62] = 40;
	}

	void Evaluation::GenerateBishopValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			int diff = min(abs(row - col), abs(7 - (row - col)));
			
			// [0,12]
			value = 4 * (3 - diff);
			if (color == Colors::White && row == 0)
			{
				value = 0;
			}
			else if (color == Colors::Black && row == 7)
			{
				value = 0;
			}

			if (color == Colors::Black)
			{
				value *= -1;
			}
			pieceSquareValues[6 * color + PieceTypes::Bishop][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::Bishop][i] = value;
		}
		// discourage camping
		pieceSquareValues[Pieces::WhiteBishop][2] = -40;
		endgamePieceSquareValues[Pieces::WhiteBishop][2] = -40;
		pieceSquareValues[Pieces::WhiteBishop][5] = -40;
		endgamePieceSquareValues[Pieces::WhiteBishop][5] = -40;

		pieceSquareValues[Pieces::BlackBishop][58] = 40;
		endgamePieceSquareValues[Pieces::BlackBishop][58] = 40;
		pieceSquareValues[Pieces::BlackBishop][61] = 40;
		endgamePieceSquareValues[Pieces::BlackBishop][61] = 40;
	}

	void Evaluation::GenerateRookValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			if (color == Colors::White)
			{
				if (row == 6)
				{
					value = 5;
				}
				else if (row == 7)
				{
					value = 3;
				}
			}
			else if (color == Colors::Black)
			{
				if (row == 1)
				{
					value = 5;
				}
				else if (row == 0)
				{
					value = 3;
				}
			}

			if (color == Colors::Black)
			{
				value *= -1;
			}
			pieceSquareValues[6 * color + PieceTypes::Rook][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::Rook][i] = value;
		}
	}

	void Evaluation::GenerateQueenValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			//center of the board
			if ((row == 3 || row == 4) && (col == 3 || col == 4))
			{
				value = 10;
			} // main diagonals.
			else if (row == col || 7 - row == col)
			{
				value = 8;
			} // edge of the board
			else if ((row == 0 || row == 7) && (col == 0 || col == 7))
			{
				value = 0;
			}
			else
			{
				value = 6;
			}
			if (color == Colors::Black)
			{
				value *= -1;
			}
			pieceSquareValues[6 * color + PieceTypes::Queen][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::Queen][i] = value;
		}
	}

	void Evaluation::GenerateKingValues(int color)
	{
		for (int i = 0; i < 64; i++)
		{
			int value = 0;
			int row = i / 8;
			int col = i % 8;

			int advance = color == Colors::White ? row : 7 - row;
		
			int dr = min(abs(row - 4), abs(row - 3));
			int dc = min(abs(col - 4), abs(col - 3));
			int endgameValue = 40 - (dr + dc) * 10;
			value = -8 * advance;
			if (color == Colors::Black)
			{
				value *= -1;
				endgameValue *= -1;
			}
			pieceSquareValues[6 * color + PieceTypes::King][i] = value;
			endgamePieceSquareValues[6 * color + PieceTypes::King][i] = endgameValue;
		}
	}

	int Evaluation::GetScore()
	{
		if (IsDrawByMaterial()) return 0;

		// game phase: 1 is middle game, 0 is endgame
		int currentMaterial =	1 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Pawn)) +
								3 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Knight) | m_board.GetPieceTypeBitBoard(PieceTypes::Bishop)) + 
								5 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Rook)) + 
								9 * BitCount(m_board.GetPieceTypeBitBoard(PieceTypes::Queen));

		const int startingMaterial = 16 + 24 + 20 + 18;
		const int endgameMaterial = 26;
		if(currentMaterial < endgameMaterial)
		{
			currentMaterial = endgameMaterial;
		}

		int phase = currentMaterial - endgameMaterial;
		int total = startingMaterial - endgameMaterial;

		// 1 = midgame, 0 = endgame
		double lerp = phase / (double)total;

		int pawn = GetPawnScore();
		int pawnPush = CalculatePawnPushScore();

		// must call mobility before king safety.
		int mobility = CalculateMobility();
		int kingSafety = int(CalculateKingSafety() * lerp);
		int positional = int(m_board.GetPositionalScore() * lerp);
		int positionalEndgame = int(m_board.GetEndgamePositionalScore() * (1 - lerp));
		int material = m_board.GetMaterialScore();
		if (BitCount(m_board.GetPieceBitBoard(Pieces::WhiteBishop)) < 2)
		{
			material -= 50;
		}
		if (BitCount(m_board.GetPieceBitBoard(Pieces::BlackBishop)) < 2)
		{
			material += 50;
		}

		return pawn + pawnPush + mobility + kingSafety + positional + positionalEndgame + material;
	}

	void Evaluation::InitializeBlockers()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int col = i % 8;
			
			//pawn shields are two in front of the king
			ulong mask = 0;
			if (row + 2 < 8)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row + 2, col - 1);
				}
				mask |= 1ULL << Square(row + 2, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row + 2, col + 1);
				}
			}
			pawnShield[Colors::White][i] = mask;

			mask = 0;
			if (row - 2 >= 0)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row - 2, col - 1);
				}
				mask |= 1ULL << Square(row - 2, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row - 2, col + 1);
				}
			}
			pawnShield[Colors::Black][i] = mask;

			//blockers are one row in front of the king
			mask = 0;
			if (row + 1 < 8)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row + 1, col - 1);
				}
				mask |= 1ULL << Square(row + 1, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row + 1, col + 1);
				}
			}
			blockers[Colors::White][i] = mask;

			mask = 0;
			if (row - 1 >= 0)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row - 1, col - 1);
				}
				mask |= 1ULL << Square(row - 1, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row - 1, col + 1);
				}
			}
			blockers[Colors::Black][i] = mask;

			//rear defense is one row behind the king
			mask = 0;
			if (row - 1 >= 0)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row - 1, col - 1);
				}
				mask |= 1ULL << Square(row - 1, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row - 1, col + 1);
				}
			}
			if (col > 0)
			{
				mask |= 1ULL << Square(row, col - 1);
			}
			if (col + 1 < 8)
			{
				mask |= 1ULL << Square(row, col + 1);
			}
			rearDefense[Colors::White][i] = mask;

			mask = 0;
			if (row + 1 < 8)
			{
				if (col > 0)
				{
					mask |= 1ULL << Square(row + 1, col - 1);
				}
				mask |= 1ULL << Square(row + 1, col);
				if (col + 1 < 8)
				{
					mask |= 1ULL << Square(row + 1, col + 1);
				}
			}
			if (col > 0)
			{
				mask |= 1ULL << Square(row, col - 1);
			}
			if (col + 1 < 8)
			{
				mask |= 1ULL << Square(row, col + 1);
			}
			rearDefense[Colors::Black][i] = mask;
		}
	}

	bool Evaluation::IsDrawByMaterial(const Board& board)
	{
		// Check for draws due to lack of material
		if(board.GetPieceTypeBitBoard(PieceTypes::Queen) == 0 && 
			board.GetPieceTypeBitBoard(PieceTypes::Rook) == 0 && 
			board.GetPieceTypeBitBoard(PieceTypes::Pawn) == 0)
		{
			int blackBits = BitCount(board.GetPieceBitBoard(Pieces::BlackBishop) | board.GetPieceBitBoard(Pieces::BlackKnight));
			int whiteBits = BitCount(board.GetPieceBitBoard(Pieces::WhiteBishop) | board.GetPieceBitBoard(Pieces::WhiteKnight));
			if(blackBits < 2 && whiteBits < 2)
			{
				return true;
			}
		}
		return false;
	}

	bool Evaluation::IsDrawByMaterial()
	{
		return Evaluation::IsDrawByMaterial(m_board);
	}
	
	int Evaluation::CalculateKingSafety()
	{
		return CalculateKingSafety(Colors::White) - CalculateKingSafety(Colors::Black);
	}

	int Evaluation::CalculateKingSafety(int color)
	{
		// king safety relies on mobility
		int kingScore = 0;
		kingSafety = 0;
		
		int otherColor = OtherColor(color);
		int safetyScore = 0;
		ulong kingBits = m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::King, color));
		int index = GetLowestSetBit(kingBits);

		int row = index / 8;

		// only do positional king safety in mid game.
		int total = BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, otherColor)))*9 + BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Rook, otherColor)))*5 + 
			BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Knight, otherColor)) | m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Bishop, otherColor)))*3 + BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Pawn, otherColor)));
		bool endGame = total <= 13;
		if (!endGame)
		{
			int col = index % 8;
			if (row >= 2) safetyScore -= 15;
			if (row == 0) safetyScore += 8;
			ulong pawns = m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Pawn, color));
			int kingPawnCount = BitCount(kpawn[color] & pawns);
			int queenPawnCount = BitCount(qpawn[color] & pawns);
			if ((kingBits & kking[color]) != 0)
			{
				safetyScore += (kingPawnCount - 3) * 7;
				safetyScore += 20;
			}
			else if ((kingBits & qking[color]) != 0)
			{
				safetyScore += (queenPawnCount - 3) * 7;
				safetyScore += 20;
			}
			else if (m_board.GetCastleRights().GetQueen(color) || m_board.GetCastleRights().GetKing(color))
			{
				int pawnCount = 0;
				if (m_board.GetCastleRights().GetQueen(color))
				{
					pawnCount = queenPawnCount;
				}
				if (m_board.GetCastleRights().GetKing(color))
				{
					pawnCount = max(pawnCount, kingPawnCount);
				}
				safetyScore += (pawnCount - 3) * 7;
			}
			else if (col == 3 || col == 4)
			{
				// stuck in the middle and can't castle
				safetyScore -= 15;
			}
		}
		else
		{
			// endgame
			safetyScore += 4 * row;
			int kingMobility = TotalKingMobility(color);
			if(kingMobility < 5) safetyScore -= 50;
			safetyScore += 2*kingMobility;
		}
		ulong theirMobility = color == Colors::White ? blackMobility : whiteMobility;
		ulong myNonKingMobility = color == Colors::White ? whiteNonKingMobility : blackNonKingMobility;
		// blockers look for defenders and pieces
		ulong blockerBits = theirMobility & blockers[color][index];
		ulong defenseBits = myNonKingMobility & blockerBits;
		ulong defenderBits = m_board.colorBitBoards[color] & blockerBits;
		int defenderCount = BitCount(blockerBits) + BitCount(blockerBits ^ defenderBits) + BitCount(defenseBits ^ blockerBits);

		// rear defence looks for defenders
		ulong pressure = theirMobility & rearDefense[color][index];
		ulong defense = myNonKingMobility & pressure;
		ulong undefendedPressure = pressure ^ defense;
		int rearDefenseCount = BitCount(pressure) + BitCount(undefendedPressure);

		int myKingAttackCount = color == Colors::White ? whiteKingAttackCount : blackKingAttackCount;
		int totalDangerCount = defenderCount + rearDefenseCount + myKingAttackCount / 2;
		totalDangerCount = min(totalDangerCount, maxKingSafetyFunction - 1);
		safetyScore -= kingSafetyFunction[totalDangerCount];

		// Penalty for check
		if ((theirMobility & kingBits) != 0)
		{
			//safetyScore -= 30;
		}
		if (total > 1)
		{
			kingScore += safetyScore;
		}
		kingSafety += safetyScore;
		return kingScore;
	}

	int Evaluation::GetPawnScore()
	{
		int pawnScore = 0;
		if(!TranspositionTable::PawnScoreTable.TryGetEntry(m_board.GetPawnZobristKey(), pawnScore))
		{
			pawnScore = CalculatePawnScore();
			TranspositionTable::PawnScoreTable.SetEntry(m_board.GetPawnZobristKey(), pawnScore);
		}
		return pawnScore;
	}
	
	int Evaluation::CalculateMobility()
	{
		whiteKingAttackCount = 0;
		blackKingAttackCount = 0;
		int whiteMobilityScore = CalculateMobility(Colors::White);
		int blackMobilityScore = CalculateMobility(Colors::Black);
		return whiteMobilityScore - blackMobilityScore;
	}

	int Evaluation::mobilityMultiplier[] = 
	{
		0, // pawn
		4, // knight
		6, // bishop
		2, // rook
		1, // queen
		0, // king
	};

	int Evaluation::attackCountMultiplier[] = 
	{
		2, // pawn
		3, // knight
		3, // bishop
		5, // rook
		8, // queen
		1, // king
	};

	int Evaluation::TotalKingMobility(int color)
	{
		const ulong boardEdge = 0x007e7e7e7e7e7e00ULL;
		ulong theKing = m_board.GetPieceBitBoard(color == Colors::White ? Pieces::WhiteKing : Pieces::BlackKing);
		
		ulong unsafeSquares = color == Colors::White ? blackMobility : whiteMobility;
		ulong openSquares = ~unsafeSquares;
		ulong next = theKing;
		ulong totals = 0;
		do
		{
			totals |= next;
			ulong move = totals & boardEdge;
			next = (move << 1) | (move << 7) | (move << 8) | (move << 9) | (move >> 1) | (move >> 7) | (move >> 8) | (move >> 9);
			next &= openSquares;
		} while((next & totals) != next);
		return BitCount(totals);
	}

	int Evaluation::CalculateMobility(int color)
	{
		ulong mobility = 0;
		int mobilityCount = 0;
		int otherColor = OtherColor(color);

		ulong allies = m_board.colorBitBoards[color];
		ulong enemies = m_board.colorBitBoards[OtherColor(color)];
		ulong notAllies = ~allies;
		bool endgame = false;
		ulong kingAttackBits = 0;
		int kingAttackCount = 0;
		
		int kingPos = GetLowestSetBit(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::King, otherColor)));
		int total = BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, otherColor))) * 2 + BitCount(m_board.GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Rook, otherColor)));
		endgame = total < 2;
		kingAttackBits = pawnShield[otherColor][kingPos] | blockers[otherColor][kingPos] | rearDefense[otherColor][kingPos];
		
		
		int myIndex = Pieces::GetPiece(PieceTypes::Pawn, color) - 1;
		ulong pawnCaptureMask = enemies | m_board.GetEnPassantRights().BitBoard();

		for (int pieceCount = 0; pieceCount < 6; pieceCount++)
		{
			if (pieceCount == 5)
			{
				if (color == Colors::White)
				{
					whiteNonKingMobility = mobility;
				}
				else
				{
					blackNonKingMobility = mobility;
				}
			}

			/*
			int multiplier = mobilityMultiplier[pieceCount];
			int aMultiplier = attackCountMultiplier[pieceCount];

			++myIndex;
			ulong myPieces = m_board.pieceBitBoards[pieceCount] & allies;

			int piece = myIndex;
			int pieceType = Pieces::GetPieceTypeFromPiece(piece);

			while (myPieces != 0)
			{
				int source = PopLowestSetBit(myPieces);
				
				if (pieceType != PieceTypes::Pawn)
				{
					ulong attackMask = m_board.GetAttackMask(source, pieceType);
					// we can't capture our own pieces.
					attackMask &= notAllies;
					mobilityCount += BitCount(attackMask) * multiplier;
					mobility |= attackMask;

					if ((attackMask & kingAttackBits) != 0)
					{
						kingAttackCount += aMultiplier;
					}
				}
			}
			*/
			mobilityCount += CalculatePieceMobility(pieceCount, color, allies, notAllies, kingAttackBits, mobility, kingAttackCount);
		}

		int score = mobilityCount * 2;
		if (color == Colors::White)
		{
			whiteMobility = mobility;
			blackKingAttackCount = kingAttackCount;
		}
		else
		{
			blackMobility = mobility;
			whiteKingAttackCount = kingAttackCount;
		}

		return score;
	}

	int Evaluation::CalculatePieceMobility(int pieceType, int color, ulong allies, ulong notAllies, ulong kingAttackBits, ulong& mobility, int& kingAttackCount)
	{
		int mobilityCount = 0;
		int multiplier = mobilityMultiplier[pieceType];
		int aMultiplier = attackCountMultiplier[pieceType];

		//skip pawns
		int piece = Pieces::GetPiece(pieceType, color);
		ulong myPieces = m_board.pieceBitBoards[pieceType] & allies;

		while (myPieces != 0)
		{
			int source = PopLowestSetBit(myPieces);

			if (pieceType != PieceTypes::Pawn)
			{
				ulong attackMask = m_board.GetAttackMask(source, pieceType);
				// we can't capture our own pieces.
				attackMask &= notAllies;
				mobilityCount += BitCount(attackMask) * multiplier;
				mobility |= attackMask;

				if ((attackMask & kingAttackBits) != 0)
				{
					kingAttackCount += aMultiplier;
				}
			}
		}
		return mobilityCount;
	}
}