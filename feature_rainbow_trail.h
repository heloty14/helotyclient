// ПАКЕТ: Rainbow Trail. За тией остаётся цветная дорожка, которая гаснет.
#include "../heloty_menu.h"

struct CHelotyTrailPoint
{
	vec2 m_Pos;
	float m_Born;
};
static std::vector<CHelotyTrailPoint> gs_vHelotyTrail;

static void HelotyHueToRgb(float H, float *pR, float *pG, float *pB)
{
	H = H - floorf(H);
	const float X = H * 6.0f;
	const int I = (int)X;
	const float F = X - (float)I;
	switch(I % 6)
	{
	case 0: *pR = 1.0f; *pG = F; *pB = 0.0f; break;
	case 1: *pR = 1.0f - F; *pG = 1.0f; *pB = 0.0f; break;
	case 2: *pR = 0.0f; *pG = 1.0f; *pB = F; break;
	case 3: *pR = 0.0f; *pG = 1.0f - F; *pB = 1.0f; break;
	case 4: *pR = F; *pG = 0.0f; *pB = 1.0f; break;
	default: *pR = 1.0f; *pG = 0.0f; *pB = 1.0f - F; break;
	}
}

static void HelotyTrailToggle(CHelotyMenu &M, bool On)
{
	if(!On)
		gs_vHelotyTrail.clear();
}

static void HelotyTrailWorld(CHelotyMenu &M)
{
	const float Now = M.Time();
	const float Life = 0.7f; // сколько секунд живёт след
	const float MaxSize = 9.0f; // размер квадратиков следа

	vec2 Pos;
	if(M.LocalPos(&Pos))
	{
		if(gs_vHelotyTrail.empty() || distance(gs_vHelotyTrail.back().m_Pos, Pos) > 3.0f)
		{
			CHelotyTrailPoint P;
			P.m_Pos = Pos;
			P.m_Born = Now;
			gs_vHelotyTrail.push_back(P);
		}
	}

	size_t Old = 0;
	while(Old < gs_vHelotyTrail.size() && Now - gs_vHelotyTrail[Old].m_Born > Life)
		Old++;
	if(Old > 0)
		gs_vHelotyTrail.erase(gs_vHelotyTrail.begin(), gs_vHelotyTrail.begin() + Old);
	if(gs_vHelotyTrail.empty())
		return;

	IGraphics *pG = M.Gfx();
	pG->TextureClear();
	pG->BlendNormal();
	pG->QuadsBegin();
	for(size_t i = 0; i < gs_vHelotyTrail.size(); i++)
	{
		const float Age = (Now - gs_vHelotyTrail[i].m_Born) / Life;
		const float Size = MaxSize * (1.0f - Age);
		float R, G, B;
		HelotyHueToRgb(gs_vHelotyTrail[i].m_Born * 0.8f, &R, &G, &B);
		pG->SetColor(R, G, B, 0.8f * (1.0f - Age));
		IGraphics::CQuadItem Item(gs_vHelotyTrail[i].m_Pos.x - Size / 2.0f, gs_vHelotyTrail[i].m_Pos.y - Size / 2.0f, Size, Size);
		pG->QuadsDrawTL(&Item, 1);
	}
	pG->QuadsEnd();
}

HELOTY_FEATURE("Rainbow Trail", HCAT_TEE, HelotyTrailToggle, 0, HelotyTrailWorld)
