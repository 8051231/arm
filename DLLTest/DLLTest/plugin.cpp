#include "stdafx.h"
#include "plugin.h"
/////////////////////////////////////////////////////////////////////////////
// Includes
#include <cstdio>
#include "eslapi/CADI.h"

#include "MyCADICallback.h"
#include "misc.h"
#include "mti.h"
#include "connect.h"
#include "callback.h"
#include "runcontrol.h"

//初始化标识：0停止，1运行 
int nSimState = 0;

eslapi::CADI *cadi = NULL;
MyCADICallback *cadi_callback = NULL;
//模拟器编号
unsigned int targetnum = 2;

int simc_plugin_run()
{
	runcontrol_step(cadi, cadi_callback, true);
	return 0;
}

int simc_plugin_init()
{
    //插件路径
	std::string strPlugin = "D:\ARM\FastModelsPortfolio_11.3\examples\MTI\SimpleTrace\x64\Release_2013\SimpleTrace.dll";
	//模拟器路径
	std::string strSim = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\LISA\\FVP_VE\\Build_Cortex-A15x1\\Win64-Release-VC2015\\cadi_system_Win64-Release-VC2015.dll";
	//std::string strSim = "cadi_system_Win64-Release-VC2015.dll";
	//应用程序路径
	std::string strApp = "D:\ARM\FastModelsPortfolio_11.3\images\brot_ve_A.axf";

	// Define the MTI plugin to use by setting environment variable FM_TRACE_PLUGINS
	set_plugin(strPlugin);

	// connect to a already running model, return a cadi pointer to the requested target
	if (NULL != cadi)
	{
		printf("The cadi has been initialized\n");
		return -1;
	}

	cadi = connect_library(strSim, targetnum, true);
	if (NULL == cadi)
	{
		printf("The cadi initialize failed\n");
		while (1)
		{
			//printf("asdf\n");
		}
		return -2;
	}
	printf("The cadi initialize success\n");

	// load application
    cadi->CADIExecLoadApplication(strApp.c_str(), true, false, NULL);

	// add a CADI callback to allow correct run controland handling of semihosting, return a pointer to the callback object
	if (NULL != cadi_callback)
	{
		printf("The cadi_callback has been initialized\n");
		return -3;
	}
	cadi_callback = callbacks(cadi, true);
	if (NULL == cadi_callback)
	{
		printf("The cadi_callback initialize failed\n");
		return -4;
	}
	printf("The cadi_callback initialize success\n");


}

int simc_plugin_get_mode()
{
	return cadi_callback->GetCurrentMode();
}