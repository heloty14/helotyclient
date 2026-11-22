#include "heloty_menu.h"

#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/ui.h>

static const char *gs_apCategoryNames[NUM_HCATS] = {"Render", "Movement", "Tee", "Misc"};

static const ColorRGBA gs_Bg(0.07f, 0.07f, 0.11f, 0.92f);
static const ColorRGBA gs_Header(0.12f, 0.12f, 0.18f, 1.0f);
static const ColorRGBA gs_Accent(0.54f, 0.36f, 1.0f, 1.0f);
static const ColorRGBA gs_Hover(1.0f, 1.0f, 1.0f, 0.08f);

static const float PANEL_W = 130.0f;
static const float HEAD_H = 20.0f;
static const float ROW_H = 18.0f;

CHelotyModule &CHelotyMenu::AddModule(const char *pName, int Category)
{
	m_vModules.emplace_back();
	CHelotyModule &Module = m_vModules.back();
	Module.m_Name = pName;
	Module.m_Category = Category;
	return Module;
}

void CHelotyMenu::OnInit()
{
	for(int i = 0; i < NUM_HCATS; i++)
		m_aPanels[i].m_Pos = vec2(30.0f + i * (PANEL_W + 12.0f), 40.0f);

	// ==== СЮДА ДОБАВЛЯЙ ФУНКЦИИ ====
	// Пример:
	// CHelotyModule &Kagune = AddModule("Kagune", HCAT_RENDER);
	// Kagune.m_OnToggle = [this](bool On) { m_pClient->m_Kagune.m_Enabled = On; };
	// Kagune.m_OnRender = [this]() { /* рисование каждый кадр */ };
}

bool CHelotyMenu::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(!m_Active)
		return false;

	Ui()->ConvertMouseMove(&x, &y, CursorType);
	m_MousePos += vec2(x, y);

	const CUIRect *pScreen = Ui()->Screen();
	m_MousePos.x = std::clamp(m_MousePos.x, 0.0f, pScreen->w);
	m_MousePos.y = std::clamp(m_MousePos.y, 0.0f, pScreen->h);
	return true;
}

bool CHelotyMenu::OnInput(const IInput::CEvent &Event)
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
				CPanel &Panel = m_aPanels[c];
				CUIRect Head = {Panel.m_Pos.x, Panel.m_Pos.y, PANEL_W, HEAD_H};
				if(Head.Inside(m_MousePos))
				{
					if(Event.m_Key == KEY_MOUSE_1)
					{
						m_Dragging = c;
						m_DragOffset = m_MousePos - Panel.m_Pos;
					}
					else
						Panel.m_Open = !Panel.m_Open;
					return true;
				}

				if(!Panel.m_Open || Event.m_Key != KEY_MOUSE_1)
					continue;

				float Y = Panel.m_Pos.y + HEAD_H;
				for(auto &Module : m_vModules)
				{
					if(Module.m_Category != c)
						continue;
					CUIRect Row = {Panel.m_Pos.x, Y, PANEL_W, ROW_H};
					if(Row.Inside(m_MousePos))
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
	// включённые модули работают всегда, даже при закрытом меню
	for(auto &Module : m_vModules)
		if(Module.m_Enabled && Module.m_OnRender)
			Module.m_OnRender();

	if(m_Active)
		DrawMenu();
}

void CHelotyMenu::DrawMenu()
{
	Ui()->MapScreen();
	const CUIRect *pScreen = Ui()->Screen();

	CUIRect Full = *pScreen;
	Full.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.4f), IGraphics::CORNER_NONE, 0.0f);

	CUIRect Title = {8.0f, 6.0f, 200.0f, 14.0f};
	TextRender()->TextColor(gs_Accent);
	Ui()->DoLabel(&Title, "Heloty Client", 12.0f, TEXTALIGN_ML);

	if(m_Dragging >= 0)
		m_aPanels[m_Dragging].m_Pos = m_MousePos - m_DragOffset;

	for(int c = 0; c < NUM_HCATS; c++)
	{
		CPanel &Panel = m_aPanels[c];
		CUIRect Head = {Panel.m_Pos.x, Panel.m_Pos.y, PANEL_W, HEAD_H};
		Head.Draw(gs_Header, IGraphics::CORNER_T, 4.0f);

		CUIRect Line = {Head.x, Head.y + HEAD_H - 1.5f, PANEL_W, 1.5f};
		Line.Draw(gs_Accent, IGraphics::CORNER_NONE, 0.0f);

		CUIRect HeadText = {Head.x + 8.0f, Head.y, PANEL_W - 8.0f, HEAD_H};
		TextRender()->TextColor(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
		Ui()->DoLabel(&HeadText, gs_apCategoryNames[c], 10.0f, TEXTALIGN_ML);

		if(!Panel.m_Open)
			continue;

		float Y = Panel.m_Pos.y + HEAD_H;
		int Count = 0;
		for(auto &Module : m_vModules)
		{
			if(Module.m_Category != c)
				continue;
			Count++;

			CUIRect Row = {Panel.m_Pos.x, Y, PANEL_W, ROW_H};
			Row.Draw(gs_Bg, IGraphics::CORNER_NONE, 0.0f);
			if(Module.m_Enabled)
			{
				CUIRect Mark = {Row.x, Row.y, 2.5f, ROW_H};
				Mark.Draw(gs_Accent, IGraphics::CORNER_NONE, 0.0f);
			}
			if(Row.Inside(m_MousePos))
				Row.Draw(gs_Hover, IGraphics::CORNER_NONE, 0.0f);

			CUIRect Text = {Row.x + 8.0f, Row.y, PANEL_W - 8.0f, ROW_H};
			TextRender()->TextColor(Module.m_Enabled ? ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f) : ColorRGBA(0.55f, 0.55f, 0.65f, 1.0f));
			Ui()->DoLabel(&Text, Module.m_Name.c_str(), 9.0f, TEXTALIGN_ML);
			Y += ROW_H;
		}

		if(Count == 0)
		{
			CUIRect Row = {Panel.m_Pos.x, Y, PANEL_W, ROW_H};
			Row.Draw(gs_Bg, IGraphics::CORNER_NONE, 0.0f);
			CUIRect Text = {Row.x + 8.0f, Row.y, PANEL_W - 8.0f, ROW_H};
			TextRender()->TextColor(ColorRGBA(0.55f, 0.55f, 0.65f, 1.0f));
			Ui()->DoLabel(&Text, "Пусто", 9.0f, TEXTALIGN_ML);
			Y += ROW_H;
		}

		// скруглённый низ панели
		CUIRect Bottom = {Panel.m_Pos.x, Y, PANEL_W, 4.0f};
		Bottom.Draw(gs_Bg, IGraphics::CORNER_B, 4.0f);
	}

	TextRender()->TextColor(TextRender()->DefaultTextColor());

	// курсор
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	IGraphics::CQuadItem Cursor(m_MousePos.x, m_MousePos.y, 5.0f, 5.0f);
	Graphics()->QuadsDrawTL(&Cursor, 1);
	Graphics()->QuadsEnd();
}
