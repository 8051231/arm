/*        HANDLE WINAPI CreateNamedPipe(  

          LPCTSTRlpName,//管道名
          DWORD dwOpenMode,//管道打开方式
          //PIPE_ACCESS_DUPLEX  该管道是双向的，服务器和客户端进程都可以从管道读取或者向管道写入数据。
          //PIPE_ACCESS_INBOUND 该管道中数据是从客户端流向服务端，即客户端只能写，服务端只能读。
          //PIPE_ACCESS_OUTBOUND 该管道中数据是从服务端流向客户端，即客户端只能读，服务端只能写。
          DWORD dwPipeMode,//管道的模式
          //PIPE_TYPE_BYTE   数据作为一个连续的字节数据流写入管道。
          //PIPE_TYPE_MESSAGE 数据用数据块（名为“消息”或“报文”）的形式写入管道。
          //PIPE_READMODE_BYTE 数据以单独字节的形式从管道中读出。
          //PIPE_READMODE_MESSAGE 数据以名为“消息”的数据块形式从管道中读出（要求指定PIPE_TYPE_MESSAGE）。
          //PIPE_WAIT 同步操作在等待的时候挂起线程。
          //PIPE_NOWAIT 同步操作立即返回。
          DWORD nMaxInstances,//表示该管道所能够创建的最大实例数量。必须是1到常数PIPE_UNLIMITED_INSTANCES(255)间的一个值。
          DWORD nOutBufferSize,//表示管道的输出缓冲区容量，为0表示使用默认大小。
          DWORD nInBufferSize,//表示管道的输入缓冲区容量，为0表示使用默认大小。
          DWORD nDefaultTimeOut,//表示管道的默认等待超时。
          LPSECURITY_ATTRIBUTES lpSecurityAttributes//表示管道的安全属性。
        );
*/
#include "stdafx.h"
#include "pipe.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <stdint.h>
#include <crtdbg.h> //要用到_CrtSetReportMode函数
#include <strsafe.h>
#include <process.h>
#include "timeque.h"
#include "type.h"

#define BUFSIZE 4096  
extern st_time_event g_time_event;
int nQueLen = 0; //队列长度
HANDLE hPipe = NULL;

BOOL fConnected;
DWORD dwThreadId;
HANDLE hThread;
//LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

const int BUFFER_MAX_LEN = 1024;
char buf[BUFFER_MAX_LEN];

HANDLE get, mSend, mutex;
//LPCRITICAL_SECTION cs;
CRITICAL_SECTION cs; 

LPTSTR pStrPipeNameGet = TEXT("\\\\.\\pipe\\Name_pipe_demon_get");


unsigned _stdcall ThreadProc(void* param);


int simc_fifo_queue_proc(uint32_t pc)
{
	int nRet = 0;
	int nTmp = 0;
	for (nTmp = 0; nTmp < nQueLen; nTmp++)
	{
		nRet = simc_time_queue_up_node(g_time_event.queue, nTmp);
		if (SIMC_CALLBACK_OK == nRet)
		{
			nTmp = nTmp - 1;
			nQueLen = simc_time_queue_len(g_time_event.queue);
			printf("simc_time_queue_up_node, callback,delnode success .after del time event que,nTmp = %d, nQueLen = %d\n", nTmp,nQueLen);
		}
		else if(SIMC_OK == nRet)
		{
			printf("simc_time_queue_up_node success\n");
		}
		else
		{
			printf("simc_time_queue_up_node failed\n");
			return nRet;
		}
	}
	return 0;
}

int simc_fifo_proc(uint32_t pc)
{

	int nRet = 0;

	nQueLen = simc_time_queue_len(g_time_event.queue);

	if(nQueLen > 0)
	{
		//遍历队列，周期减一，回调事件处理
		nRet = simc_fifo_queue_proc(pc);
	}
	else
	{
		nRet = 0;
	}

	return nRet;
}

