#include "pch.h"
#include "Tests.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "Board.h"
#include "StaticExchangeEvaluator.h"
using namespace std;

namespace SjelkjdChessEngine
{

void PerfTTests()
{
	ifstream infile("Resources\\PerfT2.txt");
	string test;
	clock_t start = clock();
	int num = 0;
	ulong totalMoves = 0;
	getline(infile, test);
	Board board;
	Search search(board);
	int failureCount = 0;
	while(infile)
	{
		// format is rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ;D1 20 ;D2 400 ;D3 8902 ;D4 197281 ;D5 4865609 ;D6 119060324
		
		string fen = test.substr(0, test.find(';'));
		string rest = test.substr(test.find(';') + 1);
		istringstream inmoves(rest);
		
		board.SetFEN(fen);
		string moveCount;
		getline(inmoves, moveCount, ';');
		
		while(inmoves)
		{
			istringstream parts(moveCount);
			char first;
			int depth;
			ulong expected;
			parts >> first >> depth >> expected;
			
			if (depth < 7) 
			{
				ulong count = search.GetMoveCount(depth);
				if (count != expected)
				{
					cout << "PerfT test " << num << " failed at depth " << depth << ".  Expected " << expected << ", got " << count << endl;
					failureCount++;
				}
				else
				{
		//			cout << "PerfT test " << num << " completed depth " << depth << ".  Found " << count << " moves." << endl;
				}
				totalMoves += count;
			}
			getline(inmoves, moveCount, ';');
		}
		num++;
		
		getline(infile, test);
	}
	clock_t end = clock();
	double elapsed = end - start;
	elapsed /= CLOCKS_PER_SEC;
	cout << "All PerfT tests are finished." << endl;
	cout << "Elapsed time:\t" << elapsed << endl;
	cout << "Total moves:\t" << totalMoves << endl;
	cout << "Moves/sec:\t" << (totalMoves / elapsed) << endl;
	if(failureCount > 0)
	{
		cout << "Failed " << failureCount << " tests." << endl;
	}
	else
	{
		cout << "All tests passed." << endl;
	}
}

void PerfTHard()
{
	string positions [] = {
		"r3k2r/8/8/8/3pPp2/8/8/R3K1RR b KQkq e3 0 1",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		"8/7p/p5pb/4k3/P1pPn3/8/P5PP/1rB2RK1 b - d3 0 28",
		"8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - -1 67",
		"rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
		"8/p7/8/1P6/K1k3p1/6P1/7P/8 w - -",
		"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - -",
		"r3k2r/p6p/8/B7/1pp1p3/3b4/P6P/R3K2R w KQkq -",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
		"8/5p2/8/2k3P1/p3K3/8/1P6/8 b - -",
		"r3k2r/pb3p2/5npp/n2p4/1p1PPB2/6P1/P2N1PBP/R3K2R w KQkq -"
	};
	ulong expected [] = {
		485647607,
706045033,
38633283,
493407574,
244063299,
193690690,
8103790,
71179139,
77054993,
178633661,
64451405,
29179893
	};
	int depth [] = {
6,6,6,7,6,5,8,6,6,7,8,5
	};

	string test;
	clock_t start = clock();
	int num = 0;
	ulong totalMoves = 0;
	Board board;
	Search search(board);
	int failureCount = 0;
	for (int i = 0; i < 12; i++)
	{
		// format is rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ;D1 20 ;D2 400 ;D3 8902 ;D4 197281 ;D5 4865609 ;D6 119060324
		board.SetFEN(positions[i]);
		ulong count = search.GetMoveCount(depth[i]);
		if (count != expected[i])
		{
			cout << "PerfT test " << num << " failed at depth " << depth[i] << ".  Expected " << expected[i] << ", got " << count << endl;
			failureCount++;
		}
		else
		{
			cout << "PerfT test " << num << " completed depth " << depth[i] << ".  Found " << count << " moves." << endl;
		}
		totalMoves += count;		
		num++;
	}
	clock_t end = clock();
	double elapsed = end - start;
	elapsed /= CLOCKS_PER_SEC;
	cout << "All PerfT tests are finished." << endl;
	cout << "Elapsed time:\t" << elapsed << endl;
	cout << "Total moves:\t" << totalMoves << endl;
	cout << "Moves/sec:\t" << (totalMoves / elapsed) << endl;
	if (failureCount > 0)
	{
		cout << "Failed " << failureCount << " tests." << endl;
	}
	else
	{
		cout << "All tests passed." << endl;
	}
}

void PlaySelf()
{
	Board board;
	Search search(board);
	board.SetFEN(Board::startPosition);
	cout << board.ToString() << endl;
	while(!board.GetIsDrawnByRepetition())
	{
		int move = search.GetBestMove(6.0);
		board.MakeMove(move, true);
		cout << board.ToString() << endl;		
	}
}

void TestRookEndgame()
{
	Board board;
	board.SetFEN("8/8/8/8/3k4/8/8/4K2R w - - 0 1");
	Search search(board);
	cout << board.ToString() << endl;
	for (; ; )
	{
		int move = search.GetBestMove(5);		
		board.MakeMove(move, true);
		cout << board.ToString() << endl;
	}
}

void TestQueenEndgame()
{
	Board board;
	board.SetFEN("k7/4K3/8/8/8/8/8/1Q6 w - - 0 1");
	Search search(board);
	cout << board.ToString() << endl;
	for (; ; )
	{
		int move = search.GetBestMove(1);
		
		board.MakeMove(move, true);
		cout << board.ToString() << endl;
	}
}

void TestInCheckMoveCount()
{
	string fen = "rn1qkbnr/pppBpppp/8/3p4/4P3/8/PPPP1PPP/RNBQK1NR b - - 0 1";

	Board board;
	board.SetFEN(fen);
	MoveGenerator gen(board);
	int moveCount = 0;
	vector<int> moves;
	gen.GenerateAllMoves(moves);
	for(uint i=0;i<moves.size();i++)
	{
		BoardState undo;
		board.MakeMove(moves[i], false, undo);
		moveCount++;
		board.UndoMove(moves[i], false, undo);
	}
	if(moveCount != 3)
	{
		cout << "Bad move count in check: " << moveCount << endl;
	}
}

void IllegalMoveCheck()
{
	Board board;
	board.SetFEN("r1bqkbnr/ppp1pppp/8/3p4/3nP3/2N5/PPP2PPP/R1BQKBNR b - - 0 1");
	MoveGenerator gen(board);
	
	vector<int> mo;
	gen.GenerateAllMoves(mo);
	
	for(unsigned int i=0;i<mo.size();i++)
	{
		//cout << mo[i].GetUCIString() << endl;
		if(GetUCIString(mo[i]) == "c6d4")
		{
			cout << "Illegal move" << GetUCIString(mo[i]) << endl;
		}
	}
}

void TwoKingsTest()
{
	string moves = "e2e4 d7d5 d2d4 d5e4 a2a4 b8c6 c1e3 g8f6 h2h4 e7e5 h1h3 c8h3 g2h3 e5d4 e3f4 f8b4 b1d2 f6d5 c2c3";
	
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
	search.GetBestMove(10000);
}

void BadMoveCheck()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "b1c3 d7d5 d2d4 g8f6 d1d3 e7e6 g1f3 b8c6 c1g5 h7h6";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
}

