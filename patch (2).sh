#!/bin/bash
# Запускается из папки с исходниками DDNet: bash patch.sh <папка_с_файлами_меню>
set -e
REPO="$1"
CPP=src/game/client/gameclient.cpp
H=src/game/client/gameclient.h

cp "$REPO/heloty16_menu.h" src/game/client/components/heloty_menu.h
cp "$REPO/heloty16_menu.cpp" src/game/client/components/heloty_menu.cpp

# файлы в список исходников CMake (рядом с players.cpp)
sed -i 's|^\([[:space:]]*\)components/players\.cpp[[:space:]]*$|&\n\1components/heloty_menu.cpp\n\1components/heloty_menu.h|' CMakeLists.txt

# gameclient.h: include и поля
sed -i 's|#include "components/menus.h"|&\n#include "components/heloty_menu.h"|' $H
sed -i 's|CMenus m_Menus;|&\n\tCHelotyMenu m_HelotyMenu;\n\tCHelotyWorld m_HelotyWorld;|' $H

# gameclient.cpp, стиль 1: m_All.Add(...) / m_Input.Add(...)
sed -i 's|m_Input.Add(&m_Menus);|m_Input.Add(\&m_HelotyMenu);\n\t&|' $CPP
sed -i 's|m_All.Add(&m_Menus);|&\n\tm_All.Add(\&m_HelotyMenu);|' $CPP
sed -i 's|m_All.Add(&m_Players);|&\n\tm_All.Add(\&m_HelotyWorld);|' $CPP

# gameclient.cpp, стиль 2: списки в фигурных скобках ({&m_Skins, &m_Menus, ...})
sed -i 's|&m_Menus,|\&m_Menus,\n\t\t\t\t\t\t\t\t\t\t\t\&m_HelotyMenu,|' $CPP
sed -i 's|&m_Players,|\&m_Players,\n\t\t\t\t\t\t\t\t\t\t\t\&m_HelotyWorld,|' $CPP

echo "--- check ---"
grep -n "heloty_menu" CMakeLists.txt || { echo "PATCH FAILED: CMakeLists.txt (components/players.cpp not found)"; exit 1; }
grep -n "Heloty\|heloty" $H || true
grep -q "CHelotyMenu m_HelotyMenu" $H || { echo "PATCH FAILED: gameclient.h member (CMenus m_Menus; not found)"; exit 1; }
grep -q 'heloty_menu.h' $H || { echo "PATCH FAILED: gameclient.h include (components/menus.h not found)"; exit 1; }
grep -n "Heloty" $CPP || true
if ! grep -q "&m_HelotyMenu" $CPP || ! grep -q "&m_HelotyWorld" $CPP; then
	echo "PATCH FAILED: gameclient.cpp registration not found. Lines with m_Menus / m_Players / m_Input / m_All:"
	grep -n "m_Menus\|m_Players\|m_Input\|m_All\|m_vpAll\|m_vpInput" $CPP | head -60 || true
	exit 1
fi
echo "patch ok"
