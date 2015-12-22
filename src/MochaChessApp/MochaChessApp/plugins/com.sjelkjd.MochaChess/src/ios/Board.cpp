#include "pch.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "TranspositionTable.h"
#include "Evaluation.h"
#include <intrin.h>

namespace SjelkjdChessEngine
{
	std::string Board::startPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	void Board::SetEmptyBoard()
	{
		ClearBitboards();
	}

	void Board::FinalizeEmptyBoard(int newTurn, CastleRights newCastleRights, EnPassantRights newEnPassantRights)
	{
		turn = newTurn;
		castleRights = newCastleRights;
		enPassantRights = newEnPassantRights;
		
		InitializeZobristKey();
		isDrawnByRepetition = false;
	
		isInCheck = IsKingInCheck(turn);
	}

	void Board::SetFEN(const string& fen)
	{
		ClearBitboards();

		std::istringstream fenIn(fen);
		string chunk;
		fenIn >> chunk;
		int row = 7, col = 0;
		for (unsigned int i=0;i<chunk.size();i++)
		{
			char c = chunk[i];
			if (c == '/')
			{
				row--;
				col = 0;
			}
			else
			{
				if (isdigit(c))
				{
					col += c - '0';
				}
				else
				{
					int pieceType = PieceTypes::None;
					int color = isupper(c) ? Colors::White : Colors::Black;
					switch (tolower(c))
					{
						case 'q': pieceType = PieceTypes::Queen; break;
						case 'k': pieceType = PieceTypes::King; break;
						case 'r': pieceType = PieceTypes::Rook; break;
						case 'b': pieceType = PieceTypes::Bishop; break;
						case 'n': pieceType = PieceTypes::Knight; break;
						case 'p': pieceType = PieceTypes::Pawn; break;
					}

					AddPiece(Square(row, col), Pieces::GetPiece(pieceType, color));
					col++;
				}
			}
		}
		fenIn >> chunk;

		turn = chunk[0] == 'w' ? Colors::White : Colors::Black;
		castleRights = CastleRights(true, true, true, true);

		fenIn >> chunk;
		if (chunk.find('K') == string::npos) castleRights.ClearWhiteKing();
		if (chunk.find('Q') == string::npos) castleRights.ClearWhiteQueen();
		if (chunk.find('k') == string::npos) castleRights.ClearBlackKing();
		if (chunk.find('q') == string::npos) castleRights.ClearBlackQueen();
		
		fenIn >> chunk;
		// handle EP
		if (chunk == "-")
		{
			enPassantRights = EnPassantRights::NoEnPassant;
		}
		else
		{
			int epcol = chunk[0] - 'a';
			int eprow = chunk[1] - '1';
			enPassantRights = EnPassantRights(eprow, epcol);
		}

		// handle draw count
		fenIn >> drawMoveCount;

		fenIn >> turnsPlayed;

		capturedPiece = PieceTypes::None;
		InitializeZobristKey();
		isDrawnByRepetition = false;
	
		isInCheck = IsKingInCheck(turn);
	}

	void Board::SetText(const string& fen, int playerTurn, CastleRights rights)
	{
		ClearBitboards();

		std::istringstream boardIn(fen);
		char c;
		for(int i=7;i>=0;i--)
		{
			for(int j=0;j<8;j++)
			{
				boardIn >> c;
				if(c != '.')
				{
					int pieceType = PieceTypes::None;
					int color = isupper(c) ? Colors::White : Colors::Black;
					switch (tolower(c))
					{
						case 'q': pieceType = PieceTypes::Queen; break;
						case 'k': pieceType = PieceTypes::King; break;
						case 'r': pieceType = PieceTypes::Rook; break;
						case 'b': pieceType = PieceTypes::Bishop; break;
						case 'n': pieceType = PieceTypes::Knight; break;
						case 'p': pieceType = PieceTypes::Pawn; break;
					}

					AddPiece(Square(i, j), Pieces::GetPiece(pieceType, color));
				}
			}
		}
		turn = playerTurn;
		castleRights = rights;

		
		enPassantRights = EnPassantRights::NoEnPassant;
		
		drawMoveCount = 0;
		turnsPlayed = 0;
		capturedPiece = PieceTypes::None;
		InitializeZobristKey();
		isDrawnByRepetition = false;
	
		isInCheck = IsKingInCheck(turn);
	}

