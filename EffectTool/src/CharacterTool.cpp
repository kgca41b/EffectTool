#include "CharacterTool.h"
#include "ComponentSystem.h"
#include "SceneMgr.h"

using namespace KGCA41B;

void CharacterTool::OnInit()
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

	reg_scene.emplace<Camera>(player_, debug_camera_);

	debug_input_.tag = "Player";
	reg_scene.emplace<InputMapping>(player_, debug_input_);

	sys_camera_.TargetTag(reg_scene, "Player");
	sys_camera_.OnCreate(reg_scene);
	sys_input_.OnCreate(reg_scene);
	sys_render_.OnCreate(reg_scene);

	character_actor.OnInit(reg_scene, AABB<3>({ 10, 10 }, { 10, 10 }));
}

void CharacterTool::OnUpdate()
{
	sys_input_.OnUpdate(reg_scene);
	sys_camera_.OnUpdate(reg_scene);

	widget_scene_.OnUpdate();
	character_actor.OnUpdate(reg_scene);
}

void CharacterTool::OnRender()
{
	widget_scene_.OnRender();
	GUI->RenderWidgets();
	sys_render_.OnUpdate(reg_scene);
}

void CharacterTool::OnRelease()
{

}

