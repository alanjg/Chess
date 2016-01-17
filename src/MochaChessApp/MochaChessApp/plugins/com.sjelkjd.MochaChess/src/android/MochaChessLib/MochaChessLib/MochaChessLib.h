#pragma once
#include <jni.h>

extern "C" {

	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_Initialize(JNIEnv *, jclass);
    JNIEXPORT jstring JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_GetBoard(JNIEnv *, jclass);
	JNIEXPORT jstring JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_GetPiece(JNIEnv *, jclass, jint, jint);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeBestMove(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_MakeMove(JNIEnv *, jclass, jint, jint, jint, jint);
	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMoveStart(JNIEnv *, jclass, jint, jint);
	JNIEXPORT jboolean JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_IsValidMove(JNIEnv *, jclass, jint, jint, jint, jint);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_SetStartPosition(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_org_sjelkjd_MochaChess_MochaChessCordova_SetPosition(JNIEnv *, jclass, jstring);

}
