//#pragma once
#ifndef _DLLTEST_H_
#define _DLLTEST_H_

#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	//初始化
	DLL_EXPORT int SIMC_Init();

	//启动线程执行程序
	DLL_EXPORT int SIMC_Run();





	DLL_EXPORT int test1(int a, int b);

	//单步执行
	DLL_EXPORT int SIMC_Step(int nCount);


	//暂停执行
	DLL_EXPORT int SIMC_Stop();

	//获得指令寄存器的值
	DLL_EXPORT int SIMC_ReadReg(UINT32 nRegID, UINT64 nValue);

	//获取当前 CPU 状态
	DLL_EXPORT UINT32 SIMC_GetStatus(UINT32* pnErrorCode);

	//获取以运行的周期数
	DLL_EXPORT int SIMC_GetExecCycle(UINT64* pnCycleCount);

	//设置指定寄存器的值
	DLL_EXPORT int SIMC_WriteReg(UINT32 nRegID, UINT64 nValue);
	
	//获取指定地址段值
	DLL_EXPORT int SIMC_ReadMem(UINT32 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);

	//设置指定地址段值
	DLL_EXPORT int SIMC_WriteMem(UINT32 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);


	//增加一个断点
	DLL_EXPORT int SIMC_AddBreakPoint(UINT64 nAddr);

	
	//删除一个断点
	DLL_EXPORT int SIMC_RemoveBreakPoint(UINT32 nbreakpointId);
	
	//删除所有断点
	//DLL_EXPORT int SIMC_RemoveAllBreakPoints(void);

	//向虚拟内核事件队列中增加一个时间事件
	DLL_EXPORT int SIMC_AddTimeEvent(void * pfCallback, UINT32 nArgument, UINT32 nDeltCycle);

	//向虚拟内核事件队列中增加一个内存事件
	DLL_EXPORT int SIMC_AddAddressEvent(void * pfCallback, UINT64 nArgument, UINT64 nType, UINT64 nAddr, UINT64 nLength);
	/*
	//设置覆盖率代码段位置
	DLL_EXPORT int SIMC_CodeCoverageEn(UINT32 StartPC, UINT32 Len);

	//获得内核执行历史记录
	DLL_EXPORT int SIMC_GetCovData(UCHAR* pCoverageFile);
	*/
#ifdef __cplusplus
}
#endif

#endif