#include "StdAfx.h"
#include "Search.h"
#include "Types.h"
#include "Evaluation.h"
using namespace std;

namespace SjelkjdChessEngine
{	
	void Search::EnsureMoveListDepth(uint depth)
	{
		while(depth >= moveLists.size())
		{
			vector<int>* newList = new vector<int>();
			newList->reserve(50);
			moveLists.push_back(newList);
			
			vector<int>* newValueList = new vector<int>();
			newValueList->reserve(50);
			moveValueLists.push_back(newValueList);

			vector<int>* newpvList = new vector<int>();
			newpvList->reserve(50);
			pvLists.push_back(newpvList);
		}
	}

	ulong Search::GetMoveCount(int depth)
	{
		static int counter = 0;
		static int genCounter = 0;
		EnsureMoveListDepth(depth);
		vector<int>& moves = *moveLists[depth];
		moves.clear();
		moveGenerator.GenerateAllMoves(moves);
		ulong total = 0;
		for(unsigned int i=0;i<moves.size();i++)
		{
			if(depth == 1)
			{
				total++;
			}
			else
			{
				BoardState undo;
				m_board.MakeMove(moves[i], false, undo);
				total += GetMoveCount(depth - 1);	
				m_board.UndoMove(moves[i], false, undo);
			}
		}
		return total;
	}
	
	int fprune = 0;
	int Search::GetBestMove(double searchTime)
	{
		return GetBestMove(searchTime, numeric_limits<int>::max());
	}
	
	int Search::GetBestMove(double searchTime, int maxDepth)
	{
#if DEBUGPV
		static std::string pvDebug = "g6f5 e3g3 f5f4 g5e6 f4g3 h6g7";
		static bool pvDebugInitialized = false;
		if (!pvDebugInitialized)
		{
			istringstream in(pvDebug);
			string move;
			while (in >> move)
			{
				pvTest.push_back(move);
			}
		}
		pvAt = 0;
#endif
		if(!setHashSize)
		{
			setHashSize = true;
			transpositionTable.SetSize(32);
		}
		fprune = 0;
		totalSearchTime = searchTime;
		timeExceeded = false;
		searchStart = clock();
		int chosenMove;
		nodeCount = 0;
		qNodeCount = 0;
		int alpha = -numeric_limits<int>::max();
		int beta = numeric_limits<int>::max();
		memset(historyTableCounter, 0, sizeof(historyTableCounter));
		memset(historyTableFailHigh, 0, sizeof(historyTableFailHigh));
		memset(historyTableFailLow, 0, sizeof(historyTableFailLow));
		vector<int> bestPvList;
		vector<int> pvList;
		int depth = 0;
		int eval = 0;
		int score = evaluation.GetScore();
		do
		{
			depth++;
			int move = NullMove;
			alpha = score - 150;
			beta = score + 150;
			score = AlphaBetaSearch(depth, move, &pvList, alpha, beta);
			if (!timeExceeded && (score <= alpha || score >= beta))
			{
				if (!suppressLogging)
				{
				cout << "aspiration failed." << endl;
				}
				alpha = -numeric_limits<int>::max();
				beta = numeric_limits<int>::max();
				move = NullMove;
				score = AlphaBetaSearch(depth, move, &pvList, alpha, beta);
			}
			if (!timeExceeded)
			{
				bestPvList = pvList;
				chosenMove = move;
				eval = score;

				int totalNodes = nodeCount + qNodeCount;
				
				double totalTime = GetElapsedTime();
				int msTime = (int)(totalTime * 1000);
				int nodesPerSecond = (int)(totalNodes / totalTime);
				int knps = nodesPerSecond / 1000;

				ostringstream sval;
				if (IsCheckmate(eval) && eval > 0)
				{
					sval << "mate";
				}
				else if (IsCheckmate(eval) && eval < 0)
				{
					sval << "loss";
				}
				else
				{
					sval << "cp " << eval;
				}
		
				if(!suppressLogging)
				{
					cout << "info depth " << depth << " nodes " << totalNodes << " score " << sval.str() << " time " << msTime << " ";
					cout << "pv";
					for (unsigned int i = 0; i < bestPvList.size(); i++)
					{
						cout << " " << GetUCIString(bestPvList[i]);
					}
					cout << endl;
				}
				if (IsCheckmate(score) ||m_board.GetIsDrawnByRepetition() || evaluation.IsDrawByMaterial())
				{
					break;
				}
			}
			else
			{
				// time exceeded.
				if (depth == 1) // If we didn't complete the first level search, just make any move.
				{
					vector<int> moves;
					moveGenerator.GenerateAllMoves(moves);
					chosenMove = moves[0];
				}
			}				
		} while (!timeExceeded && depth < maxDepth);

		if (timeExceeded)
		{
			depth--;
		}
		if(!suppressLogging)
		{
			cout << "nodes: " << nodeCount << endl;
			cout << "qnode: " << qNodeCount << endl;
			cout << "fprune: " << fprune << endl;
			cout << "pawn hash hits: " << TranspositionTable::PawnScoreTable.Hits() << ", misses: " << TranspositionTable::PawnScoreTable.Misses() << ", collisions: " << TranspositionTable::PawnScoreTable.Collisions() << endl;
			cout << "transposition hash hits: " << transpositionTable.hashTable.Hits() << ", misses: " << transpositionTable.hashTable.Misses() << ", collisions: " << transpositionTable.hashTable.Collisions() << endl;
		}
		return chosenMove;	
	}

