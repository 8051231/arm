#ifndef _MY_LINKQUEUE_H_
#define _MY_LINKQUEUE_H_

#define QUEUE_MAX 256

typedef void LinkQueue;

LinkQueue* LinkQueue_Create();

void LinkQueue_Destroy(LinkQueue* queue);

void LinkQueue_Clear(LinkQueue* queue);

int LinkQueue_Append(LinkQueue* queue, void* item);

void* LinkQueue_Retrieve(LinkQueue* queue);
void* LinkQueue_Retrieve_Pos(LinkQueue* queue, int nPos);

void* LinkQueue_Header(LinkQueue* queue);

void* LinkQueue_Pos(LinkQueue* queue, int pos);
//根据传入参数，获取队列第npos个节点
void* LinkQueue_Pos_Up_Data(LinkQueue* queue, int npos);

int LinkQueue_Length(LinkQueue* queue);

#endif //_MY_LINKQUEUE_H_