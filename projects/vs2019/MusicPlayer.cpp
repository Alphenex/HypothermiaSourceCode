#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client.h"
#include <string>

class CMusicPlayer : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(musicplayer, CMusicPlayer)
void CMusicPlayer::Spawn()
{
	if (pev->message == NULL)
	{
		ALERT(at_warning, "Music player is empty.\n");
		UTIL_Remove(this);
		return;
	}

	const char* sndchar = STRING(pev->message);
	PrecacheAudio(sndchar, 1);

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
		UTIL_EmitAmbientSound(edict(), pev->origin, STRING(pev->message), 1.0f, ATTN_NONE, SND_SPAWNING, PITCH_NORM, AudioType::Music);

		if (pev->model != NULL)
		{
			hudtextparms_t param;
			param.fadeinTime = 0.075f;
			param.fadeoutTime = 2.5f;
			param.holdTime = 7.5f;
			param.channel = 1;
			param.r1 = 0;
			param.g1 = 145;
			param.b1 = 255;
			param.a1 = 255;
			param.effect = 2;
			param.fxTime = 2.5f;
			param.x = 0;
			param.y = 0;

			std::string msg = "Now Playing: " + std::string(STRING(pev->model));
			UTIL_HudMessageAll(param, msg.c_str());
		}

		pev->iuser1 = 0;
	}	
}
