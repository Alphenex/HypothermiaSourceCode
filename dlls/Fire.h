#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"

class CFire : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Precache() override;

	void Touch(CBaseEntity* pOther) override; // Probably call CTriggerHurt::HurtTouch

	void Think() override;
 
	void StartFire();
	void KillFire();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static CFire* CreateFire(Vector vPos, float flLifetime, float flDamage = 5.0f);
	static CFire* BurnEntity(CBaseEntity* pEnt, CBaseEntity* pAttacker = nullptr, float flLifetime = -1.0f, float flDamage = 5.0f);
	static CFire* BurnEntityUntilDead(CBaseEntity* pEnt, CBaseEntity* pAttacker = nullptr, float flDamage = 5.0f);

private:
	static void HurtEntity(CFire* self, CBaseEntity* pEnt);

private:
	void CreateSprite();

private:
	static TYPEDESCRIPTION m_SaveData[];

	CSprite* m_pSprite;
	float m_flLifeTime;
	bool m_bActive;

	edict_t* m_pAttachedEdict; // Edicts are generally safer, this is used so that once the Attached doesn't exist anymore we know about it.
	bool m_bBurnAttachedTillDead;
	CBaseEntity* m_pOwner;
	bool m_bSpawnedIn;

	float m_fFireSoundTimer;
};