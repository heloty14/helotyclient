// ПАКЕТ: Snow. Белые снежинки падают по всему экрану.
#include "../heloty_menu.h"

struct CHelotySnowDrop
{
	float m_X, m_Y, m_Speed, m_Size, m_Phase;
};
static std::vector<CHelotySnowDrop> gs_vHelotySnow;

static void HelotySnowScreen(CHelotyMenu &M)
{
	const float Width = M.ScreenW();
	const float Height = M.ScreenH();
	const float Dt = M.FrameTime();
	const float Time = M.Time();
	const int Count = 120; // сколько снежинок

	if(gs_vHelotySnow.empty())
	{
		for(int i = 0; i < Count; i++)
		{
			CHelotySnowDrop D;
			D.m_X = M.Rnd() * Width;
			D.m_Y = M.Rnd() * Height;
			D.m_Speed = 15.0f + M.Rnd() * 25.0f; // скорость падения
			D.m_Size = 1.0f + M.Rnd() * 1.8f; // размер
			D.m_Phase = M.Rnd() * 6.28f;
			gs_vHelotySnow.push_back(D);
		}
	}

	std::vector<IGraphics::CQuadItem> vItems;
	vItems.reserve(gs_vHelotySnow.size());
	for(size_t i = 0; i < gs_vHelotySnow.size(); i++)
	{
		CHelotySnowDrop &D = gs_vHelotySnow[i];
		D.m_Y += D.m_Speed * Dt;
		D.m_X += sinf(Time + D.m_Phase) * 10.0f * Dt; // лёгкое покачивание
		if(D.m_Y > Height)
		{
			D.m_Y = -3.0f;
			D.m_X = M.Rnd() * Width;
		}
		if(D.m_X < 0.0f)
			D.m_X += Width;
		if(D.m_X > Width)
			D.m_X -= Width;
		vItems.push_back(IGraphics::CQuadItem(D.m_X, D.m_Y, D.m_Size, D.m_Size));
	}

	IGraphics *pG = M.Gfx();
	pG->TextureClear();
	pG->QuadsBegin();
	pG->SetColor(1.0f, 1.0f, 1.0f, 0.85f); // цвет снега
	pG->QuadsDrawTL(&vItems[0], (int)vItems.size());
	pG->QuadsEnd();
}

HELOTY_FEATURE("Snow", HCAT_RENDER, 0, HelotySnowScreen, 0)
