#pragma once


typedef HFONT (WINAPI *fnCreateFontA)(
									 int nHeight, // logical height of font height
									 int nWidth, // logical average character width
									 int nEscapement, // angle of escapement
									 int nOrientation, // base-line orientation angle
									 int fnWeight, // font weight
									 DWORD fdwItalic, // italic attribute flag
									 DWORD fdwUnderline, // underline attribute flag
									 DWORD fdwStrikeOut, // strikeout attribute flag
									 DWORD fdwCharSet, // character set identifier
									 DWORD fdwOutputPrecision, // output precision
									 DWORD fdwClipPrecision, // clipping precision
									 DWORD fdwQuality, // output quality
									 DWORD fdwPitchAndFamily, // pitch and family
									 LPCSTR lpszFace // pointer to typeface name string
									 );

typedef FARPROC (WINAPI * fnGetProcAddress)(HMODULE hModule,LPCSTR lpFuncName);

typedef DWORD (*fnPalLoadSpr) (DWORD,CHAR* FileName,PBYTE DataBuffers,DWORD DataSize);