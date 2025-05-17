//#include "cParticleEmitter_2.h"
//
//cParticleEmitter_2::cParticleEmitter_2()
//{
//	this->m_ResizeContainer();
//}
//
//cParticleEmitter_2::~cParticleEmitter_2()
//{
//
//
//}
//
//void cParticleEmitter_2::SetMaximumNumberOfParticles(unsigned int maxParticles)
//{
//	this->m_MaxNumberOfParticles = maxParticles;
//}
//
//
//void cParticleEmitter_2::SetSourceLocation(glm::vec3 positionXYZ)
//{
//	this->m_PositionXYZ = positionXYZ;
//	return;
//}
//
//void cParticleEmitter_2::SetInitalVelocity(glm::vec3 minXYZ, glm::vec3 maxXYZ)
//{
//	this->m_minVelocityXYZ = minXYZ;
//	this->m_maxVelocityXYZ = maxXYZ;
//	return;
//}
//
//void cParticleEmitter_2::SetNumberOfParticlesPerFrame(unsigned int numParticlesPerUpdate)
//{
//	this->m_numParticlesPerUpdate = numParticlesPerUpdate;
//	return;
//}
//
//void cParticleEmitter_2::Update(double deltaTime)
//{
//	// Every tick, create X number of particles and add to the vector
//	if (this->m_bIsEnabled)
//	{
//		this->m_CreateParticles(this->m_numParticlesPerUpdate);
//	}
//
//	for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
//	{
//		cParticle* pTestParticle = &(this->m_pParticles[index]);
//
//		if (pTestParticle->timeTillDeath > 0.0f)
//		{
//			pTestParticle->timeTillDeath -= (float)deltaTime;
//
//			// Euler integration
//			pTestParticle->velocity += pTestParticle->accel * (float)deltaTime;
//
//			pTestParticle->position += pTestParticle->velocity * (float)deltaTime;
//		}
//	}//for (unsigned int index
//
//
//
//
//
//	return;
//}
//
//void cParticleEmitter_2::m_ResizeContainer(void)
//{
//	// Delete all the old particles
//	delete[] this->m_pParticles;
//
//	// Allocate a new one
//	this->m_pParticles = new cParticle[this->m_MaxNumberOfParticles];
//
//	return;
//}
//
//
//void cParticleEmitter_2::m_CreateParticles(unsigned int numParticles)
//{
//	// Rather than creating (new) and adding them to the vector,
//	//	we are going to scan through the array of possible particles
//	//	and "revive" and "dead" ones we find
//
//	unsigned int numParticlesToCreate = numParticles;
//	unsigned int particleIndex = 0;
//	do
//	{
//		// is this particle dead? 
//		// NOTE: Be careful about the "&" 
//		// If you don't do this (or use a pointer, etc.)
//		//	then you are working with the COPY, the actual particle.
//		cParticle& testParticle = this->m_pParticles[particleIndex];
//
//		if (testParticle.timeTillDeath <= 0.0f)
//		{
//			// Yes. So "revive" it
//			testParticle.velocity = this->getRandom_vec3(this->m_minVelocityXYZ, this->m_maxVelocityXYZ);
//
//			float minLife = 0.1f;
//			float maxLife = 0.2f;
//			testParticle.timeTillDeath = this->getRandomFloat(minLife, maxLife);
//			testParticle.position = this->m_PositionXYZ;
//			testParticle.accel = glm::vec3(0.0f, -1.0f, 0.0f);
//
//			numParticlesToCreate--;
//		}
//
//		particleIndex++;
//
//	} while ( (numParticlesToCreate > 0) && (particleIndex < this->m_MaxNumberOfParticles) );
//
//
//	return;
//}
//
//
//
//void cParticleEmitter_2::CreateParticlesImmediately(unsigned int numParticles)
//{
//	this->m_CreateParticles(numParticles);
//	return;
//}
//
////void cParticleEmitter_2::CreateBurstParticles(unsigned int numParticles)
////{
////	unsigned int numParticlesToCreate = numParticles;
////	unsigned int particleIndex = 0;
////
////	do
////	{
////		cParticle& testParticle = this->m_pParticles[particleIndex];
////
////		if (testParticle.timeTillDeath <= 0.0f)
////		{
////			// Set up the burst particle
////			testParticle.position = this->m_PositionXYZ;
////
////			// Random direction on a sphere
////			glm::vec3 randomDir = this->getRandomDirectionOnSphere();
////			float randomSpeed = this->getRandomFloat(5.0f, 10.0f);
////			testParticle.velocity = randomDir * randomSpeed;
////
////			// Gravity
////			testParticle.accel = glm::vec3(0.0f, -9.8f, 0.0f);
////
////			// Random lifetime
////			testParticle.timeTillDeath = this->getRandomFloat(1.5f, 3.0f);
////
////			numParticlesToCreate--;
////		}
////
////		particleIndex++;
////
////	} while ((numParticlesToCreate > 0) && (particleIndex < this->m_MaxNumberOfParticles));
////
////	return;
////}
//
//void cParticleEmitter_2::CreateBurstParticles(unsigned int numParticles, glm::vec3 position)
//{
//	unsigned int numParticlesToCreate = numParticles;
//	unsigned int particleIndex = 0;
//
//	do
//	{
//		// Always revive the next particle, even if it is still alive
//		cParticle& testParticle = this->m_pParticles[particleIndex];
//
//		
//		// FORCE revive the particle
//		testParticle.position = position;
//
//		// Give random velocity in a sphere shape
//		glm::vec3 randomDir = this->getRandomDirectionOnSphere();
//		float randomSpeed = this->getRandomFloat(5.0f, 10.0f);
//		testParticle.velocity = randomDir * randomSpeed;
//
//		// Add gravity
//		testParticle.accel = glm::vec3(0.0f, -9.8f, 0.0f);
//
//		// Random time till death
//		testParticle.timeTillDeath = this->getRandomFloat(0.5f, 1.0f);
//
//		numParticlesToCreate--;
//		particleIndex++;
//
//		// Wrap back if we reach end of particle array
//		if (particleIndex >= this->m_MaxNumberOfParticles)
//		{
//			particleIndex = 0;
//		}
//
//	} while (numParticlesToCreate > 0);
//
//	return;
//}
//
//
//
//void cParticleEmitter_2::GetLiveParticles(std::vector< cParticle* >& vec_Particles)
//{
//	vec_Particles.clear();
//
//	// Copy any "live" particles to this vector we passed in
//	for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
//	{
//		cParticle* pTestParticle = &(this->m_pParticles[index]);
//
//		if (pTestParticle->timeTillDeath > 0.0f)
//		{
//			// It's alive
//			vec_Particles.push_back(pTestParticle);
//		}
//	}
//
//	return;
//}
//
//unsigned int cParticleEmitter_2::GetNumberOfLiveParticles(void)
//{
//	unsigned int numParticles = 0;
//
//	for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
//	{
//		cParticle* pTestParticle = &(this->m_pParticles[index]);
//
//		if (pTestParticle->timeTillDeath > 0.0f)
//		{
//			// It's alive
//			numParticles++;
//		}
//	}
//	return numParticles;
//}
//
//void cParticleEmitter_2::SetLocationXYZ(glm::vec3 newLocation)
//{
//	this->m_PositionXYZ = newLocation;
//	return;
//}
//
//glm::vec3 cParticleEmitter_2::GetLocation(void)
//{
//	return this->m_PositionXYZ;
//}
//
//void cParticleEmitter_2::TurnOn(void)
//{
//	this->m_bIsEnabled = true;
//	return;
//}
//
//void cParticleEmitter_2::TurnOff(void)
//{
//	this->m_bIsEnabled = false;
//	return;
//}
//bool cParticleEmitter_2::bIsOn(void)
//{
//	return this->m_bIsEnabled;
//}

