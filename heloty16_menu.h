#ifndef GAME_CLIENT_COMPONENTS_HELOTY_MENU_H
#define GAME_CLIENT_COMPONENTS_HELOTY_MENU_H

#include <base/vmath.h>
#include <engine/input.h>
#include <game/client/component.h>

#include <functional>
#include <string>
#include <vector>

// Категории (панели) меню. Хочешь новую панель: добавь сюда имя,
// потом добавь название в gs_apCategoryNames в heloty_menu.cpp.
enum
{
	HCAT_RENDER = 0,
	HCAT_MOVEMENT,
	HCAT_TEE,
	HCAT_MISC,
	NUM_HCATS
};

// Одна функция (кнопка) в меню.
struct CHelotyModule
{
	std::string m_Name; // название на кнопке
	int m_Category; // в какой панели лежит
	bool m_Enabled; // включена ли сейчас (переключается кликом)

	// Все три хука необязательные. Заполни только те, что нужны.
	std::function<void(bool)> m_OnToggle; // один раз при клике (true = включили, false = выключили)
	std::function<void()> m_OnScreenRender; // каждый кадр, рисуем поверх экрана (координаты 0..300 по высоте)
	std::function<void()> m_OnWorldRender; // каждый кадр, рисуем в мире игры (координаты карты, как у тии)

	CHelotyModule() :
		m_Category(HCAT_MISC), m_Enabled(false) {}
};

class CHelotyMenu : public CComponent
{
	// --- меню ---
	std::vector<CHelotyModule> m_vModules;
	vec2 m_aPanelPos[NUM_HCATS];
	bool m_aPanelOpen[NUM_HCATS];
	bool m_Active;
	vec2 m_MousePos;
	int m_Dragging;
	vec2 m_DragOffset;

	void Rect(float x, float y, float w, float h, float r, float g, float b, float a);
	void Label(float x, float y, float Size, const char *pText, float r, float g, float b);
	void DrawMenu();

	// --- состояние погоды ---
	struct CDrop
	{
		float m_X, m_Y, m_Speed, m_Size, m_Phase;
	};
	std::vector<CDrop> m_vSnow;
	std::vector<CDrop> m_vRain;
	unsigned m_Seed;
	float Rnd(); // случайное число 0..1

	// --- функции (добавляй свои сюда и в OnInit) ---
	void RenderKagune(); // щупальца вокруг тии (мир)
	void RenderSnow(); // снег (экран)
	void RenderRain(); // дождь (экран)

public:
	CHelotyMenu();

	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual bool OnMouseMove(float x, float y) override;
	virtual bool OnInput(IInput::CEvent Event) override;

	// Добавить кнопку в меню (см. инструкцию в heloty_menu.cpp)
	CHelotyModule &AddModule(const char *pName, int Category);

	// Вызывается из CHelotyWorld: рисует мировые функции
	void RenderWorld();
};

// Отдельный компонент, который стоит в списке рендера сразу после игроков,
// поэтому в нём координаты мира уже настроены. Трогать не нужно.
class CHelotyWorld : public CComponent
{
public:
	virtual void OnRender() override;
};

#endif
