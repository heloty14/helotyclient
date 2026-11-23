/*
 ============================================================================
  HELOTY CLIENT: КАК ДОБАВИТЬ ФУНКЦИЮ (ПАКЕТ)
 ============================================================================

  Каждая функция это отдельный файл-пакет  feature_<имя>.h  в корне
  репозитория. Сборка САМА подключает все такие файлы через #include
  (смотри patch.sh), ничего больше править не нужно:

      1. Создай файл feature_moja.h (образец: feature_template.txt).
      2. Загрузи его в корень репозитория.
      3. Сборка подключит его, и кнопка появится в меню.

  Нажатие на кнопку в игре подключает пакет (его хуки начинают работать),
  повторное нажатие отключает. Удалил файл, пакет исчез из клиента.

  В файле-пакете пишется функция и одна строка регистрации:

     Переключатель (вкл/выкл), хуки можно не использовать, ставь 0:
         HELOTY_FEATURE("Название", HCAT_RENDER, ToggleFn, ScreenFn, WorldFn)
         ToggleFn(CHelotyMenu &M, bool On)  один раз при клике
         ScreenFn(CHelotyMenu &M)           каждый кадр, поверх экрана
                                            (высота = 300, ширина = M.ScreenW())
         WorldFn(CHelotyMenu &M)            каждый кадр, в мире игры

     Кнопка-действие (как exec): нажал, выполнилось, кнопка выключилась:
         HELOTY_COMMAND("Название", HCAT_MISC, "kill")
         (несколько команд через ;  например "exec my.cfg; echo ok")

  Чем пользоваться внутри пакета (объект M):
         M.LocalPos(&Pos)   позиция твоей тии в мире (false, если тии нет)
         M.Gfx()            графика (QuadsBegin, SetColor, QuadsDrawTL ...)
         M.Rect(x,y,w,h,r,g,b,a)   прямоугольник
         M.Label(x,y,размер,"текст",r,g,b)   текст
         M.Time(), M.FrameTime(), M.ScreenW(), M.Rnd(), M.Exec("команда")

  КАТЕГОРИИ: HCAT_RENDER, HCAT_MOVEMENT, HCAT_TEE, HCAT_MISC, HCAT_MENU

  УПРАВЛЕНИЕ: Right Shift открыть. Esc закрыть. ЛКМ по кнопке нажать.
     ЛКМ по заголовку панели таскать. ПКМ по заголовку свернуть/развернуть.

  РАЗРЕШЕНИЕ: размеры считаются от высоты экрана, поэтому меню выглядит
     одинаково на любом разрешении. При смене разрешения панели
     расставляются заново, а если не помещаются, переносятся на новую строку.
     Размер можно поменять кнопкой "Scale" в панели Menu.
 ============================================================================
*/

#include "heloty_menu.h"

#include <algorithm>
#include <math.h>
#include <time.h>

#include <base/system.h>
#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>

static const char *gs_apCategoryNames[NUM_HCATS] = {"Render", "Movement", "Tee", "Misc", "Menu"};

static const float PANEL_W = 110.0f;
static const float HEAD_H = 16.0f;
static const float ROW_H = 14.0f;
static const float SCREEN_H = 300.0f;

// цветовые темы меню
struct CHelotyTheme
{
	const char *m_pName;
	float m_R, m_G, m_B;
};
static const CHelotyTheme gs_aThemes[] = {
};
static const int NUM_THEMES = 6;
static const float gs_aScales[] = {0.8f, 1.0f, 1.25f, 1.5f};
static const int NUM_SCALES = 4;
static const float gs_aOpacity[] = {0.6f, 0.75f, 0.92f, 1.0f};
static const int NUM_OPACITY = 4;

static bool Inside(vec2 P, float x, float y, float w, float h)
{
	return P.x >= x && P.x < x + w && P.y >= y && P.y < y + h;
}

// реестр пакетов-функций (заполняется регистрацией в feature_*.h)
std::vector<CHelotyFeatureDef> &HelotyFeatureRegistry()
{
	static std::vector<CHelotyFeatureDef> s_vRegistry;
	return s_vRegistry;
}

