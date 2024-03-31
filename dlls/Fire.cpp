#include "Fire.h"
#include "player.h"

#include <algorithm>

LINK_ENTITY_TO_CLASS(env_fire, CFire)

TYPEDESCRIPTION CFire::m_SaveData[] =
{
		DEFINE_FIELD(CFire, m_pFireSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_pFireGlowSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_flLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_bActive, FIELD_BOOLEAN),

		DEFINE_FIELD(CFire, m_pAttachedEdict, FIELD_EDICT),
		DEFINE_FIELD(CFire, m_bBurnFlag, FIELD_BOOLEAN),
		DEFINE_FIELD(CFire, m_pOwner, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_bSpawnedIn, FIELD_BOOLEAN),
		DEFINE_FIELD(CFire, m_bCharAttached, FIELD_BOOLEAN),

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
#define FIREGLOW_SPRITE "sprites/hotglow.spr"
#define SMOKE_SPRITE "sprites/smokepuff.spr"
#define FIRE_LOOPSOUND "ambience/burning1.wav"
#define FIRE_LOOPSOUNDLENGTH 5

void CFire::Precache()
{
	PRECACHE_MODEL(FIRE_SPRITE);
	PRECACHE_MODEL(FIREGLOW_SPRITE);
	m_iSmokeSpriteID = PRECACHE_MODEL(SMOKE_SPRITE);
	PRECACHE_SOUND(FIRE_LOOPSOUND);
}

void CFire::Spawn()
{
	Precache();

	if (m_bGravity == false)
		pev->movetype = MOVETYPE_NONE;
	else
		pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	if (!m_bSpawnedIn)
	{
		UTIL_SetSize(pev, Vector(-48, -48, -16), Vector(48, 48, 48));
		pev->scale = 1.0f;
		StartFire(); // Then create fire immediately, used so that map entities automatically start fire.
	}
	else
		UTIL_SetSize(pev, Vector(-32, -32, -32), Vector(32, 32, 32));


	if (!FBitSet(m_bBurnFlag, FBURN_UNTILDEADWLIFETIME))
	{
		if (m_flLifeTime > 0.1f)
			pev->ltime = gpGlobals->time + m_flLifeTime;
		else
			pev->ltime = -1; // Fire will basically never gonna die.
	}

	if (m_pFireSprite)
		m_pFireSprite->pev->frame = RANDOM_LONG(0, m_pFireSprite->Frames() - 1);

	pev->nextthink = gpGlobals->time;
}

void CFire::Touch(CBaseEntity* pOther)
{
	if (m_bActive)
		HurtEntity(this, pOther);
}

static int k = 0;

void CFire::Think()
{
	CBaseMonster* animattached = nullptr;
	if (m_bSpawnedIn)
		animattached = m_pAttachedEdict.Entity<CBaseMonster>();

	if ((FBitSet(m_bBurnFlag, FBURN_LIFETIME) && gpGlobals->time > pev->ltime && pev->ltime != -1 && m_bActive) || // If the lifetime is passed.
		(animattached && FBitSet(m_bBurnFlag, FBURN_UNTILDEAD) && !animattached->IsAlive() && m_bActive) || // If we have attached object, have Kill flag, and it is dead.
		(m_pAttachedEdict.Get() && animattached == nullptr)) // If attachment edict exists but it doesn't have private data.
	{
		KillFire();
		return;
	}

	float timeleft = (pev->ltime - gpGlobals->time);
	if (timeleft <= 1.0f && timeleft > 0.01f)
	{
		m_pFireSprite->SetScale(timeleft);
		m_pFireGlowSprite->SetScale(timeleft);
	}

	if (FBitSet(m_bBurnFlag, FBURN_UNTILDEADWLIFETIME) && m_bActive && !animattached->IsAlive() && pev->ltime == 0.0f)
	{
		m_bBurnFlag = FBURN_LIFETIME;
		pev->ltime = gpGlobals->time + m_flLifeTime;
	}

	if (m_bActive)
	{
		if (animattached && m_pAttachedEdict)
		{
			Vector attpelvispos = Vector(0, 0, 0);
			Vector attanglepos;

			// Nasty code, this shouldn't be done every Think BUT what we essentially do here is:
			// We check if Pelvis bone exists, if that doesn't exist just get Center.
			GET_BONE_POSITION(m_pAttachedEdict.Get(), 0, attpelvispos, attanglepos);
			if (attpelvispos == Vector(0, 0, 0))
				attpelvispos = animattached->Center();
			pev->origin = attpelvispos; // Move with the attached object!

			HurtEntity(this, animattached);
		}

		Vector norg = pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z - 8 * (!m_bSpawnedIn || m_bGravity)) * 0.5);

		if (m_pFireSprite)
		{
			m_pFireSprite->pev->origin = norg;
			m_pFireSprite->AnimateThink();
		}

		if (m_pFireGlowSprite)
			m_pFireGlowSprite->pev->origin = norg;

		if (gpGlobals->time > m_fSmokeCreateTimer)
		{
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_BUBBLETRAIL);
			WRITE_COORD(pev->origin.x - 10.0f); // min
			WRITE_COORD(pev->origin.y - 10.0f);
			WRITE_COORD(pev->origin.z - 10.0f);
			WRITE_COORD(pev->origin.x + 10.0f); // max
			WRITE_COORD(pev->origin.y + 10.0f);
			WRITE_COORD(pev->origin.z + 10.0f);
			WRITE_COORD(125.0f);		   // fly up to
			WRITE_SHORT(m_iSmokeSpriteID); // sprite id
			WRITE_BYTE(2);				   // amount
			WRITE_COORD(0.25f);			   // speed
			MESSAGE_END();
			m_fSmokeCreateTimer = gpGlobals->time + 0.05f + RANDOM_FLOAT(0.0f, 0.33f); // Every 0.05f + random secs create smoke.
		}

		if (m_bSpawnedIn)
		{
			if (gpGlobals->time > m_fFireSoundTimer)
			{
				EMIT_SOUND_DYN2(edict(), CHAN_VOICE, FIRE_LOOPSOUND, 1.0f, 0.5f, 0, PITCH_NORM + pev->idealpitch);
				m_fFireSoundTimer = gpGlobals->time + FIRE_LOOPSOUNDLENGTH;
			}

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(pev->origin.x); // origin
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_BYTE(8);	 // radius
			WRITE_BYTE(255); // R
			WRITE_BYTE(150); // G
			WRITE_BYTE(0);	 // B
			WRITE_BYTE(2);	 // life * 10
			WRITE_BYTE(0);	 // decay
			MESSAGE_END();
		}
	}

	pev->nextthink = gpGlobals->time + 0.05f;
}

