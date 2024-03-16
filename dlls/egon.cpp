/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "customentity.h"
#include "gamerules.h"
#include "UserMessages.h"
#include "Fire.h"

#ifdef CLIENT_DLL
#include "hud.h"
#include "com_weapons.h"
#endif

constexpr float EGON_SWITCH_NARROW_TIME = 0.75f; // Time it takes to switch fire modes
constexpr float EGON_SWITCH_WIDE_TIME = 1.5f;

LINK_WEAPON_TO_CLASS(weapon_egon, CEgon);

void CEgon::Spawn()
{
	Precache();
	m_iId = WEAPON_EGON;
	SET_MODEL(ENT(pev), "models/w_egon.mdl");

	m_iDefaultAmmo = EGON_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}


void CEgon::Precache()
{
	PRECACHE_MODEL("models/w_egon.mdl");
	PRECACHE_MODEL("models/v_egon.mdl");
	PRECACHE_MODEL("models/p_egon.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND(EGON_SOUND_OFF);
	PRECACHE_SOUND(EGON_SOUND_RUN);
	PRECACHE_SOUND(EGON_SOUND_STARTUP);

	PRECACHE_MODEL(EGON_BEAM_SPRITE);
	PRECACHE_MODEL(EGON_FLARE_SPRITE);

	PRECACHE_SOUND("weapons/357_cock1.wav");
}


bool CEgon::Deploy()
{
	return DefaultDeploy("models/v_egon.mdl", "models/p_egon.mdl", EGON_DRAW, "egon");
}

void CEgon::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(EGON_HOLSTER);

	EndAttack();
}

bool CEgon::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_EGON;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

constexpr float EGON_PULSE_INTERVAL = 0.1f;
constexpr float EGON_DISCHARGE_INTERVAL = 0.1f;
constexpr float FLAMEGUN_MAXDIST = 175.0f;

float CEgon::GetPulseInterval()
{
	return EGON_PULSE_INTERVAL;
}

float CEgon::GetDischargeInterval()
{
	return EGON_DISCHARGE_INTERVAL;
}

bool CEgon::HasAmmo()
{
	if (m_pPlayer->ammo_uranium <= 0)
		return false;

	return true;
}

void CEgon::UseAmmo(int count)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= count)
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= count;
	else
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
}

void CEgon::Attack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

	if (!HasAmmo())
	{
		PlayEmptySound();
		return;
	}

	Fire(vecSrc, vecAiming);
	m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;
}

void CEgon::PrimaryAttack()
{
	Attack();
}

void CEgon::Fire(const Vector& vecOrigSrc, const Vector& vecDir)
{
	Vector vecDest = vecOrigSrc + vecDir * 2048;
	edict_t* pentIgnore;
	TraceResult tr;

	pentIgnore = m_pPlayer->edict();
	Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

	UTIL_TraceLine(vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);

	if (0 != tr.fAllSolid)
		return;

#ifndef CLIENT_DLL
	if (gpGlobals->time >= m_flAmmoUseTime)
	{
		UseAmmo(1);
		m_flAmmoUseTime = gpGlobals->time + 0.2f;
	}

	CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

	float hitlen = (tr.vecEndPos - vecOrigSrc).Length();
	if (pEntity == NULL || hitlen > FLAMEGUN_MAXDIST)
		return;

	CFire* fire = CFire::BurnEntityUntilDead(pEntity, m_pPlayer);
#endif
}

void CEgon::UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend)
{

}

void CEgon::CreateEffect()
{

}


void CEgon::DestroyEffect()
{

}

void CEgon::WeaponIdle()
{
	if ((m_pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 && (m_pPlayer->pev->button & IN_ATTACK) != 0)
	{
		return;
	}

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	float flRand = RANDOM_FLOAT(0, 1);

	if (flRand <= 0.5)
	{
		iAnim = EGON_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
	else
	{
		iAnim = EGON_FIDGET1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	}

	SendWeaponAnim(iAnim);
}

void CEgon::EndAttack()
{

}

class CEgonAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_egonclip, CEgonAmmo);
