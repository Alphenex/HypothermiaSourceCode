#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "UserMessages.h"
#include <algorithm>

LINK_WEAPON_TO_CLASS(weapon_m249, CM249);

void CM249::Spawn()
{
	m_iId = WEAPON_M249;
	m_iDefaultAmmo = M249_DEFAULT_GIVE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_saw.mdl");
	m_bAlternatingEject = false;

	FallInit();
}

void CM249::Precache()
{
	PRECACHE_MODEL("models/v_saw.mdl");
	PRECACHE_MODEL("models/w_saw.mdl");
	PRECACHE_MODEL("models/p_saw.mdl");

	m_iShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iLink = PRECACHE_MODEL("models/saw_link.mdl");
	m_iSmoke = PRECACHE_MODEL("sprites/wep_smoke_01.spr");
	m_iFire = PRECACHE_MODEL("sprites/xfire.spr");

	PRECACHE_SOUND("weapons/saw_reload.wav");
	PRECACHE_SOUND("weapons/saw_reload2.wav");
	PRECACHE_SOUND("weapons/saw_fire1.wav");

	m_usFireM249 = PRECACHE_EVENT(1, "events/m249.sc");
}

bool CM249::Deploy()
{
	Vector vecSpread;

	if ((m_pPlayer->pev->button & IN_DUCK) != 0)
		vecSpread = VECTOR_CONE_2DEGREES;
	else if ((m_pPlayer->pev->button & (IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK)) != 0)
		vecSpread = VECTOR_CONE_10DEGREES;
	else
		vecSpread = VECTOR_CONE_4DEGREES;

	pev->fuser4 = vecSpread.x;
	return DefaultDeploy("models/v_saw.mdl", "models/p_saw.mdl", M249_DRAW, "m249", pev->body);
}

void CM249::Holster()
{
	SetThink(nullptr);

	SendWeaponAnim(M249_HOLSTER);

	m_bReloading = false;

	m_fInReload = false;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);
}

void CM249::WeaponIdle()
{
	ResetEmptySound();

	// Update auto-aim
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_bReloading && gpGlobals->time >= m_flReloadStart + 1.33)
	{
		m_bReloading = false;

		pev->body = 0;
		SendWeaponAnim(M249_RELOAD_END, pev->body);
	}

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		const float flNextIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		int iAnim;

		if (flNextIdle <= 0.95)
		{
			iAnim = M249_SLOWIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		else
		{
			iAnim = M249_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.16;
		}
		SendWeaponAnim(iAnim, pev->body);
	}

	pev->fov -= 0.5f * gpGlobals->frametime;
	pev->fov = std::clamp<float>(pev->fov, 0.0f, 0.80f);
	
	Vector vecSpread;

	if ((m_pPlayer->pev->button & IN_DUCK) != 0)
		vecSpread = VECTOR_CONE_2DEGREES;
	else if ((m_pPlayer->pev->button & (IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK)) != 0)
		vecSpread = VECTOR_CONE_10DEGREES + VECTOR_CONE_5DEGREES * ((m_pPlayer->pev->button & IN_SCORE) != 0);
	else
		vecSpread = VECTOR_CONE_4DEGREES;

	pev->fuser4 = vecSpread.x;
}

void CM249::PrimaryAttack()
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fInReload)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		}
		return;
	}

	--m_iClip;

	pev->body = RecalculateBody(m_iClip);

	m_bAlternatingEject = !m_bAlternatingEject;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	m_flNextAnimTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();

	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);


	pev->fov += 1.5f *gpGlobals->frametime;
	pev->fov = std::clamp<float>(pev->fov, 0.0f, 0.75f);
	Vector fovvec = Vector(pev->fov, pev->fov, pev->fov);

	Vector vecSpread = Vector(pev->fuser4, pev->fuser4, pev->fuser4);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(
		1,
		vecSrc, vecAiming, vecSpread + fovvec,
		8192.0, BULLET_PLAYER_MP5, 2, 0,
		m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	int fovspread = 0;
	memcpy(&fovspread, &pev->fov, sizeof(float));
	PLAYBACK_EVENT_FULL(
		flags, m_pPlayer->edict(), m_usFireM249, 0,
		g_vecZero, g_vecZero,
		vecDir.x, vecDir.y,
		pev->body, fovspread,
		m_bAlternatingEject ? 1 : 0, 0);


	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.067;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2;

#ifndef CLIENT_DLL
	m_pPlayer->pev->punchangle.x = RANDOM_FLOAT(-2, 2);

	m_pPlayer->pev->punchangle.y = RANDOM_FLOAT(-1, 1);
#endif
}

void CM249::Reload()
{
	if (DefaultReload(M249_MAX_CLIP, M249_RELOAD_START, 1, pev->body))
	{
		m_bReloading = true;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.78;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.78;

		m_flReloadStart = gpGlobals->time;
	}
}

int CM249::RecalculateBody(int iClip)
{
	if (iClip == 0)
	{
		return 8;
	}
	else if (iClip >= 0 && iClip <= 7)
	{
		return 9 - iClip;
	}
	else
	{
		return 0;
	}
}

bool CM249::GetItemInfo(ItemInfo* p)
{
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = M249_MAX_CARRY;
	p->pszName = STRING(pev->classname);
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = M249_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M249;
	p->iWeight = M249_WEIGHT;

	return true;
}

void CM249::GetWeaponData(weapon_data_t& data)
{
	data.iuser1 = pev->body;
}

void CM249::SetWeaponData(const weapon_data_t& data)
{
	pev->body = data.iuser1;
}