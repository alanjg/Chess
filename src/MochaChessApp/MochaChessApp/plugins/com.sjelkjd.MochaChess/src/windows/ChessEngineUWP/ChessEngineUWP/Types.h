#ifndef TYPES_H
#define TYPES_H

using std::ostringstream;
using std::string;
using std::exception;
typedef unsigned long long ulong;
typedef unsigned int uint;
typedef unsigned short uint16;
namespace ChessEngineUWP
{
	class Board;

	struct CastleRights
	{
	private:
		int value;
	public:

		bool GetKing(int color) const;
		bool GetWhiteKing() const;
		void SetWhiteKing();
		void ClearWhiteKing();

		bool GetQueen(int color) const;
		bool GetWhiteQueen() const;
		void SetWhiteQueen();
		void ClearWhiteQueen();
		
		bool GetBlackKing() const;
		void SetBlackKing();
		void ClearBlackKing();
		
		bool GetBlackQueen() const;
		void SetBlackQueen();
		void ClearBlackQueen();

		CastleRights();

		CastleRights(int orights);

		CastleRights(bool whiteKing, bool whiteQueen, bool blackKing, bool blackQueen);

		int GetRights() const;
		bool HasRights() const;

		CastleRights Reverse() const;
	};
	inline int GetRow(int square)
	{
		return square / 8;
	}
	inline int GetCol(int square)
	{
		return square & 7;
	}
	inline int Square(int row, int col)
	{
		return row * 8 + col;
	}

	struct EnPassantRights
	{
		static const int noEnPassant = 64;
	private:
		int rights;

		int Row() const
		{
			 return rights / 8;
		}

		int Col() const
		{
			return rights % 8;
		}

	public:
		static EnPassantRights NoEnPassant;
	
		bool HasEnPassant() const { return rights != noEnPassant; }

		int CaptureSquare() const
		{
			return rights;
		}

		int PawnSquare() const
		{
			int delta = rights < 32 ? 8 : -8;
			return rights + delta;
		}

		ulong BitBoard() const
		{
			return rights == noEnPassant ? 0 : 1ULL << rights;
		}

		EnPassantRights()
		{
			rights = noEnPassant;
		}

		EnPassantRights(int square)
		{
			rights = square;
		}

		EnPassantRights(int row, int col)
		{
			rights = row * 8 + col;
		}
	
		int GetRights() const { return rights; }
	};

	class PieceTypes
	{
	public:
		static const int Pawn = 0;
		static const int Knight = 1;
		static const int Bishop = 2;
		static const int Rook = 3;
		static const int Queen = 4;
		static const int King = 5;
		static const int None = 6;
	};

	class Pieces
	{
	public:
		static const int WhitePawn = 0;
		static const int WhiteKnight = 1;
		static const int WhiteBishop = 2;
		static const int WhiteRook = 3;
		static const int WhiteQueen = 4;
		static const int WhiteKing = 5;
		static const int BlackPawn = 6;
		static const int BlackKnight = 7;
		static const int BlackBishop = 8;
		static const int BlackRook = 9;
		static const int BlackQueen = 10;
		static const int BlackKing = 11;
		static const int None = 12;

		static int GetPiece(int pieceType, int color)
		{
			return color * 6 + pieceType;
		}

		static int GetPieceTypeFromPiece(int piece)
		{
			if (piece == None)
			{
				return PieceTypes::None;
			}
			else
			{
				return piece % 6;
			}
		}

		static int GetColorFromPiece(int piece)
		{
			return piece / 6;
		}
	};

	class Colors
	{
	public:
		static const int White = 0;
		static const int Black = 1;
		static const int None = 2;
	};

	int OtherColor(int color);
	static const int PromoFlag = 1 << 16;
	inline int MakeShortMove(int source, int dest)
	{
		return (dest << 6) + source;
	}
	inline int MakePromoMove(int move, int promotionType)
	{
		return (promotionType << 12) + move + PromoFlag;
	}
	inline int GetSourceFromMove(int move)
	{
		return move & 63;
	}
	inline int GetDestFromMove(int move)
	{
		return (move >> 6) & 63;
	}
	inline int GetPromoFromMove(int move)
	{
		return (move & PromoFlag) != 0 ? ((move ^ PromoFlag) >> 12) : PieceTypes::None;
	}
	inline ulong BitBoard(int square)
	{
		return 1ULL << square;
	}

	inline bool MoveEq(int m1, int m2)
	{
		return (m1 % (1 << 12)) == (m2 % (1 << 12));
	}

	const int NullMove = 1 << 20;
	string GetUCIString(int move);
	string GetShortAlgebraicString(int move, const Board& board);
}
#endif
