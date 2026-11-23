// ПАКЕТ: FPS Counter. Счётчик FPS в правом верхнем углу.
#include "../heloty_menu.h"

static float gs_HelotyFpsSmooth = 60.0f;

static void HelotyFpsScreen(CHelotyMenu &M)
{
	const float Dt = M.FrameTime();
	if(Dt > 0.0f)
		gs_HelotyFpsSmooth = gs_HelotyFpsSmooth * 0.95f + (1.0f / Dt) * 0.05f;

	char aBuf[32];
	str_format(aBuf, sizeof(aBuf), "%d FPS", (int)(gs_HelotyFpsSmooth + 0.5f));
	M.Label(M.ScreenW() - 50.0f, 4.0f, 9.0f, aBuf, 1.0f, 1.0f, 1.0f);
}

HELOTY_FEATURE("FPS Counter", HCAT_MISC, 0, HelotyFpsScreen, 0)
