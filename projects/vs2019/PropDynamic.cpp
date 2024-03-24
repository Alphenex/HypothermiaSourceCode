#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CPropDynamic : public CBaseAnimating
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

private:
	static TYPEDESCRIPTION m_SaveData[];

	int m_iBody1;
	int m_iBody2;
};

LINK_ENTITY_TO_CLASS(prop_dynamic, CPropDynamic)

TYPEDESCRIPTION CPropDynamic::m_SaveData[] = {
	DEFINE_FIELD(CPropDynamic, m_iBody1, FIELD_INTEGER),
	DEFINE_FIELD(CPropDynamic, m_iBody2, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CPropDynamic, CBaseAnimating)

void CPropDynamic::Spawn()
{
	if (pev->model == 0)
	{
		ALERT(at_warning, "prop_dynamic has no model, it will be removed.\n");
		UTIL_Remove(this);
		return;
	}

	const char* mdlchar = STRING(pev->model);

	PRECACHE_MODEL(mdlchar);
	SET_MODEL(ENT(pev), mdlchar);
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetBodygroup(0, m_iBody1);
	SetBodygroup(1, m_iBody2);

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	pev->nextthink = gpGlobals->time;
}

bool CPropDynamic::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "BODYGROUP1"))
	{
		m_iBody1 = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "BODYGROUP2"))
	{
		m_iBody2 = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CPropDynamic::Think()
{
}