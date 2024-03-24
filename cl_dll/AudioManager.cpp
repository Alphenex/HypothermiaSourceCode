#include "AudioManager.h"

#include <unordered_map>
#include <string>

#include <SDL2/SDL_messagebox.h>
#include <RAUDIO/raudio.h>

#include "extdll.h"
#include "enginecallback.h"

inline bool fileExists(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

static std::unordered_map<std::string, AudioData> m_AudioStrMap;

bool HT::InitAudioLib()
{
	InitAudioDevice();

	if (!IsAudioDeviceReady())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Failed to load Audio device, please contact mod developers for a fix?", nullptr);
		return false;
	}

	return true; // Didn't fail
}

void HT::FreeAudioLib()
{
	for (const auto& kv : m_AudioStrMap)
	{
		const AudioData& audio = kv.second;
		if (!audio.data) continue;

		if (kv.second.type == AUDIOTYPE::SOUND)
			UnloadSound(*(Sound*)audio.data);
		else
			UnloadMusicStream(*(Music*)audio.data);

		delete audio.data;
	}

	CloseAudioDevice();
}

AudioData* HT::GetAudioData(const char* path)
{
	if (path == "")
		return nullptr;

	return &m_AudioStrMap[path];
}

void HT::LoadAudio(const char* path, AUDIOTYPE type)
{
	if ((int)type < 0 || (int)type > 1 || path == "")
		return;

	std::string modpath = "hypothermia/sound/" + std::string(path);
	std::string valvepath = "valve/sound/" + std::string(path);

	const char* finalpath = "";
	if (fileExists(modpath))
		finalpath = modpath.c_str();
	else if (fileExists(valvepath))
		finalpath = valvepath.c_str();
	else
		return;
	
	AudioData& audio = m_AudioStrMap[path];

	if (audio.data) // it already exists.. DUH
		return;

	if (type == AUDIOTYPE::SOUND)
	{
		Sound* sound = new Sound;
		*sound = LoadSound(finalpath);

		if (sound->frameCount <= 0)
		{
			delete sound;
			return;
		}

		audio.data = sound;
		audio.length = (float)sound->frameCount / sound->stream.sampleRate;
	}
	else
	{
		Music* music = new Music;
		*music = LoadMusicStream(finalpath);

		if (music->frameCount <= 0)
		{
			delete music;
			return;
		}
		
		audio.data = music;
		audio.length = (float)music->frameCount / music->stream.sampleRate;
	}
	
	audio.path = path;
	audio.type = type;
}

void HT::PlayAudio(const char* path, float volume)
{
	AudioData& audio = m_AudioStrMap[path];

	if (audio.data == nullptr) // If it doesn't exist then return
		return;

	if (audio.type == AUDIOTYPE::SOUND)
	{
		Sound sound = *(Sound*)audio.data;
		SetSoundVolume(sound, volume);
		PlaySound(sound);
	}
	else
	{
		Music music = *(Music*)audio.data;
		SetMusicVolume(music, volume);
		PlayMusicStream(music);
	}
}

void HT::PlayAudio(const char* path, float volume, AUDIOTYPE type)
{
	if ((int)type < 0 || (int)type > 1 || path == "")
		return;

	AudioData& audio = m_AudioStrMap[path];
	if (audio.data == nullptr) // If it doesn't exist then load
	{
		HT::LoadAudio(path, type);
	}

	if (type == AUDIOTYPE::SOUND)
	{
		Sound sound = *(Sound*)audio.data;
		SetSoundVolume(sound, volume);
		PlaySound(sound);
	}
	else
	{
		Music music = *(Music*)audio.data;
		SetMusicVolume(music, volume);
		PlayMusicStream(music);
	}
}

void HT::StopAudio(const char* path)
{
	AudioData* audio = GetAudioData(path);
	if (!audio)
		return;

	if (audio->type == AUDIOTYPE::SOUND)
	{
		Sound& sound = *(Sound*)audio->data;
		StopSound(sound);
	}
	else
	{
		Music& music = *(Music*)audio->data;
		StopMusicStream(music);
	}
}

void HT::UpdateAudios()
{
	float vol = CVAR_GET_FLOAT("volume");
	SetMasterVolume(vol);

	for (const auto& kv : m_AudioStrMap)
	{
		const AudioData& audio = kv.second;
		if (!audio.data || audio.type != AUDIOTYPE::MUSICSTREAM)
			continue;

		Music music = *(Music*)audio.data;
		UpdateMusicStream(music);
	}
}

void HT::StopAudios()
{
	for (const auto& kv : m_AudioStrMap)
	{
		if (!kv.second.data)
			continue;

		HT::StopAudio(kv.first.c_str());
	}
}
