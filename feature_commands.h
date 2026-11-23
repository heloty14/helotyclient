// ПАКЕТ: кнопки-команды (как exec). Нажал: команда выполнилась, кнопка выключилась.
// Добавить свою кнопку: одна строка HELOTY_COMMAND("Название", HCAT_MISC, "команда")
// Несколько команд подряд пиши через ;  например "exec my.cfg; echo ok"
#include "../heloty_menu.h"

HELOTY_COMMAND("Kill", HCAT_MISC, "kill")
HELOTY_COMMAND("Reconnect", HCAT_MISC, "reconnect")
HELOTY_COMMAND("Screenshot", HCAT_MISC, "screenshot")
