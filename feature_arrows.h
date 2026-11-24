// feature_arrows.h — стрелочки вокруг персонажа, указывающие на игроков (DDNet 16.5)

namespace FeatureArrows
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

		CGameClient *pClient = Menu.GameClient();
		if(!pClient)
			return;

		vec2 MyPos;
		if(!Menu.LocalPos(&MyPos))
			return;

		const int LocalId = pClient->m_Snap.HELOTY_LOCAL_ID;
		const float OrbitRadius = 56.0f;
		const float ArrowSize = 7.0f;

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == LocalId)
				continue;

			const CGameClient::CClientData &ClientData = pClient->m_aClients[i];
			if(!ClientData.m_Active || !pClient->m_Snap.m_aCharacters[i].m_Active)
				continue;

			const vec2 TargetPos = ClientData.m_RenderPos;
			const vec2 Delta = TargetPos - MyPos;
			const float Dist = length(Delta);

			if(Dist < 90.0f)
				continue;

			const vec2 Dir = Delta / Dist;
			const vec2 Perp(-Dir.y, Dir.x);

			const vec2 Tip = MyPos + Dir * (OrbitRadius + ArrowSize);
			const vec2 Base = MyPos + Dir * OrbitRadius;
			const vec2 Left = Base + Perp * (ArrowSize * 0.65f);
			const vec2 Right = Base - Perp * (ArrowSize * 0.65f);

			const float NearFactor = std::max(0.0f, std::min(1.0f, 1.0f - (Dist - 150.0f) / 1400.0f));
			const float Alpha = 0.45f + NearFactor * 0.45f;
			pGfx->SetColor(0.25f + NearFactor * 0.75f, 0.85f - NearFactor * 0.55f, 1.0f - NearFactor * 0.75f, Alpha);

			IGraphics::CFreeformItem Tri(
				Tip.x, Tip.y,
				Left.x, Left.y,
				Right.x, Right.y,
				Right.x, Right.y);
			pGfx->QuadsDrawFreeform(&Tri, 1);
		}

		pGfx->QuadsEnd();
	}
}

static struct CRegArrows
{
	CRegArrows()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Player Arrows";
		Def.m_Category = HCAT_RENDER;
		Def.m_pfnToggle = FeatureArrows::OnToggle;
		Def.m_pfnWorld = FeatureArrows::OnWorldRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegArrows;
