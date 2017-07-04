#pragma comment(linker,"/ENTRY:main")
#pragma comment(linker,"/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker,"/SECTION:.Amano,ERW /MERGE:.text=.Amano")

#include "CatSystem2.h"
#include "Mem.cpp"

ForceInline Void main2(Int argc, WChar **argv)
{
    if (argc == 1)
        return;

    CCatSystem2Unpacker cs2;

    while (--argc)
        cs2.Auto(*++argv);
}

void __cdecl main(Int argc, WChar **argv)
{
    getargsW(&argc, &argv);
    main2(argc, argv);
    exit(0);
}