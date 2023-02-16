#pragma once
#include "Engine_include.h"
#include "Widgets.h"
#include "CharacterWidgetScene.h"

namespace KGCA41B
{
	class CharacterTool : public KGCA41B::Scene
	{
	public:
		virtual void OnInit();
		virtual void OnUpdate();
		virtual void OnRender();
		virtual void OnRelease();

	public:
		entt::registry		reg_scene;
	public:
		RenderSystem		sys_render_;
		CameraSystem		sys_camera_;
		InputSystem			sys_input_;
	public:
		CharacterActor		character_actor;
		entt::entity		player_;
		Camera				debug_camera_;
		InputMapping		debug_input_;
	public:
		CharacterWidgetScene	widget_scene_;
	public:
		WG_MainMenu			window_;
	};
}
