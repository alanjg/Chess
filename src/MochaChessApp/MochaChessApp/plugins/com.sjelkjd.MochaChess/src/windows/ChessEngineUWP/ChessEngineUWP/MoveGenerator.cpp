#include "pch.h"
#include "MoveGenerator.h"

namespace ChessEngineUWP
{
	int MoveGenerator::whiteKingOrigin = Square(0, 4);
	int MoveGenerator::whiteKingsideRook = Square(0, 7);
	int MoveGenerator::whiteKingsideKingCastle = Square(0, 6);
	int MoveGenerator::whiteKingsideRookCastle = Square(0, 5);
	int MoveGenerator::whiteQueensideRook = Square(0, 0);
	int MoveGenerator::whiteQueensideKingCastle =  Square(0, 2);
	int MoveGenerator::whiteQueensideRookCastle =  Square(0, 3);
	int MoveGenerator::blackKingOrigin =  Square(7, 4);
	int MoveGenerator::blackKingsideRook =  Square(7, 7);
	int MoveGenerator::blackKingsideKingCastle =  Square(7, 6);
	int MoveGenerator::blackKingsideRookCastle =  Square(7, 5);
	int MoveGenerator::blackQueensideRook =  Square(7, 0);
	int MoveGenerator::blackQueensideKingCastle =  Square(7, 2);
	int MoveGenerator::blackQueensideRookCastle =  Square(7, 3);

	int MoveGenerator::bdr[] = { 1, 1, -1, -1 };
	int MoveGenerator::bdc[] = { -1, 1, -1, 1 };

	int MoveGenerator::rdr[] = { 0, 1, -1, 0};
	int MoveGenerator::rdc[] = { -1, 0, 0, 1};

	int MoveGenerator::qdr[] = {  0, 1, -1, 0,  1, 1, -1, -1};
	int MoveGenerator::qdc[] = { -1, 0,  0, 1, -1, 1, -1,  1};

	int MoveGenerator::ndr[] = { 2, 2, -1, -1, 1, 1, -2, -2 };
	int MoveGenerator::ndc[] = { -1, 1, 2, -2, -2, 2, -1, 1 };

	ulong MoveGenerator::rankAttacks[64][64];
	ulong MoveGenerator::fileAttacks[64][64];
	ulong MoveGenerator::diagA1H8Attacks[64][64];
	ulong MoveGenerator::diagA8H1Attacks[64][64];
	ulong MoveGenerator::knightAttacks[64];
	ulong MoveGenerator::kingAttacks[64];
	ulong MoveGenerator::pawnAttacks[64][2];

	ulong MoveGenerator::diagA1H8Masks[64];
	ulong MoveGenerator::diagA8H1Masks[64];
	
	ulong MoveGenerator::intermediateSquares[64][64];

	MoveGenerator::CastleMove MoveGenerator::castleMoves[4];
	
	void AddPawnPromotions(int move, std::vector<int>& moves, bool quiescentMovesOnly)
	{
		moves.push_back(MakePromoMove(move, PieceTypes::Queen));
		if(!quiescentMovesOnly)
		{
			moves.push_back(MakePromoMove(move, PieceTypes::Knight));
			moves.push_back(MakePromoMove(move, PieceTypes::Rook));
			moves.push_back(MakePromoMove(move, PieceTypes::Bishop));
		}
	}

	ulong MakePawnMoveBitBoard(ulong position, int color)
	{
		if (color == Colors::White)
		{
			return position << 8;
		}
		else
		{
			return position >> 8;
		}
	}

	int MakePawnMove(int position, int color)
	{
		if (color == Colors::White)
		{
			return position + 8;
		}
		else
		{
			return position - 8;
		}
	}

	int MakePawnCaptureMove(int source, int color, int direction)
	{
		if (direction == 0)
		{
			if ((source & 7) > 0)
			{
				return source + 7 - color * 16;
			}
		}
		else
		{
			if ((source & 7) < 7)
			{
				return source + 9 - color * 16;
			}
		}
		return source;
	}

