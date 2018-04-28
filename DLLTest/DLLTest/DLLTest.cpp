// DLLTest.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DLLTest.h"
#include <Windows.h>
#include "pipe.h"
#include "plugin.h"
#include "timeque.h"
#include <stdio.h>

#include <stdlib.h>

#include "type.h"
#include <process.h>


st_event g_time_event;
st_event g_addr_event;
typedef int(*callback_type)(int);

//反始化
int SIMC_UnInit()
{
	//simc_fifo_uninit();
	//close the connection to the model
	//close(cadi, cadi_callback);
	//simulation->Release(true); // release and shut down the simulation
	//时间事件队列反初始化
	simc_event_queue_uninit(&g_time_event);
	simc_event_queue_uninit(&g_addr_event);
}
//初始化
int SIMC_Init()
{
	printf("entry SIMC_Init\n");

	//管道初始化，等待客户端连接
	simc_fifo_init();

	//插件初始化
	simc_plugin_init();

	//时间事件队列初始化
	simc_event_queue_init(&g_time_event);

	//地址事件队列初始化
	simc_event_queue_init(&g_addr_event);

	printf("leave SIMC_Init\n");
	return 0;
}

//启动线程执行程序
int SIMC_Run()
{
	printf("entry SIMC_Run\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_run();
	printf("exit SIMC_Run\n");
	return nRet;
}

// 单步执行
int SIMC_Step(int nCount)
{
	printf("entry SIMC_Step\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_stepn(nCount);
	//runcontrol_stepn(100, cadi, cadi_callback, true);
	printf("exit SIMC_Step\n");
	return nRet;
}
//停止执行
int SIMC_Stop()
{
	printf("entry SIMC_Stop\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_stop();
	//runcontrol_stepn(100, cadi, cadi_callback, true);
	printf("exit SIMC_Stop\n");
	return nRet;
}

//写入指令寄存器的值
int SIMC_WriteReg(UINT32 nRegID, UINT64 nValue)
{
	printf("entry SIMC_WriteReg\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_registers_write(nRegID, nValue);
	printf("exit SIMC_WriteReg\n");
	return nRet;
}

//获得指令寄存器的值
int SIMC_ReadReg(UINT32 nRegID, UINT64 nValue)
{
	printf("entry SIMC_ReadReg\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_registers_read(nRegID, nValue);
	printf("exit SIMC_ReadReg\n");
	return nRet;
}
//获取指定地址段值 
int SIMC_ReadMem(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth)
{
	printf("entry SIMC_ReadMem\n");
	int nRet = SIMC_OK;
	if (NULL == pbBuf)
	{
		printf("SIMC_ReadMem pbBuf is NULL\n");
		return SIMC_ERROR;
	}
	nRet = simc_plugin_mem_read(nStartAddr, nLength, pbBuf, nDataWidth);
	printf("exit SIMC_ReadMem\n");
	return nRet;
}

//设置指定地址段值
int SIMC_WriteMem(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth)
{
	printf("entry SIMC_WriteMem\n");
	int nRet = SIMC_OK;
	if (NULL == pbBuf)
	{
		printf("SIMC_WriteMem pbBuf is NULL\n");
		return SIMC_ERROR;
	}
	nRet = simc_plugin_mem_write(nStartAddr, nLength, pbBuf, nDataWidth);
	printf("exit SIMC_WriteMem\n");
	return nRet;
}

//增加一个断点
int SIMC_AddBreakPoint(UINT64 nAddr)
{
	printf("entry SIMC_AddBreakPoint\n");
	int bptnum;
	bptnum = simc_plugin_addbreakpoint(nAddr);
	printf("exit SIMC_AddBreakPoint\n");
	return bptnum;
}

// 删除一个断点
int SIMC_RemoveBreakPoint(UINT64 nbreakpointId)
{
	printf("entry SIMC_RemoveBreakPoint\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_removebreakpoint(nbreakpointId);
	printf("exit SIMC_RemoveBreakPoint\n");
	return nRet;
}

//获取当前 CPU 状态
UINT32 SIMC_GetStatus(UINT32* pnErrorCode)
{
	printf("entry SIMC_GetStatus\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_get_status(pnErrorCode);
	printf("exit SIMC_GetStatus\n");
	return nRet;
}
//获取以运行的周期数
int SIMC_GetExecCycle(UINT64* pnCycleCount)
{
	printf("entry SIMC_GetExecCycle\n");
	int nRet = SIMC_OK;
	nRet = simc_plugin_get_execcycle(pnCycleCount);
	printf("exit SIMC_GetExecCycle\n");
	return nRet;
}
//增加地址事件

int SIMC_AddAddressEvent(void * pfnCallback, UINT64 nArgument, UINT64 nType, UINT64 nAddr, UINT64 nLength)
{
	int ret = SIMC_OK;
	printf("--------------------%p\n", pfnCallback);
	if (NULL == pfnCallback)
	{
		printf("param pfnCallback is NULL\n");
		return SIMC_ERROR;
	}
	//获取状态，需要状态为停止，才可以增加时间事件
	st_addr_tx addr_data = { 0 };
	addr_data.cbaddr = pfnCallback;
	addr_data.nArgument = nArgument;
	addr_data.nType = nType;
	addr_data.nLength = nLength;
	//加入地址事件队列
	ret = simc_addr_queue_append(g_addr_event.queue, &addr_data);
	if (SIMC_OK != ret)
	{
		printf("add addr queue failed,ret = %d\n", ret);
		return ret;
	}

	//打印队列元素
	st_addr_tx* paddr_data = NULL;
	int nLen = simc_queue_len(g_addr_event.queue);
	int iPos = 0;

	printf("LinkQueue_Length = %d\n", nLen);
	for (iPos = 0; iPos < nLen; iPos++)
	{
		paddr_data = (st_addr_tx*)simc_time_queue_pos(g_addr_event.queue, iPos);
		if (paddr_data != NULL)
		{

		printf("%p\n", paddr_data->cbaddr);
		printf("%d\n", paddr_data->nArgument);
		printf("%d\n", paddr_data->nType);
		}
	}


	while (LinkQueue_Length(g_addr_event.queue) > 0)
	{
		paddr_data = (st_addr_tx*)LinkQueue_Retrieve(g_addr_event.queue);
		if (paddr_data != NULL)
		{

		printf("%p\n", paddr_data->cbaddr);
		printf("%d\n", paddr_data->nArgument);
		printf("%d\n", paddr_data->nType);
		}
	}

	/*
	((callback_type)pfCallback)(nArgument);

	printf("addtimeevent success\n");
	*/
	return ret;
}

//增加时间事件
int SIMC_AddTimeEvent(void * pfCallback, UINT32 nArgument, UINT32 nDeltCycle)
{
	int ret = SIMC_OK;
	printf("--------------------%p\n", pfCallback);
	if (NULL == pfCallback)
	{
		printf("param pfCallback is NULL\n");
		return SIMC_ERROR;
	}

	//获取状态，需要状态为停止，才可以增加时间事件
	/*if (simc_plugin_get_mode() != 0)
	{
		printf("still running, addtimeevent failed\n");
		return SIMC_ERROR;
	}
	*/
	st_time_tx tx_data = { 0 };
	tx_data.cbaddr = pfCallback;
	tx_data.nArgument = nArgument;
	tx_data.nDeltCycle = nDeltCycle;


	//加入时间事件队列
	ret = simc_time_queue_append(g_time_event.queue, &tx_data);
    if(SIMC_OK != ret)
	{
		printf("add time queue failed,ret = %d\n",ret);
		return ret;
	}


/*
	//打印队列元素
	st_tx* rx_data = NULL;
	int nLen = simc_time_queue_len(g_time_event.queue);
	int iPos = 0;

	printf("LinkQueue_Length = %d\n", nLen);
	for (iPos = 0; iPos < nLen; iPos++)
	{
		rx_data = (st_tx*)simc_time_queue_pos(g_time_event.queue, iPos);
		if (rx_data != NULL)
		{

			printf("%p\n", rx_data->cbaddr);
			printf("%d\n", rx_data->nArgument);
			printf("%d\n", rx_data->nDeltCycle);
		}
	}
	*/
	/*
	while (LinkQueue_Length(g_time_event.queue) > 0)
	{
		rx_data = (st_tx*)LinkQueue_Retrieve(g_time_event.queue);
		if (rx_data != NULL)
		{

			printf("%p\n",rx_data->cbaddr);
			printf("%d\n",rx_data->nArgument);
			printf("%d\n",rx_data->nDeltCycle);
		}
	}
	*/
/*	
	((callback_type)pfCallback)(nArgument);

	printf("addtimeevent success\n");
*/
	return ret;
}
int test1(int a, int b)
{
	return a + b;
}