#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "keydefs.h"
#include "math.h"

/*
DECLARE_MESSAGE(m_Cold, Cold)

static float fCold;

bool CHudCold::MsgFunc_Cold(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	float cold = (int)READ_COORD();

	fCold = cold;

	m_iFlags |= HUD_ACTIVE;


	return true;
}

bool CHudCold::Init()
{
	//HOOK_MESSAGE(Cold);

	gHUD.AddHudElem(this);

	return true;
}

bool CHudCold::VidInit()
{
	return true;
}

bool CHudCold::Draw(float flTime)
{
	int r, g, b, x, y;
	int a = MIN_ALPHA;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL) != 0)
		return true;

	UnpackRGB(r, g, b, RGB_BLUEISH);
	ScaleColors(r, g, b, a);

	return true;
}

void CHudCold::Reset()
{
}
*/