	ulong MakeBitboard(int row, int col)
	{
		return 1ULL << (row * 8 + col);
	}

	ulong MoveGenerator::whiteQueensideCastleBitBoard = MakeBitboard(0, 1) | MakeBitboard(0, 2) | MakeBitboard(0, 3);
	ulong MoveGenerator::whiteQueensideCastleKingBitBoard = MakeBitboard(0, 2) | MakeBitboard(0, 3) | MakeBitboard(0, 4);
	ulong MoveGenerator::whiteKingsideCastleBitBoard = MakeBitboard(0, 5) | MakeBitboard(0, 6);
	ulong MoveGenerator::whiteKingsideCastleKingBitBoard = MakeBitboard(0, 4) | MakeBitboard(0, 5) | MakeBitboard(0, 6);
	ulong MoveGenerator::blackQueensideCastleBitBoard = MakeBitboard(7, 1) | MakeBitboard(7, 2) | MakeBitboard(7, 3);
	ulong MoveGenerator::blackQueensideCastleKingBitBoard = MakeBitboard(7, 2) | MakeBitboard(7, 3) | MakeBitboard(7, 4);
	ulong MoveGenerator::blackKingsideCastleBitBoard = MakeBitboard(7, 5) | MakeBitboard(7, 6);
	ulong MoveGenerator::blackKingsideCastleKingBitBoard = MakeBitboard(7, 4) | MakeBitboard(7, 5) | MakeBitboard(7, 6);

	ulong MoveGenerator::whitePawnBitBoard = MakeBitboard(1, 0) | MakeBitboard(1, 1) | MakeBitboard(1, 2) | MakeBitboard(1, 3) | MakeBitboard(1, 4) | MakeBitboard(1, 5) | MakeBitboard(1, 6) | MakeBitboard(1, 7);
	ulong MoveGenerator::blackPawnBitBoard = MakeBitboard(6, 0) | MakeBitboard(6, 1) | MakeBitboard(6, 2) | MakeBitboard(6, 3) | MakeBitboard(6, 4) | MakeBitboard(6, 5) | MakeBitboard(6, 6) | MakeBitboard(6, 7);

