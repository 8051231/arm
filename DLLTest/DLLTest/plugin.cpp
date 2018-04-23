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
#include "type.h"

//初始化标识：0停止，1运行 
int nSimState = 0;

eslapi::CADI *cadi = NULL;
MyCADICallback *cadi_callback = NULL;
//模拟器编号
unsigned int targetnum = 2;

int simc_plugin_registers_read()
{
	{
		printf("\n***Retrieving register group information...\n");

		// Read Register Group information. The number of register groups is given by target_features.nrRegisterGroups 
		const uint32_t desiredNumberOfRegisterGroups = 100;
		eslapi::CADIRegGroup_t *register_groups = new eslapi::CADIRegGroup_t[100];

		// NOTE: The actual number of register groups might be smaller than the desired number
		// of register groups 'desiredNumberOfRegisterGroups'!
		uint32_t actualNumberOfRegisterGroups = 0;

		eslapi::CADIReturn_t status = cadi->CADIRegGetGroups(0, //groupIndex
			desiredNumberOfRegisterGroups,
			&actualNumberOfRegisterGroups,
			register_groups);

		//Flag to indicate whether register group information could be retrieved and whether write accesses can be tested
		bool RegGetGroupsFailed = false;

		if (status != eslapi::CADI_STATUS_OK)
		{
			printf("ERROR: Getting register group information from target failed. Return type is '%s'\n", CADIReturnToString(status));
			assert(eslapi::CADI_STATUS_OK == status);
			RegGetGroupsFailed = true;
		}

		if (actualNumberOfRegisterGroups == 0)
		{
			printf("ERROR: Target returned 0 register groups.\n");
			assert(actualNumberOfRegisterGroups > 0);
			RegGetGroupsFailed = true;
		}

		if (RegGetGroupsFailed == true)
		{
			printf("WARNING: Could not retrieve register group information. Skipping register write accesses.\n");
		}

		for (unsigned int i = 0; i < actualNumberOfRegisterGroups; ++i)
		{
			printf("\n\tRegister Group #%u: ID:%u   Name:%s   RegsInGroup:%u\n\n",
				i,
				register_groups[i].groupID,
				register_groups[i].name,
				register_groups[i].numRegsInGroup);


			// Get the register group's register map
			eslapi::CADIRegInfo_t* registerInfos = new eslapi::CADIRegInfo_t[register_groups[i].numRegsInGroup]();
			uint32_t actualNumberOfRegInfos = 0;

			status = cadi->CADIRegGetMap(register_groups[i].groupID,
				0, //startRegisterIndex
				register_groups[i].numRegsInGroup, // desired number of register infos
				&actualNumberOfRegInfos,  // actual number which might be smaller than the desired number
				registerInfos);

			if (status != eslapi::CADI_STATUS_OK)
			{
				printf("ERROR: Reading register map for register group '%s' (ID: %u) failed! Error status is '%s'.\n",
					register_groups[i].name,
					register_groups[i].groupID,
					CADIReturnToString(status));
				delete[] registerInfos;
				assert(eslapi::CADI_STATUS_OK == status);
				printf("Leaving 'ReadTargetRegisters()'...\n");
				return status;
			}


			if (actualNumberOfRegInfos != register_groups[i].numRegsInGroup)
			{
				printf("ERROR: Reading register map for register group '%s' (ID: %u) returned a different number of registers (%u) than expected (%u).\n",
					register_groups[i].name,
					register_groups[i].groupID,
					actualNumberOfRegInfos,
					register_groups[i].numRegsInGroup);
				delete[] registerInfos;
				assert(actualNumberOfRegInfos == register_groups[i].numRegsInGroup);
				printf("Leaving 'ReadTargetRegisters()'...\n");
				return -1;
			}

			for (unsigned int j = 0; j < actualNumberOfRegInfos; ++j)
			{
				printf("\t\tRegister #%u: \tRegNumber:%u \tName:%s  \tValue:",
					j,
					registerInfos[j].regNumber,
					registerInfos[j].name);

				// String registers might require more than one CADIRegRead as a single call is limited to 16 characters. The individual 16 byte
				// chunks are addressed by CADIReg_t::offset128 (see below). CADIRegRead is called until a 0-terminated string is received. To get
				// the full string, the received substrings must be concetenated.
				if (registerInfos[j].display == eslapi::CADI_REGTYPE_STRING)
				{
					std::string registerString("");
					bool stringFinished = false;
					uint32_t offset128 = 0; // counter for offset128 specifying the 16 byte chunk to be queried by the individual CADIRegRead
					while (!stringFinished) // Issue CADIRegRead calls until the returned string is 0-terminated
					{
						uint32_t numRegsRead = 0;
						eslapi::CADIReg_t reg;
						reg.regNumber = registerInfos[j].regNumber; // the register number must be set prior calling CADIRegRead to identify the register to be read
						reg.offset128 = offset128++;  // identify 16 byte chunk 

						status = cadi->CADIRegRead(1, // query a single set of register data
							&reg,
							&numRegsRead,
							0); //doSideEffects

						if (status != eslapi::CADI_STATUS_OK)
						{
							printf("ERROR: Reading register '%s' (RegNumber %u) failed. Return type is '%s'\n",
								registerInfos[j].name,
								registerInfos[j].regNumber,
								CADIReturnToString(status));
							delete[] registerInfos;
							assert(eslapi::CADI_STATUS_OK == status);
							printf("Leaving 'ReadTargetRegisters()'...\n");
							return status;
						}

						if (numRegsRead != 1)
						{
							printf("ERROR: Reading register '%s' (RegNumber %u) read a different number of registers (%u) than expected (1).\n",
								registerInfos[j].name,
								registerInfos[j].regNumber,
								numRegsRead);
							delete[] registerInfos;
							assert(numRegsRead == 1);
							printf("Leaving 'ReadTargetRegisters()'...\n");
							return -1;
						}

						for (unsigned int k = 0; k < sizeof(reg.bytes); ++k)
						{
							if (reg.bytes[k] == '\0')
							{
								stringFinished = true;
								break;
							}
							registerString += reg.bytes[k];
						}
					}
					printf("'%s'\n", registerString.c_str());
				}
				else
				{
					// Read a non-string register
					printf("0x");
					uint32_t regWidthInBytes = (registerInfos[j].bitsWide + 7) / 8;
					// Each CADIReg_t can be used to read a maximum of 16 bytes. If larger, the offset128 can be used to specify the 16 byte portion for the individual CADIRegRead call
					uint32_t numberOf128BitBlocks = (regWidthInBytes + 15) / 16;

					for (unsigned int offset128 = 1; offset128 <= numberOf128BitBlocks; ++offset128)
					{
						uint32_t numRegsRead = 0;
						eslapi::CADIReg_t reg;
						reg.regNumber = registerInfos[j].regNumber;
						// CADIReg_t holds the data in little endianess. in order to correctly present the value, we have to start with the most significant bytes
						reg.offset128 = numberOf128BitBlocks - offset128;

						status = cadi->CADIRegRead(1, // Query for just one register 
							&reg,
							&numRegsRead,
							0); //doSideEffects

						if (status != eslapi::CADI_STATUS_OK)
						{
							printf("ERROR: Reading string register '%s' (RegNumber %u) failed. Return type is '%s'\n",
								registerInfos[j].name,
								registerInfos[j].regNumber,
								CADIReturnToString(status));
							delete[] registerInfos;
							assert(eslapi::CADI_STATUS_OK == status);
							printf("Leaving 'ReadTargetRegisters()'...\n");
							return status;
						}

						if (numRegsRead != 1)
						{
							printf("ERROR: Reading string register '%s' (RegNumber %u) with offset %u read a different number of registers (%u) than expected (1).\n",
								registerInfos[j].name,
								registerInfos[j].regNumber,
								reg.offset128,
								numRegsRead);
							delete[] registerInfos;
							assert(numRegsRead == 1);
							printf("Leaving 'ReadTargetRegisters()'...\n");
							return -1;
						}

						for (unsigned int k = 1; k <= regWidthInBytes; ++k)
						{
							printf("%02x", reg.bytes[regWidthInBytes - k]);
						}
					}
					printf("\n");
				}
			}
			delete[] registerInfos;
			printf("\n");
		}
		delete[] register_groups;
	}
	return 0;
}
int simc_plugin_stop()
{
	eslapi::CADIReturn_t status = cadi->CADIExecStop();

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("CADIExecContinue returned an error\n");
		return status;
	}
	return status;
}

