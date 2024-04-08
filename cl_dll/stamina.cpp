#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "keydefs.h"
#include "math.h"

cvar_t* stamhide;

DECLARE_MESSAGE(m_Stamina, Stamina)

DECLARE_COMMAND(m_Stamina, SprintDown)
DECLARE_COMMAND(m_Stamina, SprintUp)


bool CHudStamina::MsgFunc_Stamina(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int stamina = (int)READ_SHORT();

	m_flStamina = (float)stamina; // Round the number.

	m_iFlags |= HUD_ACTIVE;

	return true;
}

static bool m_bSprinting;

void CHudStamina::UserCmd_SprintDown()
{
	m_bSprinting = true;
	
	if (gHUD.HasSuit())
		ServerCmd("+sprint");
}

void CHudStamina::UserCmd_SprintUp()
{
	m_bSprinting = false;
	if (gHUD.HasSuit())
		ServerCmd("-sprint");
}

bool CHudStamina::Init()
{
	HOOK_MESSAGE(Stamina);
	HOOK_COMMAND("+sprint", SprintDown);
	HOOK_COMMAND("-sprint", SprintUp);

	gHUD.AddHudElem(this);

	m_flStamina = 100.0f;
	m_flStaminaAlpha = 0.0f;
	m_bSprinting = false;

	stamhide = CVAR_CREATE("cl_staminahide", "0", 0);
	
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

static float sign = -1;
bool CHudStamina::Draw(float flTime)
{
	int r, g, b, x, y;
	int a = MIN_ALPHA;

	if (CVAR_GET_FLOAT("cl_staminahide") == 1)
		return true;

	bool pressingshift = m_bSprinting;
	bool moving = (gHUD.m_iKeyBits & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)) != 0;

	m_flStaminaAlpha += 200.0f * sign * gHUD.m_flTimeDelta;
	m_flStaminaAlpha = std::clamp<float>(m_flStaminaAlpha, 0, MIN_ALPHA);

	if (m_flStamina < 50.0f) sign = 5.0f;
	else if (!pressingshift || !moving) sign = -0.1f;
	else sign = 2.0f;

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
