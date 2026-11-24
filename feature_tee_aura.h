// feature_tee_aura.h — вращающаяся аура вокруг ти и искры при прыжке (DDNet 16.5)

namespace FeatureTeeAura
{
	struct CSpark
	{
		vec2 m_Pos;
		vec2 m_Vel;
		float m_Life;
		float m_MaxLife;
		float m_Hue;
	};

	static bool s_Enabled = false;
	static int s_LastJumped = 0;
	static std::vector<CSpark> s_vSparks;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		s_Enabled = On;
		s_vSparks.clear();
	}

	static void OnWorldRender(CHelotyMenu &Menu)
	{
		if(!s_Enabled)
			return;

		CGameClient *pClient = Menu.GameClient();
		if(!pClient || !pClient->m_Snap.m_pLocalCharacter)
			return;

		vec2 Pos;
		if(!Menu.LocalPos(&Pos))
			return;

		const float Dt = Menu.FrameTime();
		const float Now = Menu.Time();
		const CNetObj_Character *pChar = pClient->m_Snap.m_pLocalCharacter;

		if(pChar->m_Jumped != s_LastJumped)
		{
			s_LastJumped = pChar->m_Jumped;
			for(int i = 0; i < 14; i++)
			{
				CSpark Sp;
				const float Angle = (Menu.Rnd() * 140.0f + 20.0f) * (pi / 180.0f);
				const float Spd = 60.0f + Menu.Rnd() * 140.0f;
				Sp.m_Pos = Pos + vec2((Menu.Rnd() - 0.5f) * 14.0f, 12.0f);
				Sp.m_Vel = vec2(cosf(Angle) * Spd * (Menu.Rnd() > 0.5f ? 1.0f : -1.0f), sinf(Angle) * Spd);
				Sp.m_MaxLife = 0.25f + Menu.Rnd() * 0.2f;
				Sp.m_Life = Sp.m_MaxLife;
				Sp.m_Hue = Menu.Rnd();
				s_vSparks.push_back(Sp);
			}
		}

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		const int Segments = 24;
		const float Radius = 28.0f + sinf(Now * 4.0f) * 2.0f;
		for(int i = 0; i < Segments; i++)
		{
			const float T = (float)i / (float)Segments;
			const float Angle = T * 2.0f * pi + Now * 2.5f;
			const vec2 Pt = Pos + vec2(cosf(Angle), sinf(Angle)) * Radius;

			const float Wave = 0.5f + 0.5f * sinf(T * 2.0f * pi + Now * 3.0f);
			pGfx->SetColor(0.25f + 0.65f * Wave, 0.75f - 0.35f * Wave, 1.0f, 0.75f);

			const float DotSize = (i % 4 == 0) ? 3.5f : 2.2f;
			IGraphics::CQuadItem Dot(Pt.x - DotSize * 0.5f, Pt.y - DotSize * 0.5f, DotSize, DotSize);
			pGfx->QuadsDrawTL(&Dot, 1);
		}

		for(size_t i = 0; i < s_vSparks.size();)
		{
			CSpark &Sp = s_vSparks[i];
			Sp.m_Life -= Dt;
			if(Sp.m_Life <= 0.0f)
			{
				s_vSparks[i] = s_vSparks.back();
				s_vSparks.pop_back();
				continue;
			}

			Sp.m_Vel.y += 280.0f * Dt;
			Sp.m_Pos += Sp.m_Vel * Dt;

			const float Alpha = Sp.m_Life / Sp.m_MaxLife;
			const float Size = 3.2f * Alpha;
			pGfx->SetColor(1.0f, 0.55f + 0.45f * Sp.m_Hue, 0.25f + 0.75f * (1.0f - Sp.m_Hue), Alpha);

			IGraphics::CQuadItem Quad(Sp.m_Pos.x - Size * 0.5f, Sp.m_Pos.y - Size * 0.5f, Size, Size);
			pGfx->QuadsDrawTL(&Quad, 1);
			i++;
		}

		pGfx->QuadsEnd();
	}
}

static struct CRegTeeAura
{
	CRegTeeAura()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Tee Aura & Sparks";
		Def.m_Category = HCAT_TEE;
		Def.m_pfnToggle = FeatureTeeAura::OnToggle;
		Def.m_pfnWorld = FeatureTeeAura::OnWorldRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegTeeAura;
