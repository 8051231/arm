//#pragma once
#ifndef _timeque_H_
#define _timeque_H_

#include"linkqueue.h"

//时间事件队列
typedef struct _tar_time_event
{
	LinkQueue *queue;
}st_time_event, *pst_time_evnet;

//时间事件结构体
typedef struct _tar_tx
{
	void *cbaddr; //回调函数指针
	UINT32 nArgument; //回调函数参数
	UINT32 nDeltCycle; // 时间周期
}st_tx;


//时间事件队列初始化
int simc_time_event_queue_init(st_time_event *param);

//时间事件队列反初始化
int simc_time_event_queue_uninit(st_time_event *param);

//加入时间事件队列
int simc_time_queue_append(LinkQueue* queue, void *p);

//加入时间事件队列长度

int simc_time_queue_len(LinkQueue* queue);

//根据节点序号获取时间事件队列指定节点
void* simc_time_queue_pos(LinkQueue* queue, int npos);

int simc_time_queue_up_node(LinkQueue* queue, int nPos);

#endif