	void Board::InitializeZobristKey()
	{
		materialScore = 0;
		positionalScore = 0;
		endgamePositionalScore = 0;
		zobristKey = 0;
		pawnZobristKey = 0;
		if (turn == Colors::Black)
		{
			zobristKey ^= TranspositionTable::GetTurnKey();
		}
		zobristKey ^= TranspositionTable::GetCastleKey(castleRights);
		zobristKey ^= TranspositionTable::GetEnPassantKey(enPassantRights);
		for (int i = 0; i < 64; i++)
		{
			int piece = GetPiece(i);
			if (piece != Pieces::None)
			{
				zobristKey ^= TranspositionTable::GetPositionKey(i, piece);
			}
			if (Pieces::GetPieceTypeFromPiece(piece) == PieceTypes::Pawn)
			{
				pawnZobristKey ^= TranspositionTable::GetPositionKey(i, piece);
			}
			materialScore += Evaluation::PieceValue(piece);
			positionalScore += Evaluation::PieceSquareValue(piece, i);
			endgamePositionalScore += Evaluation::EndgamePieceSquareValue(piece, i);
		}
	}

	string Board::ToString()
	{
		ostringstream out;
		
		for(int row=7;row >= 0; row--)
		{
			for(int col = 0; col < 8; col++)
			{
				char c = '.';
				switch (Pieces::GetPieceTypeFromPiece(GetPiece(row*8+col)))
				{
					case PieceTypes::Pawn: c = 'p'; break;
					case PieceTypes::Knight: c = 'n'; break;
					case PieceTypes::King: c = 'k'; break;
					case PieceTypes::Bishop: c = 'b'; break;
					case PieceTypes::Queen: c = 'q'; break;
					case PieceTypes::Rook: c = 'r'; break;
				}
				if (Pieces::GetColorFromPiece(GetPiece(row*8+col)) == Colors::White)
				{
					c = toupper(c);
				}
				out << c;
			}
			out << std::endl;
		}
		return out.str();
	}

	void Board::RemovePiece(int square, int piece)
	{
		pieces[square] = Pieces::None;
		ulong mask = 1ULL << square;
		int pieceType = Pieces::GetPieceTypeFromPiece(piece);
		pieceBitBoards[pieceType] ^= mask;
		colorBitBoards[Pieces::GetColorFromPiece(piece)] ^= mask;
		occupiedSquares ^= mask;
	}

	void Board::AddPiece(int square, int piece)
	{
		pieces[square] = piece;
		ulong mask = 1ULL << square;
		int pieceType = Pieces::GetPieceTypeFromPiece(piece);
		pieceBitBoards[pieceType] |= mask;
		colorBitBoards[Pieces::GetColorFromPiece(piece)] |= mask;
		occupiedSquares ^= mask;
	}

	void Board::MovePiece(int from, int to, int piece)
	{
		ulong mask = BitBoard(from) ^ BitBoard(to);

		int pieceType = Pieces::GetPieceTypeFromPiece(piece);
		pieceBitBoards[pieceType] ^= mask;
		
		int color = Pieces::GetColorFromPiece(piece);
		colorBitBoards[color] ^= mask;
		
		occupiedSquares ^= mask;
		pieces[from] = Pieces::None;
		pieces[to] = piece;
	}

	bool Board::IsKingInCheck(int color) const
	{
		return IsSquareAttacked(GetKingSquare(color), OtherColor(color));
	}

	int Board::GetKingSquare(int color) const
	{
		// find the king
		ulong kingMask = pieceBitBoards[PieceTypes::King];
		ulong colorMask = colorBitBoards[color];

		kingMask &= colorMask;
		return GetLowestSetBit(kingMask);		
	}