int simc_plugin_stepn(int nCount)
{
    //runcontrol_stepn(nCount, cadi, cadi_callback, true);
	eslapi::CADIReturn_t status = cadi->CADIExecSingleStep(nCount, 0, 0);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("CADIExecSingleStep returned an error\n");
		return status;
	}

	return status;
}


int simc_plugin_run()
{
	eslapi::CADIReturn_t status = cadi->CADIExecContinue();

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("CADIExecContinue returned an error\n");
		return status;
	}
	return status;
}

int simc_plugin_init()
{
    //插件路径
	std::string strPlugin = "E:\\ARM\\FastModelsPortfolio_11.3\\examples\\MTI\\SimpleTrace\\x64\\Release_2013\\SimpleTrace.dll";
	//模拟器路径
	std::string strSim = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\LISA\\FVP_VE\\Build_Cortex-A15x1\\Win64-Release-VC2015\\cadi_system_Win64-Release-VC2015.dll";
	//std::string strSim = "cadi_system_Win64-Release-VC2015.dll";
	//应用程序路径
	std::string strApp = "D:\\ARM\\FastModelsPortfolio_11.3\\images\\brot_ve_A.axf";

	// Define the MTI plugin to use by setting environment variable FM_TRACE_PLUGINS
	//set_plugin(strPlugin);
	
	printf("set_plugin ....\n");
	// connect to a already running model, return a cadi pointer to the requested target
	cadi = connect_library(strSim, targetnum, true);
	if (NULL == cadi)
	{
		printf("The cadi initialize failed\n");
		return SIMC_CADI_NULL;
	}
	printf("The cadi initialize success\n");

	// load application
	eslapi::CADIReturn_t status = cadi->CADIExecLoadApplication(strApp.c_str(), true, false, NULL);
	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("CADIExecLoadApplication returned an error\n");
		return status;
	}

	// add a CADI callback to allow correct run controland handling of semihosting, return a pointer to the callback object
	cadi_callback = callbacks(cadi, true);
	if (NULL == cadi_callback)
	{
		printf("The cadi_callback initialize failed\n");
		return SIMC_CADI_CALLBACK_NULL;
	}
	printf("The cadi_callback initialize success\n");

	return SIMC_OK;
}

int simc_plugin_get_status()
{
	return cadi_callback->GetCurrentMode();
}