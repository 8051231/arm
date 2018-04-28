//#pragma once
#ifndef _PLUGIN_H_
#define _PLUGIN_H_
int simc_plugin_init();
int simc_plugin_get_status(UINT32* pnErrorCode);
int simc_plugin_get_execcycle(UINT64* pnCycleCount);
//unsigned _stdcall  simc_plugin_run(PVOID p);
int simc_plugin_run();
int simc_plugin_stepn(int nCount);
int simc_plugin_stop();
int simc_plugin_registers_read(UINT32 nRegID, UINT64 nValue);
int simc_plugin_registers_write(UINT32 nRegID, UINT64 nValue);
int simc_plugin_mem_read(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);
int simc_plugin_mem_write(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth);
int simc_plugin_addbreakpoint(UINT64 nAddr);
int simc_plugin_removebreakpoint(UINT64 nbreakpointId);

#endif