	int Search::AlphaBetaSearch(int depth, int& move, vector<int>* pvList, int alpha, int beta)
	{
		return AlphaBeta(depth, 0, move, pvList, alpha, beta);
	}

	bool Search::ShouldUseNullMove()
	{
		if(m_board.GetIsInCheck())
		{
			return false;
		}

		// null move when we have any sliders major or minor piece
		ulong myPieces = m_board.GetColorBitBoard(m_board.Turn());
		ulong sliders = m_board.GetPieceTypeBitBoard(PieceTypes::Queen) | m_board.GetPieceTypeBitBoard(PieceTypes::Rook) | m_board.GetPieceTypeBitBoard(PieceTypes::Bishop);
		if((sliders & myPieces) != 0)
		{
			return true;
		}
		return false;
	}

	int Search::AlphaBeta(int limit, int depth, int& chosenMove, vector<int>* pvList, int alpha, int beta)
	{
#if DEBUGPV
		bool match = true;
		for (int i = 0; i < pvMatch.size();i++)
		{
			if(!pvMatch[i])
			{
				match = false;
			}
		}
		if (match && pvMatch.size() == pvTest.size())
		{
			int i = 5;
		}

#endif
		int initialAlpha = alpha;
		int initialBeta = beta;
		
		if (((nodeCount + qNodeCount) & 1023) == 0)
		{
			if (GetElapsedTime() > totalSearchTime)
			{
				timeExceeded = true;
				return alpha;
			}
		}

		if (depth < limit)
		{
			nodeCount++;
		}
		else
		{
			qNodeCount++;
		}

		// draw by repetition.
		if (m_board.GetIsDrawnByRepetition() && depth > 0)
		{
			return 0;
		}

		if(evaluation.IsDrawByMaterial() && depth > 0)
		{			
			return 0;
		}

		if (m_board.GetIsInCheck())
		{
			if(depth >= limit)
			{
				limit = depth + 1;
			}
			else
			{
				limit = limit + 1;
			}
			
			// no null move when in check.
			chosenMove = NullMove;
		}

		int bestMoveScore = -CHECKMATE;
		int score = 0;
		if (depth >= limit)
		{
			score = Score(m_board.Turn());
			bestMoveScore = max(bestMoveScore, score);
			alpha = max(alpha, score);
			if (alpha >= beta)
			{
				return alpha;
			}
		}

		bool gotHashHit = false;
		int firstMove = 0;
		TranspositionTable::Entry entry;
		if (transpositionTable.TryGetEntry(m_board.GetZobristKey(), entry))
		{
			gotHashHit = true;
			if (entry.plyDepth >= limit - depth)
			{
				bool useEntry = false;
				if (entry.entryType == TranspositionTable::Exact)
				{
					useEntry = true;
				}
				else if (entry.entryType == TranspositionTable::AlphaCutoff)
				{
					// this search was previously cut off at evaluation.  Only use this entry if the new alpha would still cause the search to be cut off.
					if (entry.evaluation <= alpha)
					{
						useEntry = true;
					}
				}
				else if (entry.entryType == TranspositionTable::BetaCutoff)
				{
					// this search was previously cut off by exceeding beta.  Only use this entry if the new beta would still cause the search to be cut off.
					if (entry.evaluation >= beta)
					{
						useEntry = true;
					}
				}
				if (useEntry)
				{
					// Ensure this move is actually valid, and that it's not a hash key conflict.
					if (!moveGenerator.IsValidMove(entry.move))
					{
						useEntry = false;
						if(entry.move != NullMove)
						{
							std::cout << "Bad move in hash table." << std::endl;
							std::cout << GetUCIString(entry.move) << std::endl;
							std::cout << m_board.ToString() << std::endl;
						}
					}
					else
					{
						// Don't return the hashed null move in some cases
						if(entry.move == NullMove)
						{
							// If we are prevented from doing a null move, don't use it.
							if(chosenMove == NullMove)
							{
								useEntry = false;
							} // Don't return a hashed null move if we have a non-trivial search window, implying that we're in a pv node.
							else if(beta - alpha > 1)
							{
								useEntry = false;
							}
						}
						// Don't use the hashed move while in the principal variation
						if(beta - alpha > 1)
						{
							useEntry = false;
						}
					}
				}
				if (useEntry)
				{
					chosenMove = entry.move;
					return entry.evaluation;
				}
			}
			firstMove = entry.move;
			// Don't try the null move if this node failed low.
			if (entry.entryType == TranspositionTable::AlphaCutoff)
			{
				chosenMove = NullMove;
			}
		}

		int otherTurn = m_board.Turn() == Colors::White ? Colors::Black : Colors::White;

		int moveScore = alpha;
		bool useNullMove = chosenMove != NullMove && ShouldUseNullMove();
		
		// normal null move
		if (useNullMove && depth < limit)
		{
			// null move search
			BoardState undo;
			m_board.MakeMove(NullMove, false, undo);
#if DEBUGPV
			pvMatch.push_back(false);
#endif
			int someMove = NullMove;
			moveScore = -AlphaBeta(limit, depth + NULL_MOVE_R + 1, someMove, NULL, -beta, -beta + 1);
			
			m_board.UndoMove(NullMove, false, undo);
#if DEBUGPV
			pvMatch.pop_back();
#endif
			if (timeExceeded)
			{
				return moveScore;
			}

			if (moveScore >= beta)
			{
				StoreTranspositionEntry(NullMove, moveScore, limit, depth, initialAlpha, beta);
				return moveScore;
			}
		}

		if(!gotHashHit)
		{
			// no hash hit.  do internal deepening to find a new hash move.
			if(limit - depth > 2)
			{
				AlphaBeta(limit, depth + 2, firstMove, NULL, alpha, beta);
			}
		}

		EnsureMoveListDepth(depth);
		vector<int>& moves = *moveLists[depth];
		vector<int>& moveValues = *moveValueLists[depth];
		vector<int>* nextPvList = depth < limit ? pvLists[depth] : NULL;
		moves.clear();
		moveValues.clear();
		if (nextPvList != NULL)
		{
			nextPvList->clear();
		}
		
		if (depth < limit)
		{
			moveGenerator.GenerateAllMoves(moves);
		}
		else
		{	
			moveGenerator.GenerateQuiescentMoves(moves);
		}

		transpositionTable.EnsureKillerDepth(depth);
		//SortMoves(moves, firstMove, transpositionTable.killerMove1[depth], transpositionTable.killerMove2[depth], moveValues);
		if(depth == 0)
		{
			ScoreMovesExpensive(moves, firstMove, transpositionTable.killerMove1[depth], transpositionTable.killerMove2[depth], moveValues);
		}
		else
		{
			ScoreMoves(moves, firstMove, transpositionTable.killerMove1[depth], transpositionTable.killerMove2[depth], moveValues);
		}

		bool madeLegalMove = false;
		bool isPvNode = false;
		int moveCount = 0;

		for(uint i=0;i<moves.size();i++)
		{
			int bi = i;
			for (uint j = i + 1; j < moves.size(); j++)
			{
				if (moveValues[j] < moveValues[bi])
				{
					bi = j;
				}
			}
			if (bi != i)
			{
				swap(moves[i], moves[bi]);
				swap(moveValues[i], moveValues[bi]);
			}
		
			int move = moves[i];

			// don't do bad captures in quiescent search.
			if (depth >= limit)
			{
				if(moveValues[i] > 0) break;

				//futility pruning in quiescent search
				int destPiece = m_board.GetPiece(GetDestFromMove(move));
				if(destPiece != Pieces::None)
				{
					int val = Evaluation::PieceValue(Pieces::GetPieceTypeFromPiece(destPiece));
					if(val + score + 150 <= initialAlpha)
					{
						fprune++;
						continue;
					}
				}
			}
			
			// we null move above, skip it here.
			if (move == NullMove) continue;
			BoardState undo;
			m_board.MakeMove(move, false, undo);
#if DEBUGPV
			if (pvAt < pvTest.size())
			{
				if (pvTest[pvAt++] == GetUCIString(move))
				{
					pvMatch.push_back(true);
				}
				else
				{
					pvMatch.push_back(false);
				}
			}
#endif
			int theirMove = 0;

			if (m_board.GetCapturedPiece() == PieceTypes::King)
			{
				moveScore = CHECKMATE;
			}
			else
			{
				moveCount++;

				// depth reduction.
				int nextDepth = depth + 1;
				if (moveCount > 4 && limit - depth >= 3)
				{
					// don't reduce if in check or if material changes.
					if (!m_board.GetIsInCheck() && m_board.GetCapturedPiece() == PieceTypes::None && GetPromoFromMove(move) == PieceTypes::None &&
						// don't reduce if >= 10% of these moves fail high
						((float) historyTableFailHigh[GetSourceFromMove(move)][GetDestFromMove(move)]) / historyTableCounter[GetSourceFromMove(move)][GetDestFromMove(move)] <= 0.1)// &&
						// don't reduce if in a pv node
						//alpha + 1 < beta)
					{
						nextDepth++;
					}
				}

				if (isPvNode)
				{
					// Principle variation search.  If we have already found a result which is part of the prinicple variation, assume all others will fail.
					moveScore = -AlphaBeta(limit, nextDepth, theirMove, nextPvList, -alpha - 1, -alpha);

					// if we find a move that is not a beta cutoff, but improves the current result, we need to search with a proper window.
					if (moveScore > alpha && moveScore < beta)
					{
						moveScore = -AlphaBeta(limit, depth + 1, theirMove, nextPvList, -beta, -alpha);
					}
				}
				else
				{
					moveScore = -AlphaBeta(limit, nextDepth, theirMove, nextPvList, -beta, -alpha);
					if (nextDepth != depth + 1 && moveScore > alpha && moveScore < beta)
					{
						moveScore = -AlphaBeta(limit, depth + 1, theirMove, nextPvList, -beta, -alpha);
					}
				}

				if (moveScore != -CHECKMATE)
				{
					madeLegalMove = true;
				}
				// Lower the score for checkmate by one for every ply, ensuring that we will find the fastest capture, and will resist checkmate as much as possible.
				moveScore = Search::AdjustCheckmateForDepth(moveScore);
			}					
			m_board.UndoMove(move, false, undo);
#if DEBUGPV
			if (pvAt < pvTest.size())
			{
				pvAt--;
				pvMatch.pop_back();
			}
#endif
			if (timeExceeded)
			{
				return bestMoveScore;
			}

			bestMoveScore = max(moveScore, bestMoveScore);

			historyTableCounter[GetSourceFromMove(move)][GetDestFromMove(move)]++;
			if (moveScore > alpha)
			{
				isPvNode = true;
				alpha = moveScore;
				chosenMove = move;
				if (alpha >= beta)
				{
					historyTableFailHigh[GetSourceFromMove(move)][GetDestFromMove(move)] ++;
					StoreTranspositionEntry(chosenMove, alpha, limit, depth, initialAlpha, beta);
					return alpha;
				}
				if (depth < limit && pvList != 0)
				{
					pvList->resize(nextPvList->size() + 1);
					(*pvList)[0] = chosenMove;
					copy(nextPvList->begin(), nextPvList->end(), pvList->begin() + 1);
				}
			}
			else
			{
				historyTableFailLow[GetSourceFromMove(move)][GetDestFromMove(move)] ++;
			}
		}

		if (!madeLegalMove && depth < limit)
		{
			// if I'm not in check, this is a draw.
			if (!m_board.GetIsInCheck())
			{
				bestMoveScore = max(bestMoveScore, 0);
			}
			else
			{
				bestMoveScore = -CHECKMATE;
			}
		}
		if(madeLegalMove)
		{
			StoreTranspositionEntry(chosenMove, bestMoveScore, limit, depth, initialAlpha, beta);
		}
		return bestMoveScore;
	}
	
