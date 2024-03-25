#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client.h"

class CMusicPlayer : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(musicplayer, CMusicPlayer)

void CMusicPlayer::Spawn()
{
	if (pev->message == 0)
	{
		ALERT(at_warning, "Music player is empty.\n");
		UTIL_Remove(this);
		return;
	}

	const char* sndchar = STRING(pev->message);
	PrecacheAudio(sndchar, 1); // We don't need to precache it but precache it anyway lol

	pev->iuser1 = 1;
}

bool CMusicPlayer::KeyValue(KeyValueData* pkvd)
{
	return CBaseEntity::KeyValue(pkvd);
}

void CMusicPlayer::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->iuser1 == 1)
	{
		UTIL_EmitAmbientSound(edict(), pev->origin, STRING(pev->message), CVAR_GET_FLOAT("mp3volume") * 2.0f, ATTN_NONE, SND_SPAWNING, PITCH_NORM, AudioType::Music);
		pev->iuser1 = 0;
	}	
}
