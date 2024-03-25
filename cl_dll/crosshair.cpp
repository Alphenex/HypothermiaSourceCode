#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "keydefs.h"
#include "math.h"

DECLARE_MESSAGE(m_Crosshair, Crosshair)

cvar_t* crosshairwidth;

bool CHudCrosshair::MsgFunc_Crosshair(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	float spread = READ_FLOAT();
	float wpncone = READ_FLOAT();

	m_flSpread = spread;
	m_flWeaponcone = abs(wpncone);

	m_iFlags |= HUD_ACTIVE;

	return true;
}

bool CHudCrosshair::Init()
{
	HOOK_MESSAGE(Crosshair);

	gHUD.AddHudElem(this);

	crosshairwidth = CVAR_CREATE("cl_crosshairwidth", "4", 0);
	
	return true;
}

bool CHudCrosshair::VidInit()
{
	return true;
}

bool CHudCrosshair::Draw(float flTime)
{
	int r, g, b, x, y;
	int a = MIN_ALPHA + 75;
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL) != 0)
		return true;

	if (!gHUD.HasSuit())
		return true;

	if (m_flWeaponcone <= 0.0f)
		return true; // Don't draw if weaponcone is small AF

	UnpackRGB(r, g, b, RGB_BLUEISH);
	ScaleColors(r, g, b, a);

	x = (float)ScreenWidth / 2.0f;
	y = (float)ScreenHeight / 2.0f;

	int crosslength = (m_flSpread + m_flWeaponcone + 0.044f) * 666.6f; // mmm, hardcoding

	float crosswidth = CVAR_GET_FLOAT("cl_crosshairwidth") / 2.0f;

	FillRGBA(x - crosslength,	y,					10.0f - crosswidth,		crosswidth,					r, g, b, a);
	FillRGBA(x + crosslength,	y,					-10.0f + crosswidth,	crosswidth,				r, g, b, a);
	FillRGBA(x,					y - crosslength,	crosswidth,				10.0f - crosswidth,		r, g, b, a);
	FillRGBA(x,					y + crosslength,	crosswidth,				-10.0f + crosswidth,	r, g, b, a);

	return true;
}

void CHudCrosshair::Reset()
{
}