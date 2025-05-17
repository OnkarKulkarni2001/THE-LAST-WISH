#include "cSoundManager.h"
#include <iostream>

cSoundManager::cSoundManager() {}
cSoundManager::~cSoundManager()
{
	Shutdown();
}

bool cSoundManager::Init()
{
	if (FMOD::System_Create(&pSystem) != FMOD_OK)
	{
		std::cerr << "FMOD::System_Create failed!\n";
		return false;
	}

	if (pSystem->init(512, FMOD_INIT_3D_RIGHTHANDED, nullptr) != FMOD_OK)
	{
		std::cerr << "FMOD init failed!\n";
		return false;
	}

	return true;
}

void cSoundManager::Update()
{
	if (pSystem)
		pSystem->update();
}

void cSoundManager::LoadSound(const std::string& name, const std::string& filePath, bool is3D, bool isLooping, bool isStreaming)
{
	FMOD_MODE mode = FMOD_DEFAULT;
	if (is3D)        mode |= FMOD_3D;
	else             mode |= FMOD_2D;
	if (isLooping)   mode |= FMOD_LOOP_NORMAL;
	else             mode |= FMOD_LOOP_OFF;
	if (isStreaming) mode |= FMOD_CREATESTREAM;

	FMOD::Sound* sound = nullptr;
	if (pSystem->createSound(filePath.c_str(), mode, nullptr, &sound) == FMOD_OK)
	{
		soundMap[name] = sound;
	}
	else
	{
		std::cerr << "Failed to load sound: " << filePath << "\n";
	}
}

void cSoundManager::PlaySoundEffect(const std::string& name, float volume, bool paused)
{
	FMOD::Channel* channel = nullptr;

	auto it = soundMap.find(name);
	if (it != soundMap.end())
	{
		pSystem->playSound(it->second, nullptr, paused, &channel);
		if (channel)
		{
			channel->setVolume(volume);
			channelMap[name] = channel;
		}
	}
}

void cSoundManager::Play3DSound(const std::string& name, const FMOD_VECTOR& position, float volume, bool paused)
{
	FMOD::Channel* channel = nullptr;

	auto it = soundMap.find(name);
	if (it != soundMap.end())
	{
		pSystem->playSound(it->second, nullptr, paused, &channel);
		if (channel)
		{
			channel->setVolume(volume);
			channel->set3DAttributes(&position, nullptr);
			channelMap[name] = channel;
		}
	}
}

void cSoundManager::SetListenerPosition(const FMOD_VECTOR& pos, const FMOD_VECTOR& forward, const FMOD_VECTOR& up)
{
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f }; // Optional velocity
	pSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
}

void cSoundManager::Shutdown()
{
	for (auto& it : soundMap)
	{
		if (it.second)
			it.second->release();
	}

	soundMap.clear();

	if (pSystem)
	{
		pSystem->close();
		pSystem->release();
		pSystem = nullptr;
	}
}
