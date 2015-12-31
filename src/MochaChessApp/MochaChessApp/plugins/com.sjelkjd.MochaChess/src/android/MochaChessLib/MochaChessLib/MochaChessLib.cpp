#include "MochaChessLib.h"
#include "ChessEngine.h"
static std::auto_ptr<SjelkjdChessEngine::ChessEngine> chessEngine;
extern "C" {

	JNIEXPORT jstring JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_GetPiece(JNIEnv *je, jclass jc, jint row, jint col)
	{
		std::string piece = chessEngine->GetPiece(row, col);
		return je->NewStringUTF(piece.c_str());
	}


	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeBestMove(JNIEnv *je, jclass jc)
	{
		chessEngine->MakeComputerMove();
	}

	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeMove(JNIEnv *je, jclass jc, jint startRow, jint startCol, jint endRow, jint endCol)
	{
		chessEngine->MakeMove(startRow, startCol, endRow, endCol);
	}

	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMoveStart(JNIEnv *je, jclass jc, jint row, jint col)
	{
		
		return chessEngine->isValidMoveStart(row, col);
	}
	
	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMove(JNIEnv *je, jclass jc, jint startRow, jint startCol, jint endRow, jint endCol)
	{
		return chessEngine->isValidMove(startRow, startCol, endRow, endCol);
	}
	
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_InitializeBoard(JNIEnv *je, jclass jc)
	{
		chessEngine.reset(new SjelkjdChessEngine::ChessEngine());
		chessEngine->initializeBoard();
	}

}


