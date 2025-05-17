// cVAOManager_Async_Loader.cpp

#include "cVAOManager.h"

#include "../GLCommon.h"	// All the GLFW, etc. (OpenGL stuff)

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <fstream>

#include <Windows.h>	// For CreateThread
//#include <thread>	// for C++ 11 threads

#include <iostream>

CRITICAL_SECTION cVAOManager_CS_ModelMapLock;

void cVAOManager::m_LockModelMap(void)
{
	EnterCriticalSection(&cVAOManager_CS_ModelMapLock);
	return;
}

//bool cVAOManager::isModelMapLocked(void)
//{
//
//}

void cVAOManager::m_UnlockModelMap(void)
{
	LeaveCriticalSection(&cVAOManager_CS_ModelMapLock);
	return;
}

LONG g_Locked = 7283;		// 7283 means "unlocked"
//https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedexchange
//LONG InterlockedExchange(
//	[in, out] LONG volatile* Target,
//	[in]      LONG          Value
//);
void Lock(void)
{
}

void Unlock(void)
{
	InterlockedExchange(&g_Locked, 7283);		// 7283 means "unlocked"
	return;
}

// https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchange
bool isLocked(void)
{
//	LONG InterlockedCompareExchange(
//		[in, out] LONG volatile* Destination,
//		[in]      LONG          ExChange,
//		[in]      LONG          Comperand
//	);
	return true;
}




struct sAsyncPLYLoaderInfo
{
	// Lets us know the thread is still working
	// (can't take any more work)
	bool bThreadIsBusy = false;	
	// This controls when the thread wakes up
	bool bThreadIsAwake = false;
	DWORD threadSleepTime = 100;
	// This will "kill" the thread (i.e. make it exit)
	bool bKillThread = false;
	cVAOManager* pVAOManager = NULL;	// To call the PLY file reading code
	std::map< std::string, sModelDrawInfo>* p_mapModelName_to_VAOID;

	//
	HANDLE hThread = NULL;
	LPDWORD pThreadId = NULL;
};

sAsyncPLYLoaderInfo cVAOManager_ThreadControl;

DWORD WINAPI threadProcLoadPlyFile(LPVOID pParameter)
{
	sAsyncPLYLoaderInfo* pThreadControl = (sAsyncPLYLoaderInfo*)pParameter;

	while ( ! pThreadControl->bKillThread )
	{
		if (pThreadControl->bThreadIsAwake)
		{
			pThreadControl->bThreadIsBusy = true;

			sModelDrawInfo meshFileToLoad;
			bool bFoundModelToLoad = false;

			// Are there any ply models that need to be loaded? 
			EnterCriticalSection(&cVAOManager_CS_ModelMapLock);

			// Look through the map to see if any models haven't been loaded, yet
			for (std::map< std::string, sModelDrawInfo>::iterator itMDI =
				pThreadControl->p_mapModelName_to_VAOID->begin();
				itMDI != pThreadControl->p_mapModelName_to_VAOID->end();
				itMDI++)
			{
				if (!itMDI->second.bIsLoaded)
				{
					bFoundModelToLoad = true;
					// Make a copy of the mesh info from the map
					meshFileToLoad = itMDI->second;
					break;	// Exit loop
				}
			}
			LeaveCriticalSection(&cVAOManager_CS_ModelMapLock);

			if (bFoundModelToLoad)
			{
				// Load the file (just like we did before)
				if (pThreadControl->pVAOManager->m_readPlyFile_XYZ_Normal_UV(meshFileToLoad))
				{
					meshFileToLoad.bIsLoaded = true;

					// File was loaded OK
					meshFileToLoad.calculateExtents();
					// Add it to the map (overwrite the old mesh info, which only had the name
					EnterCriticalSection(&cVAOManager_CS_ModelMapLock);
					// Note: this is a pointer, so we can't access the [] operator,
					//	but at() is the same thing
					pThreadControl->p_mapModelName_to_VAOID->at(meshFileToLoad.meshName) = meshFileToLoad;
					// Or this nightmare:
					// (*(pThreadControl->p_mapModelName_to_VAOID))[meshFileToLoad.meshName] = meshFileToLoad;
					
					LeaveCriticalSection(&cVAOManager_CS_ModelMapLock);
				}
				else
				{
					// TODO: We didn't load the file, so indicate this some way
				}
			}
			else
			{
				// Didn't find a model to load, so go to sleep
				pThreadControl->bThreadIsBusy = false;
				// Go to sleep
				pThreadControl->bThreadIsAwake = false;
			}//if (bFoundModelToLoad)
		}
		else
		{
			Sleep(pThreadControl->threadSleepTime);
		}
	}//while ( ! pThreadControl->bKillThread )

	return 0;
}



