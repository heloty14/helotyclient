// feature_tee_wings.h — светящиеся крылья и золотая корона для вашего ти (DDNet 16.5)

namespace FeatureTeeWings
{
	static bool s_Enabled = false;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		s_Enabled = On;
	}

	static void OnWorldRender(CHelotyMenu &Menu)
	{
		if(!s_Enabled)
			return;

		vec2 Pos;
		if(!Menu.LocalPos(&Pos))
			return;

		const float Now = Menu.Time();
		const float Flap = sinf(Now * 6.5f) * 0.25f; // взмах крыльев

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		// 1. Энергетические крылья слева и справа от ти
		for(int Side = -1; Side <= 1; Side += 2)
		{
			const float BaseAngle = (Side < 0 ? pi : 0.0f) + Side * (-0.35f + Flap);
			for(int Feather = 0; Feather < 5; Feather++)
			{
				const float FAngle = BaseAngle + Side * (Feather * 0.22f);
				const float Len = 26.0f - Feather * 2.5f;

				for(int Seg = 1; Seg <= 5; Seg++)
				{
					const float T = (float)Seg / 5.0f;
					const vec2 Pt = Pos + vec2((float)Side * 10.0f, -2.0f) + vec2(cosf(FAngle), sinf(FAngle)) * (Len * T);
					const float Alpha = (1.0f - T * 0.45f) * 0.8f;

					pGfx->SetColor(0.35f + 0.45f * T, 0.8f, 1.0f, Alpha);
					const float Size = 3.6f - T * 1.2f;
					IGraphics::CQuadItem Dot(Pt.x - Size * 0.5f, Pt.y - Size * 0.5f, Size, Size);
					pGfx->QuadsDrawTL(&Dot, 1);
				}
			}
		}

		// 2. Золотая корона над головой ти (слегка парит вверх-вниз)
		const float Bob = sinf(Now * 4.0f) * 1.5f;
		const vec2 CrownBase = Pos + vec2(0.0f, -23.0f + Bob);

		// Основание короны
		pGfx->SetColor(1.0f, 0.82f, 0.15f, 0.95f);
		IGraphics::CQuadItem BaseBar(CrownBase.x - 9.0f, CrownBase.y, 18.0f, 3.0f);
		pGfx->QuadsDrawTL(&BaseBar, 1);

		// 3 зубца короны
		for(int Tooth = -1; Tooth <= 1; Tooth++)
		{
			const float Tx = CrownBase.x + Tooth * 7.0f;
			const float Th = (Tooth == 0) ? 5.5f : 4.0f;
			pGfx->SetColor(1.0f, 0.9f, 0.25f, 0.95f);
			IGraphics::CQuadItem Spike(Tx - 1.5f, CrownBase.y - Th, 3.0f, Th);
			pGfx->QuadsDrawTL(&Spike, 1);
		}

		pGfx->QuadsEnd();
	}
}

static struct CRegTeeWings
{
	CRegTeeWings()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Wings & Crown";
		Def.m_Category = HCAT_TEE;
		Def.m_pfnToggle = FeatureTeeWings::OnToggle;
		Def.m_pfnWorld = FeatureTeeWings::OnWorldRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegTeeWings;
