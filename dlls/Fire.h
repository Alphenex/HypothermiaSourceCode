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
	static CFire* BurnEntity(CBaseAnimating* pEnt, float flLifetime, float flDamage = 5.0f);
	static CFire* BurnEntityUntilDead(CBaseAnimating* pEnt, float flDamage = 5.0f);

private:
	static void HurtEntity(CFire* self, CBaseEntity* pEnt);

private:
	void CreateSprite();

private:
	static TYPEDESCRIPTION m_SaveData[];

	CSprite* m_pSprite;
	float m_flLifeTime;
	bool m_bActive;

	CBaseAnimating* m_pAttached;
	bool m_bKillAttached;

	bool m_bSpawnedIn;
};