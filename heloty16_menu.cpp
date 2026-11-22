/*
 ============================================================================
  HELOTY CLIENT: КАК ДОБАВИТЬ СВОЮ ФУНКЦИЮ В МЕНЮ
 ============================================================================

  Каждая кнопка в меню это "модуль" (CHelotyModule). Клик по кнопке включает
  или выключает модуль. Чтобы добавить новую функцию, нужно 3 шага.

  ШАГ 1. Напиши саму функцию.
     В heloty16_menu.h, в разделе "функции", объяви метод:
         void RenderMojaFunkcija();
     Ниже в этом файле (смотри RenderKagune как пример) напиши его тело:
         void CHelotyMenu::RenderMojaFunkcija()
         {
             // твой код
         }

  ШАГ 2. Добавь кнопку в функции OnInit() (ищи надпись "СЮДА ДОБАВЛЯЙ").
     Скопируй этот блок и поменяй название и категорию:

         {
             CHelotyModule &M = AddModule("Название кнопки", HCAT_MISC);
             M.m_OnWorldRender = [this]() { RenderMojaFunkcija(); };
         }

  ШАГ 3. Выбери, КОГДА вызывается функция (нужен один или несколько хуков):

     M.m_OnToggle       Вызывается один раз при клике (On = true, когда включили).
                        Подходит для чего-то разового: включить/выключить
                        настройку, написать сообщение.
                            M.m_OnToggle = [](bool On) {
                                dbg_msg("heloty", "ya %s", On ? "on" : "off");
                            };

     M.m_OnWorldRender  Вызывается КАЖДЫЙ КАДР, пока кнопка включена, в
                        координатах мира игры (как тии и снаряды).
                        Подходит для эффектов вокруг тии (кагуне, свечение).

     M.m_OnScreenRender Вызывается КАЖДЫЙ КАДР, пока кнопка включена, поверх
                        экрана. Высота экрана = 300, ширина = 300 * ScreenAspect.
                        Подходит для погоды, надписей, интерфейса.

  КАТЕГОРИИ: HCAT_RENDER, HCAT_MOVEMENT, HCAT_TEE, HCAT_MISC
     (новую панель: добавь имя в enum в .h и название в gs_apCategoryNames ниже)

  УПРАВЛЕНИЕ МЕНЮ: Right Shift открыть/закрыть. ЛКМ по кнопке включить.
     ЛКМ по заголовку панели таскать. ПКМ по заголовку свернуть/развернуть.
 ============================================================================
*/

#include "heloty_menu.h"

#include <algorithm>
#include <math.h>

#include <base/system.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>

static const char *gs_apCategoryNames[NUM_HCATS] = {"Render", "Movement", "Tee", "Misc"};

static const float PANEL_W = 110.0f;
static const float HEAD_H = 16.0f;
static const float ROW_H = 14.0f;
static const float SCREEN_H = 300.0f;

static bool Inside(vec2 P, float x, float y, float w, float h)
{
	return P.x >= x && P.x < x + w && P.y >= y && P.y < y + h;
}

CHelotyMenu::CHelotyMenu()
{
	m_Active = false;
	m_MousePos = vec2(150.0f, 150.0f);
	m_Dragging = -1;
	m_DragOffset = vec2(0.0f, 0.0f);
	m_Seed = 12345u;
	for(int i = 0; i < NUM_HCATS; i++)
	{
		m_aPanelPos[i] = vec2(20.0f + i * (PANEL_W + 8.0f), 30.0f);
		m_aPanelOpen[i] = true;
	}
}

CHelotyModule &CHelotyMenu::AddModule(const char *pName, int Category)
{
	m_vModules.push_back(CHelotyModule());
	CHelotyModule &Module = m_vModules.back();
	Module.m_Name = pName;
	Module.m_Category = Category;
	return Module;
}

float CHelotyMenu::Rnd()
{
	m_Seed = m_Seed * 1664525u + 1013904223u;
	return (float)(m_Seed >> 8) / 16777216.0f;
}

