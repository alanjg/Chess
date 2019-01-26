#pragma once
#ifndef VALIDATE
#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(nullptr, (msg), "OculusRoomTiny", MB_ICONERROR | MB_OK); exit(-1); }
#endif

#ifndef FATALERROR
#define FATALERROR(msg) { MessageBoxA(nullptr, (msg), "OculusRoomTiny", MB_ICONERROR | MB_OK); exit(-1); }
#endif

// clean up member COM pointers
template<typename T> void Release(T *&obj)
{
	if (!obj) return;
	obj->Release();
	obj = nullptr;
}