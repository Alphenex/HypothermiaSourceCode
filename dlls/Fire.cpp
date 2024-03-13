#include "Fire.h"

LINK_ENTITY_TO_CLASS(env_fire, CFire)

TYPEDESCRIPTION CFire::m_SaveData[] =
{
		DEFINE_FIELD(CFire, m_pSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_pAttached, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_flLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_bActive, FIELD_BOOLEAN),
		DEFINE_FIELD(CFire, m_bKillAttached, FIELD_BOOLEAN)
};
IMPLEMENT_SAVERESTORE(CFire, CBaseEntity)


bool CFire::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flLifeTime"))
	{
		m_flLifeTime = atof(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

#define FIRE_SPRITE "sprites/prioryflame.spr"
#define FIRE_LOOPSOUND "ambience/burning1.wav"

void CFire::Precache()
{
	PRECACHE_MODEL(FIRE_SPRITE);
	PRECACHE_SOUND(FIRE_LOOPSOUND);
}

void CFire::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;	

	UTIL_SetSize(pev, Vector(-48, -48, -16), Vector(48, 48, 48));	

	StartFire();

	if (m_flLifeTime > 0.1f)
		pev->ltime = gpGlobals->time + m_flLifeTime;
	else
		pev->ltime = -1; // Fire will basically never gonna die. 

	if (m_pSprite)
	{
		m_pSprite->pev->frame = RANDOM_LONG(0, m_pSprite->Frames() - 1);
	}

	pev->nextthink = gpGlobals->time;
}

static float dps = 0.5f;
void CFire::Touch(CBaseEntity* pOther)
{
	if (m_bActive)
	{
		HurtEntity(this, pOther);
	}
}

void CFire::Think()
{
	if ((gpGlobals->time > pev->ltime && pev->ltime != -1 && m_bActive) || (m_bKillAttached && !m_pAttached->IsAlive()))
	{
		KillFire();
		return;
	}

	if (m_bActive)
	{
		if (m_pAttached)
		{
			Vector attpelvispos;
			Vector attanglepos;

			// Nasty code, this shouldn't be done every Think BUT what we essentially do here is:
			// We check if Pelvis bone exists, if it doesn't check 0th Bone, if that also doesn't exist just get Center.
			m_pAttached->GetBonePosition(1, attpelvispos, attanglepos);
			if (attpelvispos == Vector(0, 0, 0))
				m_pAttached->GetBonePosition(0, attpelvispos, attanglepos);
			if (attpelvispos == Vector(0, 0, 0))
				attpelvispos = m_pAttached->Center();
			pev->origin = attpelvispos; // Move with the attached object!

			HurtEntity(this, m_pAttached);
		}

		if (m_pSprite)
		{
			Vector norg = pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z - 4) * 0.5);
			m_pSprite->pev->origin = norg;
			m_pSprite->AnimateThink();
		}

		if (m_bSpawnedIn)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pev->origin.x); // origin
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_BYTE(16);	 // radius
			WRITE_BYTE(255); // R
			WRITE_BYTE(80);	 // G
			WRITE_BYTE(0);	 // B
			WRITE_BYTE(10);	 // life * 10
			WRITE_BYTE(16);	 // decay
			MESSAGE_END();
		}
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

CFire* CFire::CreateFire(Vector vPos, float flLifetime, float flDamage)
{
	CFire* fire = GetClassPtr<CFire>(nullptr);
	fire->m_flLifeTime = flLifetime;
	fire->pev->origin = vPos;
	fire->pev->dmg = flDamage;
	fire->m_bSpawnedIn = true;
	fire->Spawn();

	return fire;
}

CFire* CFire::BurnEntity(CBaseAnimating* pEnt, float flLifetime, float flDamage)
{
	CFire* fire = GetClassPtr<CFire>(nullptr);
	fire->m_pAttached = pEnt;
	fire->pev->dmg = flDamage;
	fire->m_flLifeTime = flLifetime;
	fire->m_bSpawnedIn = true;
	fire->Spawn();

	pEnt->m_pFire = fire;

	return fire;
}

CFire* CFire::BurnEntityUntilDead(CBaseAnimating* pEnt, float flDamage)
{
	if (!pEnt->IsAlive())
		return nullptr;

	CFire* fire = GetClassPtr<CFire>(nullptr);
	fire->m_pAttached = pEnt;
	fire->pev->dmg = flDamage;
	fire->m_flLifeTime = -1;
	fire->m_bKillAttached = true;
	fire->m_bSpawnedIn = true;
	fire->Spawn();

	pEnt->m_pFire = fire;

	return fire;
}

void CFire::HurtEntity(CFire* self, CBaseEntity* pEnt)
{
	if (pEnt->pev->takedamage == 0)
		return;
	if (self->pev->dmgtime > gpGlobals->time && gpGlobals->time != self->pev->pain_finished)
		return;

	float fldmg = self->pev->dmg * dps;

	pEnt->TakeDamage(self->pev, self->pev, fldmg, DMG_BURN);

	self->pev->pain_finished = gpGlobals->time;
	self->pev->dmgtime = gpGlobals->time + dps; // half second delay until this trigger can hurt toucher again
}

void CFire::CreateSprite()
{
	m_pSprite = CSprite::SpriteCreate(FIRE_SPRITE, pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z - 4) * 0.5), true);
	m_pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone);
	m_pSprite->SetScale(1.0f);
	m_pSprite->pev->framerate = 10.0f;
}
