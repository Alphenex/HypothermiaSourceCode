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
#include "effects.h"

#ifdef CLIENT_DLL
#include "hud.h"
#include "com_weapons.h"
#endif

#define EGON_FIRE_SOUND_LENGTH 5.0f

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

	m_usFlameID = PRECACHE_MODEL(EGON_FLAME_SPRITE);
	m_usSmokeID = PRECACHE_MODEL(EGON_SMOKE_SPRITE);

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usEgonFire = PRECACHE_EVENT(1, "events/egon_fire.sc");
	m_usEgonStop = PRECACHE_EVENT(1, "events/egon_stop.sc");
}

bool CEgon::Deploy()
{
	m_deployed = false;
	m_fireState = FIRE_OFF;
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

#define EGON_PULSE_INTERVAL 0.1
#define EGON_DISCHARGE_INTERVAL 0.1

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

		if (m_fireState != FIRE_OFF)
		{
			EndAttack();
		}
		else
		{
			PlayEmptySound();
		}
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	switch (m_fireState)
	{
	case FIRE_OFF:
	{
		if (!HasAmmo())
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2f;
			STOP_SOUND(m_pPlayer->edict(), CHAN_VOICE, EGON_SOUND_RUN);
			PlayEmptySound();
			return;
		}
		
		m_flAmmoUseTime = gpGlobals->time + 1.0f; // start using ammo about a second later.
		
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usEgonFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, 1, 0);
		
		m_shakeTime = 0;
		
		m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		pev->fuser1 = UTIL_WeaponTimeBase() + 2;
		
		pev->dmgtime = gpGlobals->time + GetPulseInterval();
		m_fireState = FIRE_CHARGE;
		m_flFireSoundLoopTimeOffset = gpGlobals->time + 2.4f;
	}
	break;

	case FIRE_CHARGE:
	{
		Fire(vecSrc, vecAiming);
		m_pPlayer->m_iWeaponVolume = EGON_PRIMARY_VOLUME;

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usEgonFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0);

		if (gpGlobals->time > m_flFireSoundLoopTimer && gpGlobals->time > m_flFireSoundLoopTimeOffset)
		{
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, EGON_SOUND_RUN, 0.99f, ATTN_NORM, 0, PITCH_NORM);
			m_flFireSoundLoopTimer = gpGlobals->time + EGON_FIRE_SOUND_LENGTH;
		}

		if (!HasAmmo())
		{
			EndAttack();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
		}
	}
	break;
	}
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.02f;
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
		m_flAmmoUseTime = gpGlobals->time + 0.20f;
	}

	CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
	float len = (tr.vecEndPos - vecOrigSrc).Length();

	if (pev->dmgtime < gpGlobals->time)
	{
		if (!m_pPlayer->IsAlive())
			return;

		pev->dmgtime = gpGlobals->time + GetDischargeInterval();

		if (gpGlobals->time > m_shakeTime && gpGlobals->time > (m_flFireSoundLoopTimeOffset - 1.4f))
		{
			UTIL_ScreenShake(vecOrigSrc, 2.0f, 150.0f, 1.0f, 250.0f);

			Vector attPos;
			Vector attAng;
			attPos = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 48.0f + gpGlobals->v_right * 12.0f + gpGlobals->v_up * -12.0f;
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(attPos.x); // origin
			WRITE_COORD(attPos.y);
			WRITE_COORD(attPos.z);
			WRITE_BYTE(24);	 // radius
			WRITE_BYTE(255); // R
			WRITE_BYTE(150); // G
			WRITE_BYTE(0);	 // B
			WRITE_BYTE(1);	 // life * 10
			WRITE_BYTE(0);	 // decay
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_BUBBLETRAIL);
			WRITE_COORD(attPos.x - 2.5f); // min
			WRITE_COORD(attPos.y - 2.5f);
			WRITE_COORD(attPos.z - 2.5f);
			WRITE_COORD(attPos.x + 2.5f); // max
			WRITE_COORD(attPos.y + 2.5f);
			WRITE_COORD(attPos.z + 2.5f);
			WRITE_COORD(125.0f);		   // fly up to
			WRITE_SHORT(m_usSmokeID);		// sprite id
			WRITE_BYTE(1);				   // amount
			WRITE_COORD(0.05f);			   // speed
			MESSAGE_END();

			if (RANDOM_LONG(0, 100) < 20)
			{
				CFire* fire = CFire::SpawnFireAtPosition(attPos + Vector(0, 0, 16), RANDOM_FLOAT(0.0f, 10.0f), 0.0f, true);
				fire->pev->velocity = (tr.vecEndPos - vecOrigSrc).Normalize() * RANDOM_FLOAT(750.0f, 1000.0f);
				fire->pev->velocity = fire->pev->velocity +
									  gpGlobals->v_forward * RANDOM_FLOAT(-64.0f, 64.0f) +
									  gpGlobals->v_right * RANDOM_FLOAT(-64.0f, 64.0f) +
									  gpGlobals->v_up * RANDOM_FLOAT(-64.0f, 64.0f);
			}

			if (pEntity != NULL && len <= 512.0f)	
			{
				pEntity->TakeDamage(pev, m_pPlayer->pev, 3.5f, DMG_BURN);
				RadiusBurnUntilDead(tr.vecEndPos, m_pPlayer->pev, 5.0f, 256, CLASS_PLAYER, 1.0f);
			}
			
			m_shakeTime = gpGlobals->time + 0.02f;
		}
	}

#endif
}

void CEgon::WeaponIdle()
{
	if ((m_pPlayer->m_afButtonPressed & IN_ATTACK2) == 0 && (m_pPlayer->pev->button & IN_ATTACK) != 0)
	{
		return;
	}

	STOP_SOUND(m_pPlayer->edict(), CHAN_WEAPON, EGON_SOUND_STARTUP); // Just stop these sounds.
	STOP_SOUND(m_pPlayer->edict(), CHAN_VOICE, EGON_SOUND_RUN);

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fireState != FIRE_OFF)
		EndAttack();

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
	m_deployed = true;
}

void CEgon::EndAttack()
{
	bool bMakeNoise = false;

	if (m_fireState != FIRE_OFF) // Checking the button just in case!.
		bMakeNoise = true;

	if (gpGlobals->time > (m_flFireSoundLoopTimeOffset - 1.4f))
		PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_RELIABLE, m_pPlayer->edict(), m_usEgonStop, 0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0,
		static_cast<int>(bMakeNoise), 0, 0, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

	m_fireState = FIRE_OFF;
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