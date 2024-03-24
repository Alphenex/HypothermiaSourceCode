#pragma once

enum class AUDIOTYPE
{
	SOUND,
	MUSICSTREAM
};

struct AudioData
{
	float length;
	void* data;
	const char* path;
	AUDIOTYPE type;
};

namespace HT
{
	bool InitAudioLib();
	void FreeAudioLib();
	
	AudioData* GetAudioData(const char* path);
	void LoadAudio(const char* path, AUDIOTYPE type);
	void PlayAudio(const char* path, float volume);
	void PlayAudio(const char* path, float volume, AUDIOTYPE type); // Hotloads
	void StopAudio(const char* path);

	void UpdateAudios();
	void StopAudios();
}