void BadMoveCheck3()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "d4 d5 c4 dxc4 Nf3 Nf6 Qa4 Nc6 Nc3 Bg4 Ne5 Bd7 Qxc4 Nxe5 dxe5 Ng4 Bf4 Be6 Qb4 Qb8 Rd1 c6 h3 Nxe5 Bxe5 Qxe5 Qxb7 Rc8 Qxa7 g6 e3 Bh6 Be2 Kf8 f4 Qb8 Qd4 Bg7 Qd2 Kg8 Ba6 Re8 Be2 Qb7 a3 Rb8 b4 c5 Bf3 Qa7 Rc1 Qxa3 bxc5 Qxc5 0-0 Qa5 f1d1 Rc8 Nd5 Qxd2 Rxc8 Bxc8 Rxd2 Bf8 Rc2 Ba6 Rc7 e6 Nf6 Kg7 Ne8 Kg8 g4 Bb4 g5 h5 Be4 h4 Kf2 Kf8 Ra7 Bc8 Ra8 Ke7 Rxc8 Rxe8 Rc7 Kf8 Rb7 Bc3 Rc7 Bg7 Bc6 Rb8 Kf3 Rb3 Kg4 Rxe3 Kxh4 Bc3 Kg4 f5 g5f6";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
}

void BadMoveCheck4()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "Nf3 d5 g3 e6 d4 Nf6 Bf4 Ne4 b1d2 Nxd2 Qxd2 Nc6\
 e3 Be7 Ne5 Nxe5 Bxe5 f6 Bf4 g5 e4 gxf4 Qxf4 Qd6 Qxd6\
 Bxd6 Bd3 Bd7 0-0-0 Bc6 d1e1 0-0-0 f4 dxe4 Bxe4 Bb5 a3 Bc4\
 Re3 c6 h1e1 f5 Bd3 Ba2 Rxe6 Bxe6 Rxe6 h8f8 Rh6 Rd7 Bc4\
 Kc7 c3 b5 Bd3 a5 Rh5 d7f7 Rh6 a4 c4 bxc4 Bxc4 Rg7\
 Be6 c5 d5 c4 Rh5 Kb6 Bxf5 f8f7 Kb1 Kc5 b4 a4b3";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			cout << board.ToString() << endl;
			break;
		}
		board.MakeMove(move, true);
	}
}


