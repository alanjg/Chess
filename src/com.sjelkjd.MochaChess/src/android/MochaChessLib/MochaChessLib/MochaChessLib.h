#pragma once
#include <jni.h>

extern "C" {

	JNIEXPORT jstring JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_GetPiece(JNIEnv *, jclass, jint, jint);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeBestMove(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeMove(JNIEnv *, jclass, jint, jint, jint, jint);
	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMoveStart(JNIEnv *, jclass, jint, jint);
	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMove(JNIEnv *, jclass, jint, jint, jint, jint);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_InitializeBoard(JNIEnv *, jclass);

}
