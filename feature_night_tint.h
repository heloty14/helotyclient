// ПАКЕТ: Night Tint. Синеватая ночная дымка поверх экрана.
#include "../heloty_menu.h"

static void HelotyNightScreen(CHelotyMenu &M)
{
	// цвет и прозрачность (r, g, b, a)
	M.Rect(0.0f, 0.0f, M.ScreenW(), M.ScreenH(), 0.03f, 0.06f, 0.22f, 0.28f);
}

HELOTY_FEATURE("Night Tint", HCAT_RENDER, 0, HelotyNightScreen, 0)