#include "cParticleEmitter_2.h"
#include <glm/glm.hpp>
#include <random>
#include <iostream>
#include <Windows.h>

// Static critical section for particle array access
//CRITICAL_SECTION cParticleEmitter_2::m_csParticleArrayLock;
extern double deltaTime;
extern bool bUseThreading;

cParticleEmitter_2::cParticleEmitter_2()
{
    this->m_csParticleArrayLock = new CRITICAL_SECTION();
    this->m_MaxNumberOfParticles = 15'000; // Default size
    this->m_pParticles = nullptr;
    this->m_bIsEnabled = false;
    bUseThreading = true;
    this->m_numParticlesPerUpdate = 100;
    this->m_PositionXYZ = glm::vec3(0.0f);
    this->m_minVelocityXYZ = glm::vec3(-1.0f);
    this->m_maxVelocityXYZ = glm::vec3(1.0f);

    this->m_InitThreadsAndCS();
    this->m_ResizeContainer();
}

cParticleEmitter_2::~cParticleEmitter_2()
{
    this->m_TerminateThreadsAndCS();
    delete[] this->m_pParticles;
}

void cParticleEmitter_2::m_LockParticleArray(void)
{
    EnterCriticalSection(m_csParticleArrayLock);
}

void cParticleEmitter_2::m_UnlockParticleArray(void)
{
    LeaveCriticalSection(m_csParticleArrayLock);
}

