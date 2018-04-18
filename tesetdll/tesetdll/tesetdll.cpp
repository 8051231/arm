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
	Sleep(10000);
	return 0;

}



int main()
{

	HMODULE hModule = LoadLibrary(_T("DLLTest.dll"));
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



	typedef int(*TYPE_fnDllAddTimeEvent) (void * pfCallback, UINT32 nArgument, UINT32 nDeltCycle);

	TYPE_fnDllAddTimeEvent fnDllSIMC_AddTimeEvent = (TYPE_fnDllAddTimeEvent)GetProcAddress(hModule, "SIMC_AddTimeEvent");
	if (fnDllSIMC_AddTimeEvent != NULL)
	{
		printf("GetProcAddress fnDllSIMC_AddTimeEvent Success\n");
	}


	//int nArg = 100;
	//int ntCycle = 1000;
	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 5);

	printf("fnDllSIMC_AddTimeEvent return = %d\n", n);

	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 3);
//	n = fnDllSIMC_AddTimeEvent((void*)printarg, nArg, 1);
	//n = fnDllSIMC_AddTimeEvent((void*)printarg, 200, 3000);

	printf("fnDllSIMC_AddTimeEvent return = %d\n", n);



	typedef int(*TYPE_fnDllSIMC_Run) ();



	TYPE_fnDllSIMC_Run fnDllSIMC_Run = (TYPE_fnDllSIMC_Run)GetProcAddress(hModule, "SIMC_Run");
	if (fnDllSIMC_Run != NULL)
	{
		printf("GetProcAddress SIMC_Run Success\n");
	}
	for (int i = 0; i < 6; i++)
	{
		n = fnDllSIMC_Run();
	}
		




	printf("fnDllSIMC_Run return = %d\n", n);


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
	_tsystem(_T("pause"));
	return 0;
}

