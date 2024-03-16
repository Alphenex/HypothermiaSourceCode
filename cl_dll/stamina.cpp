#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "keydefs.h"
#include "math.h"

cvar_t* stamalpha;
cvar_t* stamhide;

DECLARE_MESSAGE(m_Stamina, Stamina)

bool CHudStamina::MsgFunc_Stamina(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int stamina = (int)READ_SHORT();

	m_flStamina = (float)stamina; // Round the number.

	m_iFlags |= HUD_ACTIVE;

	return true;
}

bool CHudStamina::Init()
{
	HOOK_MESSAGE(Stamina);
	m_flStamina = 100.0f;
	m_flStaminaAlpha = 0.0f;

	gHUD.AddHudElem(this);

	stamalpha = gEngfuncs.pfnRegisterVariable("cl_staminafade", "1", 0);
	stamhide = gEngfuncs.pfnRegisterVariable("cl_staminahide", "1", 0);

	return true;
}

bool CHudStamina::VidInit()
{
	return true;
}

static float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

static int sign = -1;

bool CHudStamina::Draw(float flTime)
{
	int r, g, b, x, y;
	int a = MIN_ALPHA;
	bool pressingshift = (gHUD.m_iKeyBits & (IN_SCORE)) != 0;
	bool moving = (gHUD.m_iKeyBits & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)) != 0;

	a = std::clamp<float>(((float)(100 - m_flStamina) / 100) * 255, MIN_ALPHA / 2, MIN_ALPHA);

	m_flStaminaAlpha += 200.0f * sign * gHUD.m_flTimeDelta;
	m_flStaminaAlpha = std::clamp<float>(m_flStaminaAlpha, 0, MIN_ALPHA);

	if (!pressingshift || !moving) sign = -1;
	else sign = 1;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL) != 0)
		return true;

	if (!gHUD.HasSuit())
		return true;

	UnpackRGB(r, g, b, RGB_BLUEISH);
	ScaleColors(r, g, b, m_flStaminaAlpha);
	x = 40;
	y = gHUD.m_iFontHeight * 3.0f;

	FillRGBA(x, ScreenHeight - (float)y / 1.5f, -16, -(m_flStamina / 1.5f), r, g, b, m_flStaminaAlpha);
	x += gHUD.GetHudNumberWidth((int)m_flStamina, 3, DHN_DRAWZERO);
	gHUD.DrawHudNumberReverse(x + 3, ScreenHeight - y, m_flStamina, DHN_DRAWZERO, r, g, b);
	return true;
}

void CHudStamina::Reset()
{
}