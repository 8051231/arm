#include "stdafx.h"
#include "timeque.h"
#include "type.h"
#include <stdlib.h>
#include <stdio.h>

typedef int(*callback_type)(int);

//时间事件反队列初始化
int simc_event_queue_uninit(st_event *param)
{
	if (NULL == param)
	{
		return -1;
	}
	if (param->queue)
	{
		LinkQueue_Destroy(param->queue);
	}
	return 0;
}

//时间事件队列初始化
int simc_event_queue_init(st_event *param)
{
	if (NULL == param)
	{
		return -1;
	}
	param->queue = LinkQueue_Create();
	return 0;
}

//加入地址事件队列
int simc_addr_queue_append(LinkQueue* queue, void *pdata)
{
	int ret = 0;
	if (queue != NULL)
	{
		st_addr_tx* item = (st_addr_tx*)malloc(sizeof(st_addr_tx));
		if (item == NULL)
		{
			return SIMC_ERROR;
		}

		memcpy(item, pdata, sizeof(st_time_tx));

		ret = LinkQueue_Append(queue, item);
		if (ret != SIMC_OK)
		{
			free(item);
			return SIMC_ERROR;
		}
	}
	return ret;
}


//加入时间事件队列
int simc_time_queue_append(LinkQueue* queue, void *pdata)
{
	int ret = 0;
	if (queue != NULL)
	{
		st_time_tx* item = (st_time_tx*)malloc(sizeof(st_time_tx));
		if (item == NULL)
		{
			return SIMC_ERROR;
		}

		memcpy(item, pdata, sizeof(st_time_tx));

		ret = LinkQueue_Append(queue, item);
		if (ret != SIMC_OK)
		{
			free(item);
			return SIMC_ERROR;
		}
	}
	return ret;

}

//获取时间事件队列长度
int simc_queue_len(LinkQueue* queue)
{
	if (NULL == queue)
	{
		return 0;
	}
	int nLen = LinkQueue_Length(queue);
	return nLen;

}

//根据节点序号获取时间事件队列指定节点
void*  simc_time_queue_pos(LinkQueue* queue, int npos)
{
	if (NULL == queue)
	{
		return NULL;
	}
	return LinkQueue_Pos(queue, npos);

}


//根据节点序号更新地址事件队列指定节点
int simc_addr_queue_up_node(LinkQueue* queue, int nPos, UINT32 pc)
{
	int nRet = SIMC_OK;
	if (NULL == queue)
	{
		printf("queue is NULL, return\n");
		return SIMC_QUEUE_NULL;
	}
	st_addr_tx* addr_data = NULL;

	//根据传入参数，更新队列第npos个节点
	addr_data = (st_addr_tx*)LinkQueue_Pos_Up_Data(queue, nPos);

	if (addr_data != NULL)
	{
		printf("addr node:\n");
		printf("%p\n", addr_data->cbaddr);
		printf("%d\n", addr_data->nArgument);
		printf("%d\n", addr_data->nType);
		printf("%d\n", addr_data->nAddr);
		printf("%d\n", addr_data->nLength);
	}
	else
	{
		printf("node is NULL,return\n");
		return SIMC_QUEUE_NODE_NULL;
	}

	if ((pc < (addr_data->nAddr + addr_data->nLength))&& (pc < addr_data->nAddr))
	{
		//处理回调，调用即认为成功

		((callback_type)(addr_data->cbaddr))(addr_data->nArgument);

		//删除本节点
		LinkQueue_Retrieve_Pos(queue, nPos);

		return SIMC_CALLBACK_OK;
	}
	else
	{
		return SIMC_OK;
	}
}

//根据节点序号更新时间事件队列指定节点 ncycle = ncycle - 1
int simc_time_queue_up_node(LinkQueue* queue, int nPos)
{
	int nRet = SIMC_OK;
	if (NULL == queue)
	{
		printf("queue is NULL, return\n");
		return SIMC_QUEUE_NULL;
	}
	st_time_tx* rx_data = NULL;

	//根据传入参数，更新队列第npos个节点
	rx_data = (st_time_tx*)LinkQueue_Pos_Up_Data(queue, nPos);

	if (rx_data != NULL)
	{
		printf("before sub one, node:\n");
		printf("%p\n", rx_data->cbaddr);
		printf("%d\n", rx_data->nArgument);
		printf("%d\n", rx_data->nDeltCycle);
	}
	else
	{
		printf("node is NULL,return\n");
		return SIMC_QUEUE_NODE_NULL;
	}

	rx_data->nDeltCycle = rx_data->nDeltCycle - 1;

	st_time_tx* rx_data_mod = NULL;
	rx_data_mod = (st_time_tx*)simc_time_queue_pos(queue, nPos);
	if (rx_data_mod != NULL)
	{
		printf("after sub one, node:\n");
		printf("%p\n", rx_data_mod->cbaddr);
		printf("%d\n", rx_data_mod->nArgument);
		printf("%d\n", rx_data_mod->nDeltCycle);
	}
	else
	{
		printf("after sub one, node is NULL,return\n");
		return SIMC_QUEUE_NODE_NULL;
	}

	if (0 == rx_data->nDeltCycle)
	{
		//处理回调，调用即认为成功

		((callback_type)(rx_data_mod->cbaddr))(rx_data_mod->nArgument);

		//删除本节点
		LinkQueue_Retrieve_Pos(queue, nPos);

		return SIMC_CALLBACK_OK;
	}
	else
	{
		return SIMC_OK;
	}
}