	void MoveGenerator::GenerateRankAttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			for (int j = 0; j < 64; j++)
			{
				ulong attackMask = 0;
				int piecesOnRank = (1 << 7) | (j << 1) | 1;
				for (int k = file - 1; k >= 0; k--)
				{
					attackMask |= 1ULL << (row * 8 + k);
					if (((1 << k) & piecesOnRank) != 0)
					{
						break;
					}
				}
				
				for (int k = file + 1; k < 8; k++)
				{
					attackMask |= 1ULL << (row * 8 + k);
					if (((1 << k) & piecesOnRank) != 0)
					{
						break;
					}
				}

				rankAttacks[i][j] = attackMask;
			}
		}
	}

	void MoveGenerator::GenerateFileAttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			for (int j = 0; j < 64; j++)
			{
				ulong attackMask = 0;
				int piecesOnFile = (1 << 7) | (j << 1) | 1;
				for (int k = row - 1; k >= 0; k--)
				{
					attackMask |= 1ULL << (k * 8 + file);
					if (((1 << k) & piecesOnFile) != 0)
					{
						break;
					}
				}

				for (int k = row + 1; k < 8; k++)
				{
					attackMask |= 1ULL << (k * 8 + file);
					if (((1 << k) & piecesOnFile) != 0)
					{
						break;
					}
				}

				fileAttacks[i][j] = attackMask;
			}
		}
	}

	//bits 0-6 contain files b-g
	void MoveGenerator::GenerateDiagA1H8AttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			int rank = row + 7 - file;
			int start = file;
			int length = 8 - abs(row - file);
			for (int j = 0; j < 64; j++)
			{
				ulong attackMask = 0;

				int piecesOnDiagonal = (1 << 7) | (j << 1) | 1;

				for (int k = row - 1, m = file - 1, count = start - 1; k >= 0 && m >= 0 && count >= 0; k--, m--, count--)
				{
					attackMask |= 1ULL << (k * 8 + m);
					if (((1 << count) & piecesOnDiagonal) != 0)
					{
						break;
					}
				}

				for (int k = row + 1, m = file + 1, count = start + 1; k < 8 && m < 8 && count < 8; k++, m++, count++)
				{
					attackMask |= 1ULL << (k * 8 + m);
					if (((1 << count) & piecesOnDiagonal) != 0)
					{
						break;
					}
				}

				diagA1H8Attacks[i][j] = attackMask;
			}
		}
	}

	void MoveGenerator::GenerateDiagA8H1AttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			int rank = row + file;
			int start = file;
			int length = 8 - abs(7 - row - file);
			for (int j = 0; j < 64; j++)
			{
				ulong attackMask = 0;

				int piecesOnDiagonal = (1 << 7) | (j << 1) | 1;

				for (int k = row - 1, m = file + 1, count = start + 1; k >= 0 && m < 8 && count < 8; k--, m++, count++)
				{
					attackMask |= 1ULL << (k * 8 + m);
					if (((1 << count) & piecesOnDiagonal) != 0)
					{
						break;
					}
				}

				for (int k = row + 1, m = file - 1, count = start - 1; k < 8 && m >= 0 && count >= 0; k++, m--, count--)
				{
					attackMask |= 1ULL << (k * 8 + m);
					if (((1 << count) & piecesOnDiagonal) != 0)
					{
						break;
					}
				}

				diagA8H1Attacks[i][j] = attackMask;
			}
		}
	}

	void MoveGenerator::GenerateDiagonalMasks()
	{
		for (int i = 0; i < 64; i++)
		{
			ulong attackMask = 0;
			
			int row = i / 8;
			int file = i % 8;
			for (int j = -7; j < 8; j++)
			{
				int jrow = row - j;
				int jcol = file - j;
				if(jrow >= 0 && jrow <= 7 && jcol >= 0 && jcol <= 7)
				{
					attackMask |= 1ULL << (jrow * 8 + jcol);
				}
			}
			diagA1H8Masks[i] = attackMask;

			attackMask = 0;
			for (int j = -7; j < 8; j++)
			{
				int jrow = row - j;
				int jcol = file + j;
				if(jrow >= 0 && jrow <= 7 && jcol >= 0 && jcol <= 7)
				{
					attackMask |= 1ULL << (jrow * 8 + jcol);
				}
			}
			diagA8H1Masks[i] = attackMask;
		}	
	}

	void MoveGenerator::GenerateKnightAttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			ulong attackMask = 0;
			for (int j = 0; j < 8; j++)
			{
				int nrow = row + ndr[j];
				int ncol = file + ndc[j];
				if (nrow >= 0 && nrow < 8 && ncol >= 0 && ncol < 8)
				{
					attackMask |= 1ULL << (nrow * 8 + ncol);
				}
			} 
			knightAttacks[i] = attackMask;
		}
	}

	void MoveGenerator::GenerateKingAttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			ulong attackMask = 0;
			for (int j = 0; j < 8; j++)
			{
				int nrow = row + qdr[j];
				int ncol = file + qdc[j];
				if (nrow >= 0 && nrow < 8 && ncol >= 0 && ncol < 8)
				{
					attackMask |= 1ULL << (nrow * 8 + ncol);
				}
			}
			kingAttacks[i] = attackMask;
		}
	}

	void MoveGenerator::GeneratePawnAttackBitmasks()
	{
		for (int i = 0; i < 64; i++)
		{
			int row = i / 8;
			int file = i % 8;
			ulong attackMask = 0;
			if (row < 7)
			{
				if (file > 0)
				{
					attackMask |= 1ULL << ((row + 1) * 8 + file - 1);
				}
				if (file < 7)
				{
					attackMask |= 1ULL << ((row + 1) * 8 + file + 1);
				}
			}
			// this square is attacked by black pawns on the attackMask squares.
			pawnAttacks[i][1] = attackMask;

			attackMask = 0;
			if (row > 0)
			{
				if (file > 0)
				{
					attackMask |= 1ULL << ((row - 1) * 8 + file - 1);
				}
				if (file < 7)
				{
					attackMask |= 1ULL << ((row - 1) * 8 + file + 1);
				}
			}
			// this square is attacked by white pawns on the attackMask squares.
			pawnAttacks[i][0] = attackMask;
		}
	}

	void MoveGenerator::GenerateIntermediateSquares()
	{
		for(int source = 0;source < 64; source++)
		{
			int sr = source / 8;
			int sc = source % 8;
			for(int dest = 0; dest < 64; dest++)
			{
				ulong mask = 0;
				if(source != dest)
				{
					int dr = dest / 8;
					int dc = dest % 8;

					int rdiff = dr - sr;
					int cdiff = dc - sc;
					if(rdiff == 0 || cdiff == 0 || abs(rdiff) == abs(cdiff))
					{
						int rDelta = rdiff == 0 ? 0 : rdiff / abs(rdiff);
						int cDelta = cdiff == 0 ? 0 : cdiff / abs(cdiff);
						int tr = sr + rDelta;
						int tc = sc + cDelta; 
	
						while(tr != dr || tc != dc)
						{
							mask |= 1ULL << (tr * 8 + tc);
							tr += rDelta;
							tc += cDelta;
						}
					}
				}
				intermediateSquares[source][dest] = mask;
			}
		}
	}

	void MoveGenerator::AddMoves(int pieceType, int color, ulong targets, std::vector<int>& currentMoves)
	{
		int piece = Pieces::GetPiece(pieceType, color);
		ulong myPieces = m_board.pieceBitBoards[pieceType] & m_board.colorBitBoards[color];
		while (myPieces != 0)
		{
			int source = PopLowestSetBit(myPieces);
	
			ulong attackMask = m_board.GetAttackMask(source, pieceType);
			ulong pinMask = m_board.GetPinRestrictionMask(source, color);
			attackMask &= targets;
			attackMask &= pinMask;
			while (attackMask != 0)
			{
				int dest = PopLowestSetBit(attackMask);
				int m = MakeShortMove(source, dest);
				currentMoves.push_back(m);
			}
		}
	}

	void MoveGenerator::GenerateCheckEscapes(bool quiescentMovesOnly, int toMove, std::vector<int>& currentMoves)
	{
		int otherColor = OtherColor(toMove);
		
		ulong allies = m_board.colorBitBoards[toMove];
		ulong enemies = m_board.colorBitBoards[otherColor];

		// king moves first
		int kingSquare = m_board.GetKingSquare(toMove);
		ulong attackers = m_board.GetSquareAttackers(kingSquare, otherColor);
		ulong safeMoves = m_board.GetSafeKingMoves(toMove);

		// if there's only one attacker, we can capture or block.  If more than one, only king moves will save us.
		if(HasOneBitSet(attackers))
		{
			int attackerSquare = GetLowestSetBit(attackers);

			ulong intermediate = intermediateSquares[kingSquare][attackerSquare];
	
			if(quiescentMovesOnly)
			{
				intermediate = 0;
			}
			ulong targets = attackers | intermediate;

			AddMoves(PieceTypes::Bishop, toMove, targets, currentMoves);
			AddMoves(PieceTypes::Knight, toMove, targets, currentMoves);
			AddMoves(PieceTypes::Rook, toMove, targets, currentMoves);
			AddMoves(PieceTypes::Queen, toMove, targets, currentMoves);

			ulong kingTargets = (~(allies | intermediate)) & safeMoves;
			if(quiescentMovesOnly)
			{
				kingTargets &= enemies;
			}
			AddMoves(PieceTypes::King, toMove, kingTargets, currentMoves);

			// Can only capture the attacker, can only move to block the intermediate squares.
			// include the en passant square
			ulong pawnTargets = attackers;
			if(m_board.enPassantRights.PawnSquare() == attackerSquare)
			{
				pawnTargets |= m_board.enPassantRights.BitBoard();
			}
			AddPawnMoves(quiescentMovesOnly, toMove, pawnTargets, intermediate, currentMoves);
		}
		else
		{
			ulong attackMask = 0;
			while(attackers != 0)
			{
				int attackerSquare = PopLowestSetBit(attackers);
				attackMask |= intermediateSquares[kingSquare][attackerSquare];
			}
			ulong kingTargets = (~(allies | attackMask)) & safeMoves;
			
			if(quiescentMovesOnly)
			{
				kingTargets &= enemies;
			}
			AddMoves(PieceTypes::King, toMove, kingTargets, currentMoves);
		}
	}

	void MoveGenerator::GenerateMoves(bool quiescentMovesOnly, int toMove, std::vector<int>& currentMoves)
	{
		if(m_board.GetIsInCheck())
		{
			GenerateCheckEscapes(quiescentMovesOnly, toMove, currentMoves);
			return;
		}
		int otherColor = OtherColor(toMove);
		
		ulong allies = m_board.colorBitBoards[toMove];
		ulong enemies = m_board.colorBitBoards[OtherColor(toMove)];

		ulong targets = quiescentMovesOnly ? enemies : ~allies;
		AddMoves(PieceTypes::Bishop, toMove, targets, currentMoves);
		AddMoves(PieceTypes::Knight, toMove, targets, currentMoves);
		AddMoves(PieceTypes::Rook, toMove, targets, currentMoves);
		AddMoves(PieceTypes::Queen, toMove, targets, currentMoves);

		ulong safeMoves = m_board.GetSafeKingMoves(toMove);

		AddMoves(PieceTypes::King, toMove, targets & safeMoves, currentMoves);
		
		if(!quiescentMovesOnly)
		{
			AddCastleMoves(toMove, currentMoves);
		}

		// allow promotions in qsearch
		ulong captureTargets = enemies | m_board.enPassantRights.BitBoard();
		ulong moveTargets = quiescentMovesOnly ? 0xFF000000000000FF : ~(0ULL);
		AddPawnMoves(quiescentMovesOnly, toMove, captureTargets, moveTargets, currentMoves);
	}

	bool MoveGenerator::IsValidMove(int move)
	{
		// Currently is very slow...
		return true;
		if(move == NullMove)
		{
			return !m_board.GetIsInCheck();
		}
		
		int toMove = m_board.Turn();
		int otherColor = OtherColor(toMove);
		
		ulong allies = m_board.colorBitBoards[toMove];
		ulong enemies = m_board.colorBitBoards[OtherColor(toMove)];

		ulong targets = ~allies;
		targets &= 1ULL << GetDestFromMove(move);
		int sourcePiece = m_board.GetPiece(GetSourceFromMove(move));
		std::vector<int> moves;
		ulong safeMoves;
		switch(Pieces::GetPieceTypeFromPiece(sourcePiece))
		{
		case PieceTypes::None:
			return true;
		case PieceTypes::Bishop:
			AddMoves(PieceTypes::Bishop, toMove, targets, moves);
			break;
		case PieceTypes::Knight:
			AddMoves(PieceTypes::Knight, toMove, targets, moves);
			break;
		case PieceTypes::Rook:
			AddMoves(PieceTypes::Rook, toMove, targets, moves);
			break;
		case PieceTypes::Queen:
			AddMoves(PieceTypes::Queen, toMove, targets, moves);
			break;
		case PieceTypes::King:
			safeMoves = m_board.GetSafeKingMoves(toMove);
			AddMoves(PieceTypes::King, toMove, targets & safeMoves, moves);
			AddCastleMoves(toMove, moves);
			break;
		case PieceTypes::Pawn:
			ulong captureTargets = enemies | m_board.enPassantRights.BitBoard();
			ulong moveTargets = ~(0ULL);
			AddPawnMoves(false, toMove, captureTargets, moveTargets, moves);
			break;
		}
		return std::find(moves.begin(), moves.end(), move) != moves.end();
	}
	
	void MoveGenerator::AddCastleMoves(int toMove, std::vector<int>& currentMoves)
	{
		if (m_board.GetIsInCheck())
		{
			return;
		}
		ulong castleBitBoard = m_board.occupiedSquares;
		if (toMove == Colors::White)
		{
			if (m_board.GetCastleRights().GetWhiteKing() && (whiteKingsideCastleBitBoard & castleBitBoard) == 0)
			{
				if (!m_board.IsSquareAttacked(whiteKingsideCastleSquares, Colors::Black) && !m_board.IsSquareAttacked(whiteKingsideKingCastle, Colors::Black))
				{
					int m = MakeShortMove(whiteKingOrigin, whiteKingsideKingCastle);
					currentMoves.push_back(m);
				}
			}
			if (m_board.GetCastleRights().GetWhiteQueen() && (whiteQueensideCastleBitBoard & castleBitBoard) == 0)
			{
				if (!m_board.IsSquareAttacked(whiteQueensideCastleSquares, Colors::Black) && !m_board.IsSquareAttacked(whiteQueensideKingCastle, Colors::Black))
				{
					int m = MakeShortMove(whiteKingOrigin, whiteQueensideKingCastle);
					currentMoves.push_back(m);
				}
			}
		}
		else if (toMove == Colors::Black)
		{
			if (m_board.GetCastleRights().GetBlackKing() && (blackKingsideCastleBitBoard & castleBitBoard) == 0)
			{
				if (!m_board.IsSquareAttacked(blackKingsideCastleSquares, Colors::White) && !m_board.IsSquareAttacked(blackKingsideKingCastle, Colors::White))
				{
					int m = MakeShortMove(blackKingOrigin, blackKingsideKingCastle);
					currentMoves.push_back(m);
				}
			}
			if (m_board.GetCastleRights().GetBlackQueen() && (blackQueensideCastleBitBoard & castleBitBoard) == 0)
			{
				if (!m_board.IsSquareAttacked(blackQueensideCastleSquares, Colors::White) && !m_board.IsSquareAttacked(blackQueensideKingCastle, Colors::White))
				{
					int m = MakeShortMove(blackKingOrigin, blackQueensideKingCastle);
					currentMoves.push_back(m);
				}
			}
		}
	}
	
	void MoveGenerator::AddPawnMoves(bool quiescentMovesOnly, int toMove, ulong captureTargets, ulong moveTargets, std::vector<int>& currentMoves)
	{
		// pawn promos
		ulong enemyPawns = toMove == Colors::Black ? whitePawnBitBoard : blackPawnBitBoard;
		int pawnPiece = Pieces::GetPiece(PieceTypes::Pawn, toMove);
		ulong pawns = m_board.GetPieceBitBoard(pawnPiece);	
		
		ulong advanced;
		if(toMove == Colors::White)
		{
			advanced = pawns << 8;
		}
		else
		{
			advanced = pawns >> 8;
		}
		advanced &= ~m_board.GetOccupiedSquares();
		ulong singleMove = advanced & moveTargets;
		if(toMove == Colors::White)
		{
			advanced = advanced >> 8;
			singleMove = singleMove >> 8;
		}
		else
		{	
			advanced = advanced << 8;
			singleMove = singleMove << 8;
		}

		ulong promotions = singleMove & enemyPawns;
		while(promotions != 0)
		{
			int pawn = PopLowestSetBit(promotions);
			int dest = MakePawnMove(pawn, toMove);
			ulong pinMask = m_board.GetPinRestrictionMask(pawn, toMove);
			if((pinMask & (1ULL << dest)) != 0)
			{
				int move = MakeShortMove(pawn, dest);
				AddPawnPromotions(move, currentMoves, quiescentMovesOnly);
			}
		}

		ulong captureLeft = pawns & 0xFEFEFEFEFEFEFEFE;
		if(toMove == Colors::White)
		{
			captureLeft <<= 7;
		}
		else
		{
			captureLeft >>= 9;
		}
		captureLeft &= captureTargets;
		if(toMove == Colors::White)
		{
			captureLeft >>= 7;
		}
		else
		{
			captureLeft <<= 9;
		}
		ProcessPawnCaptures(captureLeft, toMove, enemyPawns, currentMoves, quiescentMovesOnly, 0);

		ulong captureRight = pawns & 0x7F7F7F7F7F7F7F7F;
		if(toMove == Colors::White)
		{
			captureRight <<= 9;
		}
		else
		{
			captureRight >>= 7;
		}
		captureRight &= captureTargets;
		if(toMove == Colors::White)
		{
			captureRight >>= 9;
		}
		else
		{
			captureRight <<= 7;
		}
		ProcessPawnCaptures(captureRight, toMove, enemyPawns, currentMoves, quiescentMovesOnly, 1);
		
		pawns = singleMove & ~enemyPawns;
		while(pawns != 0)
		{
			int pawn = PopLowestSetBit(pawns);
			int dest = MakePawnMove(pawn, toMove);
			ulong pinMask = m_board.GetPinRestrictionMask(pawn, toMove);
			if((pinMask & (1ULL << dest)) != 0)
			{
				int move = MakeShortMove(pawn, dest);
				currentMoves.push_back(move);
			}
		}

		ulong alliedPawns = toMove == Colors::White ? whitePawnBitBoard : blackPawnBitBoard;
		ulong doubleStart = advanced & alliedPawns;

		if(toMove == Colors::White)
		{
			doubleStart = doubleStart << 16;
		}
		else
		{
			doubleStart = doubleStart >> 16;
		}
		doubleStart &= ~m_board.GetOccupiedSquares();
		doubleStart &= moveTargets;
		if(toMove == Colors::White)
		{
			doubleStart = doubleStart >> 16;	
		}
		else
		{	
			doubleStart = doubleStart << 16;
		}

		while(doubleStart != 0)
		{
			int pawn = PopLowestSetBit(doubleStart);
			int dest = MakePawnMove(MakePawnMove(pawn, toMove), toMove);
			ulong pinMask = m_board.GetPinRestrictionMask(pawn, toMove);
			if((pinMask & (1ULL << dest)) != 0)
			{
				int move = MakeShortMove(pawn, dest);
				currentMoves.push_back(move);
			}
		}
	}

	void MoveGenerator::ProcessPawnCaptures(ulong captureMask, int toMove, ulong enemyPawns, vector<int>& currentMoves, bool quiescentMovesOnly, int direction)
	{
		int epSquare = m_board.enPassantRights.CaptureSquare();
		while (captureMask != 0)
		{
			int pawn = PopLowestSetBit(captureMask);
			int dest = MakePawnCaptureMove(pawn, toMove, direction);
			ulong pinMask = m_board.GetPinRestrictionMask(pawn, toMove);
			if ((pinMask & (1ULL << dest)) != 0)
			{
				if (dest == epSquare)
				{
					if (m_board.DoesEnPassantCaptureCauseDiscoveredCheck(pawn, toMove))
					{
						continue;
					}
				}
				int move = MakeShortMove(pawn, dest);
				ulong pawnBoard = 1ULL << pawn;
				if ((pawnBoard & enemyPawns) != 0)
				{
					AddPawnPromotions(move, currentMoves, quiescentMovesOnly);
				}
				else
				{
					currentMoves.push_back(move);
				}
			}
		}
	}
}