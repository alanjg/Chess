#include "pch.h"
#include "Types.h"
#include "Board.h"
namespace SjelkjdChessEngine
{

	bool CastleRights::GetKing(int color) const
	{
		return (value & (color == Colors::White ? 1 : 4)) != 0;
	}

	bool CastleRights::GetWhiteKing() const
	{
		return (value & 1) != 0;
	}

	void CastleRights::SetWhiteKing()
	{
		value |= 1;
	}
	
	void CastleRights::ClearWhiteKing()
	{
		value &= ~1;
	}

	bool CastleRights::GetQueen(int color) const
	{
		return (value & (color == Colors::White ? 2 : 8)) != 0;
	}

	bool CastleRights::GetWhiteQueen() const
	{
		return (value & 2) != 0;
	}

	void CastleRights::SetWhiteQueen()
	{
		value |= 2;
	}
	
	void CastleRights::ClearWhiteQueen()
	{
		value &= ~2;
	}

	bool CastleRights::GetBlackKing() const
	{
		return (value & 4) != 0;
	}

	void CastleRights::SetBlackKing()
	{
		value |= 4;
	}
	
	void CastleRights::ClearBlackKing()
	{
		value &= ~4;
	}

	bool CastleRights::GetBlackQueen() const
	{
		return (value & 8) != 0;
	}

	void CastleRights::SetBlackQueen()
	{
		value |= 8;
	}
	
	void CastleRights::ClearBlackQueen()
	{
		value &= ~8;
	}
	
	CastleRights::CastleRights()
	{
	}

	CastleRights::CastleRights(int orights)
	{
		value = orights;
	}

	CastleRights::CastleRights(bool whiteKing, bool whiteQueen, bool blackKing, bool blackQueen)
	{
		value = ((whiteKing ? 1 : 0) | (whiteQueen ? 2 : 0) | (blackKing ? 4 : 0) | (blackQueen ? 8 : 0));
	}

	int CastleRights::GetRights() const { return value; }
	bool CastleRights::HasRights() const { return value != 0; }
	int OtherColor(int color)
	{
		return 1 - color;
	}

	CastleRights CastleRights::Reverse() const
	{
		return CastleRights(GetBlackKing(), GetBlackQueen(), GetWhiteKing(), GetWhiteQueen());
	}

	EnPassantRights EnPassantRights::NoEnPassant;

string GetUCIString(int move)
{
	int source = GetSourceFromMove(move);
	int dest = GetDestFromMove(move);
	ostringstream moveStr;
	moveStr << (char)('a' + GetCol(source)) << (char)('1' + GetRow(source)) << (char)('a' + GetCol(dest)) << (char)('1' + GetRow(dest));
	if (GetPromoFromMove(move) != PieceTypes::None)
	{
		char promo = 'Q';
		switch (GetPromoFromMove(move))
		{
		case PieceTypes::Bishop: promo = 'b'; break;
		case PieceTypes::Knight: promo = 'n'; break;
		case PieceTypes::Queen: promo = 'q'; break;
		case PieceTypes::Rook: promo = 'r'; break;
			default: throw new exception("Invalid promotion choice."); break;
		}
		moveStr << promo;
	}
	return moveStr.str();
}

string GetShortAlgebraicString(int move, const Board& board)
{
	ostringstream moveStr;
	int source = board.GetPiece(GetSourceFromMove(move));
	int dest = board.GetPiece(GetDestFromMove(move));
	
	switch (Pieces::GetPieceTypeFromPiece(source))
	{
	case PieceTypes::King: 
		if(abs(GetSourceFromMove(move) - GetDestFromMove(move)) == 2)
		{
			if(GetCol(GetDestFromMove(move)) == 2) return "0-0-0";
			else return "0-0";
		}
		moveStr << "K"; break;
	case PieceTypes::Queen: moveStr << "Q"; break;
	case PieceTypes::Rook: moveStr << "R"; break;
	case PieceTypes::Bishop: moveStr << "B"; break;
	case PieceTypes::Knight: moveStr << "N"; break;
	}
	if (dest != Pieces::None)
	{
		if(Pieces::GetPieceTypeFromPiece(source) == PieceTypes::Pawn)
		{
			moveStr << (char)('a' + GetCol(GetSourceFromMove(move))); 
		}
		moveStr << "x";
	}
	moveStr << (char) ('a' + GetCol(GetDestFromMove(move)));
	moveStr << (char) ('1' + GetRow(GetDestFromMove(move)));

	if (GetPromoFromMove(move) != PieceTypes::None)
	{
		char promo = 'Q';
		switch (GetPromoFromMove(move))
		{
		case PieceTypes::Bishop: promo = 'b'; break;
		case PieceTypes::Knight: promo = 'n'; break;
		case PieceTypes::Queen: promo = 'q'; break;
		case PieceTypes::Rook: promo = 'r'; break;
		}
		moveStr << "=" << promo;
	}

	return moveStr.str();
}
}