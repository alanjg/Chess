#ifndef TESTS_H
#define TESTS_H

namespace SjelkjdChessEngine
{
	void PerfTTests();
	void PerfTHard();
	void PerfTK();
	void PlaySelf();
	void TestRookEndgame();
	void TestQueenEndgame();
	void TestTranspositionCompression();
	void TestMoveUndo();
	void TestDrawByRepetition();
	void BadMoveCheck();
	void TwoKingsTest();
	void BadMoveCheck3();
	void TestInCheckMoveCount();
	void TestMoveKey();
	void SeeTests();
	void TestSee(std::string fen, std::string captures[], int results[], int testCount);

	void RunUnitTests();
	
	void BestMoveCheck();
	//void TestPins();
	void EndgameTablebaseTest();
	void DrawByRepetitionCheck();
	void EvalTest();
	void PawnPushTest();
	void AllowedDraw();
	void SacrificeBishop();
	void BadQueenAttack();
	void MissedQueenLoss();
	void BestMoveCheckPositional();
	void ConvertBestMove();
	void TestPinMask();
	void BoardCorruption();
	void TestMoveScores();
	void TestQSearch();
	void PlayGameDebug();
	void TestPawnValues();
	void TestDiscoveredCheck();
	void Benchmark();
}

#endif