void CFire::Killed(entvars_t* pevAttacker, int iGib)
{
	STOP_SOUND(edict(), CHAN_VOICE, FIRE_LOOPSOUND);
	UTIL_Remove(m_pFireSprite);
	UTIL_Remove(m_pFireGlowSprite);
	CBaseEntity* attached = m_pAttachedEdict.Entity<CBaseEntity>();
	if (attached)
		attached->m_pFire = nullptr;
}

void CFire::StartFire()
{
	if (m_bActive) // If the fire is already active then don't do anything!
		return;

	m_bActive = true;

	pev->idealpitch = RANDOM_LONG(-50, 50); // couldn't be bothered to create a new var;

	if (m_pFireSprite == nullptr) // if Sprite doesn't exist then create it, we shouldn't even ever hit this?
		CreateSprite();
}

void CFire::KillFire()
{
	STOP_SOUND(edict(), CHAN_VOICE, FIRE_LOOPSOUND);
	UTIL_Remove(m_pFireSprite);
	UTIL_Remove(m_pFireGlowSprite);
	CBaseEntity* attached = m_pAttachedEdict.Entity<CBaseEntity>();
	if (attached)
		attached->m_pFire = nullptr;
	UTIL_Remove(this);
}

CFire* CFire::CreateFire()
{
	CFire* fire = GetClassPtr<CFire>(nullptr);
	fire->pev->scale = 1.0f;
	fire->m_bSpawnedIn = true;

	return fire;
}