void BadMoveCheck2()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "e4 c5 Nf3 Nc6 d4 cxd4 Nxd4 Nf6 Nc3 e5 d4b5 d6\
 Bg5 a6 Na3 b5 Nd5 Be7 Nxe7 Qxe7 f3 0-0 c3 h6 Be3\
 Be6 h3 d5 Bd3 dxe4 fxe4 Nxe4 0-0 Nxc3 Qc1 b4 bxc3 bxa3\
 Be4 a8c8 Bd3 Nb8 Qc2 f5 Bxf5 Bxf5 Rxf5 Rxf5 Qxf5 Rxc3 Bd2\
 Rc6 Rb1 Qa7 Kh1 Nd7 Rd1 Qd4 Qf3 Qa4 Bxh6 Rxh6 Qd5 Kf8\
 Qxd7 Qxd7 Rxd7 e4 Rd8 Ke7 Rb8 Rd6 Rb7 Kf8 Rb3 Rd3 Rb8\
 Kf7 Rb7 Kg8 Re7 e3 Re8 Kf7 Re4 g5 h4 gxh4 Kh2 h3\
 gxh3 Rd2 Kg3 e2 Kf3 Rxa2 Rf4 Ke6 Kf2 Kd5 Ra4 Ke5 Rxa6\
 Kf5 Ra5 Ke6 h4 Kf6 h5 Kg7 Ra6 Kg8 Ra7 Rb2 Rxa3 Kg7\
 Ra7 Kh6 Re7 Kxh5 Rxe2 Rxe2 Kxe2 Kg6";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}

	search.SuppressLogging();
	int move = search.GetBestMove(2.0);
	board.MakeMove(move, true);
}

void TestMoveUndo()
{
	Board board;
	board.SetFEN("8/8/k7/8/2K5/8/8/1Q6 w - - 0 1");
	Search search(board);
	int move = MakeShortMove(1, 1 + 8*5);
	BoardState undo;
	board.MakeMove(move, true, undo);
	board.UndoMove(move, true, undo);
}

void TestMoveKey()
{
	Board board;
	board.SetFEN(Board::startPosition);
	MoveGenerator gen(board);
	vector<int> moves;
	gen.GenerateAllMoves(moves);
	while(moves.size() > 0)
	{
		int move = moves[rand() % moves.size()];
		BoardState undo;
		ulong key = board.GetZobristKey();
		board.MakeMove(move, true, undo);
		
		board.UndoMove(move, true, undo);
		if (key != board.GetZobristKey())
		{
			cout << "Invalid board:" << endl;
			cout << board.ToString() << endl;
		}
			
		board.MakeMove(move, true, undo);
		moves.clear();
		gen.GenerateAllMoves(moves);
	}
}

