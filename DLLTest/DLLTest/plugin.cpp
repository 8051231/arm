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

int simc_plugin_removebreakpoint(UINT64 nbreakpointId)
{

	// remove a program breakpoint at address addr
	eslapi::CADIReturn_t status = cadi->CADIBptClear(nbreakpointId);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("unable to set breakpoint\n");
		exit(1);
	}

	return SIMC_OK;
}

int simc_plugin_addbreakpoint(uint64_t nddr)
{

	// add program breakpoint to address 0x80000250
	uint32_t *bptnum;
	bptnum = (uint32_t *)breakpoints_program(cadi, nddr, 0, true);
	printf("bptnum = %d\n", *bptnum);
	return *bptnum;

}

inline std::string registers_extended_feature(eslapi::CADI *cadi, bool verbose)
{
	if (verbose)
		printf("\n***Retrieving extended feature registers...\n");

	eslapi::CADITargetFeatures_t target_features;
	eslapi::CADIReturn_t status = cadi->CADIXfaceGetFeatures(&target_features);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Getting CADI target features from target failed. These are required in the following.\n");
		assert(eslapi::CADI_STATUS_OK == status);
		exit(1);
	}

	if (verbose)
		printf("\tThe extended feature registers id = '%d'\n", target_features.nExtendedTargetFeaturesRegNum);

	std::string registerString("");
	bool stringFinished = false;
	uint32_t offset128 = 0; // counter for offset128 specifying the 16 byte chunk to be queried by the individual CADIRegRead
	while (!stringFinished) // Issue CADIRegRead calls until the returned string is 0-terminated
	{
		uint32_t numRegsRead = 0;
		eslapi::CADIReg_t reg;
		reg.regNumber = target_features.nExtendedTargetFeaturesRegNum;
		reg.offset128 = offset128++;  // identify 16 byte chunk 

		eslapi::CADIReturn_t status = cadi->CADIRegRead(1, // query a single set of register data
			&reg,
			&numRegsRead,
			0); //doSideEffects
		if (status != eslapi::CADI_STATUS_OK)
		{
			printf("ERROR: Retrieving extended feature registers failed.\n");
			assert(eslapi::CADI_STATUS_OK == status);
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

	if (verbose)
	{
		printf("\tThe extended feature registers = '%s'\n", registerString.c_str());
		printf("\tRetrieving extended feature registers succeeded.\n");
	}

	return registerString;
}

inline int32_t register_extract_extended_feature(eslapi::CADI *cadi, std::string feature, bool verbose)
{
	if (verbose)
		printf("\n***Extracting feature (%s) from extended feature registers...\n", feature.c_str());

	std::string ExtendedFeaturesRegister = registers_extended_feature(cadi, false);

	std::size_t startpos = ExtendedFeaturesRegister.find(feature);

	if (startpos == std::string::npos)
	{
		if (verbose)
			printf("\tFeature (%s) not found in ETF\n", feature.c_str());
		return -2;
	}
	else
	{
		startpos += feature.length();
		if (ExtendedFeaturesRegister[startpos] == '=')
		{
			startpos++;
			std::size_t endpos = ExtendedFeaturesRegister.find_first_of(":", startpos);
			std::string Id = ExtendedFeaturesRegister.substr(startpos, endpos);
			int32_t value = atoi(Id.c_str());
			if (verbose)
				printf("\tFeature (%s) found in ETF, and assigned value %d\n", feature.c_str(), value);
			return value;
		}
		else
		{
			if (verbose)
				printf("\tFeature (%s) found in ETF, not assigned a value\n", feature.c_str());
			return -1;
		}
	}
}

// Read a given register's value using its Id
inline uint64_t regReadByID(eslapi::CADI *cadi, uint32_t regId)
{
	eslapi::CADIReg_t reg;
	reg.regNumber = regId;
	uint32_t actualCount = 0;

	// Query the register to read it
	eslapi::CADIReturn_t status = cadi->CADIRegRead(1, &reg, &actualCount, 0);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Retrieving register id: %d failed.\n", regId);
		assert(eslapi::CADI_STATUS_OK == status);
	}

	printf("0x");
	for (unsigned int k = 1; k <= 4; ++k)
	{
		printf("%02x", reg.bytes[4 - k]);
	}
	printf("\n");

	// Fill the bytes retrieved into a 64-bit variable and return it
	uint32_t lo = reg.bytes[0] | (reg.bytes[1] << 8) | (reg.bytes[2] << 16) | (reg.bytes[3] << 24);
	uint32_t hi = reg.bytes[4] | (reg.bytes[5] << 8) | (reg.bytes[6] << 16) | (reg.bytes[7] << 24);
	return (uint64_t(hi) << 32) | lo;
}

