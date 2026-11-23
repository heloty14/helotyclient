// ПАКЕТ: KoG Helper. Пока заготовка. Скажи, что она должна делать, и я допишу.
#include "../heloty_menu.h"

static void HelotyKogToggle(CHelotyMenu &M, bool On)
{
	dbg_msg("heloty", "KoG Helper: %s", On ? "on" : "off");
}

static void HelotyKogScreen(CHelotyMenu &M)
{
	// сюда код помощника: он вызывается каждый кадр, пока кнопка включена
}

HELOTY_FEATURE("KoG Helper", HCAT_MISC, HelotyKogToggle, HelotyKogScreen, 0)