// Thread procedure declaration
DWORD WINAPI ParticleUpdateThreadProc(void* pParameter);

void cParticleEmitter_2::m_InitThreadsAndCS(void)
{
    // Initialize critical section
    InitializeCriticalSection(m_csParticleArrayLock);

    //// Determine number of threads (e.g., based on hardware concurrency)
    //unsigned int numThreads = 16; // Example: Use 4 threads; adjust based on system
    //m_vecThreadControls.resize(numThreads);

    //// Divide particle array among threads
    //unsigned int particlesPerThread = m_MaxNumberOfParticles / numThreads;
    //for (unsigned int i = 0; i < numThreads; ++i)
    //{
    //    sParticleUpdateThreadInfo& threadInfo = m_vecThreadControls[i];
    //    threadInfo.bKillThread = false;
    //    threadInfo.bThreadIsAwake = false;
    //    threadInfo.pEmitter = this;
    //    threadInfo.startIndex = i * particlesPerThread;
    //    threadInfo.endIndex = (i == numThreads - 1) ? m_MaxNumberOfParticles : (i + 1) * particlesPerThread;

    //    // Create thread
    //    threadInfo.hThread = CreateThread(
    //        NULL,                           // Default security attributes
    //        0,                              // Default stack size
    //        ParticleUpdateThreadProc,       // Thread procedure
    //        (LPVOID)&threadInfo,            // Thread parameter
    //        0,                              // Run immediately
    //        NULL                            // Thread ID
    //    );
    //}
    if (bUseThreading)
    {
        unsigned int numThreads = 16; // Use 16 threads when multithreading is enabled
        m_vecThreadControls.resize(numThreads);
        unsigned int particlesPerThread = m_MaxNumberOfParticles / numThreads;
        for (unsigned int i = 0; i < numThreads; ++i)
        {
            sParticleUpdateThreadInfo& threadInfo = m_vecThreadControls[i];
            threadInfo.bKillThread = false;
            threadInfo.bThreadIsAwake = false;
            threadInfo.pEmitter = this;
            threadInfo.startIndex = i * particlesPerThread;
            threadInfo.endIndex = (i == numThreads - 1) ? m_MaxNumberOfParticles : (i + 1) * particlesPerThread;
            threadInfo.hThread = CreateThread(NULL, 0, ParticleUpdateThreadProc, (LPVOID)&threadInfo, 0, NULL);
        }
    }
    else
    {
        m_vecThreadControls.clear(); // No threads in single-threaded mode
    }
}

