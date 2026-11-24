// feature_trajectory.h — траектория полёта и маркеры для хука (DDNet 16.5)

#include <game/mapitems.h>

namespace FeatureTrajectory
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
		if(!pClient || !pClient->m_Snap.m_pLocalCharacter || !pClient->Collision())
			return;

		vec2 Pos;
		if(!Menu.LocalPos(&Pos))
			return;

		const CNetObj_Character *pChar = pClient->m_Snap.m_pLocalCharacter;
		const vec2 Vel((float)pChar->m_VelX / 256.0f * 50.0f, (float)pChar->m_VelY / 256.0f * 50.0f);
		const CTuningParams &Tuning = pClient->m_aTuning[g_Config.m_ClDummy];

		const float HookRange = Tuning.m_HookLength;
		const float Gravity = Tuning.m_Gravity * 50.0f;

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		vec2 SimPos = Pos;
		vec2 SimVel = Vel;
		const int Steps = 24;
		const float Dt = 0.025f;

		vec2 BestHookTarget(0.0f, 0.0f);
		bool FoundHookSpot = false;
		float BestScore = 999999.0f;

		for(int i = 1; i <= Steps; i++)
		{
			SimVel.y += Gravity * Dt;
			const vec2 NextPos = SimPos + SimVel * Dt;

			const int Tile = pClient->Collision()->GetTile((int)NextPos.x, (int)NextPos.y);
			if(Tile == TILE_SOLID || Tile == TILE_NOHOOK || Tile == TILE_FREEZE || Tile == TILE_DFREEZE)
			{
				pGfx->SetColor(1.0f, 0.4f, 0.2f, 0.85f);
				IGraphics::CQuadItem EndDot(SimPos.x - 4.0f, SimPos.y - 4.0f, 8.0f, 8.0f);
				pGfx->QuadsDrawTL(&EndDot, 1);
				break;
			}

			const float Fade = 1.0f - (float)i / (float)Steps;
			pGfx->SetColor(0.3f, 0.85f, 1.0f, Fade * 0.65f);
			IGraphics::CQuadItem Dot(NextPos.x - 2.0f, NextPos.y - 2.0f, 4.0f, 4.0f);
			pGfx->QuadsDrawTL(&Dot, 1);

			if(!FoundHookSpot && i >= 3)
			{
				const int Rays = 12;
				for(int r = 0; r < Rays; r++)
				{
					const float Angle = (-165.0f + (150.0f * r / (Rays - 1))) * (pi / 180.0f);
					const vec2 RayDir(cosf(Angle), sinf(Angle));
					const vec2 EndRay = NextPos + RayDir * HookRange;

					vec2 HitPos, HitPrev;
					const int Hit = pClient->Collision()->IntersectLine(NextPos, EndRay, &HitPos, &HitPrev);
					if(Hit == TILE_SOLID)
					{
						const float Dist = distance(Pos, HitPos);
						const float ForwardBonus = (SimVel.x * (HitPos.x - Pos.x) > 0.0f) ? -80.0f : 0.0f;
						const float Score = Dist + ForwardBonus;

						if(Score < BestScore && Dist <= HookRange)
						{
							BestScore = Score;
							BestHookTarget = HitPos;
							FoundHookSpot = true;
						}
					}
				}
			}

			SimPos = NextPos;
		}

		if(FoundHookSpot)
		{
			pGfx->SetColor(0.2f, 1.0f, 0.45f, 0.9f);
			IGraphics::CQuadItem Box(BestHookTarget.x - 6.0f, BestHookTarget.y - 6.0f, 12.0f, 12.0f);
			pGfx->QuadsDrawTL(&Box, 1);

			const int Segs = 10;
			for(int s = 1; s < Segs; s += 2)
			{
				const float T = (float)s / (float)Segs;
				const vec2 P = Pos + (BestHookTarget - Pos) * T;
				pGfx->SetColor(0.2f, 1.0f, 0.45f, 0.55f);
				IGraphics::CQuadItem LineDot(P.x - 1.5f, P.y - 1.5f, 3.0f, 3.0f);
				pGfx->QuadsDrawTL(&LineDot, 1);
			}
		}

		pGfx->QuadsEnd();
	}
}

static struct CRegTrajectory
{
	CRegTrajectory()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Trajectory & Hook";
		Def.m_Category = HCAT_MOVEMENT;
		Def.m_pfnToggle = FeatureTrajectory::OnToggle;
		Def.m_pfnWorld = FeatureTrajectory::OnWorldRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegTrajectory;