// ============================================================================
//  СПИСОК ФУНКЦИЙ В МЕНЮ
// ============================================================================
void CHelotyMenu::OnInit()
{
	// ---- Render ----
	{
		CHelotyModule &M = AddModule("Kagune", HCAT_RENDER);
		M.m_OnWorldRender = [this]() { RenderKagune(); };
	}
	{
		CHelotyModule &M = AddModule("Snow", HCAT_RENDER);
		M.m_OnScreenRender = [this]() { RenderSnow(); };
	}
	{
		CHelotyModule &M = AddModule("Rain", HCAT_RENDER);
		M.m_OnScreenRender = [this]() { RenderRain(); };
	}

	// ---- Misc ----
	{
		// KoG Helper: пока заготовка. Скажи, что она должна делать, и я допишу.
		// Код помощника кладём в m_OnToggle (разово) или в m_OnWorldRender /
		// m_OnScreenRender (каждый кадр).
		CHelotyModule &M = AddModule("KoG Helper", HCAT_MISC);
		M.m_OnToggle = [](bool On) {
			dbg_msg("heloty", "KoG Helper: %s", On ? "on" : "off");
		};
	}

	// ---- СЮДА ДОБАВЛЯЙ НОВЫЕ ФУНКЦИИ (скопируй блок ниже, см. инструкцию вверху) ----
	// {
	//     CHelotyModule &M = AddModule("Моя функция", HCAT_MISC);
	//     M.m_OnToggle = [](bool On) { /* что сделать при клике */ };
	//     M.m_OnWorldRender = [this]() { /* рисование в мире, каждый кадр */ };
	//     M.m_OnScreenRender = [this]() { /* рисование на экране, каждый кадр */ };
	// }
}

// ============================================================================
//  ФУНКЦИИ
// ============================================================================

// Кагуне: 4 щупальца вокруг твоей тии (рисуется в мире).
void CHelotyMenu::RenderKagune()
{
	if(!m_pClient->m_Snap.m_pLocalCharacter || m_pClient->m_Snap.HELOTY_LOCAL_ID < 0)
		return;

	const vec2 Center = m_pClient->m_aClients[m_pClient->m_Snap.HELOTY_LOCAL_ID].m_RenderPos;
	const float Time = Client()->LocalTime();

	// Настройки внешнего вида (можно менять):
	const float aAngles[4] = {-2.4f, -0.74f, 2.5f, 0.64f}; // направление каждого щупальца (радианы)
	const float aPhases[4] = {0.0f, 1.6f, 3.1f, 4.7f}; // сдвиг покачивания
	const int Segments = 14; // из скольки частей щупальце
	const float TentacleLength = 70.0f; // длина
	const float WaveSpeed = 3.0f; // скорость покачивания

	Graphics()->TextureClear();
	Graphics()->BlendNormal();
	Graphics()->QuadsBegin();

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
				Graphics()->SetColor(0.55f + 0.4f * T, 0.02f, 0.08f + 0.1f * T, 0.95f - 0.25f * T);
				IGraphics::CFreeformItem Item(PrevL.x, PrevL.y, PrevR.x, PrevR.y, L.x, L.y, R.x, R.y);
				Graphics()->QuadsDrawFreeform(&Item, 1);
			}
			PrevL = L;
			PrevR = R;
		}
	}

	Graphics()->QuadsEnd();
}

// Снег: белые снежинки падают по всему экрану.
void CHelotyMenu::RenderSnow()
{
	const float Width = SCREEN_H * Graphics()->ScreenAspect();
	const float Dt = Client()->RenderFrameTime();
	const float Time = Client()->LocalTime();
	const int Count = 120; // сколько снежинок

	if(m_vSnow.empty())
	{
		for(int i = 0; i < Count; i++)
		{
			CDrop D;
			D.m_X = Rnd() * Width;
			D.m_Y = Rnd() * SCREEN_H;
			D.m_Speed = 15.0f + Rnd() * 25.0f; // скорость падения
			D.m_Size = 1.0f + Rnd() * 1.8f; // размер
			D.m_Phase = Rnd() * 6.28f;
			m_vSnow.push_back(D);
		}
	}

	std::vector<IGraphics::CQuadItem> vItems;
	vItems.reserve(m_vSnow.size());
	for(size_t i = 0; i < m_vSnow.size(); i++)
	{
		CDrop &D = m_vSnow[i];
		D.m_Y += D.m_Speed * Dt;
		D.m_X += sinf(Time + D.m_Phase) * 10.0f * Dt; // лёгкое покачивание
		if(D.m_Y > SCREEN_H)
		{
			D.m_Y = -3.0f;
			D.m_X = Rnd() * Width;
		}
		if(D.m_X < 0.0f)
			D.m_X += Width;
		if(D.m_X > Width)
			D.m_X -= Width;
		vItems.push_back(IGraphics::CQuadItem(D.m_X, D.m_Y, D.m_Size, D.m_Size));
	}

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.85f); // цвет снега
	Graphics()->QuadsDrawTL(&vItems[0], (int)vItems.size());
	Graphics()->QuadsEnd();
}