// Write to a register using it's Id
inline void regWriteByID(uint32_t regId, uint64_t value)
{
	eslapi::CADIReg_t reg;
	reg.regNumber = regId;

	// Fill the bytes to be stored in the register
	uint32_t lo = (uint32_t)value;
	uint32_t hi = (uint32_t)(value >> 32);
	reg.bytes[0] = (uint8_t)lo;
	reg.bytes[1] = (uint8_t)(lo >> 8);
	reg.bytes[2] = (uint8_t)(lo >> 16);
	reg.bytes[3] = (uint8_t)(lo >> 24);
	reg.bytes[4] = (uint8_t)hi;
	reg.bytes[5] = (uint8_t)(hi >> 8);
	reg.bytes[6] = (uint8_t)(hi >> 16);
	reg.bytes[7] = (uint8_t)(hi >> 24);

	uint32_t actualCount = 0;

	// Write to the register
	eslapi::CADIReturn_t status = cadi->CADIRegWrite(1, &reg, &actualCount, 0);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Writing to register id: %d failed.\n", regId);
		assert(eslapi::CADI_STATUS_OK == status);
	}
}

int simc_plugin_registers_write(UINT32 nRegID, UINT64 nValue)
{

	regWriteByID(nRegID, nValue);
	return SIMC_OK;

}

int simc_plugin_registers_read(UINT32 nRegID, UINT64 nValue)
{

	nValue = regReadByID(cadi, nRegID);
	return SIMC_OK;

}

inline uint32_t memory_spaces(eslapi::CADI *cadi, bool verbose, uint32_t memspaceid = -1)
{
	if (verbose)
		printf("\n***Retrieving memory space information...memspaceid = %d\n", memspaceid);
	eslapi::CADITargetFeatures_t target_features;
	eslapi::CADIReturn_t status = cadi->CADIXfaceGetFeatures(&target_features);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Getting CADI target features from target failed. These are required in the following.\n");
		assert(eslapi::CADI_STATUS_OK == status);
		exit(1);
	}

	//Read Memory Space information. The number of memory spaces is given by target_features.nrMemSpaces
	const uint32_t desiredNumOfMemSpaces = target_features.nrMemSpaces;
	printf("\n***desiredNumOfMemSpaces = %d...\n", desiredNumOfMemSpaces);
	eslapi::CADIMemSpaceInfo_t *memory_spaces = new eslapi::CADIMemSpaceInfo_t[desiredNumOfMemSpaces];
	//NOTE: The actual number of memory spaces might be smaller than the desired number of memory spaces
	//'desiredNumberOfMemSpaces'!
	uint32_t actualNumOfMemSpaces = 0;

	status = cadi->CADIMemGetSpaces(0, //startMemSpaceIndex
		desiredNumOfMemSpaces,
		&actualNumOfMemSpaces,
		memory_spaces);
	printf("\n***actualNumOfMemSpaces = %d...\n", actualNumOfMemSpaces);
	//Flag to indicate whether memory space information could be retrieved and whether write access can be tested
	bool MemGetSpacesFailed = false;

	// Check the returned status
	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Getting memory space information from target failed. Return type is '%s'\n",
			CADIReturnToString(status));
		assert(status == eslapi::CADI_STATUS_OK);
		MemGetSpacesFailed = true;
	}

	// Check if any memory space has been returned
	if (actualNumOfMemSpaces == 0)
	{
		printf("ERROR: Target returned 0 memory spaces.\n");
		assert(actualNumOfMemSpaces > 0);
		MemGetSpacesFailed = true;
	}

	if (MemGetSpacesFailed == true)
	{
		printf("WARNING: Could not retrieve memory space information. Skipping register write accesses.\n");
	}

	uint32_t mau = 0;

	for (unsigned int i = 0; i < actualNumOfMemSpaces; i++)
	{
		// find memory_space_index for given memSpaceId
		if (memspaceid == memory_spaces[i].memSpaceId)
			mau = memory_spaces[i].bitsPerMau;

		if (verbose)
		{
			// print information on memoryspace
			printf("\tMemoryspace %d\n", memory_spaces[i].memSpaceId);
			printf("\t\tName:             %s\n", memory_spaces[i].memSpaceName);
			printf("\t\tDescription:      %s\n", memory_spaces[i].description);
			printf("\t\tbitsPerMau:       %u\n", memory_spaces[i].bitsPerMau);
			printf("\t\tminAddress:       0x%08x%08x\n", (uint32_t)(memory_spaces[i].minAddress >> 32), (uint32_t)(memory_spaces[i].minAddress));
			printf("\t\tmaxAddress:       0x%08x%08x\n", (uint32_t)(memory_spaces[i].maxAddress >> 32), (uint32_t)(memory_spaces[i].maxAddress));
			printf("\t\tnrMemBlocks:      %u\n", memory_spaces[i].nrMemBlocks);
			printf("\t\tVirtual/Physical: ");
			if (memory_spaces[i].isVirtualMemory)
				printf("Virtual\n");
			else
				printf("Physical\n");

			printf("\n\n");
		}

	}

	return  mau;
}


