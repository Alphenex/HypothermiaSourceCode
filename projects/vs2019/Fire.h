#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"

class CFire : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd);
	void Precache() override;

	void Touch(CBaseEntity* pOther) override; // Probably call CTriggerHurt::HurtTouch
	void Think() override;

	void StartFire();
	void KillFire();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

private:
	static TYPEDESCRIPTION m_SaveData[];

	const char* m_SpritePath;
	CSprite* m_pSprite;
	float m_flLifeTime;
	bool m_bActive;
};