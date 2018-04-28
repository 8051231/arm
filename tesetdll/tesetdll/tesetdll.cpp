// tesetdll.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>

/////////////////////////////////////////////////////////////////////////////
// Includes
#include <cstdio>
#include "eslapi/CADI.h"

#include "MyCADICallback.h"
#include "misc.h"
#include "mti.h"
#include "connect.h"
#include "callback.h"
#include "runcontrol.h"
/////////////////////////////////////////////////////////////////////////////

int nArg = 100;
int ntCycle = 1;

int printarg(int n)
{
	printf("test all callback ---nArgument = %d\n", n);
	Sleep(5000);
	return 0;

}



int main()
{

	HMODULE hModule = LoadLibrary(_T("D:\\myprj\\DLLTest\\x64\\Release\\DLLTest.dll"));
	if (hModule == NULL || hModule == INVALID_HANDLE_VALUE) {
		printf("error\n");
		return -1;
	}
	int n = 0;
	/*
	WINBASEAPI FARPROC WINAPI GetProcAddress(
	_In_ HMODULE hModule,
	_In_ LPCSTR  lpProcName //这个是dump /EXPORT *.dll “name”列等号前的值
	);
	返回的是函数或变量的地址，即函数指针或指向变量地址的指针
	*/


	typedef int(*TYPE_fnDllSIMC_Init) ();



	TYPE_fnDllSIMC_Init fnDllSIMC_Init = (TYPE_fnDllSIMC_Init)GetProcAddress(hModule, "SIMC_Init");
	if (fnDllSIMC_Init != NULL)
	{
		printf("GetProcAddress SIMC_Init Success\n");
	}
	n = fnDllSIMC_Init();

	printf("fnDllSIMC_Init return = %d\n", n);

#if 0

	typedef int(*TYPE_fnDllAddTimeEvent) (void * pfCallback, UINT32 nArgument, UINT32 nDeltCycle);

	TYPE_fnDllAddTimeEvent fnDllSIMC_AddTimeEvent = (TYPE_fnDllAddTimeEvent)GetProcAddress(hModule, "SIMC_AddTimeEvent");
	if (fnDllSIMC_AddTimeEvent != NULL)
	{
		printf("GetProcAddress fnDllSIMC_AddTimeEvent Success\n");
	}



	int nArg = 100;
	int ntCycle = 1000;
//	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 5);

//	printf("fnDllSIMC_AddTimeEvent return = %d\n", n);

//	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 3);
//	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 1);
	//n = fnDllSIMC_AddTimeEvent((void*)printarg, 200, 3000);

	printf("fnDllSIMC_AddTimeEvent return = %d\n", n);

#endif

#if 1
	// 向虚拟内核事件队列中增加	一个内存事件
	//Int SIMC_AddAddressEvent(pfnCallback callback, UINT32
	//nArgument, UINT32 nType, UINT32 nAddr, UINT32 nLength);


	typedef int(*TYPE_fnDllAddAddressEvent) (void * pfnCallback, UINT64 nArgument, UINT64 nType, UINT64 nAddr, UINT64 nLength);

	TYPE_fnDllAddAddressEvent fnDllSIMC_AddAddressEvent = (TYPE_fnDllAddAddressEvent)GetProcAddress(hModule, "SIMC_AddAddressEvent");
	if (fnDllSIMC_AddAddressEvent != NULL)
	{
		printf("GetProcAddress fnDllSIMC_AddAddressEvent Success\n");
	}
	UINT64 nArgument = 100;
	UINT64 nType = 0;
	UINT64 nAddr = 0x80000004;
	UINT64 nLength = 10;
	n = fnDllSIMC_AddAddressEvent((void*)printarg, nArgument, nType, nAddr, nLength);

	//	printf("fnDllSIMC_AddTimeEvent return = %d\n", n);

#endif


#if 0
//测试单步执行
	typedef int(*TYPE_fnDllSIMC_Step) (int);



	TYPE_fnDllSIMC_Step fnDllSIMC_Step = (TYPE_fnDllSIMC_Step)GetProcAddress(hModule, "SIMC_Step");
	if (fnDllSIMC_Step != NULL)
	{
		printf("GetProcAddress fnDllSIMC_Step Success\n");
	}
	//	for (int i = 0; i < 6; i++)
	{
		n = fnDllSIMC_Step(10);
	}

#endif
	//while(1)
    Sleep(1000);
#if 0
	//测试执行程序
	typedef int(*TYPE_fnDllSIMC_Run) ();



	TYPE_fnDllSIMC_Run fnDllSIMC_Run = (TYPE_fnDllSIMC_Run)GetProcAddress(hModule, "SIMC_Run");
	if (fnDllSIMC_Run != NULL)
	{
		printf("GetProcAddress SIMC_Run Success\n");
	}
		//for (int i = 0; i < 2; i++)
	{
		n = fnDllSIMC_Run();
	}
	printf("fnDllSIMC_Run return = %d\n", n);

	Sleep(5000);

#endif
	
#if 0
	//测试获取以运行的周期数
	typedef int(*TYPE_fnDllSIMC_get_execcycle) (UINT64* pnCycleCount);



	TYPE_fnDllSIMC_get_execcycle fnDllSIMC_get_execcycle = (TYPE_fnDllSIMC_get_execcycle)GetProcAddress(hModule, "SIMC_GetExecCycle");
	if (fnDllSIMC_get_execcycle != NULL)
	{
		printf("GetProcAddress get_execcycle Success\n");
	}
	//for (int i = 0; i < 2; i++)
	UINT64 nCycleCount = 9999;

    UINT64* pnCycleCount = &nCycleCount;
	n = 0;
    n = fnDllSIMC_get_execcycle(pnCycleCount);

	printf("fnDllSIMC_get_execcycle return = %d, pnCycleCount = %I64u, nCycleCount = %I64u\n", n, *pnCycleCount, nCycleCount);
	while (1)
	Sleep(1000);

#endif

#if 0
	//测试读寄存器
	typedef UINT64 (*TYPE_fnDllSIMC_ReadReg) (UINT32 nRegID, UINT64 nValue);

	TYPE_fnDllSIMC_ReadReg fnDllSIMC_ReadReg = (TYPE_fnDllSIMC_ReadReg)GetProcAddress(hModule, "SIMC_ReadReg");
	if (fnDllSIMC_ReadReg != NULL)
	{
		printf("GetProcAddress ReadReg Success\n");
	}
	UINT32 nRegID = 16;
	UINT64 nValue = 0;
	n = fnDllSIMC_ReadReg(nRegID, nValue);

	printf("fnDllSIMC_ReadReg return = %d, nValue = %I64u\n", n, nValue);
	//while(1)
	Sleep(1000);
#endif



#if 0

	//测试写寄存器
	typedef UINT64(*TYPE_fnDllSIMC_WriteReg) (UINT32 nRegID, UINT64 nValue);

	TYPE_fnDllSIMC_WriteReg fnDllSIMC_WriteReg = (TYPE_fnDllSIMC_WriteReg)GetProcAddress(hModule, "SIMC_WriteReg");
	if (fnDllSIMC_WriteReg != NULL)
	{
		printf("GetProcAddress SIMC_WriteReg Success\n");
	}
	nRegID = 16;
	nValue = 2345678;
	n = fnDllSIMC_WriteReg(nRegID, nValue);

	printf("fnDllSIMC_WriteReg return = %d, nValue = %I64u\n", n, nValue);


	n = fnDllSIMC_ReadReg(nRegID, nValue);

	printf("fnDllSIMC_ReadReg return = %d, nValue = %I64u\n", n, nValue);


	while (1)
		Sleep(5000);
#endif

#if 0
	//测试获取指定地址段值 
	//Int SIMC_ReadMem(UINT32 nStartAddr,UINT32 nLength,UCHAR* pbBuf, UINT32 nDataWidth);

	typedef int(*TYPE_fnDllSIMC_ReadMem) (UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);

	TYPE_fnDllSIMC_ReadMem fnDllSIMC_ReadMem = (TYPE_fnDllSIMC_ReadMem)GetProcAddress(hModule, "SIMC_ReadMem");
	if (fnDllSIMC_ReadMem != NULL)
	{
		printf("GetProcAddress SIMC_ReadMem Success\n");
	}
	UINT64 nStartAddr = 0x8003;
	UINT32 nLength = 40;
	UINT32 nDataWidth = 1;

	UCHAR bBuf[40] = { 0 };
	UCHAR* pbBuf = bBuf;

	n = fnDllSIMC_ReadMem(nStartAddr, nLength, pbBuf, nDataWidth);

	printf("fnDllSIMC_ReadMem return = %d\n", n);
	int k = 0;
	for (k = 0; k < nLength; k++)
	{
		//if ((k % 4) == 0) 
		{
			printf("\t%02x", (unsigned char *)pbBuf[k]);
		}
		
	}

	while (1)
		Sleep(5000);
#endif

#if 0
	//设置指定地址段值
	//Int SIMC_WriteMem(UINT32 nStartAddr,UINT32 nLength,UCHAR* pbBuf, UINT32 nDataWidth);

	typedef int(*TYPE_fnDllSIMC_WriteMem) (UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);

	TYPE_fnDllSIMC_WriteMem fnDllSIMC_WriteMem = (TYPE_fnDllSIMC_WriteMem)GetProcAddress(hModule, "SIMC_WriteMem");
	if (fnDllSIMC_WriteMem != NULL)
	{
		printf("GetProcAddress SIMC_WriteMem Success\n");
	}
	UINT64 nStartAddr = 0x0;
	UINT32 nLength = 10;
	UINT32 nDataWidth = 1;
	
	UCHAR bBuf[20] = { 0x54,0x53,0x52,0x51,0x50,0x49,0x48,0x47,0x46,0x45,0x89,0x85,0x54,0x53,0x52,0x51,0x50,0x49,0x48,0x47 };
	UCHAR* pbBuf = bBuf;

	n = fnDllSIMC_WriteMem(nStartAddr, nLength, pbBuf, nDataWidth);

	printf("fnDllSIMC_WriteMem return = %d\n", n);

	//测试获取指定地址段值 
	//Int SIMC_ReadMem(UINT32 nStartAddr,UINT32 nLength,UCHAR* pbBuf, UINT32 nDataWidth);

	typedef int(*TYPE_fnDllSIMC_ReadMem) (UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);

	TYPE_fnDllSIMC_ReadMem fnDllSIMC_ReadMem = (TYPE_fnDllSIMC_ReadMem)GetProcAddress(hModule, "SIMC_ReadMem");
	if (fnDllSIMC_ReadMem != NULL)
	{
		printf("GetProcAddress SIMC_ReadMem Success\n");
	}
	UINT64 ntmpStartAddr = 0x0;
	UINT32 ntmpLength = 10;
	UINT32 ntmpDataWidth = 1;

	UCHAR btmpBuf[10] = { 0 };
	UCHAR* ptmpbBuf = btmpBuf;

	n = fnDllSIMC_ReadMem(ntmpStartAddr, ntmpLength, ptmpbBuf, ntmpDataWidth);

	printf("fnDllSIMC_ReadMem return = %d\n", n);
	int k = 0;
	for (k = 0; k < nLength; k++)
	{
		//if ((k % 4) == 0) 
		{
			printf("\t%02x", (unsigned char *)btmpBuf[k]);
		}

	}




	while (1)
		Sleep(5000);
#endif



#if 0
		//增加一个断点
		//Int SIMC_AddBreakPoint(UINT32 nAddr);

		typedef int(*TYPE_fnDllSIMC_AddBreakPoint) (UINT64 nAddr);

	TYPE_fnDllSIMC_AddBreakPoint fnDllSIMC_AddBreakPoint = (TYPE_fnDllSIMC_AddBreakPoint)GetProcAddress(hModule, "SIMC_AddBreakPoint");
	if (fnDllSIMC_AddBreakPoint != NULL)
	{
		printf("GetProcAddress SIMC_AddBreakPoint Success\n");
	}
	UINT64 nAddr = 0x80000000;
	n = 0;

	n = fnDllSIMC_AddBreakPoint(nAddr);

	printf("fnDllSIMC_AddBreakPoint return = %d\n", n);
	nAddr = 0x80000004;
	n = fnDllSIMC_AddBreakPoint(nAddr);
	printf("fnDllSIMC_AddBreakPoint return = %d\n", n);


	//while (1)
		Sleep(5000);

	//测试执行程序
	typedef int(*TYPE_fnDllSIMC_Run) ();



	TYPE_fnDllSIMC_Run fnDllSIMC_Run = (TYPE_fnDllSIMC_Run)GetProcAddress(hModule, "SIMC_Run");
	if (fnDllSIMC_Run != NULL)
	{
		printf("GetProcAddress SIMC_Run Success\n");
	}
	//for (int i = 0; i < 2; i++)
	{
	//	n = fnDllSIMC_Run();
	}
	printf("fnDllSIMC_Run return = %d\n", n);

	//while (1)
		Sleep(5000);
#endif

#if 0
	//删除一个断点
	//Int SIMC_RemoveBreakPoint(UINT64 nAddr); 删除一个断点

	typedef int(*TYPE_fnDllSIMC_RemoveBreakPoint) (UINT64 nAddr);

	TYPE_fnDllSIMC_RemoveBreakPoint fnDllSIMC_RemoveBreakPoint = (TYPE_fnDllSIMC_RemoveBreakPoint)GetProcAddress(hModule, "SIMC_RemoveBreakPoint");
	if (fnDllSIMC_RemoveBreakPoint != NULL)
	{
		printf("GetProcAddress SIMC_RemoveBreakPoint Success\n");
	}
	UINT64 nbreakpointId = 2153;
	n = 0;

	n = fnDllSIMC_RemoveBreakPoint(nbreakpointId);

	printf("fnDllSIMC_RemoveBreakPoint return = %d\n", n);


	//while (1)
		Sleep(5000);


	//for (int i = 0; i < 2; i++)
	{
		n = fnDllSIMC_Run();
	}
	printf("fnDllSIMC_Run return = %d\n", n);

	while (1)
		Sleep(5000);
#endif

#if 0
	//测试停止程序
	typedef int(*TYPE_fnDllSIMC_Stop) ();



	TYPE_fnDllSIMC_Stop fnDllSIMC_Stop = (TYPE_fnDllSIMC_Stop)GetProcAddress(hModule, "SIMC_Stop");
	if (fnDllSIMC_Stop != NULL)
	{
		printf("GetProcAddress SIMC_Stop Success\n");
	}

    n = fnDllSIMC_Stop();

	printf("fnDllSIMC_Stop return = %d\n", n);
#endif
	//Sleep(5000);
	//n = fnDllSIMC_Step(10);


	//printf("c = %d\n", c);

	//if (fnDllDemo != NULL)
	//	printf("fnDllDemo() = %d\n", fnDllDemo(1));


	/*
		// Define the MTI plugin to use by setting environment variable FM_TRACE_PLUGINS
    printf("entry SIMC_Init\n");
	std::string strPlugin;

	strPlugin = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\MTI\\SimpleTrace\\x64\\Release_2013\\SimpleTrace.dll";
	set_plugin("D:\\ARM\\FastModelsPortfolio_11.3\\examples\\MTI\\SimpleTrace\\x64\\Release_2013\\SimpleTrace.dll");

	// connect to a already running model, return a cadi pointer to the requested target
	std::string strSim;
	unsigned int targetnum = 2;
	strSim = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\LISA\\FVP_VE\\Build_Cortex-A15x1\\Win64-Release-VC2015\\cadi_system_Win64-Release-VC2015.dll";
	eslapi::CADI *cadi = connect_library(strSim, targetnum, true);


	// load application
	std::string strApp;
	strApp = "D:\\ARM\\FastModelsPortfolio_11.3\\images\\brot_ve_A.axf";
	cadi->CADIExecLoadApplication(strApp.c_str(), true, false, NULL);

	// add a CADI callback to allow correct run controland handling of semihosting, return a pointer to the callback object
	MyCADICallback *cadi_callback = callbacks(cadi, true);

	int i = 0;
	for (i = 0; i <1; i++)
	{
		runcontrol_run(cadi, cadi_callback, true);
	}


	printf("leave SIMC_Init\n");

	while (1)
	{
		Sleep(1000);
	}
	*/
	while (1)
		Sleep(100000);
	_tsystem(_T("pause"));
	return 0;
}

