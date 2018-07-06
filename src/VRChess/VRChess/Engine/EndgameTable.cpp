#include "StdAfx.h"
#include "EndgameTable.h"
#include "MoveGenerator.h"
#include "Evaluation.h"
#include "TranspositionTable.h"
using namespace std;

namespace SjelkjdChessEngine
{

uint16 EndgameTableEntry::Compress()
{
	uint16 compressed = 0;
	compressed |= result;
	compressed |= bestMoveIndex << 8;
	compressed |= checkmateDistance & ((1 << 8) - 1);
	return result;
}

EndgameTableEntry::EndgameTableEntry()
{
	result = Unknown;
}

EndgameTableEntry::EndgameTableEntry(uint16 compressed)
{
	result = (EndgameTableEntryResult)(compressed >> 14);

	compressed &= ((1 << 14) - 1);
	bestMoveIndex = compressed >> 8;

	compressed &= ((1 << 8) - 1);
	checkmateDistance = compressed;
}

EndgameTableEntry EndgameTableEntry::Reverse() const
{
	EndgameTableEntry reversed;
	reversed.bestMoveIndex = bestMoveIndex;
	reversed.checkmateDistance = checkmateDistance;
	switch(result)
	{
		case Unknown: reversed.result = Unknown; break;
		case Draw: reversed.result = Draw; break;
		case Win: reversed.result = Loss; break;
		case Loss: reversed.result = Win; break;
	}
	return reversed;
}

const int Max21Size = 1 << 19;
int Generate21Index(int whiteKing, int blackKing, int pieceLocation, int turn)
{
	int index = 0;
	index += whiteKing;
	index <<= 6;
	index += blackKing;
	index <<= 6;
	index += pieceLocation;
	index <<= 1;
	index += turn;
	return index;
}

const int Max22Size = 1 << 26;
int Generate22Index(int whiteKing, int blackKing, int whitePieceLocation, int blackPieceLocation, int turn, bool canEnPassant)
{
	int index = canEnPassant ? 1 : 0;
	index <<= 6;
	index += whiteKing;
	index <<= 6;
	index += blackKing;
	index <<= 6;
	index += whitePieceLocation;
	index <<= 6;
	index += blackPieceLocation;	
	index <<= 1;
	index += turn;
	return index;
}

const int Max31Size = 1 << 25;
int Generate31Index(int whiteKing, int blackKing, int piece1Location, int piece2Location, int turn)
{
	int index = 0;
	index += whiteKing;
	index <<= 6;
	index += blackKing;
	index <<= 6;
	index += piece1Location;
	index <<= 6;
	index += piece2Location;
	index <<= 1;
	index += turn;
	return index;
}

EndgameTableEntry EndgameTableDatabase::GetSetTableEntry(Board& board, EndgameTableEntry* entryToSet)
{
	ulong whiteBits = board.GetColorBitBoard(Colors::White);
	ulong blackBits = board.GetColorBitBoard(Colors::Black);

	int whiteCount = BitCount(whiteBits);
	int blackCount = BitCount(blackBits);

	ulong whiteKingBits = board.GetPieceBitBoard(Pieces::WhiteKing);
	int whiteKing = GetLowestSetBit(whiteKingBits);
	ulong blackKingBits = board.GetPieceBitBoard(Pieces::BlackKing);
	int blackKing = GetLowestSetBit(blackKingBits);

	if(whiteCount + blackCount == 3)
	{
		int pieceLocation = GetLowestSetBit((whiteBits | blackBits) ^ whiteKingBits ^ blackKingBits);
		int pieceType = Pieces::GetPieceTypeFromPiece(board.GetPiece(pieceLocation));
		CastleRights castleRights = board.GetCastleRights();
		int turn = board.Turn();

		bool reverseResult = whiteCount == 2;
		if(reverseResult)
		{
			turn = OtherColor(turn);
			castleRights = castleRights.Reverse();
			std::swap(whiteKing, blackKing);
		}
		int index = Generate21Index(whiteKing, blackKing, pieceLocation, turn);
		uint16* table = endgameTables21[pieceType];
		if(entryToSet)
		{
			EndgameTableEntry entry = *entryToSet;
			if(reverseResult)
			{
				entry = entry.Reverse();
			}
			table[index] = entry.Compress();
			return entry;
		}
		else
		{
			EndgameTableEntry result(table[index]);
		
			if(reverseResult)
			{
				result = result.Reverse();
			}
			return result;
		}
	}
	else if(whiteCount + blackCount == 4)
	{
		if(whiteCount == 2)
		{
			int whitePieceLocation = GetLowestSetBit(whiteBits ^ whiteKingBits);
			int whitePieceType = Pieces::GetPieceTypeFromPiece(board.GetPiece(whitePieceLocation));
			int blackPieceLocation = GetLowestSetBit(blackBits ^ blackKingBits);
			int blackPieceType = Pieces::GetPieceTypeFromPiece(board.GetPiece(blackPieceLocation));
			CastleRights castleRights = board.GetCastleRights();
			int turn = board.Turn();

			bool reverseResult = false;
			if(reverseResult)
			{
				turn = OtherColor(turn);
				castleRights = castleRights.Reverse();
				std::swap(whiteKing, blackKing);
				std::swap(whitePieceType, blackPieceType);
				std::swap(whitePieceLocation, blackPieceLocation);
			}
			bool canEnPassant = board.GetEnPassantRights().HasEnPassant() && false;  ///!!!!!!!!!!!

			int index = Generate22Index(whiteKing, blackKing, whitePieceLocation, blackPieceLocation, turn, canEnPassant);
			uint16* table = endgameTables22[whitePieceType][blackPieceType];
			if(entryToSet)
			{
				EndgameTableEntry entry = *entryToSet;
				if(reverseResult)
				{
					entry = entry.Reverse();
				}
				
				table[index] = entry.Compress();
				return entry;
			}
			else
			{
				EndgameTableEntry result(table[index]);
			
				if(reverseResult)
				{
					result = result.Reverse();
				}
				return result;
			}
		}
		else
		{
			CastleRights castleRights = board.GetCastleRights();
			int turn = board.Turn();

			bool reverseResult = whiteCount == 1;
			if(reverseResult)
			{
				turn = OtherColor(turn);
				castleRights = castleRights.Reverse();
				std::swap(whiteKing, blackKing);
				std::swap(whiteBits, blackBits);
				std::swap(whiteKingBits, blackKingBits);
			}
			int piece1Location = GetLowestSetBit(whiteBits ^ whiteKingBits);
			int piece1Type = Pieces::GetPieceTypeFromPiece(board.GetPiece(piece1Location));
			int piece2Location = GetLowestSetBit(blackBits ^ blackKingBits);
			int piece2Type = Pieces::GetPieceTypeFromPiece(board.GetPiece(piece2Location));
			
			int index = Generate31Index(whiteKing, blackKing, piece1Location, piece2Location, turn);
			uint16* table = endgameTables31[piece1Type][piece2Type];
			if(entryToSet)
			{
				EndgameTableEntry entry = *entryToSet;
				if(reverseResult)
				{
					entry = entry.Reverse();
				}
				
				table[index] = entry.Compress();
				return entry;
			}
			else
			{
				EndgameTableEntry result(table[index]);
			
				if(reverseResult)
				{
					result = result.Reverse();
				}
				return result;
			}
		}
	}
	else
	{
		return EndgameTableEntry();
	}
}

EndgameTableDatabase::EndgameTableDatabase()
{
	endgameTables21 = new uint16*[5];
	for(int i=0;i<5;i++)
	{
		endgameTables21[i] = 0;
	}
	endgameTables22 = new uint16**[5];
	endgameTables31 = new uint16**[5];
	for(int i=0;i<5;i++)
	{
		endgameTables22[i] = new uint16*[5];
		endgameTables31[i] = new uint16*[5];
		for(int j=0;j<5;j++)
		{
			endgameTables22[i][j] = 0;
			endgameTables31[i][j] = 0;
		}
	}
}

EndgameTableDatabase::~EndgameTableDatabase()
{
	for(int i=0;i<5;i++)
	{
		for(int j=0;j<5;j++)
		{
			delete [] endgameTables22[i][j];
			delete [] endgameTables31[i][j];
		}
		
		delete [] endgameTables21[i];
		delete [] endgameTables22[i];
		delete [] endgameTables31[i];
	}

	delete [] endgameTables21;
	delete [] endgameTables22;
	delete [] endgameTables31;	
}

void EndgameTableDatabase::GenerateTables()
{
	GenerateTables21();
}

void EndgameTableDatabase::GenerateTables21()
{
	TranspositionTable t;
	for(int i=0;i<5;i++)
	{
		endgameTables21[i] = new uint16[Max21Size];
		
		fill(endgameTables21[i], endgameTables21[i] + Max21Size, 0);
		for(int j=i;j<5;j++)
		{
	//		endgameTables22[i][j] = new uint16[Max22Size];
	//		endgameTables31[i][j] = new uint16[Max31Size];
		}
	}
	EnPassantRights enPassantRights(0);
	CastleRights castleRights(false, false, false, false);
	Board board;
	board.SetEmptyBoard();
	for(int turn=Colors::White;turn <= Colors::Black;turn++)
	{
		for(int whiteKing=0;whiteKing<64;whiteKing++)
		{
			int whiteKingRow = whiteKing / 8;
			int whiteKingCol = whiteKing % 8;
			for(int blackKing=0;blackKing<64;blackKing++)
			{
				if(whiteKing != blackKing)
				{
					int blackKingRow = blackKing / 8;
					int blackKingCol = blackKing % 8;

					if (abs(whiteKingRow-blackKingRow) <= 1 && abs(whiteKingCol-blackKingCol) <= 1)
					{
						continue;
					}

					for(int whitePieceType = Pieces::WhitePawn; whitePieceType != Pieces::WhiteKing; whitePieceType++)
					{
						for(int whitePiece=0;whitePiece<64;whitePiece++)
						{
							if(whiteKing != whitePiece && blackKing != whitePiece)
							{
								int whitePieceRow = whitePiece / 8;
								int whitePieceCol = whitePiece % 8;

								// pawns cannot be on the first or last row.
								if(whitePieceType == Pieces::WhitePawn && (whitePieceRow == 0 || whitePieceRow == 7))
								{
									continue;
								}								
								
								board.SetEmptyBoard();
								board.AddPiece(whiteKing, Pieces::WhiteKing);
								board.AddPiece(blackKing, Pieces::BlackKing);
								board.AddPiece(whitePiece, whitePieceType);
								board.FinalizeEmptyBoard(turn, castleRights, enPassantRights);

								// If the player not to move is in check, this board is illegal.
								if(!board.IsKingInCheck(OtherColor(turn)))
								{
									EndgameTableEntry result = MiniMax(board, 0);
									cout << board.ToString() << endl;
									cout << (result.result == Win ? "Win" : (result.result == Draw ? "Draw" : "Loss")) << endl;
								}								
							}
						}
					}
				}
			}
		}
	}
}
void EndgameTableDatabase::EnsureMoveListDepth(uint depth)
{
	while(depth >= moveLists.size())
	{
		vector<int>* newList = new vector<int>();
		newList->reserve(50);
		moveLists.push_back(newList);
	}
}

EndgameTableEntry EndgameTableDatabase::MiniMax(Board& board, int depth)
{
	EndgameTableEntry entry = GetSetTableEntry(board, 0);
	if(entry.result != Unknown)
	{
		return entry;
	}
	
	EndgameTableEntry placeHolder;
	placeHolder.result = Draw;
	GetSetTableEntry(board, &placeHolder);

	bool madeLegalMove = false;
	// check for win/loss/draw conditions.
	bool isDraw = /*board.GetIsDrawnByRepetition() ||*/ Evaluation::IsDrawByMaterial(board);

	EndgameTableEntry bestResult;
	if(!isDraw)
	{
		MoveGenerator mg(board);
		EnsureMoveListDepth(depth);
		vector<int>& moves = *moveLists[depth];
		moves.clear();
		mg.GenerateAllMoves(moves);
		for(unsigned int i=0;i<moves.size();i++)
		{
			BoardState undoState;
			board.MakeMove(moves[i], true, undoState);
			EndgameTableEntry entry = MiniMax(board, depth+1);
			if(bestResult.result == Unknown ||
				bestResult.result == Win && (entry.result == Draw || entry.result == Loss) ||
				bestResult.result == Win && entry.result == Win && bestResult.checkmateDistance < entry.checkmateDistance ||
				bestResult.result == Draw && entry.result == Loss ||
				bestResult.result == Loss && entry.result == Loss && bestResult.checkmateDistance > entry.checkmateDistance)
			{
				EndgameTableEntry newBestResult;
				switch(entry.result)
				{
					case Win: newBestResult.result = Loss; break;
					case Draw: newBestResult.result = Draw; break;
					case Loss: newBestResult.result = Win; break;
				}
				newBestResult.bestMoveIndex = i;
				newBestResult.checkmateDistance = entry.result == Draw ? 0 : entry.bestMoveIndex + 1;
				bestResult = newBestResult;
			}
			
			board.UndoMove(moves[i], true, undoState);
		}
	}
	bool isLoss = false;
	if(!madeLegalMove && !isDraw)
	{
		if(board.GetIsInCheck())
		{
			isLoss = true;
		}
		else
		{
			isDraw = true;
		}
	}
	if(isLoss)
	{
		EndgameTableEntry entry;
		entry.bestMoveIndex = 0;
		entry.checkmateDistance = 0;
		entry.result = Loss;
		bestResult = entry;
	}
	else if(isDraw)
	{
		EndgameTableEntry entry;
		entry.bestMoveIndex = 0;
		entry.checkmateDistance = 0;
		entry.result = Draw;
		bestResult = entry;
	}
	
	GetSetTableEntry(board, &bestResult);

	cout << board.ToString() << endl;
	cout << (bestResult.result == Win ? "Win" : (bestResult.result == Draw ? "Draw" : "Loss")) << endl;
	return bestResult;
}

void EndgameTableDatabase::LoadTables(string file)
{

}

void EndgameTableDatabase::SaveTables(string file)
{

}

}