void cParticleEmitter_2::m_TerminateThreadsAndCS(void)
{
    //// Signal all threads to terminate
    //for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
    //{
    //    threadInfo.bKillThread = true;
    //    threadInfo.bThreadIsAwake = true; // Wake threads to check kill flag
    //}

    //// Wait for threads to exit
    //for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
    //{
    //    if (threadInfo.hThread)
    //    {
    //        WaitForSingleObject(threadInfo.hThread, INFINITE);
    //        CloseHandle(threadInfo.hThread);
    //    }
    //}

    //// Clean up critical section
    //DeleteCriticalSection(m_csParticleArrayLock);
    if (bUseThreading)
    {
        // Signal all threads to terminate
        for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
        {
            threadInfo.bKillThread = true;
            threadInfo.bThreadIsAwake = true;
        }

        // Wait for threads to exit
        for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
        {
            if (threadInfo.hThread)
            {
                WaitForSingleObject(threadInfo.hThread, INFINITE);
                CloseHandle(threadInfo.hThread);
            }
        }
    }

    DeleteCriticalSection(m_csParticleArrayLock);
    delete m_csParticleArrayLock;
    m_csParticleArrayLock = nullptr;
}

DWORD WINAPI ParticleUpdateThreadProc(LPVOID pParameter)
{
    cParticleEmitter_2::sParticleUpdateThreadInfo* pThreadInfo = (cParticleEmitter_2::sParticleUpdateThreadInfo*)pParameter;

    while (!pThreadInfo->bKillThread)
    {
        if (pThreadInfo->bThreadIsAwake)
        {
            pThreadInfo->bThreadIsBusy = true;

            // Update particles in the assigned range
            //pThreadInfo->pEmitter->m_LockParticleArray();
            for (unsigned int index = pThreadInfo->startIndex; index < pThreadInfo->endIndex; ++index)
            {
                cParticle& particle = pThreadInfo->pEmitter->m_pParticles[index];
                if (particle.timeTillDeath > 0.0f)
                {
                    particle.timeTillDeath -= (float)pThreadInfo->deltaTime;
                    particle.velocity += particle.accel * (float)pThreadInfo->deltaTime;
                    particle.position += particle.velocity * (float)pThreadInfo->deltaTime;
                }
            }
            //pThreadInfo->pEmitter->m_UnlockParticleArray();

            pThreadInfo->bThreadIsBusy = false;
            pThreadInfo->bThreadIsAwake = false; // Go to sleep
        }
        else
        {
            Sleep(pThreadInfo->threadSleepTime);
        }
    }

    return 0;
}

void cParticleEmitter_2::Update(double deltaTime)
{
    //// Create new particles if enabled
    //if (this->m_bIsEnabled)
    //{
    //    this->m_LockParticleArray();
    //    this->m_CreateParticles(this->m_numParticlesPerUpdate);
    //    this->m_UnlockParticleArray();
    //}

    //// Wake threads to update particles
    //for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
    //{
    //    threadInfo.deltaTime = deltaTime;
    //    threadInfo.bThreadIsAwake = true;
    //}
    // Create new particles if enabled


    if (bUseThreading)
    {
        if (this->m_bIsEnabled)
        {
            //this->m_LockParticleArray();
            this->m_CreateParticles(this->m_numParticlesPerUpdate);
            //this->m_UnlockParticleArray();
        }
        // Wake threads to update particles
        for (std::vector<cParticleEmitter_2::sParticleUpdateThreadInfo>::iterator::value_type& threadInfo : m_vecThreadControls)
        {
            threadInfo.deltaTime = deltaTime;
            threadInfo.bThreadIsAwake = true;
        }
    }
    else
    {
        if (this->m_bIsEnabled)
        {
            this->m_CreateParticles(this->m_numParticlesPerUpdate);
        }
        // Single-threaded update: process all particles in main thread
        //m_LockParticleArray();
        for (unsigned int index = 0; index < m_MaxNumberOfParticles; ++index)
        {
            cParticle& particle = m_pParticles[index];
            if (particle.timeTillDeath > 0.0f)
            {
                particle.timeTillDeath -= (float)deltaTime;
                particle.velocity += particle.accel * (float)deltaTime;
                particle.position += particle.velocity * (float)deltaTime;
            }
        }
        //m_UnlockParticleArray();
    }
}