	int Search::Score(int activeColor, Board& board)
	{
		return evaluation.GetScore() * (activeColor == Colors::White ? 1 : -1);
	}

	int Search::Score(int activeColor)
	{
		return Score(activeColor, m_board);
	}

	void Search::StoreTranspositionEntry(int move, int eval, int limit, int depth, int alpha, int beta)
	{
		StoreTranspositionEntry(move, eval, limit, depth, alpha, beta, m_board);
	}
	void Search::StoreTranspositionEntry(int move, int eval, int limit, int depth, int alpha, int beta, Board& board)
	{
		// don't store uninitialized moves.
		if(move == 0)// || move == NullMove)
		{
			return;
		}
		if(!moveGenerator.IsValidMove(move) && (move != NullMove || eval > alpha))
		{
			cout << "bad hash " << endl;
			cout << GetUCIString(move) << endl;
			cout << m_board.ToString() << endl;
			if(m_board.GetEnPassantRights().HasEnPassant())
			{
				cout << "En passant: " << GetRow(m_board.GetEnPassantRights().CaptureSquare()) << " " << GetCol(m_board.GetEnPassantRights().CaptureSquare()) << endl;
			}
		}
		// don't store quiescent search results in the table
		if (depth < limit)
		{
			TranspositionTable::EntryType entryType;

			if (eval <= alpha)
			{
				entryType = TranspositionTable::AlphaCutoff;
				eval = alpha;
			}
			else if (eval >= beta)
			{
				entryType = TranspositionTable::BetaCutoff;
				eval = beta;
			}
			else
			{
				entryType = TranspositionTable::Exact;
			}

			transpositionTable.SetEntry(board.GetZobristKey(), move, eval, limit - depth, entryType);

			if (move != NullMove && entryType == TranspositionTable::BetaCutoff)
			{
				transpositionTable.EnsureKillerDepth(depth);

				int destPiece = m_board.GetPiece(GetDestFromMove(move));
				// don't store a capture or promotion
				if (destPiece == Pieces::None && GetPromoFromMove(move) == PieceTypes::None)
				{
					if (!MoveEq(transpositionTable.killerMove1[depth], move) && !MoveEq(transpositionTable.killerMove2[depth], move))
					{
						transpositionTable.killer2Score[depth] = transpositionTable.killer1Score[depth];
						transpositionTable.killerMove2[depth] = transpositionTable.killerMove1[depth];
						transpositionTable.killer1Score[depth] = eval;
						transpositionTable.killerMove1[depth] = move;	
					}
				}
			}
		}
	}
	
