#pragma once

#include <../FMOD_API/core/inc/fmod.hpp>
#include <string>
#include <map>

class cSoundManager
{
public:
	cSoundManager();
	~cSoundManager();

	bool Init();
	void Update();
	void Shutdown();

	void LoadSound(const std::string& name, const std::string& filePath, bool is3D = false, bool isLooping = false, bool isStreaming = false);
	void PlaySoundEffect(const std::string& name, float volume = 1.0f, bool paused = false);
	void Play3DSound(const std::string& name, const FMOD_VECTOR& position, float volume = 1.0f, bool paused = false);
	void SetListenerPosition(const FMOD_VECTOR& pos, const FMOD_VECTOR& forward, const FMOD_VECTOR& up);

private:
	FMOD::System* pSystem = nullptr;
	std::map<std::string, FMOD::Sound*> soundMap;
	std::map<std::string, FMOD::Channel*> channelMap;
};