void cParticleEmitter_2::m_ResizeContainer(void)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        delete[] this->m_pParticles;
        this->m_pParticles = new cParticle[this->m_MaxNumberOfParticles];

        // Reassign thread ranges if necessary
        if (!m_vecThreadControls.empty())
        {
            unsigned int numThreads = m_vecThreadControls.size();
            unsigned int particlesPerThread = m_MaxNumberOfParticles / numThreads;
            for (unsigned int i = 0; i < numThreads; ++i)
            {
                m_vecThreadControls[i].startIndex = i * particlesPerThread;
                m_vecThreadControls[i].endIndex = (i == numThreads - 1) ? m_MaxNumberOfParticles : (i + 1) * particlesPerThread;
            }
        }
        //m_UnlockParticleArray();
    }
    else {
    	// Delete all the old particles
    	delete[] this->m_pParticles;
    
    	// Allocate a new one
    	this->m_pParticles = new cParticle[this->m_MaxNumberOfParticles];
    
    	return;
    }
}

void cParticleEmitter_2::m_CreateParticles(unsigned int numParticles)
{
    unsigned int numParticlesToCreate = numParticles;
    unsigned int particleIndex = 0;
    do
    {
        cParticle& testParticle = this->m_pParticles[particleIndex];
        if (testParticle.timeTillDeath <= 0.0f)
        {
            testParticle.velocity = this->getRandom_vec3(this->m_minVelocityXYZ, this->m_maxVelocityXYZ);
            float minLife = 0.1f;
            float maxLife = 0.2f;
            testParticle.timeTillDeath = this->getRandomFloat(minLife, maxLife);
            testParticle.position = this->m_PositionXYZ;
            testParticle.accel = glm::vec3(0.0f, -1.0f, 0.0f);
            numParticlesToCreate--;
        }
        particleIndex++;
    } while ((numParticlesToCreate > 0) && (particleIndex < this->m_MaxNumberOfParticles));
}

void cParticleEmitter_2::CreateParticlesImmediately(unsigned int numParticles)
{
    //if (bUseThreading) {
        m_LockParticleArray();
        this->m_CreateParticles(numParticles);
        m_UnlockParticleArray();
    //}
    //else {
    //    this->m_CreateParticles(numParticles);
    //}
}

void cParticleEmitter_2::CreateBurstParticles(unsigned int numParticles, glm::vec3 position)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        unsigned int numParticlesToCreate = numParticles;
        unsigned int particleIndex = 0;
        //SetNumberOfParticlesPerFrame(100);
        do
        {
            cParticle& testParticle = this->m_pParticles[particleIndex];
            if (testParticle.timeTillDeath <= 0.0f)
            {
                testParticle.position = position;
                glm::vec3 randomDir = this->getRandomDirectionOnSphere();
                float randomSpeed = this->getRandomFloat(5.0f, 10.0f);
                testParticle.velocity = randomDir * randomSpeed;
                testParticle.accel = glm::vec3(0.0f, -9.8f, 0.0f);
                testParticle.timeTillDeath = this->getRandomFloat(0.5f, 1.5f);
                numParticlesToCreate--;
            }
            particleIndex++;
        } while ((numParticlesToCreate > 0) && (particleIndex < this->m_MaxNumberOfParticles));
       //m_UnlockParticleArray();
    }
    else {
        unsigned int numParticlesToCreate = numParticles;
        unsigned int particleIndex = 0;

        do
        {
            cParticle& testParticle = this->m_pParticles[particleIndex];
            if (testParticle.timeTillDeath <= 0.0f)
            {
                testParticle.position = position;
                glm::vec3 randomDir = this->getRandomDirectionOnSphere();
                float randomSpeed = this->getRandomFloat(5.0f, 10.0f);
                testParticle.velocity = randomDir * randomSpeed;
                testParticle.accel = glm::vec3(0.0f, -9.8f, 0.0f);
                testParticle.timeTillDeath = this->getRandomFloat(0.1f, 0.2f);
                numParticlesToCreate--;
            }
            particleIndex++;
        } while ((numParticlesToCreate > 0) && (particleIndex < this->m_MaxNumberOfParticles));
    }
}