	bool Board::IsSquareAttacked(int square, int attackerColor) const
	{
		ulong colorMask = colorBitBoards[attackerColor];

		ulong attackMask = MoveGenerator::rankAttacks[square][GetRankStatus(square)];
		ulong attackers = (pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Rook]) & colorMask;
		if ((attackers & attackMask) != 0)
		{
			return true;
		}

		attackMask = MoveGenerator::fileAttacks[square][GetFileStatus(square)];
		if ((attackers & attackMask) != 0)
		{
			return true;
		}

		attackMask = MoveGenerator::diagA1H8Attacks[square][GetDiagA1H8Status(square)];
		attackers = (pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Bishop]) & colorMask;
		if ((attackers & attackMask) != 0)
		{
			return true;
		}

		attackMask = MoveGenerator::diagA8H1Attacks[square][GetDiagA8H1Status(square)];
		if ((attackers & attackMask) != 0)
		{
			return true;
		}

		if ((MoveGenerator::knightAttacks[square] & pieceBitBoards[PieceTypes::Knight] & colorMask) != 0)
		{
			return true;
		}

		if ((MoveGenerator::kingAttacks[square] & pieceBitBoards[PieceTypes::King] & colorMask) != 0)
		{
			return true;
		}

		if ((MoveGenerator::pawnAttacks[square][attackerColor] & pieceBitBoards[PieceTypes::Pawn] & colorMask) != 0)
		{
			return true;
		}
		return false;
	}
	/*
	ulong Board::GetPinnedPieces(int square, int pinnedColor) const
	{
		ulong allies = colorBitBoards[pinnedColor];
		ulong enemies = colorBitBoards[OtherColor(pinnedColor)];
		ulong attackPieces = 0;

		ulong attackMask = MoveGenerator::rankPins[square][GetRankStatus(square, allies)][GetRankStatus(square, enemies)] | MoveGenerator::filePins[square][GetFileStatus(square, allies)][GetFileStatus(square, enemies)];
		ulong attackers = pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Rook];
		attackPieces |= attackers & attackMask;

		attackMask = MoveGenerator::diagA1H8Pins[square][GetDiagA1H8Status(square, allies)][GetDiagA1H8Status(square, enemies)] | MoveGenerator::diagA8H1Pins[square][GetDiagA8H1Status(square, allies)][GetDiagA8H1Status(square, enemies)];
		attackers = pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Bishop];
		attackPieces |= attackers & attackMask;

		return attackPieces & enemies;
	}
	*/
	int Board::GetDiagA1H8Status(int square) const
	{
		return GetDiagA1H8Status(square, occupiedSquares);
	}

	int Board::GetDiagA1H8Status(int square, ulong targets) const
	{
		const ulong bFile = 0x0202020202020202;
		ulong bits = MoveGenerator::diagA1H8Masks[square];
		bits &= targets;
		bits *= bFile;
		bits >>= 58;
		return int(bits & 63);
	}

	int Board::GetDiagA8H1Status(int square) const
	{
		return GetDiagA8H1Status(square, occupiedSquares);
	}

	int Board::GetDiagA8H1Status(int square, ulong targets) const
	{
		const ulong bFile = 0x0202020202020202;
		ulong bits = (targets & MoveGenerator::diagA8H1Masks[square]) * bFile;
		bits >>= 58;
		return int(bits & 63);
	}
	
	int Board::GetRankStatus(int square) const
	{
		return GetRankStatus(square, occupiedSquares);
	}

	int Board::GetRankStatus(int square, ulong targets) const
	{
		int row = square / 8;
		return (int)(targets >> (row * 8 + 1)) & 63;
	}

	int Board::GetFileStatus(int square) const
	{
		return GetFileStatus(square, occupiedSquares);
	}

	int Board::GetFileStatus(int square, ulong targets) const
	{
		const ulong aFile	= 0x0101010101010101;
		const ulong c7h2Diag = 0x0004081020408000;
		ulong aFileBits = targets >> (square & 7);
		ulong bits = aFile & aFileBits;
		bits *= c7h2Diag;
		bits >>= 58;
		return int(bits & 63);
	}
	
	ulong Board::GetAttackMask(int square, int attackerType) const
	{
		switch (attackerType)
		{
		case PieceTypes::Pawn:
			return 0;

		case PieceTypes::Knight:
			return MoveGenerator::knightAttacks[square];

		case PieceTypes::Bishop:
				return MoveGenerator::diagA1H8Attacks[square][GetDiagA1H8Status(square)] | 
					MoveGenerator::diagA8H1Attacks[square][GetDiagA8H1Status(square)];
				
		case PieceTypes::Rook:
				return MoveGenerator::rankAttacks[square][GetRankStatus(square)] | 
					MoveGenerator::fileAttacks[square][GetFileStatus(square)];

		case PieceTypes::Queen:
				return MoveGenerator::rankAttacks[square][GetRankStatus(square)] | 
					MoveGenerator::fileAttacks[square][GetFileStatus(square)] |
					MoveGenerator::diagA1H8Attacks[square][GetDiagA1H8Status(square)] | 
					MoveGenerator::diagA8H1Attacks[square][GetDiagA8H1Status(square)];

		case PieceTypes::King:
				return MoveGenerator::kingAttacks[square];

			default:
				return 0;
		}
	}

	void Board::MakeMove(int move, bool updateDrawState)
	{
		BoardState undoState;
		MakeMove(move, updateDrawState, undoState);
	}

	void Board::MakeMove(int move, bool updateDrawState, BoardState& undoState)
	{
		updateDrawState = true;

		undoState.isInCheck = isInCheck;
		undoState.capturedPiece = capturedPiece;
		undoState.castleRights = castleRights;
		undoState.enPassantRights = enPassantRights;
		undoState.drawMoveCount = drawMoveCount;
		undoState.zobristKey = zobristKey;
		undoState.pawnZobristKey = pawnZobristKey;
		undoState.materialScore = materialScore;
		undoState.positionalScore = positionalScore;
		undoState.endgamePositionalScore = endgamePositionalScore;

		if (move != NullMove)
		{
			drawMoveCount++;

			int source = GetSourceFromMove(move);
			int destination = GetDestFromMove(move);

			int piece = GetPiece(source);
			int destPiece = GetPiece(destination);

			MakeCaptureMove(destination, destPiece);
			MovePiece(source, destination, piece);

			MakePromotionMove(move, source, destination, piece);
			MakeCastleMove(piece, destPiece, source, destination);

			bool isPawn = Pieces::GetPieceTypeFromPiece(piece) == PieceTypes::Pawn;
			MakeEnPassantMove(isPawn, source, destination);
			
			turn = OtherColor(turn);
			zobristKey ^= TranspositionTable::GetTurnKey();

			//assert(!IsKingInCheck(OtherColor(turn)));
			ulong oldKey = TranspositionTable::GetPositionKey(source, piece);
			ulong newKey = TranspositionTable::GetPositionKey(destination, piece);
			ulong diff = oldKey ^ newKey;
			zobristKey ^= diff;

			if (isPawn)
			{
				pawnZobristKey ^= diff;
				drawMoveCount = 0;
			}

			positionalScore -= Evaluation::PieceSquareValue(piece, source);
			positionalScore += Evaluation::PieceSquareValue(piece, destination);

			endgamePositionalScore -= Evaluation::EndgamePieceSquareValue(piece, source);
			endgamePositionalScore += Evaluation::EndgamePieceSquareValue(piece, destination);

			MakeMoveDrawState(updateDrawState);
			turnsPlayed++;
			
			isInCheck = IsKingInCheck(turn);
		}
		else
		{
			// don't allow a null move when in check.
			assert(!GetIsInCheck());
			zobristKey ^= TranspositionTable::GetEnPassantKey(enPassantRights);
			enPassantRights = EnPassantRights::NoEnPassant;
			zobristKey ^= TranspositionTable::GetEnPassantKey(enPassantRights);
			
			turn = OtherColor(turn);
			zobristKey ^= TranspositionTable::GetTurnKey();
		}
	}

	void Board::MakeCaptureMove(int destination, int destPiece)
	{
		if(destPiece != Pieces::None)
		{
			RemovePiece(destination, destPiece);
			capturedPiece = Pieces::GetPieceTypeFromPiece(destPiece);
				
			ulong oldKey = TranspositionTable::GetPositionKey(destination, destPiece);
			zobristKey ^= oldKey;
			if (capturedPiece == PieceTypes::Pawn)
			{
				pawnZobristKey ^= oldKey;
			}
			materialScore -= Evaluation::PieceValue(destPiece);
			positionalScore -= Evaluation::PieceSquareValue(destPiece, destination);
			endgamePositionalScore -= Evaluation::EndgamePieceSquareValue(destPiece, destination);
			drawMoveCount = 0;
		}
		else
		{
			capturedPiece = PieceTypes::None;
		}
	}

	void Board::MakePromotionMove(int move, int source, int destination, int oldPiece)
	{
		int promoType = GetPromoFromMove(move);
		if (promoType != PieceTypes::None)
		{
			RemovePiece(destination, oldPiece);
			int newPiece = Pieces::GetPiece(promoType, turn);
			AddPiece(destination, newPiece);

			ulong oldKey = TranspositionTable::GetPositionKey(destination, oldPiece);
			ulong newKey = TranspositionTable::GetPositionKey(destination, newPiece);
			
			zobristKey ^= oldKey;
			zobristKey ^= newKey;			
			pawnZobristKey ^= oldKey;
			
			positionalScore -= Evaluation::PieceSquareValue(oldPiece, destination);
			positionalScore += Evaluation::PieceSquareValue(newPiece, destination);
			
			endgamePositionalScore -= Evaluation::EndgamePieceSquareValue(oldPiece, destination);
			endgamePositionalScore += Evaluation::EndgamePieceSquareValue(newPiece, destination);
			
			materialScore -= Evaluation::PieceValue(oldPiece);
			materialScore += Evaluation::PieceValue(newPiece);
		}
	}

	void Board::MakeCastleMove(int piece, int destPiece, int source, int destination)
	{
		// Handle castling
		if (Pieces::GetPieceTypeFromPiece(piece) == PieceTypes::King)
		{
			int delta = destination - source;
			if(delta == 2 || delta == -2)
			{
				int rookOrigin = destination + delta / 2;
				if(delta < 0)
				{
					rookOrigin--;
				}
				int rookDestination = destination - delta / 2;
				int rook = Pieces::GetPiece(PieceTypes::Rook, turn);
				MovePiece(rookOrigin, rookDestination, rook);

				ulong oldKey = TranspositionTable::GetPositionKey(rookOrigin, rook);
				ulong newKey = TranspositionTable::GetPositionKey(rookDestination, rook);
				
				zobristKey ^= oldKey;
				zobristKey ^= newKey;			
				
				positionalScore -= Evaluation::PieceSquareValue(rook, rookOrigin);
				positionalScore += Evaluation::PieceSquareValue(rook, rookDestination);

				endgamePositionalScore -= Evaluation::EndgamePieceSquareValue(rook, rookOrigin);
				endgamePositionalScore += Evaluation::EndgamePieceSquareValue(rook, rookDestination);
			}
		}

		if(castleRights.HasRights())
		{
			CastleRights newCastleRights = castleRights;

			// Update castle rights
			if (piece == Pieces::WhiteKing)
			{
				newCastleRights.ClearWhiteKing();
				newCastleRights.ClearWhiteQueen();
			}
			else if(piece == Pieces::BlackKing)
			{
				newCastleRights.ClearBlackKing();
				newCastleRights.ClearBlackQueen();
			}
			else if (piece == Pieces::WhiteRook)
			{
				if (source == MoveGenerator::whiteKingsideRook)
				{
					newCastleRights.ClearWhiteKing();
				}
				else if (source == MoveGenerator::whiteQueensideRook)
				{
					newCastleRights.ClearWhiteQueen();
				}
			}
			else if (piece == Pieces::BlackRook)
			{
				if (source == MoveGenerator::blackKingsideRook)
				{
					newCastleRights.ClearBlackKing();
				}
				else if (source == MoveGenerator::blackQueensideRook)
				{
					newCastleRights.ClearBlackQueen();
				}
			}
			
			// did we capture a rook?
			if (destPiece == Pieces::BlackRook)
			{
				if (destination == MoveGenerator::blackKingsideRook)
				{
					newCastleRights.ClearBlackKing();
				}
				else if (destination == MoveGenerator::blackQueensideRook)
				{
					newCastleRights.ClearBlackQueen();
				}
			}
			else if(destPiece == Pieces::WhiteRook)
			{
				if (destination == MoveGenerator::whiteKingsideRook)
				{
					newCastleRights.ClearWhiteKing();
				}
				else if (destination == MoveGenerator::whiteQueensideRook)
				{
					newCastleRights.ClearWhiteQueen();
				}
			}

			if(newCastleRights.GetRights() != castleRights.GetRights())
			{
				zobristKey ^= TranspositionTable::GetCastleKey(castleRights);
				castleRights = newCastleRights;
				zobristKey ^= TranspositionTable::GetCastleKey(castleRights);
			}
		}
	}

	void Board::MakeEnPassantMove(bool isPawnMove, int source, int destination)
	{
		if (isPawnMove && enPassantRights.CaptureSquare() == destination)
		{
			int enPassantSquare = enPassantRights.PawnSquare();
			int otherPawn = Pieces::GetPiece(PieceTypes::Pawn, OtherColor(turn));
			RemovePiece(enPassantSquare, otherPawn);
			ulong oldKey = TranspositionTable::GetPositionKey(enPassantSquare, otherPawn);
			
			zobristKey ^= oldKey;
			pawnZobristKey ^= oldKey;
			
			materialScore -= Evaluation::PieceValue(otherPawn);
			positionalScore -= Evaluation::PieceSquareValue(otherPawn, enPassantSquare);
			endgamePositionalScore -= Evaluation::EndgamePieceSquareValue(otherPawn, enPassantSquare);
			
			capturedPiece = PieceTypes::Pawn;
		}		

		EnPassantRights newEnPassantRights = EnPassantRights::NoEnPassant;
		// Update en passant rights
		if (isPawnMove)
		{
			int delta = source - destination;
			if(delta == 16 || delta == -16)
			{
				newEnPassantRights = EnPassantRights((source + destination) / 2);
			}
		}

		if(enPassantRights.GetRights() != newEnPassantRights.GetRights())
		{
			zobristKey ^= TranspositionTable::GetEnPassantKey(enPassantRights);
			enPassantRights = newEnPassantRights;
			zobristKey ^= TranspositionTable::GetEnPassantKey(enPassantRights);
		}
	}

	void Board::MakeMoveDrawState(bool updateDrawState)
	{
		int positionCount = 0;
	
		if(updateDrawState)
		{
			positions.push_back(zobristKey);
		}
		size_t index = positions.size() - 1;
		for(unsigned int i=2;i<std::min<size_t>(drawMoveCount, positions.size());i+=2)
		{
			if(positions[index - i] == zobristKey)
			{
				positionCount++;
			}
		}
		positionCount++;
		if (positionCount == 3)
		{
			isDrawnByRepetition = true;
		}
	}

	void Board::UndoMoveDrawState(bool updateDrawState)
	{
		if(updateDrawState)
		{
			positions.pop_back();
		}
		isDrawnByRepetition = false;
	}

	void Board::UndoMove(int move, bool updateDrawState, const BoardState& undoState)
	{
		updateDrawState = true;
		isInCheck = undoState.isInCheck;
		castleRights = undoState.castleRights;
		enPassantRights = undoState.enPassantRights;
		drawMoveCount = undoState.drawMoveCount;
		materialScore = undoState.materialScore;
		positionalScore = undoState.positionalScore;
		endgamePositionalScore = undoState.endgamePositionalScore;

		if (move != NullMove)
		{
			turnsPlayed--;
			UndoMoveDrawState(updateDrawState);

			int source = GetSourceFromMove(move);
			int dest = GetDestFromMove(move);
			int piece = GetPiece(dest);

			MovePiece(dest, source, piece);
			
			if (GetPromoFromMove(move) != PieceTypes::None)
			{
				RemovePiece(source, piece);
				int oldPiece = Pieces::GetPiece(PieceTypes::Pawn, OtherColor(turn));
				AddPiece(source, oldPiece);
			}

			if (capturedPiece != PieceTypes::None)
			{
				int capture = Pieces::GetPiece(capturedPiece, turn);
				// was this an en passant capture? 
				if (enPassantRights.HasEnPassant() && dest == enPassantRights.CaptureSquare() && Pieces::GetPieceTypeFromPiece(piece) == PieceTypes::Pawn)
				{
					AddPiece(enPassantRights.PawnSquare(), capture);
				}
				else
				{
					AddPiece(dest, capture);
				}
			}

			// if this was a castle move, restore the rook
			if (Pieces::GetPieceTypeFromPiece(piece) == PieceTypes::King)
			{
				int delta = dest - source;
				if(delta == 2 || delta == -2)
				{
					int rookOrigin = dest + delta / 2;
					if(delta < 0)
					{
						rookOrigin--;
					}
					int rookDestination = dest - delta / 2;
					int rook = Pieces::GetPiece(PieceTypes::Rook, OtherColor(turn));
					MovePiece(rookDestination, rookOrigin, rook);
				}
			}

			capturedPiece = undoState.capturedPiece;
		}

		turn = OtherColor(turn);
		zobristKey = undoState.zobristKey;
		pawnZobristKey = undoState.pawnZobristKey;
	}

	ulong Board::GetSquareAttackers(int square, int attackerColor) const
	{
		ulong attackPieces = 0;

		ulong attackMask = MoveGenerator::rankAttacks[square][GetRankStatus(square)] | MoveGenerator::fileAttacks[square][GetFileStatus(square)];
		ulong attackers = pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Rook];
		attackPieces |= attackers & attackMask;

		attackMask = MoveGenerator::diagA1H8Attacks[square][GetDiagA1H8Status(square)] | MoveGenerator::diagA8H1Attacks[square][GetDiagA8H1Status(square)];
		attackers = pieceBitBoards[PieceTypes::Queen] | pieceBitBoards[PieceTypes::Bishop];
		attackPieces |= attackers & attackMask;

		attackPieces |= MoveGenerator::knightAttacks[square] & pieceBitBoards[PieceTypes::Knight];
		attackPieces |= MoveGenerator::kingAttacks[square] & pieceBitBoards[PieceTypes::King];
		attackPieces |= MoveGenerator::pawnAttacks[square][attackerColor] & pieceBitBoards[PieceTypes::Pawn];
		return attackPieces & colorBitBoards[attackerColor];
	}

	ulong Board::GetPinRestrictionMask(int square, int color)
	{
		int attackerColor = OtherColor(color);
		int kingSquare = GetKingSquare(color);
		int kingRow = GetRow(kingSquare);
		int kingCol = GetCol(kingSquare);
		int pieceRow = GetRow(square);
		int pieceCol = GetCol(square);
		int dr = pieceRow - kingRow;
		int dc = pieceCol - kingCol;
		ulong defenderKing = 1ULL << kingSquare;
			
		if(dr == 0)
		{
			ulong rankAttacks = MoveGenerator::rankAttacks[square][GetRankStatus(square)];
			ulong defenderKing = 1ULL << kingSquare;
			if((defenderKing & rankAttacks) != 0)
			{
				ulong attackers = GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Rook, attackerColor)) |
					GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, attackerColor));
				if((rankAttacks & attackers) != 0)
				{
					return rankAttacks;
				}
			}
		}
		else if(dc == 0)
		{
			ulong fileAttacks = MoveGenerator::fileAttacks[square][GetFileStatus(square)];
			if((defenderKing & fileAttacks) != 0)
			{
				ulong attackers = GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Rook, attackerColor)) |
					GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, attackerColor));
				if((fileAttacks & attackers) != 0)
				{
					return fileAttacks;
				}
			}
		}
		else if(dr == dc || dr == -dc)
		{
			ulong diagAttacks = 0;
			if(dr == dc)
			{
				diagAttacks = MoveGenerator::diagA1H8Attacks[square][GetDiagA1H8Status(square)];
			}
			else
			{
				diagAttacks = MoveGenerator::diagA8H1Attacks[square][GetDiagA8H1Status(square)];
			}
			if((defenderKing & diagAttacks) != 0)
			{
				ulong attackers = GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Bishop, attackerColor)) |
					GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, attackerColor));
				if((diagAttacks & attackers) != 0)
				{
					return diagAttacks;
				}
			}
		}
		return ~(0ULL);
	}

	bool Board::DoesEnPassantCaptureCauseDiscoveredCheck(int square, int color)
	{
		assert(this->enPassantRights.HasEnPassant());
		int attackerColor = OtherColor(color);
		int kingSquare = GetKingSquare(color);
		int kingRow = GetRow(kingSquare);
		int pieceRow = GetRow(square);
		int dr = pieceRow - kingRow;
		ulong defenderKing = 1ULL << kingSquare;

		if (dr == 0)
		{
			ulong myRankAttacks = MoveGenerator::rankAttacks[square][GetRankStatus(square)];
			ulong theirRankAttacks = MoveGenerator::rankAttacks[enPassantRights.PawnSquare()][GetRankStatus(enPassantRights.PawnSquare())];
			ulong defenderKing = 1ULL << kingSquare;
			if ((defenderKing & myRankAttacks) != 0 || (defenderKing & theirRankAttacks) != 0)
			{
				ulong attackers = GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Rook, attackerColor)) |
					GetPieceBitBoard(Pieces::GetPiece(PieceTypes::Queen, attackerColor));
				if ((myRankAttacks & attackers) != 0 || (theirRankAttacks & attackers) != 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	ulong Board::GetSafeKingMoves(int color)
	{
		ulong safeMoves = 0;	
		int kingSquare = GetKingSquare(color);
		ulong moves = MoveGenerator::kingAttacks[kingSquare];
		ulong newMoves = moves & (~colorBitBoards[color]);
		moves = newMoves;
		if(moves != 0)
		{
			RemovePiece(kingSquare, Pieces::GetPiece(PieceTypes::King, color));
		
			while(moves != 0)
			{
				int move = PopLowestSetBit(moves);
				if(!IsSquareAttacked(move, OtherColor(color)))
				{
					safeMoves |= BitBoard(move);
				}
			}
			AddPiece(kingSquare, Pieces::GetPiece(PieceTypes::King, color));
		}

		return safeMoves; 
	}

	int GetLowestSetBit(ulong mask)
	{
		unsigned long index;
#if X64
		_BitScanForward64(&index, mask);
#else
		unsigned char isNonZero;
		unsigned int lower = (unsigned int)mask;
		isNonZero = _BitScanForward(&index, lower);
		if(!isNonZero)
		{
			unsigned int upper = mask >> 32;
			_BitScanForward(&index, upper);
			index += 32;
		}
#endif
		return index;
	}

	int BitCount(ulong mask)
	{
#if X64POPCOUNT
		return (int)__popcnt64(mask);
#else
		int count = 0;
		while (mask != 0)
		{
			count++;
			mask &= mask - 1;
		}
		return count;
#endif
	}

	int PopLowestSetBit(ulong& mask)
	{
		int ret = GetLowestSetBit(mask);
		mask &= mask - 1;
		return ret;
	}

	bool HasOneBitSet(ulong mask)
	{
		return (mask & (mask - 1)) == 0;
	}
}