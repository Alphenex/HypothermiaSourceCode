#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"

enum BurnFlag
{
	FBURN_LIFETIME = (1 << 0),
	FBURN_UNTILDEAD = (1 << 1),
	FBURN_UNTILDEADWLIFETIME = (1 << 2), // Basically burns until the attached is dead and continues after lifetime
};

class CFire : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Precache() override;

	void Touch(CBaseEntity* pOther) override; // Probably call CTriggerHurt::HurtTouch

	void Think() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;
 
	void StartFire();
	void KillFire();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static CFire* CreateFire();

	static CFire* SpawnFireAtPosition(Vector vPos, float flLifetime, float flDamage = 5.0f, bool bGravity = false);
	static CFire* BurnEntity(CBaseEntity* pEnt, CBaseEntity* pAttacker = nullptr, float flDamage = 5.0f, float flLifetime = -1.0f, bool charattached = true);
	static CFire* BurnEntityUntilDead(CBaseEntity* pEnt, CBaseEntity* pAttacker = nullptr, float flDamage = 5.0f, bool charattached = true);
	static CFire* BurnEntityUntilDeadWithLifetime(CBaseEntity* pEnt, CBaseEntity* pAttacker = nullptr, float flDamage = 5.0f, float flLifetime = -1.0f, bool charattached = true);

private:
	static void HurtEntity(CFire* self, CBaseEntity* pEnt);

private:
	void CreateSprite();

private:
	static TYPEDESCRIPTION m_SaveData[];

	CSprite* m_pFireSprite;
	CSprite* m_pFireGlowSprite;
	short m_iSmokeSpriteID;
	float m_flLifeTime;
	bool m_bActive;

	EHANDLE m_pAttachedEdict;
	BurnFlag m_bBurnFlag;
	CBaseEntity* m_pOwner;
	bool m_bSpawnedIn;
	bool m_bCharAttached;

	float m_fSmokeCreateTimer;
	float m_fFireSoundTimer;
	bool m_bGravity;
};