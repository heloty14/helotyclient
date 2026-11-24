// feature_fps_optimizer.h — оптимизатор FPS для DDNet 16.5 (Windows 7 / слабые ПК)

namespace FeatureFpsOptimizer
{
	static bool s_Saved = false;
	static int s_OldHighDetail = 1;
	static int s_OldShowDeco = 1;
	static int s_OldShowOthers = 1;
	static int s_OldGfxFinish = 0;
	static int s_OldParticles = 100;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		if(On)
		{
			if(!s_Saved)
			{
				s_OldHighDetail = g_Config.m_GfxHighDetail;
				s_OldShowDeco = g_Config.m_ClShowDeco;
				s_OldGfxFinish = g_Config.m_GfxFinish;
				s_Saved = true;
			}

			// Отключаем тяжёлые декорации карты и синхронизацию кадров GPU
			Menu.Exec("gfx_high_detail 0");
			Menu.Exec("cl_show_deco 0");
			Menu.Exec("gfx_finish 0");
			Menu.Exec("gfx_asyncrender_old 1");
			Menu.Exec("cl_predict_players 1");
		}
		else if(s_Saved)
		{
			// Восстанавливаем исходные настройки пользователя
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "gfx_high_detail %d", s_OldHighDetail);
			Menu.Exec(aBuf);
			str_format(aBuf, sizeof(aBuf), "cl_show_deco %d", s_OldShowDeco);
			Menu.Exec(aBuf);
			str_format(aBuf, sizeof(aBuf), "gfx_finish %d", s_OldGfxFinish);
			Menu.Exec(aBuf);
			s_Saved = false;
		}
	}
}

static struct CRegFpsOptimizer
{
	CRegFpsOptimizer()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "FPS Optimizer";
		Def.m_Category = HCAT_MISC;
		Def.m_pfnToggle = FeatureFpsOptimizer::OnToggle;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegFpsOptimizer;
