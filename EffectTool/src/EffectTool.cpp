#include "EffectTool.h"
#include "ComponentSystem.h"
#include "SceneMgr.h"

using namespace KGCA41B;

void EffectTool::OnInit()
{
	GUI->AddWidget("MainMenu", &window_);
	DATA->Init("D:/Data");
	RESOURCE->Init("D:/Contents"); 
	DINPUT->Init(ENGINE->GetWindowHandle(), ENGINE->GetInstanceHandle());

	RENDER_TARGET->Init("BackBuffer");
	widget_scene_.OnInit();

	SCENE->PushScene("EffectTool", this);
	SCENE->PushScene("Widget", &widget_scene_);

	debug_camera_.position = { 0, 0, -10, 0 };
	debug_camera_.look = { 0, 0, 1, 0 };
	debug_camera_.up = { 0, 1, 0, 0 };
	debug_camera_.near_z = 1.f;
	debug_camera_.far_z = 10000.f;
	debug_camera_.fov = XMConvertToRadians(45);
	debug_camera_.yaw = 0;
	debug_camera_.pitch = 0;
	debug_camera_.roll = 0;
	debug_camera_.speed = 100;
	debug_camera_.tag = "Player";

	reg_effect_tool_.emplace<Camera>(player_, debug_camera_);

	debug_input_.tag = "Player";
	reg_effect_tool_.emplace<InputMapping>(player_, debug_input_);

	sys_camera_.TargetTag(reg_effect_tool_, "Player");
	sys_camera_.OnCreate(reg_effect_tool_);
	sys_input_.OnCreate(reg_effect_tool_);
	sys_render_.OnCreate(reg_effect_tool_);

	uv_sprite_.OnInit(reg_effect_tool_, AABB<3>(), {});
	tex_sprite_.OnInit(reg_effect_tool_, AABB<3>(), {});
}

void EffectTool::OnUpdate()
{
	sys_input_.OnUpdate(reg_effect_tool_);
	sys_camera_.OnUpdate(reg_effect_tool_);

	widget_scene_.OnUpdate();
	uv_sprite_.OnUpdate(reg_effect_tool_);
	tex_sprite_.OnUpdate(reg_effect_tool_);
}

void EffectTool::OnRender()
{
	widget_scene_.OnRender();
	GUI->RenderWidgets();
	sys_render_.OnUpdate(reg_effect_tool_);
}

void EffectTool::OnRelease()
{

}
