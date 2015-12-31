#ifndef BOARD_H
#define BOARD_H

#include "Types.h"
using std::vector;
using std::string;
using std::max;

namespace SjelkjdChessEngine
{
	int GetLowestSetBit(ulong mask);
	int BitCount(ulong mask);
	int PopLowestSetBit(ulong& mask);
	bool HasOneBitSet(ulong mask);

	struct BoardState
	{
		BoardState()
		{
		}

		bool isInCheck;
		int capturedPiece;
		CastleRights castleRights;
		EnPassantRights enPassantRights;
		int drawMoveCount;
		ulong zobristKey;
		ulong pawnZobristKey;
		int positionalScore;
		int endgamePositionalScore;
		int materialScore;
	};

	class Board
	{
		friend class Evaluation;
		friend class StaticExchangeEvaluator;
		friend class MoveGenerator;
		friend class EndgameTableDatabase;

	private:
		// This is faster for checking piece positions than getting it from the bitboards
		int pieces[64];
		ulong occupiedSquares;
		ulong colorBitBoards[2];
		ulong pieceBitBoards[6];

		int turn;
		int turnsPlayed;
		bool isDrawnByRepetition;

		bool isInCheck;
		int capturedPiece;
		CastleRights castleRights;
		EnPassantRights enPassantRights;
		int drawMoveCount;
		ulong zobristKey;
		ulong pawnZobristKey;
		int materialScore;
		int positionalScore;
		int endgamePositionalScore;
		std::vector<ulong> positions;
	public:	
		static string startPosition;
		CastleRights GetCastleRights() { return castleRights; } const
		EnPassantRights GetEnPassantRights() { return enPassantRights; } const
		int Turn() const
		{
			return turn;
		}
		int TurnsPlayed() const
		{
			return turnsPlayed;
		}

		int OtherTurn() const
		{
			return OtherColor(turn);
		}
		
		bool IsKingInCheck(int color) const;

		bool GetIsInCheck() const
		{
			return isInCheck;
		}

		ulong GetSafeKingMoves(int color);
		// returns a bit mask of allowable moves due to a potential pin.  No pin = 0xff...
		ulong GetPinRestrictionMask(int square, int color);

		// returns true if the en passant move is disallowed because removing the piece on the capture square results in discovered check.
		bool DoesEnPassantCaptureCauseDiscoveredCheck(int square, int color);

		int GetPiece(int square) const
		{
			return pieces[square];
		}

		ulong GetPieceBitBoard(int piece) const
		{
			int pieceType = Pieces::GetPieceTypeFromPiece(piece);
			int color = Pieces::GetColorFromPiece(piece);
			return GetPieceTypeBitBoard(pieceType) & colorBitBoards[color];
		}

		ulong GetPieceTypeBitBoard(int pieceType) const
		{
			return pieceBitBoards[pieceType];
		}

		ulong GetColorBitBoard(int color) const
		{
			return colorBitBoards[color];
		}
		private:
		
		// Gets the status of this row, already adjusted to not have the end bits.
		int GetRankStatus(int square) const;
		int GetRankStatus(int square, ulong targets) const;
		int GetDiagA1H8Status(int square) const;
		int GetDiagA1H8Status(int square, ulong targets) const;
		int GetDiagA8H1Status(int square) const;
		int GetDiagA8H1Status(int square, ulong targets) const;
		int GetFileStatus(int square) const;
		int GetFileStatus(int square, ulong targets) const;

		void InitializeZobristKey();
		void ClearBitboards()
		{
			std::fill(pieces, pieces + 64, Pieces::None);
			occupiedSquares = 0;
			std::fill(colorBitBoards, colorBitBoards+2, 0);
			std::fill(pieceBitBoards, pieceBitBoards+6, 0);
			materialScore = 0;
			positionalScore = 0;
			endgamePositionalScore = 0;
		}		

		ulong GetOccupiedSquares() const
		{
			return occupiedSquares;
		}
	
		void UndoMoveDrawState(bool updateDrawState);
		void MakeMoveDrawState(bool updateDrawState);
		void MakeCaptureMove(int destination, int destPiece);
		void MakePromotionMove(int move, int source, int destination, int oldPiece);
		void MakeEnPassantMove(bool isPawnMove, int source, int destination);
		void MakeCastleMove(int piece, int destPiece, int source, int destination);

		void MovePiece(int from, int to, int piece);
		void RemovePiece(int square, int piece);
		void AddPiece(int square, int piece);

	public:
		int GetKingSquare(int color) const;
		bool IsSquareAttacked(int square, int attackerColor) const;
		int GetPositionalScore() const
		{
			return positionalScore;
		}
		
		int GetEndgamePositionalScore() const
		{
			return endgamePositionalScore;
		}

		int GetMaterialScore() const
		{
			return materialScore;
		}

		void SetFEN(const string& fen);
		void SetText(const string& fen, int turn, CastleRights rights);
		void SetEmptyBoard();
		void FinalizeEmptyBoard(int newTurn, CastleRights newCastleRights, EnPassantRights newEnPassantRights);
		string ToString();
			
		ulong GetAttackMask(int square, int attackerType) const;
		ulong GetSquareAttackers(int square, int attackerColor) const;
		//ulong GetPinnedPieces(int square, int pinnedColor) const;

		ulong GetZobristKey() const
		{
			return zobristKey;
		}

		ulong GetPawnZobristKey() const
		{
			return pawnZobristKey;
		}
		
		bool GetIsDrawnByRepetition() const
		{
			return isDrawnByRepetition || drawMoveCount >= 100;
		}

		int GetCapturedPiece() const
		{
			return capturedPiece;
		}
		
		void MakeMove(int move, bool updateDrawState);
		void MakeMove(int move, bool updateDrawState, BoardState& undoState);
		void UndoMove(int move, bool updateDrawState, const BoardState& undoState);
	};
}

#endif