#include "Fire.h"

LINK_ENTITY_TO_CLASS(env_fire, CFire)

TYPEDESCRIPTION CFire::m_SaveData[] =
{
		DEFINE_FIELD(CFire, m_pFireSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_flLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_bActive, FIELD_BOOLEAN),

		DEFINE_FIELD(CFire, m_pAttachedEdict, FIELD_EDICT),
		DEFINE_FIELD(CFire, m_bBurnAttachedTillDead, FIELD_BOOLEAN),
		DEFINE_FIELD(CFire, m_pOwner, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_bSpawnedIn, FIELD_BOOLEAN),

		DEFINE_FIELD(CFire, m_fSmokeCreateTimer, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_fFireSoundTimer, FIELD_FLOAT)
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
#define SMOKE_SPRITE "sprites/smokepuff.spr"
#define FIRE_LOOPSOUND "ambience/burning1.wav"
#define FIRE_LOOPSOUNDLENGTH 5

void CFire::Precache()
{
	PRECACHE_MODEL(FIRE_SPRITE);
	m_iSmokeSpriteID = PRECACHE_MODEL(SMOKE_SPRITE);
	PRECACHE_SOUND(FIRE_LOOPSOUND);
}

void CFire::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;	

	if (!m_bSpawnedIn)
	{
		UTIL_SetSize(pev, Vector(-48, -48, -16), Vector(48, 48, 48));
		pev->scale = 1.0f;
		StartFire(); // Then create fire immediately, used so that map entities automatically start fire.
	}

	if (m_flLifeTime > 0.1f)
		pev->ltime = gpGlobals->time + m_flLifeTime;
	else
		pev->ltime = -1; // Fire will basically never gonna die. 

	if (m_pFireSprite)
		m_pFireSprite->pev->frame = RANDOM_LONG(0, m_pFireSprite->Frames() - 1);

	pev->nextthink = gpGlobals->time;
}

void CFire::Touch(CBaseEntity* pOther)
{
	if (m_bActive)
		HurtEntity(this, pOther);
}

void CFire::Think()
{
	CBaseAnimating* animattached = nullptr;

	if (m_bSpawnedIn)
		animattached = (CBaseAnimating*)m_pAttachedEdict->pvPrivateData;

	if ((gpGlobals->time > pev->ltime && pev->ltime != -1 && m_bActive) || // If the lifetime is passed.
		(animattached && !m_bBurnAttachedTillDead && !animattached->IsAlive()) ||  // If we have attached object, have Kill flag, and it is dead.
		(m_pAttachedEdict && animattached == nullptr)) // If attachment edict exists but it doesn't have private data.
	{
		KillFire();
		return;
	}

	if (m_bActive)
	{
		if (animattached && m_pAttachedEdict && m_pAttachedEdict->pvPrivateData != nullptr)
		{
			Vector attpelvispos;
			Vector attanglepos;

			// Nasty code, this shouldn't be done every Think BUT what we essentially do here is:
			// We check if Pelvis bone exists, if that doesn't exist just get Center.
			animattached->GetBonePosition(1, attpelvispos, attanglepos);
			if (attpelvispos == Vector(0, 0, 0))
				attpelvispos = animattached->Center();
			pev->origin = attpelvispos; // Move with the attached object!

			HurtEntity(this, animattached);
		}

		if (m_pFireSprite)
		{
			Vector norg = pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z - 4 * !m_bSpawnedIn) * 0.5);
			m_pFireSprite->pev->origin = norg;
			m_pFireSprite->AnimateThink();
		}

		if (gpGlobals->time > m_fSmokeCreateTimer)
		{
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_BUBBLES);
			WRITE_COORD(pev->origin.x - 10.0f); // min
			WRITE_COORD(pev->origin.y - 10.0f);
			WRITE_COORD(pev->origin.z - 10.0f);
			WRITE_COORD(pev->origin.x + 10.0f); // max
			WRITE_COORD(pev->origin.y + 10.0f);
			WRITE_COORD(pev->origin.z + 10.0f);
			WRITE_COORD(125.0f);		   // fly up to
			WRITE_SHORT(m_iSmokeSpriteID); // sprite id
			WRITE_BYTE(2);				   // amount
			WRITE_COORD(0.5f);			   // speed
			MESSAGE_END();
			m_fSmokeCreateTimer = gpGlobals->time + 0.05f + RANDOM_FLOAT(0.0f, 0.33f); // Every 0.05f + random secs create smoke.
		}

		if (m_bSpawnedIn)
		{
			if (gpGlobals->time > m_fFireSoundTimer)
			{
				EMIT_SOUND(edict(), CHAN_VOICE, FIRE_LOOPSOUND, 7.5f, 0.5f);
				m_fFireSoundTimer = gpGlobals->time + FIRE_LOOPSOUNDLENGTH;
			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pev->origin.x); // origin
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_BYTE(12);	 // radius
			WRITE_BYTE(255); // R
			WRITE_BYTE(80);	 // G
			WRITE_BYTE(32);	 // B
			WRITE_BYTE(2);	 // life * 10
			WRITE_BYTE(0);	 // decay
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

	if (m_pFireSprite == nullptr) // if Sprite doesn't exist then create it, we shouldn't even ever hit this?
		CreateSprite();
}

