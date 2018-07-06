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

struct Utility
{
	void Output(const char * fnt, ...)
	{
		static char string_text[1000];
		va_list args; va_start(args, fnt);
		vsprintf_s(string_text, fnt, args);
		va_end(args);
		OutputDebugStringA(string_text);
	}
} static Util;