//#pragma once
#ifndef _timeque_H_
#define _timeque_H_

#include"linkqueue.h"

//事件队列
typedef struct _tar_event
{
	LinkQueue *queue;
}st_event, *pst_evnet;

//时间事件结构体
typedef struct _tar_time_tx
{
	void *cbaddr; //回调函数指针
	UINT32 nArgument; //回调函数参数
	UINT32 nDeltCycle; // 时间周期
}st_time_tx;


//地址事件结构体
typedef struct _tar_addr_tx
{
	void *cbaddr; //回调函数指针
	UINT64 nArgument; //回调函数参数
	UINT64 nType; 
	UINT64 nAddr; //起始地址
	UINT64 nLength; // 长度
}st_addr_tx;


//事件队列初始化
int simc_event_queue_init(st_event *param);

//事件队列反初始化
int simc_event_queue_uninit(st_event *param);

//加入时间事件队列
int simc_time_queue_append(LinkQueue* queue, void *pdata);

//加入地址事件队列
int simc_addr_queue_append(LinkQueue* queue, void *pdata);

//事件队列长度

int simc_queue_len(LinkQueue* queue);

//根据节点序号获取时间事件队列指定节点
void* simc_time_queue_pos(LinkQueue* queue, int npos);

int simc_time_queue_up_node(LinkQueue* queue, int nPos);

int simc_addr_queue_up_node(LinkQueue* queue, int nPos, UINT32 pc);

#endif