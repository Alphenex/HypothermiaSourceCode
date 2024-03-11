#include "Fire.h"

LINK_ENTITY_TO_CLASS(env_fire, CFire)

TYPEDESCRIPTION CFire::m_SaveData[] =
{
		DEFINE_FIELD(CFire, m_pSprite, FIELD_CLASSPTR),
		DEFINE_FIELD(CFire, m_flLifeTime, FIELD_FLOAT),
		DEFINE_FIELD(CFire, m_bActive, FIELD_BOOLEAN)
};
IMPLEMENT_SAVERESTORE(CFire, CBaseEntity);

void CFire::Spawn()
{
}

bool CFire::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "startdist"))
	{
		
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "sprite"))
	{
		int k = atoi(pkvd->szValue);
		
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CFire::Precache()
{

}

void CFire::Touch(CBaseEntity* pOther)
{
}

void CFire::Think()
{
}

void CFire::StartFire()
{
	m_bActive = true;
}

void CFire::KillFire()
{
	m_bActive = false;
}