// Дождь: тонкие косые капли.
void CHelotyMenu::RenderRain()
{
	const float Width = SCREEN_H * Graphics()->ScreenAspect();
	const float Dt = Client()->RenderFrameTime();
	const int Count = 160; // сколько капель

	if(m_vRain.empty())
	{
		for(int i = 0; i < Count; i++)
		{
			CDrop D;
			D.m_X = Rnd() * Width;
			D.m_Y = Rnd() * SCREEN_H;
			D.m_Speed = 170.0f + Rnd() * 100.0f; // скорость
			D.m_Size = 0.6f + Rnd() * 0.6f; // длина (коэффициент)
			D.m_Phase = 0.0f;
			m_vRain.push_back(D);
		}
	}

	std::vector<IGraphics::CFreeformItem> vItems;
	vItems.reserve(m_vRain.size());
	for(size_t i = 0; i < m_vRain.size(); i++)
	{
		CDrop &D = m_vRain[i];
		D.m_Y += D.m_Speed * Dt;
		D.m_X -= D.m_Speed * 0.12f * Dt; // наклон
		if(D.m_Y > SCREEN_H)
		{
			D.m_Y = -8.0f;
			D.m_X = Rnd() * (Width + 40.0f);
		}
		if(D.m_X < -10.0f)
			D.m_X += Width + 20.0f;

		const float Len = 8.0f * D.m_Size;
		const float Dx = -Len * 0.12f;
		vItems.push_back(IGraphics::CFreeformItem(D.m_X, D.m_Y, D.m_X + 0.5f, D.m_Y, D.m_X + Dx, D.m_Y + Len, D.m_X + Dx + 0.5f, D.m_Y + Len));
	}

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.7f, 0.8f, 1.0f, 0.6f); // цвет дождя
	Graphics()->QuadsDrawFreeform(&vItems[0], (int)vItems.size());
	Graphics()->QuadsEnd();
}

// ============================================================================
//  ВВОД, РИСОВАНИЕ МЕНЮ (обычно трогать не нужно)
// ============================================================================

bool CHelotyMenu::HandleMove(float x, float y)
{
	if(!m_Active)
		return false;

	const float Scale = SCREEN_H / (float)Graphics()->ScreenHeight();
	const float Width = SCREEN_H * Graphics()->ScreenAspect();
	m_MousePos += vec2(x, y) * Scale;
	m_MousePos.x = std::max(0.0f, std::min(m_MousePos.x, Width));
	m_MousePos.y = std::max(0.0f, std::min(m_MousePos.y, SCREEN_H));
	return true;
}

#if defined(HELOTY_CURSOR_NEW)
bool CHelotyMenu::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	return HandleMove(x, y);
}
#elif defined(HELOTY_CURSOR_OLD)
bool CHelotyMenu::OnMouseMove(float x, float y)
{
	return HandleMove(x, y);
}
#endif

bool CHelotyMenu::OnInput(IInput::CEvent Event)
{
	if((Event.m_Flags & IInput::FLAG_PRESS) && Event.m_Key == KEY_RSHIFT)
	{
		m_Active = !m_Active;
		m_Dragging = -1;
		return true;
	}

	if(!m_Active)
		return false;

	if((Event.m_Flags & IInput::FLAG_RELEASE) && Event.m_Key == KEY_MOUSE_1)
		m_Dragging = -1;

	if(Event.m_Flags & IInput::FLAG_PRESS)
	{
		if(Event.m_Key == KEY_ESCAPE)
		{
			m_Active = false;
			return true;
		}

		if(Event.m_Key == KEY_MOUSE_1 || Event.m_Key == KEY_MOUSE_2)
		{
			for(int c = 0; c < NUM_HCATS; c++)
			{
				const vec2 P = m_aPanelPos[c];
				if(Inside(m_MousePos, P.x, P.y, PANEL_W, HEAD_H))
				{
					if(Event.m_Key == KEY_MOUSE_1)
					{
						m_Dragging = c;
						m_DragOffset = m_MousePos - P;
					}
					else
						m_aPanelOpen[c] = !m_aPanelOpen[c];
					return true;
				}

				if(!m_aPanelOpen[c] || Event.m_Key != KEY_MOUSE_1)
					continue;

				float Y = P.y + HEAD_H;
				for(size_t i = 0; i < m_vModules.size(); i++)
				{
					CHelotyModule &Module = m_vModules[i];
					if(Module.m_Category != c)
						continue;
					if(Inside(m_MousePos, P.x, Y, PANEL_W, ROW_H))
					{
						Module.m_Enabled = !Module.m_Enabled;
						if(Module.m_OnToggle)
							Module.m_OnToggle(Module.m_Enabled);
						return true;
					}
					Y += ROW_H;
				}
			}
		}
	}
	return true; // пока меню открыто, ввод в игру не уходит
}