inline void memory_read(eslapi::CADI *cadi, uint64_t addr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth, uint32_t memspace, bool verbose)
{
	unsigned char * buf = NULL;
	buf = (unsigned char *)pbBuf;
	// Reading the first 20 units of the memory block. Starting from addr
	eslapi::CADIAddrComplete_t startAddress;
	startAddress.location.space = memspace;
	startAddress.location.addr = addr;

	//uint32_t unitsToRead = 20; // First 20 units of target are going to be read
	uint32_t unitsToRead = nLength;
							   // Find mau for requested memory space
	uint32_t bitsPerMauMemoryspace = memory_spaces(cadi, true, memspace);

	// Convert bits per Minimum Access Unit (MAU) to bytes per MAU. The resulting number must always be a supported multiple of MAU.
	uint32_t unitSizeInBytes = (bitsPerMauMemoryspace + 7) / 8;
	uint32_t actualNumOfUnitsRead = 0;
	uint8_t  doSideEffects = 0; // Side effects are omitted (target has to decide which side effects are mandatory).
	//uint32_t memAccessInBytes = unitsToRead * unitSizeInBytes;

	uint32_t memAccessInBytes = nLength * nDataWidth;  // nLength * unitSizeInBytes


	//Allocating and initializing data buffer for memory read access
	uint8_t  *data = new uint8_t[memAccessInBytes];
	memset(data, 0, memAccessInBytes);

	// Do the read access
	eslapi::CADIReturn_t status = cadi->CADIMemRead(startAddress,
		unitsToRead,
		unitSizeInBytes,
		data,
		&actualNumOfUnitsRead,
		doSideEffects);


	// Check return status
	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Reading %u units of %u bytes starting from address 0x%08x%08x failed. Return type is '%s'\n",
			unitsToRead,
			unitSizeInBytes,
			(uint32_t)(startAddress.location.addr >> 32),
			(uint32_t)(startAddress.location.addr),
			CADIReturnToString(status));
		delete[] data;
		assert(eslapi::CADI_STATUS_OK == status);
		printf("Leaving 'ReadTargetMemories()'...\n");
		return;
	}


	// Check if all requested units were read
	if (actualNumOfUnitsRead != unitsToRead)
	{
		printf("ERROR: Less units (%u) of %u bytes than requested (%u) have been read starting from address 0x%08x%08x .\n",
			actualNumOfUnitsRead,
			unitSizeInBytes,
			unitsToRead,
			(uint32_t)(startAddress.location.addr >> 32),
			(uint32_t)(startAddress.location.addr));
		delete[] data;
		assert(actualNumOfUnitsRead == unitsToRead);
		printf("Leaving 'ReadTargetMemories()'...\n");
		return;
	}

	// Print out the read memory contents
	if (verbose)
	{
		printf("\n***Reading target memory...\n");
		printf("\tMemoryspace:             %d\n", memspace);
		printf("\tStart address:           0x%08x%08x\n", (uint32_t)(startAddress.location.addr >> 32), (uint32_t)startAddress.location.addr);
		printf("\tNumber of bytes to read: %d", actualNumOfUnitsRead);
		for (unsigned int k = 0; k < actualNumOfUnitsRead; k++)
		{
			if ((k % 4) == 0)
			{
				uint64_t displayAddr = startAddress.location.addr + k;
				printf("\n\t\t0x%08x%08x",
					(uint32_t)(displayAddr >> 32),
					(uint32_t)(displayAddr));
			}
			printf("\t%02x", data[k]);
		}

		printf("\nafter copy\n");
		int i = 0;
		for (i; i < actualNumOfUnitsRead; i++)
		{
			memcpy(buf, &data[i],1);

			//buf[i] = (unsigned char *)data[i];
			printf("\t%02x", *buf);
			buf += 1;
		//	printf("\t%02x", *pbBuf+i);
			
		}
		printf("\norinal buff\n");
		i = 0;
		for (i; i < actualNumOfUnitsRead; i++)
		{
			//buf[i] = (unsigned char *)data[i];
			printf("\t%02x", (unsigned char *)pbBuf[i]);
			//	printf("\t%02x", *pbBuf+i);

		}

		printf("\n\n");
	}
	delete[] data;
}


