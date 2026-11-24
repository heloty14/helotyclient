// feature_keystrokes.h — HUD нажатых клавиш и спидометр для DDNet 16.5

namespace FeatureKeystrokes
{
	static bool s_Enabled = false;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		s_Enabled = On;
	}

	static void DrawKey(CHelotyMenu &Menu, float x, float y, float w, float h, const char *pLabel, bool Active)
	{
		// Фон клавиши
		if(Active)
			Menu.Rect(x, y, w, h, 0.25f, 0.65f, 1.0f, 0.85f);
		else
			Menu.Rect(x, y, w, h, 0.08f, 0.09f, 0.12f, 0.65f);

		// Тонкая верхняя рамка
		Menu.Rect(x, y, w, 1.0f, Active ? 0.8f : 0.35f, Active ? 0.95f : 0.4f, 1.0f, 0.8f);

		// Текст по центру клавиши
		const float FontSize = 6.5f;
		const float TextX = x + w * 0.5f - (float)str_length(pLabel) * FontSize * 0.28f;
		const float TextY = y + (h - FontSize) * 0.5f - 1.0f;
		Menu.Label(TextX, TextY, FontSize, pLabel, Active ? 0.05f : 0.9f, Active ? 0.05f : 0.9f, Active ? 0.08f : 0.95f);
	}

	static void OnScreenRender(CHelotyMenu &Menu)
	{
		if(!s_Enabled)
			return;

		CGameClient *pClient = Menu.m_pClient;
		if(!pClient)
			return;

		const CNetObj_PlayerInput &Input = pClient->m_Controls.m_InputData[g_Config.m_ClDummy];
		const bool Left = Input.m_Direction < 0;
		const bool Right = Input.m_Direction > 0;
		const bool Jump = Input.m_Jump != 0;
		const bool Fire = (Input.m_Fire & 1) != 0;
		const bool Hook = Input.m_Hook != 0;

		const float H = Menu.ScreenH();
		const float BaseX = 8.0f;
		const float BaseY = H - 64.0f;
		const float KeyW = 20.0f;
		const float KeyH = 16.0f;
		const float Gap = 2.0f;

		// Ряд 1: A | JUMP | D
		DrawKey(Menu, BaseX, BaseY, KeyW, KeyH, "A", Left);
		DrawKey(Menu, BaseX + KeyW + Gap, BaseY, KeyW * 1.4f, KeyH, "JUMP", Jump);
		DrawKey(Menu, BaseX + KeyW * 2.4f + Gap * 2.0f, BaseY, KeyW, KeyH, "D", Right);

		// Ряд 2: LMB (Fire) | RMB (Hook)
		const float Row2Y = BaseY + KeyH + Gap;
		const float TotalW = KeyW * 3.4f + Gap * 2.0f;
		const float HalfW = (TotalW - Gap) * 0.5f;
		DrawKey(Menu, BaseX, Row2Y, HalfW, KeyH - 2.0f, "LMB", Fire);
		DrawKey(Menu, BaseX + HalfW + Gap, Row2Y, HalfW, KeyH - 2.0f, "RMB", Hook);

		// Ряд 3: Спидометр (скорость в тайлах/сек)
		const float Row3Y = Row2Y + (KeyH - 2.0f) + Gap;
		Menu.Rect(BaseX, Row3Y, TotalW, 13.0f, 0.08f, 0.09f, 0.12f, 0.65f);

		float SpeedTiles = 0.0f;
		if(pClient->m_Snap.m_pLocalCharacter)
		{
			const vec2 Vel(
				(float)pClient->m_Snap.m_pLocalCharacter->m_VelX / 256.0f * 50.0f,
				(float)pClient->m_Snap.m_pLocalCharacter->m_VelY / 256.0f * 50.0f);
			SpeedTiles = length(Vel) / 32.0f;
		}

		char aSpeedBuf[32];
		str_format(aSpeedBuf, sizeof(aSpeedBuf), "Speed: %.1f t/s", SpeedTiles);
		Menu.Label(BaseX + 4.0f, Row3Y + 2.0f, 6.2f, aSpeedBuf, 0.75f, 0.9f, 1.0f);
	}
}

static struct CRegKeystrokes
{
	CRegKeystrokes()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Keystrokes HUD";
		Def.m_Category = HCAT_RENDER;
		Def.m_pfnToggle = FeatureKeystrokes::OnToggle;
		Def.m_pfnScreen = FeatureKeystrokes::OnScreenRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegKeystrokes;