void cParticleEmitter_2::GetLiveParticles(std::vector<cParticle*>& vec_Particles)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        vec_Particles.clear();
        for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
        {
            cParticle* pTestParticle = &(this->m_pParticles[index]);
            if (pTestParticle->timeTillDeath > 0.0f)
            {
                vec_Particles.push_back(pTestParticle);
            }
        }
        //m_UnlockParticleArray();
    }
    else {
        vec_Particles.clear();
        for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
        {
            cParticle* pTestParticle = &(this->m_pParticles[index]);
            if (pTestParticle->timeTillDeath > 0.0f)
            {
                vec_Particles.push_back(pTestParticle);
            }
        }
    }
}

unsigned int cParticleEmitter_2::GetNumberOfLiveParticles(void)
{
    unsigned int numParticles = 0;

    if (bUseThreading) {
        //m_LockParticleArray();
        for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
        {
            cParticle* pTestParticle = &(this->m_pParticles[index]);
            if (pTestParticle->timeTillDeath > 0.0f)
            {
                numParticles++;
            }
        }
        //m_UnlockParticleArray();
    }
    else {
        for (unsigned int index = 0; index != this->m_MaxNumberOfParticles; index++)
        {
            cParticle* pTestParticle = &(this->m_pParticles[index]);
            if (pTestParticle->timeTillDeath > 0.0f)
            {
                numParticles++;
            }
        }
    }
    return numParticles;
}

void cParticleEmitter_2::SetMaximumNumberOfParticles(unsigned int maxParticles)
{
    this->m_MaxNumberOfParticles = maxParticles;
    this->m_ResizeContainer();
}

void cParticleEmitter_2::SetSourceLocation(glm::vec3 positionXYZ)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_PositionXYZ = positionXYZ;
        //m_UnlockParticleArray();
    }
    else {
        this->m_PositionXYZ = positionXYZ;
    }
}

void cParticleEmitter_2::SetInitalVelocity(glm::vec3 minXYZ, glm::vec3 maxXYZ)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_minVelocityXYZ = minXYZ;
        this->m_maxVelocityXYZ = maxXYZ;
        //m_UnlockParticleArray();
    }
    else {
        this->m_minVelocityXYZ = minXYZ;
        this->m_maxVelocityXYZ = maxXYZ;
    }
}

void cParticleEmitter_2::SetNumberOfParticlesPerFrame(unsigned int numParticlesPerUpdate)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_numParticlesPerUpdate = numParticlesPerUpdate;
        //m_UnlockParticleArray();
    }
    else {
        this->m_numParticlesPerUpdate = numParticlesPerUpdate;
    }
}

void cParticleEmitter_2::SetLocationXYZ(glm::vec3 newLocation)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_PositionXYZ = newLocation;
        //m_UnlockParticleArray();
    }
    else {
        this->m_PositionXYZ = newLocation;
    }
}

glm::vec3 cParticleEmitter_2::GetLocation(void)
{
    glm::vec3 location = glm::vec3(0.0f);
    if (bUseThreading) {
        //m_LockParticleArray();
        location = this->m_PositionXYZ;
        //m_UnlockParticleArray();
    }
    else location = this->m_PositionXYZ;
    return location;
}

void cParticleEmitter_2::TurnOn(void)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_bIsEnabled = true;
        //m_UnlockParticleArray();
    }
    else this->m_bIsEnabled = true;
}

void cParticleEmitter_2::TurnOff(void)
{
    if (bUseThreading) {
        //m_LockParticleArray();
        this->m_bIsEnabled = false;
        //m_UnlockParticleArray();
    }
    else this->m_bIsEnabled = false;
}

bool cParticleEmitter_2::bIsOn(void)
{
    bool isOn = false;
    if (bUseThreading) {
        //m_LockParticleArray();
        isOn = this->m_bIsEnabled;
        //m_UnlockParticleArray();
    }
    else isOn = this->m_bIsEnabled;
    return isOn;
}