CFire* CFire::SpawnFireAtPosition(Vector vPos, float flLifetime, float flDamage, bool bGravity)
{
	CFire* fire = CreateFire();
	fire->m_flLifeTime = flLifetime;
	fire->pev->origin = vPos;
	fire->pev->dmg = flDamage;
	fire->m_bGravity = bGravity;
	fire->m_bBurnFlag = FBURN_LIFETIME;
	fire->Spawn();
	fire->StartFire();

	return fire;
}

static int s_DontBurnList[] = {
	CLASS_BARNACLE,
	CLASS_MACHINE,
	CLASS_NONE // We have no idea what it is, just don't burn it >:(
};

CFire* CFire::BurnEntity(CBaseEntity* pEnt, CBaseEntity* pAttacker, float flDamage, float flLifetime, bool charattached)
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
	fire->m_pAttachedEdict.Set(pEnt->edict());
	fire->m_bBurnFlag = FBURN_LIFETIME;
	fire->pev->scale = ratiogen;
	fire->m_bCharAttached = charattached;
	fire->Spawn();
	fire->StartFire();

	pEnt->m_pFire = fire;

	return fire;
}

CFire* CFire::BurnEntityUntilDead(CBaseEntity* pEnt, CBaseEntity* pAttacker, float flDamage, bool charattached)
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
	fire->m_bBurnFlag = FBURN_UNTILDEAD;
	fire->m_pOwner = pAttacker;
	fire->m_pAttachedEdict.Set(pEnt->edict());
	fire->pev->scale = ratiogen;
	fire->m_bCharAttached = charattached;
	fire->Spawn();
	fire->StartFire();

	pEnt->m_pFire = fire;

	return fire;
}

CFire* CFire::BurnEntityUntilDeadWithLifetime(CBaseEntity* pEnt, CBaseEntity* pAttacker, float flDamage, float flLifetime, bool charattached)
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
	fire->m_flLifeTime = flLifetime;
	fire->m_bBurnFlag = FBURN_UNTILDEADWLIFETIME;
	fire->m_pOwner = pAttacker;
	fire->m_pAttachedEdict.Set(pEnt->edict());
	fire->pev->scale = ratiogen;
	fire->m_bCharAttached = charattached;
	fire->Spawn();
	fire->StartFire();

	pEnt->m_pFire = fire;

	return fire;
}

constexpr float dps = 1.0f; // Damage Per Second Timer.
void CFire::HurtEntity(CFire* self, CBaseEntity* pEnt)
{
	if (!pEnt->IsAlive()) return; // Why should we hurt a dead ass guy?
	if (self->pev->dmg <= 0.0f) return;
	if (pEnt->pev->takedamage == 0) return;
	if (self->pev->dmgtime > gpGlobals->time && gpGlobals->time != self->pev->pain_finished) return;

	float fldmg = self->pev->dmg * 0.5f;

	if (!self->m_pOwner) pEnt->TakeDamage(self->pev, self->pev, fldmg, DMG_BURN);
	else pEnt->TakeDamage(self->pev, self->m_pOwner->pev, fldmg, DMG_BURN);

	self->pev->pain_finished = gpGlobals->time;
	self->pev->dmgtime = gpGlobals->time + dps;
}

void CFire::CreateSprite()
{
	Vector offset = Vector(0, 0, (pev->mins.z + pev->maxs.z - 4) * 0.5);

	CBasePlayer* attached = nullptr;
	if (m_bSpawnedIn)
		attached = m_pAttachedEdict.Entity<CBasePlayer>();
	
	if (!attached || !attached->IsPlayer())
	{
		m_pFireSprite = CSprite::SpriteCreate(FIRE_SPRITE, pev->origin + offset, true);
		m_pFireSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 220, kRenderFxNone);
		m_pFireSprite->SetScale(pev->scale);
		m_pFireSprite->pev->framerate = 10.0f;
	}

	m_pFireGlowSprite = CSprite::SpriteCreate(FIREGLOW_SPRITE, pev->origin + offset, true);
	m_pFireGlowSprite->SetTransparency(kRenderGlow, 255, 255, 255, 175, kRenderFxNoDissipation);
	m_pFireGlowSprite->SetScale(pev->scale);
}