void TestDrawByRepetition()
{
	Board board;
	board.SetFEN("7k/8/8/8/8/8/r7/7K w - - 0 1");
	
	Search search(board);
	int whiteLeftMove = MakeShortMove(7, 6);
	int whiteRightMove = MakeShortMove(6, 7);

	int blackLeftMove = MakeShortMove(56+7, 56+6);
	int blackRightMove = MakeShortMove(56+6, 56+7);

	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(whiteLeftMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(blackLeftMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(whiteRightMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(blackRightMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}

	board.MakeMove(whiteLeftMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(blackLeftMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(whiteRightMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(blackRightMove, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn too early" << endl;
	}
	board.MakeMove(whiteLeftMove, true);
	if(!board.GetIsDrawnByRepetition())
	{
		cout << "Not drawn" << endl;
	}
}

void TestTranspositionCompression()
{
	TranspositionTable t;
	t.SetSize(32);
	TranspositionTable::Entry e;
	e.entryType = TranspositionTable::AlphaCutoff;
	e.evaluation = -45;
	e.plyDepth = 4;
	e.move = MakeShortMove(2, 22);
	t.SetEntry(0xfeedbeef, e.move, e.evaluation, e.plyDepth, e.entryType);

	TranspositionTable::Entry e2;
	t.TryGetEntry(0xfeedbeef, e2);
	if(e.entryType != e2.entryType)
	{
		cout << "Entry type mismatch" << endl;
	}
	if(e.evaluation != e2.evaluation)
	{
		cout << "Entry evaluation mismatch" << endl;
	}
	if(e.plyDepth != e2.plyDepth)
	{
		cout << "Entry plyDepth mismatch" << endl;
	}
	if(e.move != e2.move)
	{
		cout << "Entry move mismatch" << endl;
	}

	e.entryType = TranspositionTable::AlphaCutoff;
	e.evaluation = 58;
	e.plyDepth = 1;
	e.move = 1739;
	t.SetEntry(0x149e7ee16831a608, e.move, e.evaluation, e.plyDepth, e.entryType);

	bool res = t.TryGetEntry(0x149e7ee16831a608, e2);
	if(!res)
	{
		cout << "Entry does not exist"  << endl;
	}
	if(e.entryType != e2.entryType)
	{
		cout << "Entry type mismatch" << endl;
	}
	if(e.evaluation != e2.evaluation)
	{
		cout << "Entry evaluation mismatch" << endl;
	}
	if(e.plyDepth != e2.plyDepth)
	{
		cout << "Entry plyDepth mismatch" << endl;
	}
	if(e.move != e2.move)
	{
		cout << "Entry move mismatch" << endl;
	}
}

void RunUnitTests()
{
	TestTranspositionCompression();
	TestDrawByRepetition();
	TestMoveUndo();
	BadMoveCheck();
	BadMoveCheck2();
	BadMoveCheck3();
	BadMoveCheck4();
	SeeTests();
	IllegalMoveCheck();
	//TestPins();
}
/*
void TestPins()
{
	string fen="Qq1krR1R/2ppp3/1B1R1B2/3R4/3r4/3R4/8/K7 w - - 0 1";
	Board board;
	board.SetFEN(fen);
	ulong pinned = board.GetPinnedPieces(Square(7, 3), Colors::Black);
	if (BitCount(pinned) != 5)
	{
		cout << "Pinned count incorrect.  Expected (7, 1) (7, 4) (6, 1) (6, 2) (6, 3)" << endl;
		while (pinned != 0)
		{
			int location = PopLowestSetBit(pinned);
			cout << "Found row " << GetRow(location) << " col " << GetCol(location) << endl;
		}
	}
}
*/
void EndgameTablebaseTest()
{
	string fen = "8/8/4kpP1/4p1n1/4K3/3P2R1/8/8 w - - 0 1";
	Board board;
	board.SetFEN(fen);
	Search search(board);
	int move = search.GetBestMove(20.0);
	cout << GetUCIString(move) << endl;
}

void SeeTests()
{
	string test1s[]= { "c3b5" };
	int test1i[]={ -200 };// Nxp, nxN
	TestSee("7k/2n5/8/1p6/8/2N5/8/7K w - - 0 1", test1s, test1i, 1);

	string test2s[]={ "c3b5",	"a3b5" };
	int test2i[]	={ 100,		100 };  // Nxp, nxN, Nxn 
	TestSee("7k/2n5/8/1p6/8/N1N5/8/7K w - - 0 1", test2s, test2i, 2);

	string test3s[]={ "c3b5" };
	int test3i[]	={ -200 };// Nxp, pxN, Nxp? bad
	TestSee("7k/2n5/p7/1p6/8/N1N5/8/7K w - - 0 1", test3s, test3i, 1);
	
	
	string test4s[]={	"e3d4",	"e4d4"	 };
	int test4i[]	={		300,	100 };
	TestSee("K6k/8/8/8/3nR3/4Pn/8/8 w - - 0 1", test4s, test4i, 2);
	
	
	string test5s[]={ "e3d4", "e4d4" };
	int test5i[]	={ 300, 100 };
	TestSee("K6k/8/8/8/3nR3/4Pn/8/8 w - - 0 1", test5s, test5i, 2);
	
	
	string test6s[]={	"g4c4", "c3c4" };
	int test6i[]	={		300,	-49700 };
	TestSee("8/8/8/2k5/2n3Q1/2K5/8/8 w - - 0 1", test6s, test6i, 2);
	
	
	string test7s[]={ "g6h7"};
	int test7i[]	= { 900 };
	TestSee("7k/7q/6B1/5Q1/8/8/8/K7 w - - 0 1", test7s, test7i, 1);
	
	
	string test8s[]={ "h7g6" };
	int test8i[]	={ -600 };
	TestSee("7k/7q/6B1/5Q2/8/8/8/7K b - - 0 1", test8s, test8i, 1);
	
	
	string test9s[]={ "g6h7" };
	int test9i[]	={ 600};
	TestSee("6kr/7q/6B1/5Q1/8/8/8/K7 w - - 0 1", test9s, test9i, 1);
	
	
	string test10s[]={	"d4e5",	"f4e5",	"e3e5" };
	int test10i[]	={		800,	800,	100 };
	TestSee("7k/4r3/2nprpn1/4q3/3P1P2/2B1Q1B1/4R3/K3R3 w - - 0 1", test10s, test10i, 3);
	
	
	string test11s[]={ "e6e3" };
	int test11i[]	={		300};
	TestSee("4q2k/4r3/4r3/8/8/4B3/4R3/K3R3 b - - 0 1", test11s, test11i, 1);
	
	
	string test12s[]={ "g6h7" };
	int test12i[]	={ -41000 };
	TestSee("6kr/8/5b2/4q3/3K4/2Q5/8/8 w - - 0 1", test12s, test12i, 1);
	
	
	string test13s[]={ "c8c3" };
	int test13i[]	={ -200 };
	TestSee("2r5/p4pk1/1p3n2/3p1Qpq/3P4/P1N1P2P/1P5r/1R2K1R1 b - - 0 1", test13s, test13i, 1);
}

void TestSee(std::string fen, std::string captures[], int results[], int testcount)
{
	Board board;
	board.SetFEN(fen);
	StaticExchangeEvaluator staticExchangeEvaluator(board);
	MoveGenerator moveGenerator(board);
	std::vector<int> moves;
	moveGenerator.GenerateAllMoves(moves);
	for(uint i=0;i<moves.size();i++)
	{
		int move = moves[i];
		int destPiece = board.GetPiece(GetDestFromMove(move));
		if (destPiece != Pieces::None)
		{
			int seeValue = staticExchangeEvaluator.Evaluate(move);
			for (int i = 0; i < testcount; i++)
			{
				if (captures[i] == GetUCIString(move))
				{
					if (results[i] != seeValue)
					{
						cout << "See error for move:" << GetUCIString(move) << endl;
						cout << board.ToString() << endl;
						cout << "Expected " << results[i] << ", got " << seeValue << endl;
					}
					break;
				}
			}
		}
	}
}

void BestMoveCheck()
{
	ifstream in("moves.txt");
	int num = 0;
	int yes = 0;
	int no = 0;
	string line;
	while(getline(in, line))
	{
		// format is rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ;D1 20 ;D2 400 ;D3 8902 ;D4 197281 ;D5 4865609 ;D6 119060324
		string fen = line.substr(0, line.find(';'));
		string bestMoveChoice = line.substr(line.find(';') + 1);
		Board board;
		Search search(board);
		board.SetFEN(fen);
		vector<int> moves;
		MoveGenerator moveGenerator(board);
		moveGenerator.GenerateAllMoves(moves);
		int myBestMove = search.GetBestMove(.05);
		int bestMove = NullMove;
		for(uint i=0;i<moves.size();i++)
		{
			int move = moves[i];
			if (GetShortAlgebraicString(move, board) == bestMoveChoice || GetUCIString(move) == bestMoveChoice)
			{
				bestMove = move;
			}
		}
		if (bestMove == NullMove)
		{
			//Console.WriteLine("Could not find a best move for FEN=" + fen + ", bestMoveChoice=" + bestMoveChoice);
			continue;
		}

		if (GetUCIString(myBestMove) == bestMoveChoice || GetShortAlgebraicString(myBestMove, board) == bestMoveChoice)
		{
			yes++;
		}
		else
		{
			no++;
		}
		

		num++;
	}
	cout << "Got " << yes << " correct, " << no << " incorrect, out of " << num <<endl;
}
void DrawByRepetitionCheck()
{
	

	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "e2e4 d7d5 e4d5 d8d5 \
b1c3 d5e6 \
f1e2 e6g6 \
e2f3 e7e5 \
d2d3 f8e7 \
d1e2 b8c6 \
c1e3 c8f5 \
e1c1 a7a6 \
 h2h3 g8f6 \
 g2g4 f5e6 \
 f3g2 h7h5 \
 f2f3 e8c8 \
 g4g5 f6d5 \
 c3d5 e6d5 \
 h3h4 d5a2 \
 b2b3 e7a3 \
 c1d2 a3b4 \
 d2c1 b4a3 \
 c1d2 a3b4 \
d2c1";// b4a3

	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;
	board.MakeMove(move, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn. " << endl;
	}
}

void EvalTest()
{
	string fen = "2k1r3/1p3rpp/pP6/4p1b1/3NR3/2P3KP/P5P1/3R4 w - - 0 1";
	Board board;
	board.SetFEN(fen);
	Evaluation eval(board);
	Search search(board);
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;
}

void PawnPushTest()
{
	string fen = "8/8/3kp1P1/2p4R/5P1P/5K2/r7/8 b - - 0 1";
	//fen = "8/6P1/3kp3/2p4R/5P1P/5K2/8/r7 b - - 0 1";
	//fen = "8/8/3kp1P1/2p4R/5P1P/5K2/8/r7 w - - 0 1";
	Board board;
	board.SetFEN(fen);
	Evaluation eval(board);
	Search search(board);
	int move = search.GetBestMove(10.0);
	cout << GetUCIString(move) << endl;
}

void BadQueenAttack()
{
	string fen = "4k1r1/p4p1p/1p2p1p1/2bqB3/2R5/3Q3P/P1P2P1P/1R1K4 b - - 0 1";
	Board board;
	board.SetFEN(fen);
	Search search(board);
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;
}

void SacrificeBishop()
{
	string fen = "4r1k1/6pp/2p2p2/p1qp2Qn/1rP3bP/1P1BNP2/P4K2/R6R w - - 0 1";
	Board board;
	board.SetFEN(fen);
	Search search(board);
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;
}

void BoardCorruption()
{
	string fen = "8/8/6pk/8/5p1P/6n1/6PK/R7 b - - 0 1";
	Board board;
	board.SetFEN(fen);
	Search search(board);

	if(0) // check pv
	{
		string moves = "g6g5 h4g5 h6g5 a1a5 g5f6 h2h3 f6e7";
		istringstream in(moves);
		string moveStr;
		MoveGenerator gen(board);
		cout << board.ToString() << endl;
		while(in >> moveStr)
		{
			vector<int> mo;
			gen.GenerateAllMoves(mo);
			int move = NullMove;
			for(unsigned int i=0;i<mo.size();i++)
			{
				if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
				{
					move = mo[i];
				}
			}
			board.MakeMove(move, true);
			cout << board.ToString() << endl;
		}
	}
	else
	{
		int move = search.GetBestMove(0.05);
		cout << GetUCIString(move) << endl;
	}
}

void AllowedDraw()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	string moves = "c4 e5 Nc3 Nf6 Nf3 Nc6 d4 exd4 Nxd4 Bb4 Bf4 Ne4 Nxc6 Nxc3 Qb3 Nd5 Nxb4 Nxf4 g3 Ne6 Bg2 Qg5 h4 Qg4 Bf3 Qd4 e3 Qf6 Be4 Nc5 Qc2 Nxe4 Qxe4 Qe6 Qd4 0-0 Nd5 Qd6 c5 Qa6 b4 d6 Nxc7 Qc6 Nd5 dxc5 bxc5 Kh8 e4 Be6 0-0 f6 a1b1 a8c8 f1d1 Rb8 a4 Bg4 Rd3 Be6 f4 b8d8 f5 Bf7 Rd2 Bh5 a5 Qa6 Ra2 Be2 Rd2 Rd7 Qe3 Bc4 Qc3 f8d8 b1d1 Rc8 Qb4 Re8 Qa4 Bb5 Qb4 Bc4 Qa4 Bb5";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;
	board.MakeMove(move, true);
	if(board.GetIsDrawnByRepetition())
	{
		cout << "Drawn. " << endl;
	}
}

void MissedQueenLoss()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Search search(board);
	
	string moves = "e2e4 d7d6 d2d4 g8f6 b1c3 g7g6 f1c4 b8c6 c1f4 e7e5 d4e5 c8g4 f2f3 f6h5 f4e3 c6e5 c4d5 g4c8 g2g4 h5f6 d5b3 c7c5 g4g5 f6h5 f3f4 e5c6 f4f5 c6a5 b3a4 c8d7 a4d7 d8d7 d1e2 a5c6 e1c1 b7b6 g1h3 h5g7 c3d5 e8d8 d5f6 d7c8 h3f4 g6f5 e3d2 c6d4 e2c4 g7e6 c2c3 e6g5 c4d5 g5e4 d5f7 e4f2 c3d4 c5d4 c1b1";
		istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			break;
		}
		board.MakeMove(move, true);
	}
	int move = search.GetBestMove(2.0);
	cout << GetUCIString(move) << endl;

	board.MakeMove(move, true);
	
}

void BestMoveCheckPositional()
{
	ifstream in("C:\\data\\NativeTest\\positional.txt");
	int num = 0;
	int yes = 0;
	int no = 0;
	string line;
	while(getline(in, line))
	{
		// format is rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ;D1 20 ;D2 400 ;D3 8902 ;D4 197281 ;D5 4865609 ;D6 119060324
		string fen = line.substr(0, line.find(';'));
		string bestMoveChoice = line.substr(line.find(';') + 1);
		bestMoveChoice = bestMoveChoice.substr(0, bestMoveChoice.find(';'));
		Board board;
		Search search(board);
		board.SetFEN(fen);
		vector<int> moves;
		MoveGenerator moveGenerator(board);
		moveGenerator.GenerateAllMoves(moves);
		int myBestMove = search.GetBestMove(2.0);
		int bestMove = NullMove;
		for(uint i=0;i<moves.size();i++)
		{
			int move = moves[i];
			if (GetShortAlgebraicString(move, board) == bestMoveChoice || GetUCIString(move) == bestMoveChoice)
			{
				bestMove = move;
			}
		}
		if (bestMove == NullMove)
		{
			cout <<  "Could not find a best move for FEN=" << fen << ", bestMoveChoice=" << bestMoveChoice << endl;
			continue;
		}

		if (GetUCIString(myBestMove) == bestMoveChoice || GetShortAlgebraicString(myBestMove, board) == bestMoveChoice)
		{
			yes++;
		}
		else
		{
			no++;
		}
		num++;
	}
	cout << "Got " << yes << " correct, " << no << " incorrect, out of " << num <<endl;
}

void PinMaskIteration(string fen, int row, int col, int color, ulong expected)
{
	Board board;
	MoveGenerator gen(board);
	board.SetFEN(fen);
	ulong actual = board.GetPinRestrictionMask(Square(row, col), color);
	if(actual != expected)
	{
		cout << "failed pin mask with fen " << fen << ".  Expected: " << expected << ", got " << actual << endl;
	}
}
void TestPinMask()
{
	PinMaskIteration("7K/8/8/8/2k1r2R/8/8/8 b - - 0 1", 3, 4, Colors::Black, (1ULL << Square(3, 2)) | (1ULL << Square(3, 3)) |(1ULL << Square(3, 5)) | (1ULL << Square(3, 6)) | (1ULL << Square(3, 7)));
	PinMaskIteration("7K/8/8/8/2k1r2Q/8/8/8 b - - 0 1", 3, 4, Colors::Black, (1ULL << Square(3, 2)) | (1ULL << Square(3, 3)) |(1ULL << Square(3, 5)) | (1ULL << Square(3, 6)) | (1ULL << Square(3, 7)));
	PinMaskIteration("7K/8/8/8/2k1r2P/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("7K/8/8/8/2k1r2N/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("7K/8/8/8/2k1r2p/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("7K/8/8/8/2k1r2B/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);

	PinMaskIteration("7k/8/8/8/2K1R2r/8/8/8 b - - 0 1", 3, 4, Colors::White, (1ULL << Square(3, 2)) | (1ULL << Square(3, 3)) |(1ULL << Square(3, 5)) | (1ULL << Square(3, 6)) | (1ULL << Square(3, 7)));
	PinMaskIteration("7k/8/8/8/2K1R2q/8/8/8 b - - 0 1", 3, 4, Colors::White, (1ULL << Square(3, 2)) | (1ULL << Square(3, 3)) |(1ULL << Square(3, 5)) | (1ULL << Square(3, 6)) | (1ULL << Square(3, 7)));
	PinMaskIteration("7k/8/8/8/2K1R2p/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("7k/8/8/8/2K1R2n/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("7k/8/8/8/2K1R2P/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("7k/8/8/8/2K1R2b/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);

	PinMaskIteration("7K/8/8/8/1R2r2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, (1ULL << Square(3, 1)) | (1ULL << Square(3, 2)) |(1ULL << Square(3, 3)) | (1ULL << Square(3, 5)) | (1ULL << Square(3, 6))| (1ULL << Square(3, 7)));
	PinMaskIteration("7K/8/8/8/1Q2r2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, (1ULL << Square(3, 1)) | (1ULL << Square(3, 2)) |(1ULL << Square(3, 3)) | (1ULL << Square(3, 5)) | (1ULL << Square(3, 6))| (1ULL << Square(3, 7)));
	PinMaskIteration("7K/8/8/8/1P2r2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("7K/8/8/8/1B2r2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("7K/8/8/8/1N2r2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	PinMaskIteration("8/8/8/8/1K2p2k/8/8/8 b - - 0 1", 3, 4, Colors::Black, ~0);
	
	PinMaskIteration("7k/8/8/8/1r2R2K/8/8/8 b - - 0 1", 3, 4, Colors::White, (1ULL << Square(3, 1)) | (1ULL << Square(3, 2)) |(1ULL << Square(3, 3)) | (1ULL << Square(3, 5)) | (1ULL << Square(3, 6))| (1ULL << Square(3, 7)));
	PinMaskIteration("7k/8/8/8/1q2R2K/8/8/8 b - - 0 1", 3, 4, Colors::White, (1ULL << Square(3, 1)) | (1ULL << Square(3, 2)) |(1ULL << Square(3, 3)) | (1ULL << Square(3, 5)) | (1ULL << Square(3, 6))| (1ULL << Square(3, 7)));
	PinMaskIteration("7k/8/8/8/1p2R2K/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("7k/8/8/8/1b2R2K/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("7k/8/8/8/1n2R2K/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	PinMaskIteration("8/8/8/8/1k2P2K/8/8/8 b - - 0 1", 3, 4, Colors::White, ~0);
	
	/*
	.......K
	.......Q
	........
	........
	........
	...p....
	........
	.k......
	*/
	PinMaskIteration("7K/7Q/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, (1ULL << Square(0, 1)) | (1ULL << Square(1, 2)) |(1ULL << Square(3, 4)) | (1ULL << Square(4, 5)) | (1ULL << Square(5, 6))| (1ULL << Square(6, 7)));
	PinMaskIteration("7K/7B/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, (1ULL << Square(0, 1)) | (1ULL << Square(1, 2)) |(1ULL << Square(3, 4)) | (1ULL << Square(4, 5)) | (1ULL << Square(5, 6))| (1ULL << Square(6, 7)));
	PinMaskIteration("7K/7R/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, ~0);
	PinMaskIteration("7K/7N/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, ~0);
	PinMaskIteration("7K/7P/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, ~0);
	PinMaskIteration("7K/7p/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, ~0);
	PinMaskIteration("8/7K/8/8/8/3p4/8/1k6 b - - 0 1", 2, 3, Colors::Black, ~0);

	PinMaskIteration("7k/7q/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, (1ULL << Square(0, 1)) | (1ULL << Square(1, 2)) |(1ULL << Square(3, 4)) | (1ULL << Square(4, 5)) | (1ULL << Square(5, 6))| (1ULL << Square(6, 7)));
	PinMaskIteration("7k/7b/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, (1ULL << Square(0, 1)) | (1ULL << Square(1, 2)) |(1ULL << Square(3, 4)) | (1ULL << Square(4, 5)) | (1ULL << Square(5, 6))| (1ULL << Square(6, 7)));
	PinMaskIteration("7k/7r/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, ~0);
	PinMaskIteration("7k/7n/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, ~0);
	PinMaskIteration("7k/7p/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, ~0);
	PinMaskIteration("7k/7P/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, ~0);
	PinMaskIteration("8/7k/8/8/8/3P4/8/1K6 b - - 0 1", 2, 3, Colors::White, ~0);
}

void TestMoveScores()
{
	Board board;
	MoveGenerator gen(board);
	/*
	.......k
	........
	........
	....q...
	........
	..R...p.
	.K.PP...
	........
	*/
	board.SetFEN("7k/8/8/4q3/8/2R3p1/1K1PP3/8 b - - 0 1");
	std::vector<int> moves;
	std::vector<int> moveValues;
	gen.GenerateAllMoves(moves);
	Search s(board);
	int best = s.GetBestMove(1.0);
	cout << GetUCIString(best) << endl;
	return;
	s.ScoreMoves(moves, 0, 0, 0, moveValues);
	//s.SortMoves(moves, 0, 0, 0, moveValues);
	for(int i=0;i<moves.size();i++)
	{
	//	if(moves[i] == best) cout<< "Best"<<endl;
		cout << i << " " << GetUCIString(moves[i]) << " " << moveValues[i] << endl;
		BoardState undoState;
		board.MakeMove(moves[i], true, undoState);
		cout << board.ToString() << endl;
		board.UndoMove(moves[i], true, undoState);
	}
}

void TestQSearch()
{
	Board board;
	MoveGenerator gen(board);
	/*
	.......k
	........
	........
	P......q
	........
	.K......
	......R.
	........
	*/
	Search s(board);
	board.SetFEN("7k/8/8/P6q/8/1K6/6R1/8 b - - 0 1");
	std::vector<int> moves;
	std::vector<int> moveValues;
	gen.GenerateAllMoves(moves);
	int best = s.GetBestMove(600.0, 1);
	string expected = "h5d5";
	if(expected != GetUCIString(best))
	{
		cout << "Qsearch failed to find check & capture" << endl;
	cout << GetUCIString(best) << endl;
	}
	return;
}

void TestPawnValues()
{
	Board board;
	MoveGenerator gen(board);
	string text = "\
	.......k\
	ppp.pppp\
	........\
	.P.p....\
	...P....\
	....P...\
	P.P..PPP\
	.......K\
	";
	Search s(board);
	board.SetText(text, 0, CastleRights(false, false, false, false));

	std::vector<int> moves;
	std::vector<int> moveValues;
	gen.GenerateAllMoves(moves);
	
	Evaluation eval(board);
	int score = eval.GetPawnScore();
	cout << score << endl;
	cout << board.ToString() << endl;
	
	return;	
}

void PlayGameDebug()
{
	Board board;
	board.SetFEN(Board::startPosition);
	Evaluation eval(board);
	Search search(board);
	string moves = "e4 c5 Nf3 d6 d4 cxd4 Nxd4 Nf6 Nc3 a6 Be3 e5\
 Nb3 Be6 f3 d5 exd5 Bxd5 Qd3 Nc6 0-0-0 Be6 Nc5 Bxc5 Bxc5\
 Qa5 Qe3 Nd7 Ba3 f6 Rd6 Bf7 Bd3 Rd8 Rd1 Ne7 Kb1 b6\
 Be4 Bc4 Qd2 Nd5";
	istringstream in(moves);
	string moveStr;
	MoveGenerator gen(board);
	while(in >> moveStr)
	{
		vector<int> mo;
		gen.GenerateAllMoves(mo);
		int move = NullMove;
		for(unsigned int i=0;i<mo.size();i++)
		{
			if(GetUCIString(mo[i]) == moveStr || GetShortAlgebraicString(mo[i], board) == moveStr)
			{
				move = mo[i];
			}
		}
		if(move == NullMove)
		{
			cout << "Illegal move " << moveStr << endl;
			cout << board.ToString() << endl;
			break;
		}
		cout << (board.Turn() == Colors::White ? "white" : "black") << " to play" << endl;
		cout << board.ToString() << endl;
		cout << "Score is " << eval.GetScore() << " material score is " << board.GetMaterialScore() << endl;
		cout << endl;
		board.MakeMove(move, true);
		
	}
	search.GetBestMove(6);
	
	
}

void TestDiscoveredCheck()
{
	Board board;
	MoveGenerator gen(board);
	Search s(board);
	board.SetFEN("r3r1k1/1b1n1p1p/1qpp1npQ/p3pNN1/P2PP3/1Pp1R2P/2B2PP1/R5K1 b - - 0 23");
	std::vector<int> moves;
	std::vector<int> moveValues;
	gen.GenerateAllMoves(moves);
	int best = s.GetBestMove(100, 8);

	//pv should be g6f5 e3g3 f5f4 g5e6+ f4g3 h6g7++
}

void Benchmark()
{
	Board board;
	MoveGenerator gen(board);
	Search s(board);
	board.SetFEN(Board::startPosition);
	std::vector<int> moves;
	std::vector<int> moveValues;
	gen.GenerateAllMoves(moves);
	int best = s.GetBestMove(1000, 10);

	//pv should be g6f5 e3g3 f5f4 g5e6+ f4g3 h6g7++
}
}