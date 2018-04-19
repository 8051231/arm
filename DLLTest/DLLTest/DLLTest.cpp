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
st_time_event g_time_event;
typedef int(*callback_type)(int);

//反始化
int SIMC_UnInit()
{
	//simc_fifo_uninit();
	//close the connection to the model
	//close(cadi, cadi_callback);
	//simulation->Release(true); // release and shut down the simulation
	//时间事件队列反初始化
	simc_time_event_queue_uninit(&g_time_event);
}
//初始化
int SIMC_Init()
{
	printf("entry SIMC_Init\n");

	//管道初始化，等待客户端连接
	//simc_fifo_init();

	//插件初始化
	simc_plugin_init();

	//时间事件队列初始化
	//simc_time_event_queue_init(&g_time_event);


	printf("leave SIMC_Init\n");
	return 0;
}

//启动线程执行程序
int SIMC_Run()
{
	printf("entry SIMC_Run\n");
	simc_plugin_run();
	printf("exit SIMC_Run\n");
	return 0;
}

// 单步执行
int SIMC_Step(int nCount)
{
	printf("entry SIMC_Step\n");
	simc_plugin_stepn(nCount);
	//runcontrol_stepn(100, cadi, cadi_callback, true);
	printf("exit SIMC_Step\n");
	return 0;
}
//停止执行
int SIMC_Stop()
{
	printf("entry SIMC_Stop\n");
	simc_plugin_stop();
	//runcontrol_stepn(100, cadi, cadi_callback, true);
	printf("exit SIMC_Stop\n");
}

//获得指令寄存器的值
int SIMC_ReadReg(UINT32 nRegID, UCHAR* pbValue)
{
	return 0;
}

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
	st_tx tx_data = { 0 };
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