void CHelotyMenu::OnRender()
{
	// экранные функции (погода и т.п.) работают, даже когда меню закрыто
	bool Mapped = false;
	for(size_t i = 0; i < m_vModules.size(); i++)
	{
		if(m_vModules[i].m_Enabled && m_vModules[i].m_OnScreenRender)
		{
			if(!Mapped)
			{
				Graphics()->MapScreen(0.0f, 0.0f, SCREEN_H * Graphics()->ScreenAspect(), SCREEN_H);
				Graphics()->BlendNormal();
				Mapped = true;
			}
			m_vModules[i].m_OnScreenRender();
		}
	}

	if(m_Active)
		DrawMenu();
}

void CHelotyMenu::RenderWorld()
{
	for(size_t i = 0; i < m_vModules.size(); i++)
		if(m_vModules[i].m_Enabled && m_vModules[i].m_OnWorldRender)
			m_vModules[i].m_OnWorldRender();
}

void CHelotyWorld::OnRender()
{
	m_pClient->m_HelotyMenu.RenderWorld();
}

void CHelotyMenu::Rect(float x, float y, float w, float h, float r, float g, float b, float a)
{
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(r, g, b, a);
	IGraphics::CQuadItem Item(x, y, w, h);
	Graphics()->QuadsDrawTL(&Item, 1);
	Graphics()->QuadsEnd();
}

void CHelotyMenu::Label(float x, float y, float Size, const char *pText, float r, float g, float b)
{
	TextRender()->TextColor(r, g, b, 1.0f);
	TextRender()->Text(0, x, y, Size, pText, -1);
}

void CHelotyMenu::DrawMenu()
{
	const float Width = SCREEN_H * Graphics()->ScreenAspect();
	Graphics()->MapScreen(0.0f, 0.0f, Width, SCREEN_H);
	Graphics()->BlendNormal();

	Rect(0.0f, 0.0f, Width, SCREEN_H, 0.0f, 0.0f, 0.0f, 0.4f);
	Label(6.0f, 4.0f, 10.0f, "Heloty Client", 0.54f, 0.36f, 1.0f);

	if(m_Dragging >= 0)
		m_aPanelPos[m_Dragging] = m_MousePos - m_DragOffset;

	for(int c = 0; c < NUM_HCATS; c++)
	{
		const vec2 P = m_aPanelPos[c];

		Rect(P.x, P.y, PANEL_W, HEAD_H, 0.12f, 0.12f, 0.18f, 1.0f);
		Rect(P.x, P.y + HEAD_H - 1.5f, PANEL_W, 1.5f, 0.54f, 0.36f, 1.0f, 1.0f);
		Label(P.x + 6.0f, P.y + 3.0f, 9.0f, gs_apCategoryNames[c], 1.0f, 1.0f, 1.0f);

		if(!m_aPanelOpen[c])
			continue;

		float Y = P.y + HEAD_H;
		int Count = 0;
		for(size_t i = 0; i < m_vModules.size(); i++)
		{
			const CHelotyModule &Module = m_vModules[i];
			if(Module.m_Category != c)
				continue;
			Count++;

			Rect(P.x, Y, PANEL_W, ROW_H, 0.07f, 0.07f, 0.11f, 0.92f);
			if(Module.m_Enabled)
				Rect(P.x, Y, 2.5f, ROW_H, 0.54f, 0.36f, 1.0f, 1.0f);
			if(Inside(m_MousePos, P.x, Y, PANEL_W, ROW_H))
				Rect(P.x, Y, PANEL_W, ROW_H, 1.0f, 1.0f, 1.0f, 0.08f);

			if(Module.m_Enabled)
				Label(P.x + 6.0f, Y + 2.5f, 8.0f, Module.m_Name.c_str(), 1.0f, 1.0f, 1.0f);
			else
				Label(P.x + 6.0f, Y + 2.5f, 8.0f, Module.m_Name.c_str(), 0.55f, 0.55f, 0.65f);
			Y += ROW_H;
		}

		if(Count == 0)
		{
			Rect(P.x, Y, PANEL_W, ROW_H, 0.07f, 0.07f, 0.11f, 0.92f);
			Label(P.x + 6.0f, Y + 2.5f, 8.0f, "Empty", 0.55f, 0.55f, 0.65f);
		}
	}

	// курсор
	Rect(m_MousePos.x, m_MousePos.y, 4.0f, 4.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}
