#include "Fire.h"

LINK_ENTITY_TO_CLASS(env_fire, CFire)

TYPEDESCRIPTION CFire::m_SaveData[] =
{
		DEFINE_FIELD(CFire, m_pSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_flLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_bActive, FIELD_BOOLEAN)
};
IMPLEMENT_SAVERESTORE(CFire, CBaseEntity)

#define FIRE_SPRITE "sprites/prioryflame.spr"

bool CFire::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flLifeTime"))
	{
		m_flLifeTime = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CFire::Precache()
{
	PRECACHE_MODEL(FIRE_SPRITE);
}

void CFire::Spawn()
{
	m_pSprite = nullptr; // Just in case, I don't want to deal with a headache

	Precache();

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;	

	UTIL_SetSize(pev, Vector(-48, -48, -16), Vector(48, 48, 48));	

	StartFire();

	if (m_flLifeTime > 0.1f)
		pev->ltime = gpGlobals->time + m_flLifeTime;
	else
		pev->ltime = -1; // Fire will basically never gonna die. 

	pev->nextthink = gpGlobals->time;
}

static float dps = 0.5f;
void CFire::Touch(CBaseEntity* pOther)
{
	if (m_bActive)
	{
		if (pOther->pev->takedamage == 0) return;
		if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished) return;

		float fldmg = pev->dmg * dps;

		pOther->TakeDamage(pev, pev, fldmg, DMG_BURN);

		pev->pain_finished = gpGlobals->time;
		pev->dmgtime = gpGlobals->time + dps; // half second delay until this trigger can hurt toucher again
	}
}

void CFire::Think()
{
	if (m_pSprite && m_bActive)
	{
		Vector norg = pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5);
		m_pSprite->pev->origin = norg;
		m_pSprite->AnimateThink();
	}

	if (gpGlobals->time > pev->ltime && pev->ltime != -1)
	{
		KillFire(); // RIP
		return;
	}

	pev->nextthink = gpGlobals->time + 0.05f;
}

void CFire::StartFire()
{
	if (m_bActive) // If the fire is already active then don't do anything!
		return;

	m_bActive = true;

	if (m_pSprite == nullptr) // if Sprite doesn't exist then create it, we shouldn't even ever hit this?
		CreateSprite();
}

void CFire::KillFire()
{
	UTIL_Remove(m_pSprite);
	UTIL_Remove(this);
}

void CFire::CreateSprite()
{
	m_pSprite = CSprite::SpriteCreate(FIRE_SPRITE, pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), true);
	m_pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone);
	m_pSprite->SetScale(1.0f);
	m_pSprite->pev->framerate = 10.0f;
}
