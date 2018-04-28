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
extern st_event g_time_event;
extern st_event g_addr_event;
int nQueLen = 0; //队列长度
HANDLE hServerPipe = NULL;

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


#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096



unsigned _stdcall ThreadProc(void* param);

#define BUFSIZE 512

int simc_fifo_queue_proc_addr(uint32_t pc)
{
	int nRet = 0;
	int nTmp = 0;
	for (nTmp = 0; nTmp < nQueLen; nTmp++)
	{
		nRet = simc_addr_queue_up_node(g_addr_event.queue, nTmp, pc);
		if (SIMC_CALLBACK_OK == nRet)
		{
			nTmp = nTmp - 1;
			nQueLen = simc_queue_len(g_time_event.queue);
			printf("simc_queue_len, callback,delnode success .after del time event que,nTmp = %d, nQueLen = %d\n", nTmp, nQueLen);
		}
		else if (SIMC_OK == nRet)
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
			nQueLen = simc_queue_len(g_time_event.queue);
			printf("simc_queue_len, callback,delnode success .after del time event que,nTmp = %d, nQueLen = %d\n", nTmp,nQueLen);
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
	//处理时间队列事件
	nQueLen = simc_queue_len(g_time_event.queue);

	if(nQueLen > 0)
	{
		//遍历队列，周期减一，回调事件处理
		nRet = simc_fifo_queue_proc(pc);
	}
	else
	{
		nRet = 0;
	}
	//处理地址队列事件
	nQueLen = simc_queue_len(g_addr_event.queue);

	if(nQueLen > 0)
	{
		//遍历队列，查询地址空间
		nRet = simc_fifo_queue_proc_addr(pc);
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
void bosagent_data_syslog(char *msg, void * data, unsigned short len)
{
	if (msg == NULL || data == NULL || len < 1)
	{
		return;
	}
	unsigned char * pbuf = (unsigned char*)data;

	int i = 0;
	int j = 0;

	char tmp_buff[1024] = { 0 };

	for (j = 0; j < len;)
	{
		memset(tmp_buff, 0, sizeof(tmp_buff));

		for (i = 0; i < sizeof(tmp_buff) - 3 && j < len; j++)
		{
			sprintf(tmp_buff + i, "%02x ", pbuf[j]);
			i = strlen(tmp_buff);
		}

		printf("%s%s\n", msg, tmp_buff);
	}

}

void  PrintHex(char *msg, void * data, unsigned short len)
{
	bosagent_data_syslog(msg, data, len);
}


unsigned _stdcall InstanceThread(PVOID p)
{
	printf("ServerPipe waiting for connection......\n");
    if (ConnectNamedPipe(hServerPipe, NULL) != NULL)//等待连接。  
	{
		BOOL fSuccess = FALSE;
		printf("The connection is successful ,ServerPipe starts to receive the data\n");
		while (true)
		{
			printf("**************ServerPipe true**************\n");
			//WaitForSingleObject(mutex, INFINITE);
			//EnterCriticalSection(&cs);
			//接收客户端发送的数据
			int ret = SIMC_OK;
			uint32_t addrpc = 0;
			DWORD nRcvLen = 0;
			unsigned char sRcvbuf[SIMC_MSG_LEN] = {0};

			fSuccess = ReadFile(hServerPipe, sRcvbuf, SIMC_MSG_LEN, &nRcvLen, NULL);
			if (!fSuccess || nRcvLen == 0)
			{
				if (GetLastError() == ERROR_BROKEN_PIPE)
				{
					_tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
				}
				else
				{
					_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
				}
				break;
			}

			PrintHex("Server_rcv: ", sRcvbuf, nRcvLen);

			addrpc = (sRcvbuf[3] << 24) | (sRcvbuf[2] << 16) | (sRcvbuf[1] << 8) | (sRcvbuf[0]);
			printf("ServerPipe received data from the client %d bytes,contents are:0x%08x\n", nRcvLen, addrpc);
			//全局计数器加1
/*
			int ret1 = 0;
			char sSendbuf[SIMC_MSG_LEN] = { 0 };
		    ret1 = snprintf(sSendbuf, sizeof(sSendbuf), "12345678");
			printf("ServerPipe simc_fifo_proc ret =  %d, %s\n", ret1, sSendbuf);
			*/
			//业务逻辑处理
			ret = simc_fifo_proc(addrpc);
			printf("ServerPipe simc_fifo_proc ret =  %d\n", ret);
			//FlushFileBuffers(hServerPipe);

			if ((SIMC_CALLBACK_OK == ret) || (SIMC_OK == ret))
			{
				//管道回复
				unsigned char sSendbuf[SIMC_MSG_LEN] = {0 };
				memcpy(&sSendbuf[0], &addrpc, 4);
				DWORD nAckLen = 0;
				printf("ServerPipe  WriteFile to client begin......\n");


				//向客户端发送数据  
				fSuccess = WriteFile(hServerPipe, sSendbuf, 4, &nAckLen, NULL);
				if (!fSuccess || nAckLen != SIMC_MSG_LEN)
				{
					_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
					break;
				}
				else
				{
					printf("ServerPipe WriteFile to client  success, nAckLen = %d, contents are:0x%08x\n",nAckLen, addrpc);
				}
				printf("ServerPipe WriteFile to client end......\n");
				PrintHex("Server_send: ", sSendbuf, nAckLen);
			}
			
			//huikui TracePC .......

//            if (ret != 0)
//			{
//				printf("simc_fifo_proc failed\n");
//			}
//			else//管道回复6666
		/*	{
				uint32_t pc = 6666;
				char pbuf[4] = { 0 };
				DWORD dwLen = 0;
				memcpy(&pbuf[0], &pc, 4);

				char buf[4] = { 66 };
				printf("begin WriteFile ......\n");
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
			//Sleep(500);
			//ReleaseSemaphore(mutex, 1, NULL);
			//putchar('\n');
		}
	}
	else
	{
		printf("连接失败\n");
	}

	DisconnectNamedPipe(hServerPipe);
	CloseHandle(hServerPipe);//关闭管道
	return 0;
}
int simc_fifo_uninit()
{

}

int simc_fifo_init()
{

	printf("entry simc_fifo_init\n");

	hServerPipe = CreateNamedPipe(pStrPipeNameGet, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_NOWAIT, 0);
	if (INVALID_HANDLE_VALUE == hServerPipe)
	{
		printf("CreateNamedPipe failed with error %x \n", GetLastError());
		return 0;
	}

	printf("CreateNamedPipe success hServerPipe = %d\n", hServerPipe);


	//InitializeCriticalSection(&cs);
	//mutex = CreateSemaphore(NULL, 1, 1, TEXT("mutex"));
	HANDLE handleGet = NULL;
	handleGet = (HANDLE)_beginthreadex(NULL, 0, InstanceThread, NULL, NULL, NULL);
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
	printf("InstanceThread 1 exited with code %u\n", dwExitCode);
	CloseHandle(handleGet);
	//	CloseHandle(handleSend);
	//	DeleteCriticalSection(&cs);
	return 0;

}