int simc_plugin_mem_read(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth)
{
	// Read and display the memory spaces on the targets, return the mau for memspace0
	uint32_t bitsPerMauMemoryspace = memory_spaces(cadi, 1);
	printf("bitsPerMauMemoryspace = %d\n", bitsPerMauMemoryspace);

	// Read and display 20bytes from address 0x8000 of memspace0
	memory_read(cadi, nStartAddr, nLength, pbBuf, nDataWidth,bitsPerMauMemoryspace, true);
	return SIMC_OK;
}

void ReadTargetMemories(eslapi::CADI* cadi,
	eslapi::CADIMemSpaceInfo_t memory_spaces[],
	uint32_t numberOfMemorySpaces)
{
	eslapi::CADIReturn_t status;


	// Iterating over each available memory space
	printf("\n***Current memory block information and contents of each block's first 20 bytes...\n");
	for (unsigned int i = 0; i < numberOfMemorySpaces; i++)
	{
		// Print out some of the memory space information
		printf("\n\tMemory Space #%u: ID:%u   Name:%s   bitsPerMau:%u   minAddress:0x%08x%08x   maxAddress:0x%08x%08x   nrMemBlocks:%u\n",
			i,
			memory_spaces[i].memSpaceId,
			memory_spaces[i].memSpaceName,
			memory_spaces[i].bitsPerMau,
			(uint32_t)(memory_spaces[i].minAddress >> 32),
			(uint32_t)(memory_spaces[i].minAddress),
			(uint32_t)(memory_spaces[i].maxAddress >> 32),
			(uint32_t)(memory_spaces[i].maxAddress),
			memory_spaces[i].nrMemBlocks);

		// Prepare data for getting the memory block information for the current memory space.
		uint32_t actualNumOfMemBlocks = 0;
		eslapi::CADIMemBlockInfo_t *memory_blocks = new eslapi::CADIMemBlockInfo_t[memory_spaces[i].nrMemBlocks];

		status = cadi->CADIMemGetBlocks(memory_spaces[i].memSpaceId,
			0, //memBlockIndex
			memory_spaces[i].nrMemBlocks,
			&actualNumOfMemBlocks,
			memory_blocks);

		// Check the return status.
		if (status != eslapi::CADI_STATUS_OK)
		{
			printf("ERROR: Retrieving memory block information for memory space '%s' (ID: %u) failed! Error staus is '%s'.\n",
				memory_spaces[i].memSpaceName,
				memory_spaces[i].memSpaceId,
				CADIReturnToString(status));
			delete[] memory_blocks;
			assert(eslapi::CADI_STATUS_OK == status);
			printf("Leaving 'ReadTargetMemories()'...\n");
			return;
		}

		// Check if all requested memory blocks have been returned
		if (actualNumOfMemBlocks != memory_spaces[i].nrMemBlocks)
		{
			printf("ERROR: Retrieving memory block information for memory space '%s' (ID: %u) returned a different number of registers (%u) than expected (%u).\n",
				memory_spaces[i].memSpaceName,
				memory_spaces[i].memSpaceId,
				actualNumOfMemBlocks,
				memory_spaces[i].nrMemBlocks);
			delete[] memory_blocks;
			assert(actualNumOfMemBlocks == memory_spaces[i].nrMemBlocks);
			printf("Leaving 'ReadTargetMemories()'...\n");
			return;
		}

		// Iterating over all available memory blocks of the current memory space.
		for (unsigned int j = 0; j < actualNumOfMemBlocks; j++)
		{
			printf("\n\t\tMemory Block #%u: ID:%u   Name:%s   startAddr:0x%08x%08x   endAddr:0x%08x%08x\n",
				j,
				memory_blocks[j].id,
				memory_blocks[j].name,
				(uint32_t)(memory_blocks[j].startAddr >> 32),
				(uint32_t)(memory_blocks[j].startAddr),
				(uint32_t)(memory_blocks[j].endAddr >> 32),
				(uint32_t)(memory_blocks[j].endAddr));


			// Check if the current memory block addressed for read accesses.
			if (memory_blocks[j].readWrite == eslapi::CADI_MEM_WriteOnly)
			{
				// Memory block is write-only
				printf("WARNING: Memory Block is of type \"CADI_MEM_WriteOnly\", cannot read it!\n");
			}
			else
			{
				// Reading the first 20 units of the memory block. Starting from the beginning of the memory block.
				eslapi::CADIAddrComplete_t startAddress;
				//startAddress.overlay set to default value by constructor of CADIAddrComplete_t, not supported by model
				startAddress.location.space = memory_spaces[i].memSpaceId;
				startAddress.location.addr = memory_blocks[j].startAddr;

				uint32_t unitsToRead = 20; // First 20 units of target are going to be read

										   // Read the entire memory block if it is smaller than 20 units
				if ((memory_blocks[j].endAddr - memory_blocks[j].startAddr) < unitsToRead)
					unitsToRead = (uint32_t)(memory_blocks[j].endAddr - memory_blocks[j].startAddr); // Type cast is safe, difference is maximum 20

																									 // Convert bits per Minimum Access Unit (MAU) to bytes per MAU. The resulting number must always be a supported multiple of MAU.
				uint32_t unitSizeInBytes = (memory_spaces[i].bitsPerMau + 7) / 8;
				uint32_t actualNumOfUnitsRead = 0;
				uint8_t  doSideEffects = 0; // Side effects are omitted (target has to decide which side effects are mandatory).
				uint32_t memAccessInBytes = unitsToRead * unitSizeInBytes;

				//Allocating and initializing data buffer for memory read access
				uint8_t  *data = new uint8_t[memAccessInBytes];
				memset(data, 0, memAccessInBytes);

				// Do the read access
				status = cadi->CADIMemRead(startAddress,
					unitsToRead,
					unitSizeInBytes,
					data,
					&actualNumOfUnitsRead,
					doSideEffects);


				// Check return status
				if (status != eslapi::CADI_STATUS_OK)
				{
					printf("ERROR: Reading %u units of %u bytes starting from address 0x%08x%08x of memory block '%s' (ID %u) failed. Return type is '%s'\n",
						unitsToRead,
						unitSizeInBytes,
						(uint32_t)(startAddress.location.addr >> 32),
						(uint32_t)(startAddress.location.addr),
						memory_blocks[j].name,
						memory_blocks[j].id,
						CADIReturnToString(status));
					// Print out Errors for the read memory contents
					for (unsigned int k = 0; k < unitsToRead; k++)
					{
						if ((k % 5) == 0)
						{
							uint64_t displayAddr = startAddress.location.addr + k;
							printf("\n\t\t0x%08x%08x",
								(uint32_t)(displayAddr >> 32),
								(uint32_t)(displayAddr));
						}
						printf("\tError");
					}
					printf("\n\n");
					delete[] memory_blocks;
					delete[] data;
					return;
				}


				// Check if all requested units were read
				if (actualNumOfUnitsRead != unitsToRead)
				{
					printf("ERROR: Less units (%u) of %u bytes than requested (%u) have been read starting from address 0x%08x%08x of memory block '%s' (ID %u).\n",
						actualNumOfUnitsRead,
						unitSizeInBytes,
						unitsToRead,
						(uint32_t)(startAddress.location.addr >> 32),
						(uint32_t)(startAddress.location.addr),
						memory_blocks[j].name,
						memory_blocks[j].id);
					delete[] memory_blocks;
					delete[] data;
					assert(actualNumOfUnitsRead == unitsToRead);
					printf("Leaving 'ReadTargetMemories()'...\n");
					return;
				}

				// Print out the read memory contents
				for (unsigned int k = 0; k < actualNumOfUnitsRead; k++)
				{
					if ((k % 5) == 0)
					{
						uint64_t displayAddr = startAddress.location.addr + k;
						printf("\n\t\t0x%08x%08x",
							(uint32_t)(displayAddr >> 32),
							(uint32_t)(displayAddr));
					}
					printf("\t%02x", data[k]);
				}
				printf("\n\n");


				// Free the data buffer of the read access
				delete[] data;
				data = 0;
			}
		}

		// Free the memory allocated for the memory block information
		delete[] memory_blocks;
	}
}

