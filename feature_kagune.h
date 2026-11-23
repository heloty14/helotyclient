// ПАКЕТ: Kagune. 4 щупальца вокруг твоей тии (рисуется в мире).
// Этот файл подключается через #include (см. patch.sh).
#include "../heloty_menu.h"

static void HelotyKaguneWorld(CHelotyMenu &M)
{
	vec2 Center;
	if(!M.LocalPos(&Center))
		return;

	IGraphics *pG = M.Gfx();
	const float Time = M.Time();

	// Настройки внешнего вида (можно менять):
	const float aAngles[4] = {-2.4f, -0.74f, 2.5f, 0.64f}; // направление каждого щупальца (радианы)
	const float aPhases[4] = {0.0f, 1.6f, 3.1f, 4.7f}; // сдвиг покачивания
	const int Segments = 14; // из скольки частей щупальце
	const float TentacleLength = 70.0f; // длина
	const float WaveSpeed = 3.0f; // скорость покачивания

	pG->TextureClear();
	pG->BlendNormal();
	pG->QuadsBegin();

	for(int k = 0; k < 4; k++)
	{
		float Angle = aAngles[k];
		vec2 Pos = Center + vec2(cosf(Angle), sinf(Angle)) * 12.0f;
		vec2 PrevL = Pos;
		vec2 PrevR = Pos;
		const float SegLen = TentacleLength / Segments;

		for(int i = 0; i < Segments; i++)
		{
			const float T = i / (float)(Segments - 1);
			Angle += sinf(Time * WaveSpeed + aPhases[k] + i * 0.55f) * 0.12f * (0.4f + T);
			Pos += vec2(cosf(Angle), sinf(Angle)) * SegLen;

			const float Thick = 5.0f * (1.0f - T) + 0.5f; // сужение к кончику
			const vec2 Normal = vec2(-sinf(Angle), cosf(Angle)) * Thick;
			const vec2 L = Pos + Normal;
			const vec2 R = Pos - Normal;

			if(i > 0)
			{
				// цвет: тёмно-красный у основания, ярче к кончику (r, g, b, прозрачность)
				pG->SetColor(0.55f + 0.4f * T, 0.02f, 0.08f + 0.1f * T, 0.95f - 0.25f * T);
				IGraphics::CFreeformItem Item(PrevL.x, PrevL.y, PrevR.x, PrevR.y, L.x, L.y, R.x, R.y);
				pG->QuadsDrawFreeform(&Item, 1);
			}
			PrevL = L;
			PrevR = R;
		}
	}

	pG->QuadsEnd();
}

HELOTY_FEATURE("Kagune", HCAT_RENDER, 0, 0, HelotyKaguneWorld)