void cVAOManager::ComputeTangentsAndBitangents(sModelDrawInfo& modelDrawInfo)
{
	// Clear tangents and bitangents
	for (unsigned int i = 0; i < modelDrawInfo.numberOfVertices; ++i)
	{
		modelDrawInfo.pVertices[i].tx = 0.0f;
		modelDrawInfo.pVertices[i].ty = 0.0f;
		modelDrawInfo.pVertices[i].tz = 0.0f;
		//modelDrawInfo.pVertices[i].bx = 0.0f;
		//modelDrawInfo.pVertices[i].by = 0.0f;
		//modelDrawInfo.pVertices[i].bz = 0.0f;
	}

	for (unsigned int i = 0; i < modelDrawInfo.numberOfIndices; i += 3)
	{
		// Get the indices of the triangle
		unsigned int i0 = modelDrawInfo.pIndices[i];
		unsigned int i1 = modelDrawInfo.pIndices[i + 1];
		unsigned int i2 = modelDrawInfo.pIndices[i + 2];

		sVertex_SHADER_FORMAT_xyz_rgb_N_UV& v0 = modelDrawInfo.pVertices[i0];
		sVertex_SHADER_FORMAT_xyz_rgb_N_UV& v1 = modelDrawInfo.pVertices[i1];
		sVertex_SHADER_FORMAT_xyz_rgb_N_UV& v2 = modelDrawInfo.pVertices[i2];

		// Positions
		glm::vec3 pos0(v0.x, v0.y, v0.z);
		glm::vec3 pos1(v1.x, v1.y, v1.z);
		glm::vec3 pos2(v2.x, v2.y, v2.z);

		// UVs
		glm::vec2 uv0(v0.u, v0.v);
		glm::vec2 uv1(v1.u, v1.v);
		glm::vec2 uv2(v2.u, v2.v);

		// Edges of the triangle : position delta
		glm::vec3 deltaPos1 = pos1 - pos0;
		glm::vec3 deltaPos2 = pos2 - pos0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		//glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		// Accumulate tangents and bitangents
		v0.tx += tangent.x; v0.ty += tangent.y; v0.tz += tangent.z;
		v1.tx += tangent.x; v1.ty += tangent.y; v1.tz += tangent.z;
		v2.tx += tangent.x; v2.ty += tangent.y; v2.tz += tangent.z;

		/*v0.bx += bitangent.x; v0.by += bitangent.y; v0.bz += bitangent.z;
		v1.bx += bitangent.x; v1.by += bitangent.y; v1.bz += bitangent.z;
		v2.bx += bitangent.x; v2.by += bitangent.y; v2.bz += bitangent.z;*/
	}

	// Normalize the tangents and bitangents
	for (unsigned int i = 0; i < modelDrawInfo.numberOfVertices; ++i)
	{
		glm::vec3 t(modelDrawInfo.pVertices[i].tx,
			modelDrawInfo.pVertices[i].ty,
			modelDrawInfo.pVertices[i].tz);
		/*glm::vec3 b(modelDrawInfo.pVertices[i].bx,
			modelDrawInfo.pVertices[i].by,
			modelDrawInfo.pVertices[i].bz);*/

		t = glm::normalize(t);
		//b = glm::normalize(b);

		modelDrawInfo.pVertices[i].tx = t.x;
		modelDrawInfo.pVertices[i].ty = t.y;
		modelDrawInfo.pVertices[i].tz = t.z;

		//modelDrawInfo.pVertices[i].bx = b.x;
		//modelDrawInfo.pVertices[i].by = b.y;
		//modelDrawInfo.pVertices[i].bz = b.z;
	}
}

void cVAOManager::m_InitThead_and_CS(void)
{
	// Init the critical section
	InitializeCriticalSection(&cVAOManager_CS_ModelMapLock);

	// Create thread, but alseep
	cVAOManager_ThreadControl.bKillThread = false;
	cVAOManager_ThreadControl.bThreadIsAwake = true;
	cVAOManager_ThreadControl.pVAOManager = this;
	cVAOManager_ThreadControl.p_mapModelName_to_VAOID = &(this->m_map_ModelName_to_VAOID);

	// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
	cVAOManager_ThreadControl.hThread 
		= CreateThread(
			NULL,                           // LPSECURITY_ATTRIBUTES lpThreadAttributes
			0,                              // SIZE_T dwStackSize,
			threadProcLoadPlyFile,    // LPTHREAD_START_ROUTINE lpStartAddress,
			(void*)(&cVAOManager_ThreadControl),       //__drv_aliasesMem LPVOID lpParameter,
			0,                              // DWORD dwCreationFlags,
			NULL);                          // LPDWORD lpThreadId


	return;
}


void cVAOManager::m_TerminateThread_and_CS(void)
{
	DeleteCriticalSection(&cVAOManager_CS_ModelMapLock);
	// Shut down the thread

	return;
}

