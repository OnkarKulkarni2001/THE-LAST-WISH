#include "Animator.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

bool bUseThreading = false;

Animator::Animator(Animation * animation)
{

	deltaTime = 0.0f;
	currentTime = 0.0f;
	currentAnimation = animation;

	finalBoneMatrices.reserve(100);

	for (int i = 0; i < 100; ++i)
	{
		finalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	boneMatrixCriticalSection = new CRITICAL_SECTION();
	InitializeCriticalSection(boneMatrixCriticalSection);

}

Animator::Animator() 
{
	boneMatrixCriticalSection = new CRITICAL_SECTION();
	InitializeCriticalSection(boneMatrixCriticalSection);
}

Animator::~Animator() {
	if (boneMatrixCriticalSection)
	{
		DeleteCriticalSection(boneMatrixCriticalSection);
		delete boneMatrixCriticalSection;
		boneMatrixCriticalSection = nullptr;
	}
}

void Animator::UpdateAnimation(float dt)
{

	deltaTime = dt;

	if (!currentAnimation) return;

	currentTime += currentAnimation->GetTicksPerSecond() * dt * animationSpeed;

	if (currentAnimation->GetLooping(currentAnimation))
	{
		//timeBeforeThreading = 0.0f;
		currentTime = fmod(currentTime, currentAnimation->GetDuration());
		/*CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));*/
		//timeBeforeThreading += dt;
		//std::cout << "timeAfterThreading: " << timeBeforeThreading << std::endl;
	}
	else // Check if animation exists for the else case
	{
		//timeBeforeThreading = 0;
		// Clamp the time to not exceed duration for non-looping animations
		if (currentTime >= currentAnimation->GetDuration())
		{
			currentTime = currentAnimation->GetDuration();  // Stop at the end
		}
		//CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));
		//timeBeforeThreading += dt;
		//std::cout << "timeAfterThreadingNotLooped: " << timeBeforeThreading << std::endl;
	}
	if (bUseThreading) {
		//timeBeforeThreading = 0.0f;

		StartBoneTransform();

	}
	else {
		//timeBeforeThreading = 0.0f;

		CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));

	}
}

DWORD WINAPI BoneTransformThread(LPVOID lpParam)
{
	ThreadData* data = static_cast<ThreadData*>(lpParam);
	data->animator->CalculateBoneTransform(data->node, data->parentTransform);
	delete data;
	return 0;
}

void Animator::StartBoneTransform()
{
	const AssimpNodeData* rootNode = &currentAnimation->GetRootNode();
	glm::mat4 identity = glm::mat4(1.0f);

	std::string rootName = rootNode->name;
	std::map<std::string, BoneInfo> boneInfoMap = currentAnimation->getBoneInfoMap();

	glm::mat4 parentTransform = identity;


	if (boneInfoMap.find(rootName) != boneInfoMap.end())
	{
		int index = boneInfoMap[rootName].id;
		glm::mat4 offset = boneInfoMap[rootName].offset;

		EnterCriticalSection(boneMatrixCriticalSection);
		finalBoneMatrices[index] = identity * offset;
		LeaveCriticalSection(boneMatrixCriticalSection);
		
		parentTransform = identity * rootNode->transformation;
	}
	else
	{
		// Even if not a bone, the root node's transformation should be applied
		parentTransform = identity * rootNode->transformation;
	}

	std::vector<HANDLE> threadHandles;

	for (int i = 0; i < rootNode->childrenCount; ++i)
	{
		ThreadData* data = new ThreadData{ this, &rootNode->children[i], parentTransform };

		HANDLE hThread = CreateThread(
			NULL, 0, BoneTransformThread, data, 0, NULL);

		if (hThread)
			threadHandles.push_back(hThread);
		else
			delete data;
	}

	if (!threadHandles.empty()) {
		WaitForMultipleObjects((DWORD)threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

		for (HANDLE h : threadHandles)
			CloseHandle(h);
	}
}


void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* bone = currentAnimation->FindBone(nodeName);

	if (bone)
	{
		if (currentAnimation->GetLooping(currentAnimation)) 
		{
			bone->Update(currentTime);
			nodeTransform = bone->GetLocalTransform();
		}
		else {
			bone->Update(currentTime - deltaTime);
		}
		nodeTransform = bone->GetLocalTransform();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	std::map<std::string, BoneInfo> boneInfoMap = currentAnimation->getBoneInfoMap();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;

		//EnterCriticalSection(boneMatrixCriticalSection);
		finalBoneMatrices[index] = globalTransformation * offset;
		//LeaveCriticalSection(boneMatrixCriticalSection);
	}

	for (int i = 0; i < node->childrenCount; ++i)
	{
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
{
	return finalBoneMatrices;
}

void Animator::SetCurrentAnimation(Animation* currentAnimation)
{
	this->currentAnimation = currentAnimation;
}

bool Animator::IsAnimationCompleted(Animation* animation)
{
	if (!animation) return true;  // If no animation, consider it "completed"

	// If animation is looping, it's never truly completed
	if (animation->GetLooping(animation)) {
		return false;
	}

	// For non-looping animations, check if currentTime has reached or exceeded duration
	return currentTime >= animation->GetDuration();
}

float Animator::GetDeltaTime()
{
	return deltaTime;
}

float Animator::GetAnimationCurrentTime()
{
	return currentTime;
}

void Animator::ResetCurrentTime()
{
	this->currentTime = 0.0f;
}

void Animator::SpeedUpAnimation(float value)
{
	bAmplifySpeed = true;
	animationSpeed = value;
}

float Animator::GetCurrentAnimationSpeed()
{
	return animationSpeed;
}

Animation* Animator::GetCurrentAnimation()
{
	return currentAnimation;
}

void Animator::CalculateBoneTransformMultiThreaded(const AssimpNodeData* node, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* bone = currentAnimation->FindBone(nodeName);

	if (bone)
	{
		if (currentAnimation->GetLooping(currentAnimation))
		{
			bone->Update(currentTime);
			nodeTransform = bone->GetLocalTransform();
		}
		else
		{
			bone->Update(currentTime - deltaTime);
		}
		nodeTransform = bone->GetLocalTransform();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	std::map<std::string, BoneInfo> boneInfoMap = currentAnimation->getBoneInfoMap();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;

		EnterCriticalSection(this->boneMatrixCriticalSection);
		finalBoneMatrices[index] = globalTransformation * offset;
		LeaveCriticalSection(this->boneMatrixCriticalSection);
	}

	// ====== Multithreading for children ======
	std::vector<HANDLE> threadHandles;

	for (int i = 0; i < node->childrenCount; ++i)
	{
		ThreadData* threadData = new ThreadData();
		threadData->animator = this;
		threadData->node = &node->children[i];
		threadData->parentTransform = globalTransformation;

		HANDLE hThread = CreateThread(
			NULL,                   // default security attributes
			0,                      // default stack size
			ThreadFunction,         // thread function
			threadData,             // thread function argument
			0,                      // default creation flags
			NULL                    // receive thread identifier (not used)
		);

		if (hThread != NULL)
		{
			threadHandles.push_back(hThread);
		}
	}

	// Wait for all threads to finish
	WaitForMultipleObjects((DWORD)threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

	// Close all thread handles
	for (HANDLE hThread : threadHandles)
	{
		CloseHandle(hThread);
	}
}

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	ThreadData* data = (ThreadData*)lpParam;

	data->animator->CalculateBoneTransform(data->node, data->parentTransform);

	delete data; // Important to free memory!
	return 0;
}