int simc_plugin_mem_write(UINT64 nStartAddr, UINT32 nLength, UCHAR* pbBuf, UINT32 nDataWidth)
{
	unsigned char * buf = NULL;
	buf = (unsigned char *)pbBuf;

	// Read the target features; these expose information on the target like 
	// the number of register groups or memory spaces and the register number
	// of a program counter (if available)
	eslapi::CADITargetFeatures_t target_features;
	eslapi::CADIReturn_t status = cadi->CADIXfaceGetFeatures(&target_features);

	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Getting CADI target features from target failed. These are required in the following.\n");
		assert(eslapi::CADI_STATUS_OK == status);
		exit(1);
	}

	printf("\tSuccessfully obtained CADI target features.\n");

	printf("\n\tConnected to target '%s'\n", target_features.targetName);
	printf("\t\t-->Number of registers groups: %u\n\n", target_features.nrRegisterGroups);
	/////////////////////////////////
	//       Memory Accesses       //
	/////////////////////////////////

	printf("\n***Retrieving memory space information...\n");
	//Read Memory Space information. The number of memory spaces is given by target_features.nrMemSpaces
	const uint32_t desiredNumOfMemSpaces = target_features.nrMemSpaces;
	eslapi::CADIMemSpaceInfo_t *memory_spaces = new eslapi::CADIMemSpaceInfo_t[desiredNumOfMemSpaces];
	//NOTE: The actual number of memory spaces might be smaller than the desired number of memory spaces
	//'desiredNumberOfMemSpaces'!
	uint32_t actualNumOfMemSpaces = 0;

	status = cadi->CADIMemGetSpaces(0, //startMemSpaceIndex
		desiredNumOfMemSpaces,
		&actualNumOfMemSpaces,
		memory_spaces);

	//Flag to indicate whether memory space information could be retrieved and whether write access can be tested
	bool MemGetSpacesFailed = false;

	// Check the returned status
	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("ERROR: Getting memory space information from target failed. Return type is '%s'\n",
			CADIReturnToString(status));
		assert(status == eslapi::CADI_STATUS_OK);
		MemGetSpacesFailed = true;
	}

	// Check if any memory space has been returned
	if (actualNumOfMemSpaces == 0)
	{
		printf("ERROR: Target returned 0 memory spaces.\n");
		assert(actualNumOfMemSpaces > 0);
		MemGetSpacesFailed = true;
	}

	// Check if all requested memory spaces have been returned
	if (actualNumOfMemSpaces != target_features.nrMemSpaces)
	{
		printf("ERROR: Target returned a different number of memory spaces (%u) than expected (%u).\n",
			actualNumOfMemSpaces,
			target_features.nrMemSpaces);
		assert(actualNumOfMemSpaces == target_features.nrMemSpaces);
		MemGetSpacesFailed = true;
	}

	if (MemGetSpacesFailed == true)
	{
		printf("WARNING: Could not retrieve memory space information. Skipping register write accesses.\n");
	}
	else //Memory space information could be retrieved, performing memory accesses
	{
		printf("\tSuccessfully obtained memory space information.\n");

		// Read and print all memory block information, read and display the first 20 access units of each memory block
		ReadTargetMemories(cadi,
			memory_spaces,
			actualNumOfMemSpaces);

		// Start preparations for memory write access.
		// First retrieve the information for the first memory block of the first memory space. It is required as it is not
		// mandatory that 'minAddress' of CADIMemSpaceInfo_t is equal to 'startAddr' of CADIMemBlock_t
		printf("\n***Getting memory block information of first memory block of first memory space for following write access...\n");
		// Getting first memory block of first memory space
		eslapi::CADIMemBlockInfo_t targetMemBlock;
		uint32_t actualNumOfMemBlocks = 0;

		// Only retrieving the first memory block
		status = cadi->CADIMemGetBlocks(memory_spaces[0].memSpaceId,
			0, //memBlockIndex
			1, //desiredNumOfMemBlocks
			&actualNumOfMemBlocks,
			&targetMemBlock);

		//Flag to indicate whether memory space information could be retrieved and whether write access can be tested
		bool MemGetBlocksFailed = false;

		// Check the returned status
		if (eslapi::CADI_STATUS_OK != status)
		{
			printf("ERROR: Retrieving memory block information of first memory block of memory space '%s' (ID: %u) failed.\n",
				memory_spaces[0].memSpaceName,
				memory_spaces[0].memSpaceId);
			assert(eslapi::CADI_STATUS_OK == status);
			MemGetBlocksFailed = true;
		}

		// Check if exactly one memory block has been returned (as requested)
		if (actualNumOfMemBlocks != 1)
		{
			printf("ERROR: Retrieving memory block information of first memory block of memory space '%s' (ID: %u) returned wrong actual number of memory blocks (%u).\n",
				memory_spaces[0].memSpaceName,
				memory_spaces[0].memSpaceId,
				actualNumOfMemBlocks);
			assert(actualNumOfMemBlocks == 1);
			MemGetBlocksFailed = true;
		}


		if (MemGetBlocksFailed == true)
		{
			printf("WARNING: Could not retrieve memory block information. Skipping memory write access.\n");
		}
		else //Memory block information could be retrieved, performing memory accesses
		{
			printf("\tSuccessfully retrieved memory block information of first memory block of first memory space.\n");

			// Prepare the write access parameters as well as the data to write
			printf("\n***Preparing data for memory write access to memory block '%s' (ID: %u) of memory space '%s' (ID: %u)...\n",
				targetMemBlock.name,
				targetMemBlock.id,
				memory_spaces[0].memSpaceName,
				memory_spaces[0].memSpaceId);

			// Writing values to five addresses of the first memory space's block; using an offset of 7 if enough memory is available
			/*
			uint32_t writeOffset = 7; 
			uint32_t unitsToWrite = 5;
			*/
			//begin of mod by cxl
			uint32_t writeOffset = 0; //修改偏移为0；

			uint32_t unitsToWrite = nLength;
            //end of mod by cxl
			if ((targetMemBlock.endAddr - targetMemBlock.startAddr) < writeOffset) /* Check if offset is too large. If this is the case,
																				   start from the startAddr of the memory block. */
			{
				writeOffset = 0;
			}

			if ((targetMemBlock.endAddr - targetMemBlock.startAddr) < (writeOffset + unitsToWrite)) /* Check if all memory addresses can be written.
																									If the available memory range is to small,
																									write to all accessible memory addresses. */
			{
				unitsToWrite = (uint32_t)(targetMemBlock.endAddr - targetMemBlock.startAddr) - writeOffset;
			}

			// Start address of the write access
			eslapi::CADIAddrComplete_t writeAddress;
			// writeAddress.overlay set to default value by constructor of CADIAddrComplete_t, not supported by model
			writeAddress.location.space = memory_spaces[0].memSpaceId;
			//writeAddress.location.addr = targetMemBlock.startAddr + writeOffset;

			//begin of mod by cxl
			writeAddress.location.addr = nStartAddr + writeOffset;
			//end of mod by cxl
			//writeUnitSizeInBytes计算的结果和nDataWidth是一致的。这里修改为 writeUnitSizeInBytes = nDataWidth 
			uint32_t writeUnitSizeInBytes = nDataWidth;
			//uint32_t writeUnitSizeInBytes = (memory_spaces[0].bitsPerMau + 7) / 8;
			uint32_t actualNumOfUnitsWritten = 0;
			uint32_t writeAccessInBytes = writeUnitSizeInBytes * unitsToWrite;

			// Allocate and fill the buffer for the write access
			uint8_t *writeData = new uint8_t[writeAccessInBytes];
			for (unsigned int i = 0; i < unitsToWrite; i++)
			{
				//此处修改
				//writeData[i] = 5 + 3 * i;
                memcpy(&writeData[i], buf, 1);
				printf("\t%02x", writeData[i]);
				buf += 1;
			}

			// Just a short printout of the most important parameters for the write access
			printf("\n***Performing write access:\n\n");
			printf("\taddress: 0x%08x%08x of memory space '%s' (ID: %u).\n",
				(uint32_t)(writeAddress.location.addr >> 32),
				(uint32_t)(writeAddress.location.addr),
				memory_spaces[0].memSpaceName,
				memory_spaces[0].memSpaceId);
			printf("\tunitsToWrite: %u\n",
				unitsToWrite);
			printf("\tunitSizeInBytes: %u\n\n",
				writeUnitSizeInBytes);
			for (unsigned int i = 0; i < unitsToWrite; i++)
			{
				printf("\t\tdata[%u]: 0x%02x\n",
					i,
					writeData[i]);
			}
			printf("\n");

			// Do the memory access
			status = cadi->CADIMemWrite(writeAddress,
				unitsToWrite,
				writeUnitSizeInBytes,
				writeData,
				&actualNumOfUnitsWritten,
				0); // doSideEffects (set to '0', so no side effects are done)

					// Free the allocated data buffer
			delete[] writeData;
			writeData = 0;

			// Read and print out the target memories again to show the effect of the write access
			// (only displaying the first 20 units of each memory block).
			ReadTargetMemories(cadi,
				memory_spaces,
				actualNumOfMemSpaces);
		}

	}

	// Memory space information no longer needed. Freeing the allocated memory.
	delete[] memory_spaces;
	memory_spaces = 0;

	return SIMC_OK;
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
	std::string strPlugin = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\MTI\\SimpleTrace\\x64\\Release_2013\\SimpleTrace.dll";
	//模拟器路径
	std::string strSim = "D:\\ARM\\FastModelsPortfolio_11.3\\examples\\LISA\\FVP_VE\\Build_Cortex-A15x1\\Win64-Release-VC2015\\cadi_system_Win64-Release-VC2015.dll";
	//std::string strSim = "cadi_system_Win64-Release-VC2015.dll";
	//应用程序路径
	std::string strApp = "D:\\ARM\\FastModelsPortfolio_11.3\\images\\brot_ve_A.axf";

	// Define the MTI plugin to use by setting environment variable FM_TRACE_PLUGINS
	set_plugin(strPlugin);
	
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

int simc_plugin_get_status(UINT32 * pnErrorCode)
{
	*pnErrorCode = cadi_callback->GetCurrentMode();
	return SIMC_OK;
}

int simc_plugin_get_execcycle(UINT64* pnCycleCount)
{
	eslapi::CADIReturn_t status = cadi->CADIGetCycleCount(*pnCycleCount, false);
	//eslapi::CADIReturn_t status = cadi->CADIGetCycleCount(tmp, true);
	if (status != eslapi::CADI_STATUS_OK)
	{
		printf("CADIExecContinue returned an error，status = %d, pnCycleCount = %I64u\n", status, *pnCycleCount);
		return (int)status;
	}
	return (int)status;
}

