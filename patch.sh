#!/bin/bash
# Запускается из папки с исходниками DDNet: bash patch.sh <папка_с_файлами_меню>
set -e
REPO="$1"

cp "$REPO/heloty16_menu.h" src/game/client/components/heloty_menu.h
cp "$REPO/heloty16_menu.cpp" src/game/client/components/heloty_menu.cpp

# файлы в список исходников CMake (рядом с players.cpp)
sed -i 's|^\([[:space:]]*\)components/players\.cpp[[:space:]]*$|&\n\1components/heloty_menu.cpp\n\1components/heloty_menu.h|' CMakeLists.txt

# gameclient.h: include и поля
sed -i 's|#include "components/menus.h"|&\n#include "components/heloty_menu.h"|' src/game/client/gameclient.h
sed -i 's|CMenus m_Menus;|&\n\tCHelotyMenu m_HelotyMenu;\n\tCHelotyWorld m_HelotyWorld;|' src/game/client/gameclient.h

# gameclient.cpp: ввод и рисование меню, мировой компонент сразу после игроков
sed -i 's|m_Input.Add(&m_Menus);|m_Input.Add(\&m_HelotyMenu);\n\t&|' src/game/client/gameclient.cpp
sed -i 's|m_All.Add(&m_Menus);|&\n\tm_All.Add(\&m_HelotyMenu);|' src/game/client/gameclient.cpp
sed -i 's|m_All.Add(&m_Players);|&\n\tm_All.Add(\&m_HelotyWorld);|' src/game/client/gameclient.cpp

echo "--- check ---"
grep -n "heloty_menu" CMakeLists.txt || { echo "PATCH FAILED: CMakeLists.txt (components/players.cpp not found)"; exit 1; }
grep -n "Heloty\|heloty" src/game/client/gameclient.h
grep -q "CHelotyMenu m_HelotyMenu" src/game/client/gameclient.h || { echo "PATCH FAILED: gameclient.h member (CMenus m_Menus; not found)"; exit 1; }
grep -q 'heloty_menu.h' src/game/client/gameclient.h || { echo "PATCH FAILED: gameclient.h include (components/menus.h not found)"; exit 1; }
grep -n "Heloty" src/game/client/gameclient.cpp
grep -q "m_Input.Add(&m_HelotyMenu)" src/game/client/gameclient.cpp || { echo "PATCH FAILED: gameclient.cpp input (m_Input.Add(&m_Menus) not found)"; exit 1; }
grep -q "m_All.Add(&m_HelotyMenu)" src/game/client/gameclient.cpp || { echo "PATCH FAILED: gameclient.cpp render menu (m_All.Add(&m_Menus) not found)"; exit 1; }
grep -q "m_All.Add(&m_HelotyWorld)" src/game/client/gameclient.cpp || { echo "PATCH FAILED: gameclient.cpp world (m_All.Add(&m_Players) not found)"; exit 1; }
echo "patch ok"
