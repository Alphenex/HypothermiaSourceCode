#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

bool CHudStamina::Init()
{
	return true;
}

bool CHudStamina::VidInit()
{
	return true;
}

bool CHudStamina::Draw(float flTime)
{
	int r, g, b, x, y, a;
	Rect rc;

	if (!gHUD.HasSuit())
		return true;

	UnpackRGB(r, g, b, RGB_BLUEISH);

	ScaleColors(r, g, b, a);

	return true;
}