// This will load models on a separate thread
// (i.e. it will return right away)
bool cVAOManager::LoadModelIntoVAO_Async(std::string fileName,
	sModelDrawInfo& drawInfo,
	unsigned int shaderProgramID)
{

	drawInfo.meshName = fileName;
	drawInfo.bIsLoaded = false;
	drawInfo.bIsCopiedToVAO = false;

	// TODO: Check if that file exists BEFORE we load it...
	// (or do we want to do this in the thread??)
	std::ifstream theFile(fileName);
	if ( ! theFile.is_open() )
	{
		// Can't find the file
		return false;
	}
	theFile.close();

	// Add this to the vector of meshes, but keep bIsLoaded = false
	this->m_LockModelMap();
	this->m_map_ModelName_to_VAOID[drawInfo.meshName] = drawInfo;
	this->m_UnlockModelMap();

	return true;
}

// Will only look check the map of meshes every 30 frames
const int CVAOMANAGER_ASYNC_LOADING_FRAME_DELAY = 30;
int cVAOManager_LoadAsyncModels_Countdown = 0;

// This is called every frame to deal with 
//	any loaded models from the async thread
void cVAOManager::LoadAsynModels(unsigned int shaderProgramID)
{
	// only run this every so often
	cVAOManager_LoadAsyncModels_Countdown++;
	if (cVAOManager_LoadAsyncModels_Countdown < CVAOMANAGER_ASYNC_LOADING_FRAME_DELAY)
	{
		// DON'T run 
		return;
	}
	// Reset the counter
	cVAOManager_LoadAsyncModels_Countdown = 0;

	// Check to see if there are any models that haven't been loaded
	// If there are, wake up the thread and have it load the file.
	// Then load that file onto the VAO

	// Wake the thread up to see if there are any more models to be loaded
	cVAOManager_ThreadControl.bThreadIsAwake = true;



	// Unlike the thread, I'm looking for models that ARE loaded, but 
	//	NOT coped to the VAO yet (i.e. the thread loaded it from a file, 
	//	but we still need to copy the info to the VAO)
	// (and we need to do that on THIS thread)

	sModelDrawInfo meshToLoadToVAO;
	bool bFoundModelToCopyToVAO = false;

	this->m_LockModelMap();
	for (std::map<std::string, sModelDrawInfo>::iterator itMDI = this->m_map_ModelName_to_VAOID.begin();
		itMDI != this->m_map_ModelName_to_VAOID.end(); itMDI++)
	{
		if ( ! itMDI->second.bIsCopiedToVAO )
		{
			// Is it loaded? (did the thread load it already)
			if (itMDI->second.bIsLoaded)
			{
				meshToLoadToVAO = itMDI->second;
				bFoundModelToCopyToVAO = true;
				break;
			}
		}
	}//for (...iterator
	this->m_UnlockModelMap();

	if (bFoundModelToCopyToVAO)
	{
		// This has been loaded and needs to be copied to the VAO
		this->m_CopyLoadedModelToVAO(meshToLoadToVAO, shaderProgramID);
	}

	return;
}




//class asyncUpdaterThread {
//	// Overload () operator
//	void operator()(sAsyncPLYLoaderInfo threadControlInfo)
//	{
//		// SAME stuff I did before
//
//
//		return;
//	}
//};
//
//
//class asyncUpdaterThread_B {
//	// Overload () operator
//	void operator()(
//		bool bThreadIsBusy, 
//		bool bThreadIsAwake,
//		DWORD threadSleepTime,
//		bool bKillThread,
//		cVAOManager* pVAOManager,
//		std::map< std::string, sModelDrawInfo>* p_mapModelName_to_VAOID,
//		HANDLE hThread, 
//		LPDWORD pThreadId)
//	{
//		// SAME stuff I did before
//
//
//		return;
//	}
//};
//void StartThread()
//{
//	sAsyncPLYLoaderInfo threadControlInfo;
//	std::thread thread_object(asyncUpdaterThread(), threadControlInfo);
//
//
//	bool bThreadIsBusy = false;
//	bool bThreadIsAwake = true; 
//	DWORD threadSleepTime = 100;
//	bool bKillThread = false;
//	cVAOManager* pVAOManager = NULL;
//	std::map< std::string, sModelDrawInfo>* p_mapModelName_to_VAOID = NULL;
//	HANDLE hThread = NULL;
//	LPDWORD pThreadId = NULL;
//
//	std::thread thread_object_B(asyncUpdaterThread_B(), 
//		bThreadIsBusy, bThreadIsAwake, threadSleepTime, bKillThread, 
//		pVAOManager, p_mapModelName_to_VAOID, hThread, pThreadId);
//
//	return;
//}




LONG64 g_Total = 0;


void IncrementByOne(void)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedincrement64	
	//	LONG64 InterlockedIncrement64(
	//		[in, out] LONG64 volatile* Addend
	//	);

	LONG64 newValue = InterlockedIncrement64(&g_Total);

	std::cout << "Total now: " << newValue << std::endl;

	return;
}

void Add(__int64 addedNum)
{
	// https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedexchangeadd64
	//	LONG64 InterlockedExchangeAdd64(
	//		[in, out] LONG64 volatile* Addend,
	//		[in]      LONG64          Value
	//	);

	LONG64 result = InterlockedExchangeAdd64(&g_Total, addedNum);
	
	std::cout << "Total now: " << result << std::endl;

	return;
}