/*
int simc_fifo_init()
{
	// The main loop creates an instance of the named pipe and   
	// then waits for a client to connect to it. When the client   
	// connects, a thread is created to handle communications   
	// with that client, and the loop is repeated.   

	hPipe = CreateNamedPipe(
		lpszPipename,             // pipe name   
		PIPE_ACCESS_DUPLEX,       // read/write access   
		PIPE_TYPE_MESSAGE |       // message type pipe   
		PIPE_READMODE_MESSAGE |   // message-read mode   
		PIPE_WAIT,                // blocking mode   
		PIPE_UNLIMITED_INSTANCES, // max. instances    
		BUFSIZE,                  // output buffer size   
		BUFSIZE,                  // input buffer size   
		0,                        // client time-out   
		NULL);                    // default security attribute   

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		printf("CreatePipe failed\n");
		return -1;
	}
	printf("CreatePipe Success\n");
	return 0;
}
*/
/*
unsigned _stdcall  beginSendThread(PVOID p) {
	printf("服务器Send\n");
	printf("等待连接......\n");

	HANDLE hPipe = CreateNamedPipe(pStrPipeNameSend, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, 0);
	if (ConnectNamedPipe(hPipe, NULL) != NULL)//等待连接。  
	{
		printf("连接成功，开始发送缓冲区数据至B\n");
		while (true)
		{
			WaitForSingleObject(mutex, INFINITE);
			EnterCriticalSection(cs);
			WriteFile(hPipe, buf, (int)dwLen, &dwLen, NULL);
			LeaveCriticalSection(cs);
			Sleep(500);
			ReleaseSemaphore(mutex, 1, NULL);
		}
	}
	else
	{
		printf("连接失败\n");
	}
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);//关闭管道
	return 0;
}
*/

unsigned _stdcall beginGetThread(PVOID p)
{
	printf("Pipe server waiting for connection......\n");
    if (ConnectNamedPipe(hPipe, NULL) != NULL)//等待连接。  
	{
		printf("The connection is successful ,Pipe starts to receive the data\n");
		while (true)
		{
			printf("dlltest pipe true\n");
			//WaitForSingleObject(mutex, INFINITE);
			//EnterCriticalSection(&cs);
			//接收客户端发送的数据
			uint32_t pc = 0;
			int ret = 0;
			DWORD dwLen = 0;
			char buf[4] = {0};
			dwLen = 0;
		//	ReadFile(hPipe, buf, 4, &dwLen, NULL);
			pc = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0]);
			printf("received data from the plug-in %d bytes,contents are:0x%08x\n", dwLen,pc);
		//	printf("------------PC: 0x%08x\n", pc);
			//业务逻辑处理
			
		//	ret = simc_fifo_proc(pc);
			printf("simc_fifo_proc ret =  %d\n", ret);


		/*	if (ret != 0)
			{
				printf("simc_fifo_proc failed\n");
			}
			else//管道回复6666
			{
				uint32_t pc = 6666;
				char pbuf[4] = { 0 };
				DWORD dwLen = 0;
				memcpy(&pbuf[0], &pc, 4);

				char buf[4] = { 66 };
				//向服务端发送数据  
				if (WriteFile(hPipe, pbuf, 4, &dwLen, NULL))
				{
					printf("dll Data write to %d bytes\n", dwLen);
				}
				else
				{
					printf("dll Data write failed\n");
				}
			}

*/
			/*
			int bufSize;
			for (bufSize = 0; bufSize < (int)dwLen; bufSize++) {
				//putchar(buf[bufSize]);
				printf("%c", &buf[bufSize]);
			}
			*/
			//LeaveCriticalSection(&cs);
			Sleep(500);
			//ReleaseSemaphore(mutex, 1, NULL);
			//putchar('\n');
		}
	}
	else
	{
		printf("连接失败\n");
	}

	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);//关闭管道
	return 0;
}
int simc_fifo_uninit()
{

}

int simc_fifo_init()
{
	printf("simc_fifo_init\n");

	hPipe = CreateNamedPipe(pStrPipeNameGet, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_NOWAIT, 0);
	if (INVALID_HANDLE_VALUE == hPipe)
	{
		printf("CreateNamedPipe failed with error %x \n", GetLastError());
		return 0;
	}

	printf("CreateNamedPipe success hPipe = %d\n", hPipe);


	//InitializeCriticalSection(&cs);
	//mutex = CreateSemaphore(NULL, 1, 1, TEXT("mutex"));
	HANDLE handleGet = NULL;
	handleGet = (HANDLE)_beginthreadex(NULL, 0, beginGetThread, NULL, NULL, NULL);
	if (handleGet == NULL)
	{
		printf("create thread failed\n");
	//	system("pause");
	//	DeleteCriticalSection(&cs);
	//	return 0;
	}

/*	HANDLE handleSend = (HANDLE)_beginthreadex(NULL, 0, beginSendThread, NULL, NULL, NULL);
	if (handleSend == NULL)
	{
		printf("create thread failed\n");
		system("pause");
		DeleteCriticalSection(cs);
		return 0;
	}
*/
//	WaitForSingleObject(handleGet, INFINITE);
//	WaitForSingleObject(handleSend, INFINITE);
	DWORD   dwExitCode;
	GetExitCodeThread(handleGet, &dwExitCode);
	printf("beginGetThread 1 exited with code %u\n", dwExitCode);
	CloseHandle(handleGet);
//	CloseHandle(handleSend);
//	DeleteCriticalSection(&cs);

}