	void Search::ScoreMoves(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList)
	{
		for (uint i = 0; i < moves.size(); i++)
		{
			int move = moves[i];
			int moveValue;
			if (MoveEq(move, firstMove))
			{
				moveValue  = -999999997;
			}
			else if (MoveEq(move, killer1))
			{
				moveValue  = -999999996;
			}
			else if (MoveEq(move, killer2))
			{
				moveValue  = -999999995;
			}
			else
			{
				moveValue  = MoveValue(move) + i;
			}
			moveValueList.push_back(moveValue);
		}
	}	
	
	void Search::ScoreMovesExpensive(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList)
	{
		for (uint i = 0; i < moves.size(); i++)
		{
			int move = moves[i];
			int moveValue;
			if (MoveEq(move, firstMove))
			{
				moveValue  = -999999997;
			}
			else if (MoveEq(move, killer1))
			{
				moveValue  = -999999996;
			}
			else if (MoveEq(move, killer2))
			{
				moveValue  = -999999995;
			}
			else
			{
				moveValue  = MoveValueExpensive(move) + i;
			}
			moveValueList.push_back(moveValue);
		}
	}

	void Search::SortMoves(vector<int>& moves, int firstMove, int killer1, int killer2, vector<int>& moveValueList)
	{
		static vector<pair<int, int>> moveList;
		moveList.clear();
		
		for (uint i = 0; i < moves.size(); i++)
		{
			int move = moves[i];
			int moveValue;
			if (MoveEq(move,firstMove))
			{
				moveValue = -999999997;
			}
			else if (MoveEq(move, killer1))
			{
				moveValue = -999999996;
			}
			else if (MoveEq(move,killer2))
			{
				moveValue = -999999995;
			}
			else
			{
				moveValue = MoveValue(move);
			}
			moveList.push_back(make_pair(moveValue, move));
		}

		sort(moveList.begin(), moveList.end());

		for (uint i = 0; i < moves.size(); i++)
		{
			moves[i] = moveList[i].second;
			moveValueList.push_back(moveList[i].first);
		}
	}