CHelotyMenu::CHelotyMenu()
{
	m_Active = false;
	m_MousePos = vec2(150.0f, 150.0f);
	m_Dragging = -1;
	m_DragOffset = vec2(0.0f, 0.0f);
	m_EscWasDown = false;
	m_Seed = 12345u;

	m_Theme = 0;
	m_ScaleIdx = 1;
	m_OpacityIdx = 2;
	m_Dim = true;
	m_VW = 480.0f;
	m_VH = SCREEN_H;
	m_NeedLayout = true;
	m_LastW = 0;
	m_LastH = 0;
	for(int i = 0; i < NUM_HCATS; i++)
	{
		m_aPanelPos[i] = vec2(10.0f, 22.0f);
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

CHelotyModule &CHelotyMenu::AddButton(const char *pName, int Category, std::function<void()> Action)
{
	CHelotyModule &Module = AddModule(pName, Category);
	Module.m_Button = true;
	Module.m_OnToggle = [Action](bool) { Action(); };
	return Module;
}

CHelotyModule &CHelotyMenu::AddCommand(const char *pName, int Category, const char *pConsoleLine)
{
	const std::string Line = pConsoleLine;
	return AddButton(pName, Category, [this, Line]() { Exec(Line.c_str()); });
}

// ============================================================================
//  API ДЛЯ ПАКЕТОВ-ФУНКЦИЙ
// ============================================================================
IGraphics *CHelotyMenu::Gfx() { return Graphics(); }
ITextRender *CHelotyMenu::Txt() { return TextRender(); }
float CHelotyMenu::Time() { return Client()->LocalTime(); }
float CHelotyMenu::FrameTime() { return Client()->RenderFrameTime(); }
float CHelotyMenu::ScreenW() { return SCREEN_H * Graphics()->ScreenAspect(); }
float CHelotyMenu::ScreenH() { return SCREEN_H; }

bool CHelotyMenu::LocalPos(vec2 *pPos)
{
	if(!m_pClient->m_Snap.m_pLocalCharacter || m_pClient->m_Snap.HELOTY_LOCAL_ID < 0)
		return false;
	*pPos = m_pClient->m_aClients[m_pClient->m_Snap.HELOTY_LOCAL_ID].m_RenderPos;
	return true;
}

void CHelotyMenu::Exec(const char *pLine)
{
	m_pClient->Console()->ExecuteLine(pLine);
}

float CHelotyMenu::Rnd()
{
	m_Seed = m_Seed * 1664525u + 1013904223u;
	return (float)(m_Seed >> 8) / 16777216.0f;
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

// ============================================================================
//  ПАКЕТЫ-ФУНКЦИИ: patch.sh собирает сюда все feature_*.h через #include
// ============================================================================
#include "heloty_features.inc"

// ============================================================================
//  СПИСОК КНОПОК В МЕНЮ
// ============================================================================
void CHelotyMenu::OnInit()
{
	// ---- пакеты-функции (feature_*.h) ----
	const std::vector<CHelotyFeatureDef> &vFeatures = HelotyFeatureRegistry();
	for(size_t i = 0; i < vFeatures.size(); i++)
	{
		const CHelotyFeatureDef Def = vFeatures[i];
		if(Def.m_pCommand)
		{
			AddCommand(Def.m_pName, Def.m_Category, Def.m_pCommand);
			continue;
		}

		CHelotyModule &M = AddModule(Def.m_pName, Def.m_Category);
		if(Def.m_pfnToggle)
		{
			void (*Fn)(CHelotyMenu &, bool) = Def.m_pfnToggle;
			M.m_OnToggle = [this, Fn](bool On) { Fn(*this, On); };
		}
		if(Def.m_pfnScreen)
		{
			void (*Fn)(CHelotyMenu &) = Def.m_pfnScreen;
			M.m_OnScreenRender = [this, Fn]() { Fn(*this); };
		}
		if(Def.m_pfnWorld)
		{
			void (*Fn)(CHelotyMenu &) = Def.m_pfnWorld;
			M.m_OnWorldRender = [this, Fn]() { Fn(*this); };
		}
	}

	// ---- Menu (кастомизация самого меню) ----
	{
		CHelotyModule &M = AddButton("Theme", HCAT_MENU, [this]() { m_Theme = (m_Theme + 1) % NUM_THEMES; });
		M.m_GetLabel = [this]() { return std::string("Theme: ") + gs_aThemes[m_Theme].m_pName; };
	}
	{
		CHelotyModule &M = AddButton("Scale", HCAT_MENU, [this]() {
			m_ScaleIdx = (m_ScaleIdx + 1) % NUM_SCALES;
			m_NeedLayout = true;
		});
		M.m_GetLabel = [this]() {
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "Scale: %d%%", (int)(gs_aScales[m_ScaleIdx] * 100.0f + 0.5f));
			return std::string(aBuf);
		};
	}
	{
		CHelotyModule &M = AddButton("Opacity", HCAT_MENU, [this]() { m_OpacityIdx = (m_OpacityIdx + 1) % NUM_OPACITY; });
		M.m_GetLabel = [this]() {
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "Opacity: %d%%", (int)(gs_aOpacity[m_OpacityIdx] * 100.0f + 0.5f));
			return std::string(aBuf);
		};
	}
	{
		CHelotyModule &M = AddButton("Dim", HCAT_MENU, [this]() { m_Dim = !m_Dim; });
		M.m_GetLabel = [this]() { return std::string("Dim: ") + (m_Dim ? "On" : "Off"); };
	}
	AddButton("Reset panels", HCAT_MENU, [this]() { m_NeedLayout = true; });
}

// ============================================================================
//  РАСКЛАДКА И ВВОД (обычно трогать не нужно)
// ============================================================================

// Размер "виртуального экрана" меню. Считается от высоты окна, поэтому
// на любом разрешении меню выглядит одинаково. При смене разрешения
// панели расставляются заново.
void CHelotyMenu::UpdateView()
{
	m_VH = SCREEN_H / gs_aScales[m_ScaleIdx];
	m_VW = m_VH * Graphics()->ScreenAspect();

	const int W = Graphics()->ScreenWidth();
	const int H = Graphics()->ScreenHeight();
	if(W != m_LastW || H != m_LastH)
	{
		m_LastW = W;
		m_LastH = H;
		m_NeedLayout = true;
	}
}

float CHelotyMenu::PanelHeight(int Category)
{
	int Count = 0;
	for(size_t i = 0; i < m_vModules.size(); i++)
		if(m_vModules[i].m_Category == Category)
			Count++;
	return HEAD_H + (float)std::max(1, Count) * ROW_H;
}

// Расставляет панели в ряд, если не помещаются, переносит на следующую строку.
void CHelotyMenu::LayoutPanels()
{
	float x = 10.0f;
	float y = 22.0f;
	float RowMax = 0.0f;
	for(int c = 0; c < NUM_HCATS; c++)
	{
		if(x + PANEL_W > m_VW - 6.0f && x > 10.0f)
		{
			x = 10.0f;
			y += RowMax + 8.0f;
			RowMax = 0.0f;
		}
		m_aPanelPos[c] = vec2(x, y);
		x += PANEL_W + 8.0f;
		RowMax = std::max(RowMax, PanelHeight(c));
	}
	m_NeedLayout = false;
}

bool CHelotyMenu::HandleMove(float x, float y)
{
	if(!m_Active)
		return false;

	UpdateView();
	const float Scale = m_VH / (float)Graphics()->ScreenHeight();
	m_MousePos += vec2(x, y) * Scale;
	m_MousePos.x = std::max(0.0f, std::min(m_MousePos.x, m_VW));
	m_MousePos.y = std::max(0.0f, std::min(m_MousePos.y, m_VH));
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
			m_Dragging = -1;
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
						if(Module.m_Button)
						{
							// кнопка-действие: сработала один раз и "выключилась"
							Module.m_FlashUntil = Client()->LocalTime() + 0.25f;
							if(Module.m_OnToggle)
								Module.m_OnToggle(true);
						}
						else
						{
							Module.m_Enabled = !Module.m_Enabled;
							if(Module.m_OnToggle)
								Module.m_OnToggle(Module.m_Enabled);
						}
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
	UpdateView();
	if(m_NeedLayout)
		LayoutPanels();

#ifdef HELOTY_HAS_KEYISPRESSED
	// запасной способ закрыть меню по Esc, если нажатие дошло не до нас
	const bool EscDown = Input()->KeyIsPressed(KEY_ESCAPE);
	if(m_Active && EscDown && !m_EscWasDown)
	{
		m_Active = false;
		m_Dragging = -1;
	}
	m_EscWasDown = EscDown;
#endif

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
	if(Mapped)
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

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

void CHelotyMenu::DrawMenu()
{
	const CTheme &T = gs_aThemes[m_Theme];
	const float Op = gs_aOpacity[m_OpacityIdx];
	const float Now = Client()->LocalTime();

	Graphics()->MapScreen(0.0f, 0.0f, m_VW, m_VH);
	Graphics()->BlendNormal();

	if(m_Dim)
		Rect(0.0f, 0.0f, m_VW, m_VH, 0.0f, 0.0f, 0.0f, 0.4f);
	Label(6.0f, 4.0f, 10.0f, "Heloty Client", T.m_R, T.m_G, T.m_B);

	if(m_Dragging >= 0)
		m_aPanelPos[m_Dragging] = m_MousePos - m_DragOffset;

	for(int c = 0; c < NUM_HCATS; c++)
	{
		// панель не должна уезжать за экран
		m_aPanelPos[c].x = std::max(0.0f, std::min(m_aPanelPos[c].x, m_VW - PANEL_W));
		m_aPanelPos[c].y = std::max(0.0f, std::min(m_aPanelPos[c].y, m_VH - HEAD_H));
		const vec2 P = m_aPanelPos[c];

		Rect(P.x, P.y, PANEL_W, HEAD_H, 0.12f, 0.12f, 0.18f, Op);
		Rect(P.x, P.y + HEAD_H - 1.5f, PANEL_W, 1.5f, T.m_R, T.m_G, T.m_B, 1.0f);
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

			const bool On = Module.m_Button ? (Module.m_FlashUntil > Now) : Module.m_Enabled;

			Rect(P.x, Y, PANEL_W, ROW_H, 0.07f, 0.07f, 0.11f, Op);
			if(On)
				Rect(P.x, Y, 2.5f, ROW_H, T.m_R, T.m_G, T.m_B, 1.0f);
			if(Inside(m_MousePos, P.x, Y, PANEL_W, ROW_H))
				Rect(P.x, Y, PANEL_W, ROW_H, 1.0f, 1.0f, 1.0f, 0.08f);

			const std::string Text = Module.m_GetLabel ? Module.m_GetLabel() : Module.m_Name;
			if(On)
				Label(P.x + 6.0f, Y + 2.5f, 8.0f, Text.c_str(), 1.0f, 1.0f, 1.0f);
			else
				Label(P.x + 6.0f, Y + 2.5f, 8.0f, Text.c_str(), 0.55f, 0.55f, 0.65f);
			Y += ROW_H;
		}

		if(Count == 0)
		{
			Rect(P.x, Y, PANEL_W, ROW_H, 0.07f, 0.07f, 0.11f, Op);
			Label(P.x + 6.0f, Y + 2.5f, 8.0f, "Empty", 0.55f, 0.55f, 0.65f);
		}
	}

	// курсор
	const float Cur = 4.0f * (m_VH / SCREEN_H);
	Rect(m_MousePos.x, m_MousePos.y, Cur, Cur, 1.0f, 1.0f, 1.0f, 1.0f);

	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}
