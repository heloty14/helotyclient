#ifndef GAME_CLIENT_COMPONENTS_HELOTY_MENU_H
#define GAME_CLIENT_COMPONENTS_HELOTY_MENU_H

#include <base/vmath.h>
#include <game/client/component.h>

#include <functional>
#include <string>
#include <vector>

enum EHelotyCategory
{
	HCAT_RENDER = 0,
	HCAT_MOVEMENT,
	HCAT_TEE,
	HCAT_MISC,
	NUM_HCATS
};

struct CHelotyModule
{
	std::string m_Name;
	int m_Category = HCAT_MISC;
	bool m_Enabled = false;
	std::function<void(bool)> m_OnToggle; // вызывается при вкл/выкл
	std::function<void()> m_OnRender; // вызывается каждый кадр, пока модуль включён
};

class CHelotyMenu : public CComponent
{
	struct CPanel
	{
		vec2 m_Pos;
		bool m_Open = true;
	};

	std::vector<CHelotyModule> m_vModules;
	CPanel m_aPanels[NUM_HCATS];
	bool m_Active = false;
	vec2 m_MousePos = vec2(300.0f, 200.0f);
	int m_Dragging = -1;
	vec2 m_DragOffset = vec2(0.0f, 0.0f);

	void DrawMenu();

public:
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;
	virtual bool OnInput(const IInput::CEvent &Event) override;

	// Добавить функцию в меню
	CHelotyModule &AddModule(const char *pName, int Category);
};

#endif
