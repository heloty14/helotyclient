// feature_target_hud.h — Target HUD в стиле Minecraft-клиентов (DDNet 16.5)

namespace FeatureTargetHud
{
	static bool s_Enabled = false;
	static int s_TargetId = -1;
	static float s_SmoothDistTiles = 0.0f;

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		s_Enabled = On;
		s_TargetId = -1;
		s_SmoothDistTiles = 0.0f;
	}

	static int FindTarget(CGameClient *pClient, vec2 MyPos, vec2 AimDir)
	{
		const int LocalId = pClient->m_Snap.HELOTY_LOCAL_ID;
		int BestId = -1;
		float BestScore = 999999.0f;

		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == LocalId)
				continue;

			const CGameClient::CClientData &Data = pClient->m_aClients[i];
			if(!Data.m_Active || !pClient->m_Snap.m_aCharacters[i].m_Active)
				continue;

			const vec2 Delta = Data.m_RenderPos - MyPos;
			const float Dist = length(Delta);
			if(Dist < 1.0f || Dist > 1600.0f)
				continue;

			const vec2 Dir = Delta / Dist;
			const float Dot = dot(AimDir, Dir);
			const float AnglePenalty = (1.0f - Dot) * 450.0f;
			const float Score = Dist + AnglePenalty;

			if(Score < BestScore)
			{
				BestScore = Score;
				BestId = i;
			}
		}
		return BestId;
	}

	static void OnWorldRender(CHelotyMenu &Menu)
	{
		s_TargetId = -1;
		if(!s_Enabled)
			return;

		CGameClient *pClient = Menu.GameClient();
		if(!pClient || !pClient->m_Snap.m_pLocalCharacter)
			return;

		vec2 MyPos;
		if(!Menu.LocalPos(&MyPos))
			return;

		vec2 AimDir = pClient->m_Controls.m_aMousePos[g_Config.m_ClDummy];
		if(length(AimDir) > 0.01f)
			AimDir = normalize(AimDir);
		else
			AimDir = vec2(1.0f, 0.0f);

		s_TargetId = FindTarget(pClient, MyPos, AimDir);
		if(s_TargetId < 0)
			return;

		const vec2 TargetPos = pClient->m_aClients[s_TargetId].m_RenderPos;
		const vec2 Delta = TargetPos - MyPos;
		const float Dist = length(Delta);
		const float Now = Menu.Time();

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		const int Steps = std::min(64, std::max(6, (int)(Dist / 16.0f)));
		const float AnimOffset = fmodf(Now * 3.0f, 1.0f);

		for(int i = 1; i < Steps; i++)
		{
			const float T = ((float)i + AnimOffset) / (float)Steps;
			if(T >= 1.0f)
				continue;

			const vec2 Pt = MyPos + Delta * T;
			pGfx->SetColor(0.25f + 0.75f * T, 0.75f - 0.5f * T, 1.0f - 0.55f * T, 0.7f);

			const float DotSize = 2.2f + 1.4f * T;
			IGraphics::CQuadItem Dot(Pt.x - DotSize * 0.5f, Pt.y - DotSize * 0.5f, DotSize, DotSize);
			pGfx->QuadsDrawTL(&Dot, 1);
		}

		const float RingRadius = 24.0f + sinf(Now * 5.0f) * 1.5f;
		const int RingPts = 32;
		for(int i = 0; i < RingPts; i++)
		{
			const float T = (float)i / (float)RingPts;
			const float Angle = T * 2.0f * pi + Now * 2.8f;
			const vec2 Pt = TargetPos + vec2(cosf(Angle), sinf(Angle)) * RingRadius;

			const float Glow = 0.5f + 0.5f * sinf(T * 4.0f * pi + Now * 4.0f);
			pGfx->SetColor(1.0f, 0.25f + 0.55f * Glow, 0.45f + 0.55f * (1.0f - Glow), 0.9f);

			const float Size = (i % 8 == 0) ? 4.2f : 2.5f;
			IGraphics::CQuadItem PtQuad(Pt.x - Size * 0.5f, Pt.y - Size * 0.5f, Size, Size);
			pGfx->QuadsDrawTL(&PtQuad, 1);
		}

		const float BoxHalf = 21.0f;
		const float CornerLen = 7.0f;
		const float Thick = 2.0f;
		pGfx->SetColor(1.0f, 0.35f, 0.45f, 0.95f);

		IGraphics::CQuadItem C1(TargetPos.x - BoxHalf, TargetPos.y - BoxHalf, CornerLen, Thick);
		IGraphics::CQuadItem C2(TargetPos.x - BoxHalf, TargetPos.y - BoxHalf, Thick, CornerLen);
		IGraphics::CQuadItem C3(TargetPos.x + BoxHalf - CornerLen, TargetPos.y - BoxHalf, CornerLen, Thick);
		IGraphics::CQuadItem C4(TargetPos.x + BoxHalf - Thick, TargetPos.y - BoxHalf, Thick, CornerLen);
		IGraphics::CQuadItem C5(TargetPos.x - BoxHalf, TargetPos.y + BoxHalf - Thick, CornerLen, Thick);
		IGraphics::CQuadItem C6(TargetPos.x - BoxHalf, TargetPos.y + BoxHalf - CornerLen, Thick, CornerLen);
		IGraphics::CQuadItem C7(TargetPos.x + BoxHalf - CornerLen, TargetPos.y + BoxHalf - Thick, CornerLen, Thick);
		IGraphics::CQuadItem C8(TargetPos.x + BoxHalf - Thick, TargetPos.y + BoxHalf - CornerLen, Thick, CornerLen);

		pGfx->QuadsDrawTL(&C1, 1);
		pGfx->QuadsDrawTL(&C2, 1);
		pGfx->QuadsDrawTL(&C3, 1);
		pGfx->QuadsDrawTL(&C4, 1);
		pGfx->QuadsDrawTL(&C5, 1);
		pGfx->QuadsDrawTL(&C6, 1);
		pGfx->QuadsDrawTL(&C7, 1);
		pGfx->QuadsDrawTL(&C8, 1);

		pGfx->QuadsEnd();
	}

	static void OnScreenRender(CHelotyMenu &Menu)
	{
		if(!s_Enabled || s_TargetId < 0)
			return;

		CGameClient *pClient = Menu.GameClient();
		if(!pClient)
			return;

		vec2 MyPos;
		if(!Menu.LocalPos(&MyPos))
			return;

		const CGameClient::CClientData &Target = pClient->m_aClients[s_TargetId];
		const CNetObj_Character &TargetChar = pClient->m_Snap.m_aCharacters[s_TargetId].m_Cur;

		const float DistTiles = distance(MyPos, Target.m_RenderPos) / 32.0f;
		const float Dt = Menu.FrameTime();
		if(Dt > 0.0f && Dt < 0.25f)
			s_SmoothDistTiles += (DistTiles - s_SmoothDistTiles) * std::min(1.0f, Dt * 15.0f);
		else
			s_SmoothDistTiles = DistTiles;

		const float W = Menu.ScreenW();
		const float H = Menu.ScreenH();
		const float CardW = 112.0f;
		const float CardH = 34.0f;
		const float CardX = W * 0.5f + 22.0f;
		const float CardY = H * 0.5f + 14.0f;

		Menu.Rect(CardX, CardY, CardW, CardH, 0.07f, 0.08f, 0.11f, 0.82f);
		Menu.Rect(CardX, CardY, CardW, 1.8f, 1.0f, 0.35f, 0.55f, 0.95f);

		const bool IsFrozen = (TargetChar.m_Weapon == WEAPON_NINJA);
		if(IsFrozen)
			Menu.Rect(CardX + 3.0f, CardY + 5.0f, 3.0f, CardH - 8.0f, 0.3f, 0.85f, 1.0f, 0.95f);
		else
			Menu.Rect(CardX + 3.0f, CardY + 5.0f, 3.0f, CardH - 8.0f, 0.3f, 1.0f, 0.5f, 0.95f);

		Menu.Label(CardX + 9.0f, CardY + 4.0f, 7.2f, Target.m_aName, 1.0f, 1.0f, 1.0f);

		char aInfo[64];
		str_format(aInfo, sizeof(aInfo), "%.1f tiles  %s", s_SmoothDistTiles, IsFrozen ? "[FREEZE]" : "[ACTIVE]");
		Menu.Label(CardX + 9.0f, CardY + 14.0f, 6.0f, aInfo, 0.8f, 0.85f, 0.95f);

		const float BarX = CardX + 9.0f;
		const float BarY = CardY + 25.0f;
		const float BarW = CardW - 14.0f;
		const float BarH = 4.0f;
		const float FillRatio = std::max(0.05f, std::min(1.0f, 1.0f - (s_SmoothDistTiles / 50.0f)));

		Menu.Rect(BarX, BarY, BarW, BarH, 0.16f, 0.18f, 0.24f, 0.9f);
		Menu.Rect(BarX, BarY, BarW * FillRatio, BarH, 1.0f, 0.35f, 0.55f, 0.95f);
	}
}

static struct CRegTargetHud
{
	CRegTargetHud()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Target HUD";
		Def.m_Category = HCAT_RENDER;
		Def.m_pfnToggle = FeatureTargetHud::OnToggle;
		Def.m_pfnWorld = FeatureTargetHud::OnWorldRender;
		Def.m_pfnScreen = FeatureTargetHud::OnScreenRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegTargetHud;
