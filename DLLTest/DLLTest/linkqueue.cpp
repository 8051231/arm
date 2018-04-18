#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "linkqueue.h"
#include "linklist.h"

typedef struct _tag_LinkQueueNode
{
	LinkListNode node;

	void *item;
}TLinkQueueNode;

LinkQueue* LinkQueue_Create()
{
	return LinkList_Create();
}

void LinkQueue_Destroy(LinkQueue* queue)
{
	LinkQueue_Clear(queue);
	LinkList_Destroy(queue);
	return;
}

void LinkQueue_Clear(LinkQueue* queue)
{
	while (LinkQueue_Length(queue) > 0)
	{
		LinkQueue_Retrieve(queue);
	}
	return;
}

int LinkQueue_Append(LinkQueue* queue, void* item)
{
	int ret = 0;
	TLinkQueueNode *node = NULL;

	if (LinkQueue_Length(queue)  >  QUEUE_MAX)
	{
		return -1;
	}

	node = (TLinkQueueNode *)malloc(sizeof(TLinkQueueNode));
	if (node == NULL)
	{
		return -1;
	}
	memset(node, 0, sizeof(TLinkQueueNode));
	node->item = item;

	ret = LinkList_Insert(queue, (LinkListNode *)node, LinkList_Length(queue));
	if (ret != 0)
	{
		free(node);
		return ret;
	}

	return ret;
}


//删除队列首节点

void* LinkQueue_Retrieve(LinkQueue* queue)
{
	int ret = 0;
	void *item = NULL;
	TLinkQueueNode *node = NULL;

	node = (TLinkQueueNode *)LinkList_Delete(queue, 0);
	if (node == NULL)
	{
		return (void *)-1;
	}
	item = node->item;
	if (node != NULL)
	{
		free(node);
		node = NULL;
	}
	return item;
}

//删除队列第nPos 个节点

void* LinkQueue_Retrieve_Pos(LinkQueue* queue,int nPos)
{
	int ret = 0;
	void *item = NULL;
	TLinkQueueNode *node = NULL;

	node = (TLinkQueueNode *)LinkList_Delete(queue, nPos);
	if (node == NULL)
	{
		return (void *)-1;
	}
	item = node->item;
	if (node != NULL)
	{
		free(node);
		node = NULL;
	}
	return item;
}

//获取队列第一个节点
void* LinkQueue_Header(LinkQueue* queue)
{
	int ret = 0;
	void *item = NULL;
	TLinkQueueNode *node = NULL;
	node = (TLinkQueueNode *)LinkList_Get(queue, 0);
	if (node == NULL)
	{
		return NULL;
	}
	item = node->item;
	return item;
}

//根据传入参数，获取队列第npos个节点
void* LinkQueue_Pos(LinkQueue* queue, int npos)
{
	int ret = 0;
	void *item = NULL;
	TLinkQueueNode *node = NULL;
	node = (TLinkQueueNode *)LinkList_Get(queue, npos);
	if (node == NULL)
	{
		return NULL;
	}
	item = node->item;
	return item;
}

//根据传入参数，获取队列第npos个节点
void* LinkQueue_Pos_Up_Data(LinkQueue* queue, int npos)
{
	int ret = 0;
	void *item = NULL;
	TLinkQueueNode *node = NULL;
	node = (TLinkQueueNode *)LinkList_Get(queue, npos);
	if (node == NULL)
	{
		return NULL;
	}
	item = node->item;
	return item;
}


int LinkQueue_Length(LinkQueue* queue)
{
	return LinkList_Length(queue);
}