void CFire::KillFire()
{
	STOP_SOUND(edict(), CHAN_VOICE, FIRE_LOOPSOUND);
	UTIL_Remove(m_pFireSprite);
	UTIL_Remove(this);
}

CFire* CFire::CreateFire()
{
	CFire* fire = GetClassPtr<CFire>(nullptr);
	fire->pev->scale = 1.0f;
	fire->m_bSpawnedIn = true;

	return fire;
}

CFire* CFire::SpawnFireAtPosition(Vector vPos, float flLifetime, float flDamage)
{
	CFire* fire = CreateFire();
	fire->m_flLifeTime = flLifetime;
	fire->pev->origin = vPos;
	fire->pev->dmg = flDamage;
	fire->Spawn();
	fire->StartFire();

	return fire;
}

static int s_DontBurnList[] = {
	CLASS_BARNACLE,
	CLASS_MACHINE,
	CLASS_NONE // We have no idea what it is, just don't burn it >:(
};

CFire* CFire::BurnEntity(CBaseEntity* pEnt, CBaseEntity* pAttacker, float flLifetime, float flDamage)
{
	for (unsigned int i = 0; i < sizeof(s_DontBurnList) / sizeof(int); i++)
	{
		if (pEnt->Classify() == s_DontBurnList[i])
			return nullptr;
	}

	if (pEnt->m_pFire)
		return nullptr;

	Vector plyhull = VEC_HUMAN_HULL_MAX - VEC_HUMAN_HULL_MIN; // used for comparison
	Vector enthull = pEnt->pev->maxs - pEnt->pev->mins;
	Vector ratiohull = Vector(enthull.x / plyhull.x, enthull.y / plyhull.y, enthull.z / plyhull.z);
	float ratiogen = (float)(ratiohull.x + ratiohull.y + ratiohull.z) / 3.0f;

	CFire* fire = CreateFire();
	fire->pev->dmg = flDamage;
	fire->m_flLifeTime = flLifetime;
	fire->m_pOwner = pAttacker;
	fire->m_pAttachedEdict = pEnt->edict();
	fire->pev->scale = ratiogen;
	fire->Spawn();
	fire->StartFire();

	pEnt->m_pFire = fire;

	return fire;
}

CFire* CFire::BurnEntityUntilDead(CBaseEntity* pEnt, CBaseEntity* pAttacker, float flDamage)
{
	for (unsigned int i = 0; i < sizeof(s_DontBurnList) / sizeof(int); i++)
	{
		if (pEnt->Classify() == s_DontBurnList[i])
			return nullptr;
	}

	if (!pEnt->IsAlive() || pEnt->m_pFire)
		return nullptr;

	Vector plyhull = VEC_HUMAN_HULL_MAX - VEC_HUMAN_HULL_MIN; // used for comparison
	Vector enthull = pEnt->pev->maxs - pEnt->pev->mins;
	Vector ratiohull = Vector(enthull.x / plyhull.x, enthull.y / plyhull.y, enthull.z / plyhull.z);
	float ratiogen = (float)(ratiohull.x + ratiohull.y + ratiohull.z) / 3.0f;

	CFire* fire = CreateFire();
	fire->pev->dmg = flDamage;
	fire->m_flLifeTime = -1;
	fire->m_bBurnAttachedTillDead = true;
	fire->m_pOwner = pAttacker;
	fire->m_pAttachedEdict = pEnt->edict();
	fire->pev->scale = ratiogen;
	fire->Spawn();
	fire->StartFire();

	pEnt->m_pFire = fire;

	return fire;
}

constexpr float dps = 0.5f;
void CFire::HurtEntity(CFire* self, CBaseEntity* pEnt)
{
	if (pEnt->pev->takedamage == 0) return;
	if (self->pev->dmgtime > gpGlobals->time && gpGlobals->time != self->pev->pain_finished) return;

	float fldmg = self->pev->dmg * dps;

	if (!self->m_pOwner) pEnt->TakeDamage(self->pev, self->pev, fldmg, DMG_BURN);
	else pEnt->TakeDamage(self->pev, self->m_pOwner->pev, fldmg, DMG_BURN);

	self->pev->pain_finished = gpGlobals->time;
	self->pev->dmgtime = gpGlobals->time + dps; // half second delay until this trigger can hurt toucher again
}

void CFire::CreateSprite()
{
	m_pFireSprite = CSprite::SpriteCreate(FIRE_SPRITE, pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z - 4) * 0.5), true);
	m_pFireSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 200, kRenderFxNone);
	m_pFireSprite->SetScale(pev->scale);
	m_pFireSprite->pev->framerate = 10.0f;
}
