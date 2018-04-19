//#pragma once
#ifndef _PLUGIN_H_
#define _PLUGIN_H_
int simc_plugin_init();
int simc_plugin_get_mode();
//unsigned _stdcall  simc_plugin_run(PVOID p);
int simc_plugin_run();
int simc_plugin_stepn(int nCount);
int simc_plugin_stop();
#endif