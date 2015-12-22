#include "pch.h"
using namespace std;

#include "Types.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "EndgameTable.h"
#include "Tests.h"
#include "Search.h"
#include "Evaluation.h"

using namespace ChessEngineUWP;

void PlayUCI()
{
	Board board;
	const int minHashSize = 1;
	int hashSize = 32;
	Search search(board);
	MoveGenerator generator(board);
	bool initializedHashTable = false;
	for (; ; )
	{
		string line;
		getline(cin, line);
		if (!cin)
		{
			break;
		}
		vector<string> arguments;
		istringstream tokenizer(line);
		string token;
		while(tokenizer >> token)
		{
			arguments.push_back(token);
		}
		string command = arguments[0];
		if (command == "uci")
		{
			cout << "id name MochaChess" << endl;
			cout << "id author Alan Gasperini"<< endl;
			cout << "option name Hash type spin default 32 min 1 max 1000" << endl;
			cout << "uciok" << endl;
		}
		else if (command == "isready")
		{
			cout << "readyok" << endl;
		}
		else if(command == "setoption")
		{
			if(arguments.size() == 5)
			{
				if(arguments[1] == "name" && arguments[3] == "value")
				{
					if(arguments[2] == "Hash")
					{
						istringstream str(arguments[4]);
						int hashSizeInput;
						str >> hashSizeInput;
						if(hashSizeInput >= TranspositionTable::minHashSize && hashSizeInput <= TranspositionTable::maxHashSize)
						{
							hashSize = hashSizeInput;
						}
					}
				}
			}
		}
		else if (command == "position")
		{
			uint at;
			if (arguments[1] == "startpos")
			{
				board.SetFEN(Board::startPosition);
				at = 2;
			}
			else if (arguments[1] == "fen")
			{
				// Fen input
				string fen = "";
				for (int i = 2; i <= 7; i++)
				{
					fen += arguments[i];
					if (i != 1) fen += " ";
				}
				board.SetFEN(fen);
				at = 8;
			}
			else
			{
				throw exception("arguments should equal fen");
			}

			if (at < arguments.size())
			{
				if (arguments[at] != "moves")
				{
					throw exception("don't understand fen");
				}
				at++;
			}
			
			while (at < arguments.size())
			{
				bool madeMove = false;
				vector<int> moves;
				generator.GenerateAllMoves(moves);
				for(uint i=0;i<moves.size();i++)
				{
					int move = moves[i];
					if (GetUCIString(move) == arguments[at])
					{
						board.MakeMove(move, true);
						madeMove = true;
						break;
					}
				}
				if (!madeMove)
				{
					string message = "Illegal move passed in: ";
					message += arguments[at];
					throw exception(message.c_str());
				}
				at++;
			}
		}
		else if (command == "go")
		{
			if(!initializedHashTable)
			{
				initializedHashTable = true;
				search.SetHashTableSize(hashSize);
			}
			int wtime = -1, btime = -1, winc = 0, binc = 0;
			int depth = -1;
			int movetime = -1;
			for (uint i = 1; i + 1 < arguments.size(); )
			{
				string arg = arguments[i++];
				string argval = arguments[i++];
				istringstream argin(argval);
				if(arg == "wtime")
				{
					argin >> wtime;
				} 
				else if(arg == "btime")
				{
					argin >> btime;
				}
				else if(arg == "winc")
				{
					argin >> winc;
				}
				else if(arg == "binc")
				{
					argin >> binc;
				}
				else if(arg == "depth")
				{
					argin >> depth;
				}
				else if(arg == "movetime")
				{
					argin >> movetime;
				}
			}

			if (movetime == -1)
			{
				if (board.Turn() == Colors::Black) movetime = btime + binc*40; else movetime = wtime + winc*40;
				if (movetime == -1)
				{
					movetime = 10000;
				}
				else
				{	
					// this is how much time we have left in the game, search for 1/40th of this time
					movetime = movetime / 40;
				}
			}

			int move = search.GetBestMove(movetime/1000.0);
			if (move == 0)
			{
				// we have a checkmate or a stalemate, send a null move
				cout << "bestmove 0000" << endl;
			}
			else
			{
				cout << "bestmove " << GetUCIString(move) << endl;
			}
		}
	}
}

int main(int argc,char** argv)
{
	srand(123456789);
	Evaluation::Initialize();
	bool testMode = false;
	if(argc == 2)
	{
		string arg = argv[1];
		if(arg == "/Test")
		{
			testMode = true;
		}
	}
	if(!testMode)
	{
		PlayUCI();
	}
	else
	{
		//PerfTHard();
		//TestPinMask();
	//	EndgameTableDatabase b;
	//	b.GenerateTables();
	//	RunUnitTests();
		//MissedQueenLoss();
		//BadQueenAttack();
		//SacrificeBishop();
		//AllowedDraw();
		//PawnPushTest();
		//EvalTest();
		//DrawByRepetitionCheck();
		//TwoKingsTest();
		//TestMoveScores();
		//return 0;
	//	TestTranspositionCompression();
		//TestRookEndgame();
		//TestQueenEndgame();
		//TestMoveUndo();
		//PerfTTests();
	//	TestInCheckMoveCount();
		//TestMoveKey();
		
		//BoardCorruption();

		// 4088/6541 with 2 seconds to search(Jan 5)
		// 4251/6541 with 2 seconds to search(Aug 2013)
		//BestMoveCheck(1, 60);

		//BestMoveCheckPositional();
		//EndgameTablebaseTest();
		//TestQSearch();
		//PlayGameDebug();
		//TestPawnValues();
		//Benchmark();
		//TestDiscoveredCheck();
		//return 0;
		
		PlaySelf();
	}	
}