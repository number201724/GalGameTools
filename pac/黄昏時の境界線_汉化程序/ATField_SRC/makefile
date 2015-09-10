ATField.dll:main.obj
	link /subsystem:windows /DLL /DEBUG /OPT:REF main.obj detours.lib gdi32.lib kernel32.lib user32.lib /OUT:ATField.dll
main.obj:main.cpp
	cl /c /O2 /Ob1 /EHa /D_CRT_SECURE_NO_DEPRECATE main.cpp