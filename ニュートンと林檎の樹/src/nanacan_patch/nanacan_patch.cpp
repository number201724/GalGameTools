// nanacan_patch.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "nanacan_patch.h"


// 这是导出变量的一个示例
NANACAN_PATCH_API int nnanacan_patch=0;

// 这是导出函数的一个示例。
NANACAN_PATCH_API int fnnanacan_patch(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 nanacan_patch.h
Cnanacan_patch::Cnanacan_patch()
{
	return;
}
