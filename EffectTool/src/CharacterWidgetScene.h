#pragma once
#include "Engine_include.h"
#include "Widgets.h"

namespace KGCA41B
{
	class CharacterWidgetScene : public KGCA41B::Scene
	{
	public:
		string rt_id;
		//FX_Particles
	public:
		virtual void OnInit();
		virtual void OnUpdate();
		virtual void OnRender();
		virtual void OnRelease();
	private:
		entt::registry reg_scene;

		KGCA41B::RenderSystem sys_render;
	};

}

