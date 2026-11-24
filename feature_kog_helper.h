// feature_kog_helper.h — KoG Helper (свап на молот при атаке и возврат оружия) для DDNet 16.5

namespace FeatureKogHelper
{
	static bool s_Enabled = false;
	static int s_LastWeapon = -1;
	static bool s_Swapped = false;
	static int s_LastFire = 0;
	static float s_ReturnTimer = 0.0f;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		s_Enabled = On;
		s_LastWeapon = -1;
		s_Swapped = false;
		s_ReturnTimer = 0.0f;
	}

	static void OnWorldRender(CHelotyMenu &Menu)
	{
		if(!s_Enabled)
			return;

		CGameClient *pClient = Menu.m_pClient;
		if(!pClient || !pClient->m_Snap.m_pLocalCharacter)
			return;

		const CNetObj_Character *pChar = pClient->m_Snap.m_pLocalCharacter;
		const int CurWeapon = pChar->m_Weapon;

		// Читаем текущее состояние кнопки атаки из контролов клиента
		const int Fire = pClient->m_Controls.m_InputData[g_Config.m_ClDummy].m_Fire;
		const bool FirePressed = (Fire & 1) != 0 && (s_LastFire & 1) == 0;
		const bool FireHeld = (Fire & 1) != 0;
		s_LastFire = Fire;

		// 1. Игрок нажал атаку, держа НЕ молот (WEAPON_HAMMER == 0) и НЕ ниндзю (WEAPON_NINJA == 5)
		if(FirePressed && CurWeapon != WEAPON_HAMMER && CurWeapon != WEAPON_NINJA && !s_Swapped)
		{
			s_LastWeapon = CurWeapon;
			s_Swapped = true;
			s_ReturnTimer = 0.14f; // задержка на срабатывание удара молотом (~140 мс)

			// Переключаем на молот (в консоли DDNet: +weapon 1 — молот, +weapon 2 — пистолет и т.д.)
			Menu.Exec("+weapon 1");
			return;
		}

		// 2. Возвращаем прошлое оружие после удара и отпускания кнопки огня
		if(s_Swapped)
		{
			if(s_ReturnTimer > 0.0f)
				s_ReturnTimer -= Menu.FrameTime();

			if(s_ReturnTimer <= 0.0f && !FireHeld)
			{
				if(s_LastWeapon >= 0 && s_LastWeapon != WEAPON_HAMMER)
				{
					char aCmd[32];
					str_format(aCmd, sizeof(aCmd), "+weapon %d", s_LastWeapon + 1);
					Menu.Exec(aCmd);
				}
				s_Swapped = false;
				s_LastWeapon = -1;
			}
		}
	}
}

static struct CRegKogHelper
{
	CRegKogHelper()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "KoG Helper";
		Def.m_Category = HCAT_MISC;
		Def.m_pfnToggle = FeatureKogHelper::OnToggle;
		Def.m_pfnWorld = FeatureKogHelper::OnWorldRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegKogHelper;
