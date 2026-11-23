#ifndef GAME_CLIENT_COMPONENTS_HELOTY_MENU_H
#define GAME_CLIENT_COMPONENTS_HELOTY_MENU_H

#include <base/vmath.h>
#include <engine/input.h>
#include <game/client/component.h>

// heloty_cfg.h создаёт patch.sh во время сборки: он смотрит, какие методы есть в этой версии DDNet
#include "heloty_cfg.h"

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
	HCAT_MENU,
	NUM_HCATS
};

class CHelotyMenu;

// ============================================================================
//  ПАКЕТЫ-ФУНКЦИИ
//  Каждая функция лежит в своём файле feature_<имя>.h в корне репозитория.
//  Сборка подключает такие файлы через #include (см. patch.sh), а кнопка в
//  меню включает и выключает пакет прямо в игре.
// ============================================================================
struct CHelotyFeatureDef
{
	const char *m_pName; // название кнопки
	int m_Category; // панель
	void (*m_pfnToggle)(CHelotyMenu &, bool); // при клике (или 0)
	void (*m_pfnScreen)(CHelotyMenu &); // каждый кадр поверх экрана (или 0)
	void (*m_pfnWorld)(CHelotyMenu &); // каждый кадр в мире игры (или 0)
	const char *m_pCommand; // консольная команда для кнопки-действия (или 0)
};

std::vector<CHelotyFeatureDef> &HelotyFeatureRegistry();

struct CHelotyFeatureRegistrar
{
	CHelotyFeatureRegistrar(const CHelotyFeatureDef &Def) { HelotyFeatureRegistry().push_back(Def); }
};

#define HELOTY_JOIN2(a, b) a##b
#define HELOTY_JOIN(a, b) HELOTY_JOIN2(a, b)

// Регистрирует переключатель. Ненужные хуки пиши как 0.
//   HELOTY_FEATURE("Название", HCAT_RENDER, ToggleFn, ScreenFn, WorldFn)
#define HELOTY_FEATURE(Name, Category, ToggleFn, ScreenFn, WorldFn) \
	static CHelotyFeatureRegistrar HELOTY_JOIN(g_HelotyFeature, __COUNTER__)(CHelotyFeatureDef{Name, Category, ToggleFn, ScreenFn, WorldFn, 0});

// Регистрирует кнопку-действие, которая выполняет консольную команду (как exec).
//   HELOTY_COMMAND("Название", HCAT_MISC, "kill")
#define HELOTY_COMMAND(Name, Category, Command) \
	static CHelotyFeatureRegistrar HELOTY_JOIN(g_HelotyFeature, __COUNTER__)(CHelotyFeatureDef{Name, Category, 0, 0, 0, Command});

// Одна кнопка в меню.
struct CHelotyModule
{
	std::string m_Name; // название на кнопке
	int m_Category; // в какой панели лежит
	bool m_Enabled; // включена ли сейчас (для обычных переключателей)
	bool m_Button; // true = кнопка-действие: срабатывает один раз и сама "выключается" (как exec)
	float m_FlashUntil; // до какого времени кнопка-действие подсвечена после нажатия

	std::function<void(bool)> m_OnToggle; // один раз при клике (true = включили, false = выключили)
	std::function<void()> m_OnScreenRender; // каждый кадр, рисуем поверх экрана (высота экрана = 300)
	std::function<void()> m_OnWorldRender; // каждый кадр, рисуем в мире игры (координаты карты)
	std::function<std::string()> m_GetLabel; // текст на кнопке, который меняется на лету (необязательно)

	CHelotyModule() :
		m_Category(HCAT_MISC), m_Enabled(false), m_Button(false), m_FlashUntil(0.0f) {}
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
	bool m_EscWasDown;
	unsigned m_Seed;

	// --- вид меню (кастомизация) ---
	int m_Theme; // цвет акцента
	int m_ScaleIdx; // размер меню
	int m_OpacityIdx; // прозрачность панелей
	bool m_Dim; // затемнять экран под меню
	float m_VW, m_VH; // размер "виртуального экрана" меню (считается из разрешения)
	bool m_NeedLayout; // нужно заново расставить панели
	int m_LastW, m_LastH; // прошлое разрешение окна

	void UpdateView();
	void LayoutPanels();
	float PanelHeight(int Category);
	bool HandleMove(float x, float y); // движение мыши (общая часть)
	void DrawMenu();

public:
	CHelotyMenu();

#ifdef HELOTY_HAS_SIZEOF
	virtual int Sizeof() const override { return sizeof(*this); }
#endif
	virtual void OnInit() override;
	virtual void OnRender() override;
#if defined(HELOTY_CURSOR_NEW)
	virtual bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;
#elif defined(HELOTY_CURSOR_OLD)
	virtual bool OnMouseMove(float x, float y) override;
#endif
	virtual bool OnInput(IInput::CEvent Event) override;

	// Добавить переключатель (вкл/выкл) в меню
	CHelotyModule &AddModule(const char *pName, int Category);
	// Добавить кнопку-действие: нажал, она сработала один раз и сама "выключилась"
	CHelotyModule &AddButton(const char *pName, int Category, std::function<void()> Action);
	// Добавить кнопку, которая выполняет консольную команду (как exec)
	CHelotyModule &AddCommand(const char *pName, int Category, const char *pConsoleLine);

	// Вызывается из CHelotyWorld: рисует мировые функции
	void RenderWorld();

	// ========================================================================
	//  API ДЛЯ ПАКЕТОВ-ФУНКЦИЙ (то, чем пользуются файлы feature_*.h)
	// ========================================================================
	class IGraphics *Gfx(); // графика
	class ITextRender *Txt(); // текст
	float Time(); // секунды с запуска игры
	float FrameTime(); // длительность кадра в секундах
	float ScreenW(); // ширина экрана для экранных функций (высота всегда 300)
	float ScreenH(); // высота экрана для экранных функций (300)
	bool LocalPos(vec2 *pPos); // позиция твоей тии в мире (false, если тии сейчас нет)
	void Exec(const char *pLine); // выполнить консольную команду
	void Rect(float x, float y, float w, float h, float r, float g, float b, float a); // залитый прямоугольник
	void Label(float x, float y, float Size, const char *pText, float r, float g, float b); // текст
	float Rnd(); // случайное число от 0 до 1
};

// Отдельный компонент, который стоит в списке рендера сразу после игроков,
// поэтому в нём координаты мира уже настроены. Трогать не нужно.
class CHelotyWorld : public CComponent
{
public:
#ifdef HELOTY_HAS_SIZEOF
	virtual int Sizeof() const override { return sizeof(*this); }
#endif
	virtual void OnRender() override;
};

#endif
