// feature_motion_blur.h — эффект размытия в движении (Motion Blur) для DDNet 16.5

namespace FeatureMotionBlur
{
	static vec2 s_LastPos = vec2(0.0f, 0.0f);
	static bool s_HasLastPos = false;
	static vec2 s_SmoothVel = vec2(0.0f, 0.0f);

	static void OnToggle(CHelotyMenu &Menu, bool On)
	{
		if(!On)
		{
			s_HasLastPos = false;
			s_SmoothVel = vec2(0.0f, 0.0f);
		}
	}

	static void OnScreenRender(CHelotyMenu &Menu)
	{
		vec2 Pos;
		if(!Menu.LocalPos(&Pos))
		{
			s_HasLastPos = false;
			return;
		}

		const float Dt = Menu.FrameTime();
		if(Dt <= 0.0001f || Dt > 0.25f)
			return;

		if(!s_HasLastPos)
		{
			s_LastPos = Pos;
			s_HasLastPos = true;
			return;
		}

		// Вычисляем мгновенную скорость и сглаживаем её (инерция кадра)
		vec2 RawVel = (Pos - s_LastPos) / Dt;
		s_LastPos = Pos;

		// Если произошёл телепорт, сбрасываем инерцию
		if(length(RawVel) > 4500.0f)
		{
			s_SmoothVel = vec2(0.0f, 0.0f);
			return;
		}

		const float Blend = std::min(1.0f, Dt * 14.0f);
		s_SmoothVel = s_SmoothVel + (RawVel - s_SmoothVel) * Blend;

		const float Speed = length(s_SmoothVel);
		if(Speed < 180.0f)
			return;

		// Интенсивность размытия (от 0.0 до 1.0)
		const float Intensity = std::min(1.0f, (Speed - 180.0f) / 1400.0f);
		const vec2 Dir = normalize(s_SmoothVel);

		const float W = Menu.ScreenW();
		const float H = Menu.ScreenH();
		const vec2 Center(W * 0.5f, H * 0.5f);

		IGraphics *pGfx = Menu.Gfx();
		pGfx->TextureClear();
		pGfx->QuadsBegin();

		// 1. Слой инерционного шлейфа (смещённые полупрозрачные слои по вектору движения)
		const int Layers = 5;
		for(int i = 1; i <= Layers; i++)
		{
			const float Factor = (float)i / (float)Layers;
			const float Alpha = Intensity * 0.045f * (1.0f - Factor * 0.65f);
			const vec2 Shift = -Dir * (Intensity * 18.0f * Factor);

			pGfx->SetColor(0.85f, 0.92f, 1.0f, Alpha);
			IGraphics::CQuadItem Quad(Shift.x, Shift.y, W, H);
			pGfx->QuadsDrawTL(&Quad, 1);
		}

		// 2. Периферийное смазывание по краям экрана (усиливает ощущение скорости)
		const float EdgeAlpha = Intensity * 0.16f;
		const float EdgeSize = 35.0f * Intensity;

		// Верхний и нижний градиентные края
		pGfx->SetColor(0.05f, 0.07f, 0.12f, EdgeAlpha);
		IGraphics::CQuadItem TopEdge(0.0f, 0.0f, W, EdgeSize);
		IGraphics::CQuadItem BotEdge(0.0f, H - EdgeSize, W, EdgeSize);
		pGfx->QuadsDrawTL(&TopEdge, 1);
		pGfx->QuadsDrawTL(&BotEdge, 1);

		pGfx->QuadsEnd();
	}
}

// Регистрация модуля в категории Render
static struct CRegMotionBlur
{
	CRegMotionBlur()
	{
		CHelotyFeatureDef Def;
		Def.m_pName = "Motion Blur";
		Def.m_Category = HCAT_RENDER;
		Def.m_pfnToggle = FeatureMotionBlur::OnToggle;
		Def.m_pfnScreen = FeatureMotionBlur::OnScreenRender;
		HelotyFeatureRegistry().push_back(Def);
	}
} gs_RegMotionBlur;