	int Search::MoveValueExpensive(int move)
	{
		int dest = m_board.GetPiece(GetDestFromMove(move));
		int source = m_board.GetPiece(GetSourceFromMove(move));
		int makeItPositive = m_board.Turn() == Colors::White ? 1 : -1;
		int makeItNegative = -makeItPositive;
		int moveDelta = (Evaluation::PieceSquareValue(source, GetDestFromMove(move)) - Evaluation::PieceSquareValue(source, GetSourceFromMove(move))) * 100;
		
		// good move ordering here:
		int history = -(historyTableFailHigh[GetSourceFromMove(move)][GetDestFromMove(move)] - historyTableFailLow[GetSourceFromMove(move)][GetDestFromMove(move)]) * 10000 / (historyTableCounter[GetSourceFromMove(move)][GetDestFromMove(move)] + 1);

		BoardState undoState;
		m_board.MakeMove(move, false, undoState);
		int score = evaluation.GetScore() * makeItNegative * 1000;
		m_board.UndoMove(move, false, undoState);

		// do captures next
		if (dest != Pieces::None)
		{
			int seeValue = staticExchangeEvaluator.Evaluate(move);
			
			// good capture, do it first.
			if (seeValue > 0)
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative + history + score;
				return val;
			}
			else if (seeValue == 0) // neutral captures next.  mark as negative so that we check quiescent search.
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative - 100000 + history + score;
				return val;
			}
			else // bad captures.  Do them last.
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative + 2000000 + history + score;
				return val;
			}
		} // promotions should be searched early
		else if (GetPromoFromMove(move) != PieceTypes::None)
		{
			int promotionValue = -Evaluation::PieceValue(GetPromoFromMove(move));
			// don't bother searching non-queen promotions early
			if (GetPromoFromMove(move) != PieceTypes::Queen)
			{
				promotionValue = 1000000;
			}
			int val = promotionValue*10000 + moveDelta * makeItNegative - 50000000 + history + score;
			return val;
		}
		else
		{
			// do normal moves last, in order of piece value
			// big constant to ensure this comes after all good captures.

			int val = moveDelta * makeItNegative + 1000000 + history + score;
			return val;
		}
	}

	// A numeric value representing the value of a move.  Lower values are better and should be tried first.	
	int Search::MoveValue(int move)
	{
		int dest = m_board.GetPiece(GetDestFromMove(move));
		int source = m_board.GetPiece(GetSourceFromMove(move));
		int makeItPositive = m_board.Turn() == Colors::White ? 1 : -1;
		int makeItNegative = -makeItPositive;
		int moveDelta = (Evaluation::PieceSquareValue(source, GetDestFromMove(move)) - Evaluation::PieceSquareValue(source, GetSourceFromMove(move))) * 100;
		
		// good move ordering here:
		int history = -(historyTableFailHigh[GetSourceFromMove(move)][GetDestFromMove(move)] - historyTableFailLow[GetSourceFromMove(move)][GetDestFromMove(move)]) * 10000 / (historyTableCounter[GetSourceFromMove(move)][GetDestFromMove(move)] + 1);

		// do captures next
		if (dest != Pieces::None)
		{
			int seeValue = staticExchangeEvaluator.Evaluate(move);
			
			// good capture, do it first.
			if (seeValue > 0)
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative + history;
				return val;
			}
			else if (seeValue == 0) // neutral captures next.  mark as negative so that we check quiescent search.
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative - 100000 + history;
				return val;
			}
			else // bad captures.  Do them last.
			{
				int val = -seeValue * 10000 + moveDelta * makeItNegative + 2000000 + history;
				return val;
			}
		} // promotions should be searched early
		else if (GetPromoFromMove(move) != PieceTypes::None)
		{
			int promotionValue = -Evaluation::PieceValue(GetPromoFromMove(move));
			// don't bother searching non-queen promotions early
			if (GetPromoFromMove(move) != PieceTypes::Queen)
			{
				promotionValue = 1000000;
			}
			int val = promotionValue*10000 + moveDelta * makeItNegative - 50000000 + history;
			return val;
		}
		else
		{
			// do normal moves last, in order of piece value
			// big constant to ensure this comes after all good captures.
			int val = moveDelta * makeItNegative + 1000000 + history;
			return val;
		}
	}
}