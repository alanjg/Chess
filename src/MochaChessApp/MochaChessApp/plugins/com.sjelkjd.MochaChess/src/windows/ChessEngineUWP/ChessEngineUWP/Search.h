#ifndef SEARCH_H
#define SEARCH_H

#include "Board.h"
#include "Types.h"
#include "MoveGenerator.h"
#include "TranspositionTable.h"
#include "StaticExchangeEvaluator.h"
#include "Evaluation.h"

namespace ChessEngineUWP
{
	class Search
	{
		friend struct SearchNode;
	protected:
#if DEBUGPV
		std::vector<std::string> pvTest;
		std::vector<bool> pvMatch;
		int pvAt;
#endif
		std::vector<std::vector<int>*> pvLists;
		std::vector<std::vector<int>*> moveLists;
		std::vector<std::vector<int>*> moveValueLists;
		Board& m_board;
		MoveGenerator moveGenerator;
		StaticExchangeEvaluator staticExchangeEvaluator;
		Evaluation evaluation;
		void EnsureMoveListDepth(uint depth);

		// checkmate needs to be large enough to avoid confusion with piece scores.
		// The limit needs to exceed a reasonable upper bound on position score.
		// Also, the per-move checkmate adjustment should be large enough to avoid spurious captures due to evaluation.
		static const int CHECKMATE = 100000000;
		static const int CHECKMATE_ADJUSTMENT = 1000;
		static const int MIN_CHECKMATE = CHECKMATE - CHECKMATE_ADJUSTMENT * 100;
		
		static const int NULL_MOVE_R = 3;
		int historyTableCounter[64][64];
		int historyTableFailHigh[64][64];
		int historyTableFailLow[64][64];
		bool setHashSize;
		bool suppressLogging;
	protected:
		double totalSearchTime;

		int nodeCount;
		int qNodeCount;
		TranspositionTable transpositionTable;
		std::clock_t searchStart;
		
		bool timeExceeded;
		static bool IsCheckmate(int eval)
		{
			if (eval > MIN_CHECKMATE)
			{
				return true;
			}
			else if (eval < -MIN_CHECKMATE)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		static int AdjustCheckmateForDepth(int score)
		{
			if (score < 0)
			{
				if (score < -MIN_CHECKMATE)
				{
					score += CHECKMATE_ADJUSTMENT;
				}
			}
			else
			{
				if (score > MIN_CHECKMATE)
				{
					score -= CHECKMATE_ADJUSTMENT;
				}
			}
			return score;
		}

		
		double GetElapsedTime()
		{
			std::clock_t now = std::clock();
			double elapsed = (now - searchStart);
			return elapsed / CLOCKS_PER_SEC;
		}

		void StoreTranspositionEntry(int move, int eval, int limit, int depth, int alpha, int beta);
		void StoreTranspositionEntry(int move, int eval, int limit, int depth, int alpha, int beta, Board& board);
		
		
		int MoveValue(int move);
		int MoveValueExpensive(int move);	

	public:
		Search(Board& board)
			: m_board(board), moveGenerator(board), moveLists(50), moveValueLists(50), pvLists(50), staticExchangeEvaluator(board), evaluation(board), transpositionTable()
		{
			for(int i=0;i<50;i++)
			{
				moveLists[i] = new std::vector<int>();
				moveLists[i]->reserve(50);

				moveValueLists[i] = new std::vector<int>();
				moveValueLists[i]->reserve(50);

				pvLists[i] = new std::vector<int>();
				pvLists[i]->reserve(50);
			}

			totalSearchTime = 1.0;
			setHashSize = false;
			suppressLogging = false;
			memset(historyTableCounter, 0, sizeof(historyTableCounter));
			memset(historyTableFailHigh, 0, sizeof(historyTableFailHigh));
			memset(historyTableFailLow, 0, sizeof(historyTableFailLow));
		}

		virtual ~Search()
		{
			for(uint i=0;i<moveLists.size();i++)
			{
				delete moveLists[i];
				delete moveValueLists[i];
				delete pvLists[i];
			}
		}
		
		void SetHashTableSize(int megabytes)
		{
			setHashSize = true;
			transpositionTable.SetSize(megabytes);
		}

		bool ShouldUseNullMove();

		void SuppressLogging() { suppressLogging = true; }
		void RestoreLogging() { suppressLogging = false; }
		ulong GetMoveCount(int depth);

		int GetBestMove(double searchTime);
		int GetBestMove(double searchTime, int maxDepth);

		virtual int AlphaBetaSearch(int depth, int& move, vector<int>* pvList, int alpha, int beta);
		int AlphaBeta(int limit, int depth, int& chosenMove, vector<int>* pvList, int alpha, int beta);
		
		void PrintPrincipleVariation(int maxDepth);

		int Score(int activeColor, Board& board);

		int Score(int activeColor);

		void ScoreMovesExpensive(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList);
		void ScoreMoves(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList);
		void SortMoves(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList);

	};
}

#endif