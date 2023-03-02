#include "Widgets.h"
#include "imfilebrowser.h"
#include "SceneMgr.h"
#include "EffectTool.h"

using namespace KGCA41B;

#define LISTBOX_WIDTH 300.0f
#define TEXT_WIDTH 150.0f

#define GET_VARIABLE_NAME(n) #n

#define MIN 0
#define MAX 1

void WG_MainMenu::Update()
{
	ImGui::SetCurrentContext(GUI->GetContext());
	//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void WG_MainMenu::Render()
{
	ImGui::BeginMainMenuBar();
	{
		if (ImGui::MenuItem("Effect"))
		{
			auto effect_tool = GUI->FindWidget("EffectTool");
			if (effect_tool == nullptr)
				GUI->AddWidget("EffectTool", new WG_EffectWindow);
			else
				effect_tool->InvertOpen();
		}
	}

	ImGui::EndMainMenuBar();

	ImGui::Begin("Render Option Window", &open_);
	
	string fps = "FPS : " + to_string(TIMER->GetFPS());
	ImGui::Text(fps.c_str());
	
	static bool bWireFrame = false;

	ImGui::Checkbox("WireFrame", &bWireFrame);

	auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
	scene->bWireFrame = bWireFrame;

	ImGui::End();
}

void WG_EffectWindow::Update()
{
	ImGui::SetCurrentContext(GUI->GetContext());
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void WG_EffectWindow::Render()
{
	ImGui::SetNextWindowSize(ImVec2(450, 600));
	ImGui::Begin("Effect Tool", &open_, ImGuiWindowFlags_MenuBar);
	{
		ImGui::BeginMenuBar();
		{
			if (ImGui::BeginMenu("New Effect"))
			{
				if (ImGui::MenuItem("UV Sprite"))
				{
					type_ = UV_SPRITE;
				}
				if (ImGui::MenuItem("Texture Sprite"))
				{
					type_ = TEX_SPRITE;
				}
				if (ImGui::MenuItem("Emitter"))
				{
					type_ = EMITTER;
				}
				if (ImGui::MenuItem("Effect"))
				{
					type_ = EFFECT;
				}
				ImGui::EndMenu();
			}
			LoadingData();
		}
		ImGui::EndMenuBar();


		static char particle_name[255] = { 0, };

		switch (type_)
		{
		case NONE:
		{

		}
		break;
		case UV_SPRITE:
		{
			UVSpriteBoard();
		}
		break;
		case TEX_SPRITE:
		{
			TexSpriteBoard();
		}
		break;
		case EMITTER:
		{
			EmitterBoard(emitter_data_);

			if (ImGui::TreeNode("Emitter Save & Reset & Render"))
			{
				ImGui::SetNextItemWidth(TEXT_WIDTH);
				ImGui::InputTextWithHint("Emitter Name", "Name", particle_name, IM_ARRAYSIZE(particle_name));
				if (ImGui::Button("Save Emitter"))
				{
					SaveEmitter(particle_name);
				}
				if (ImGui::Button("Render Emitter"))
				{
					auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
					if (scene)
						scene->AddEmitter(make_shared<Emitter>(emitter_data_));
				}
				if (ImGui::Button("Reset Emitter Data"))
				{
					emitter_data_ = Emitter();
				}
				if (ImGui::Button("Reset Rendering Emitter"))
				{
					auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
					if (scene)
						scene->ResetEmitter();
				}
				

				ImGui::TreePop();
			}
			
		}break;
		case EFFECT:
		{
			EffectBoard();
		}break;
		}
	}
	ImGui::End();
}

void WG_EffectWindow::UVSpriteBoard()
{
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static char sprite_name[255] = { 0, };

	if (ImGui::TreeNode("UV Setting"))
	{

		ImVec2 img_size = { 200, 200 };

		tex = RESOURCE->UseResource<Texture>(uv_sprite_data_.tex_id);

		if (tex)
			ImGui::Image((void*)tex->srv.Get(), img_size);

		if (uv_sprite_data_.uv_list.size() > 0 && tex)
		{
			ImGui::SameLine();
			auto uv = uv_sprite_data_.uv_list[min(cur_frame - 1, (int)uv_sprite_data_.uv_list.size() - 1)];
			float tex_width = (float)tex->texture_desc.Width;
			float tex_height = (float)tex->texture_desc.Height;
			ImVec2 start(uv.first.x / tex_width, uv.first.y / tex_height);
			ImVec2 end(uv.second.x / tex_width, uv.second.y / tex_height);
			ImGui::Image((void*)tex->srv.Get(), img_size, start, end);
		}

		// 프레임 선택
		SelectFrame(uv_sprite_data_.uv_list.size(), cur_frame);

		// 텍스쳐 선택
		SelectTexture(uv_sprite_data_.tex_id);

		// UV 값 설정
		SelectUV(uv_sprite_data_.uv_list);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("UV Sprite Save & Reset"))
	{
		ImGui::SetNextItemWidth(TEXT_WIDTH);
		ImGui::InputTextWithHint("sprite name", "Name", sprite_name, IM_ARRAYSIZE(sprite_name));

		if (ImGui::Button("Save"))
		{
			SaveUVSprite(sprite_name);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			tex = nullptr;
			cur_frame = 1;
			memset(sprite_name, 0, sizeof(char) * strlen(sprite_name));

			uv_sprite_data_.tex_id = "";
			uv_sprite_data_.uv_list.clear();
		}

		ImGui::TreePop();
	}
	
}

void WG_EffectWindow::TexSpriteBoard()
{
	static string texture_id = "";
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static char sprite_name[255] = { 0, };


	if (ImGui::TreeNode("Texture Setting"))
	{
		ImVec2 img_size = { 200, 200 };
		tex = RESOURCE->UseResource<Texture>(texture_id);

		if (tex)
			ImGui::Image((void*)tex->srv.Get(), img_size);


		if (tex_sprite_data_.tex_id_list.size() > 0)
		{
			ImGui::SameLine();
			tex = RESOURCE->UseResource<Texture>(tex_sprite_data_.tex_id_list[min((int)cur_frame - 1, (int)tex_sprite_data_.tex_id_list.size() - 1)]);
			ImGui::Image((void*)tex->srv.Get(), img_size);
		}


		// 프레임 선택
		SelectFrame(tex_sprite_data_.tex_id_list.size(), cur_frame);

		// 텍스쳐 선택
		SelectTexture(texture_id);

		// 텍스쳐 배열 추가
		if (ImGui::Button("Add Texture"))
		{
			tex_sprite_data_.tex_id_list.push_back(texture_id);
		}

		ImGui::SetNextItemWidth(LISTBOX_WIDTH);
		static int item_current_idx = 0;
		if (ImGui::BeginListBox("Tex ID per Frame"))
		{
			for (int n = 0; n < tex_sprite_data_.tex_id_list.size(); n++)
			{
				const bool is_selected = (item_current_idx == n);
				string str = "[" + to_string(n + 1) + "] " + tex_sprite_data_.tex_id_list[n];
				if (ImGui::Selectable(str.c_str(), is_selected))
					item_current_idx = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		if (ImGui::Button("Delete Last Frame"))
		{
			if (tex_sprite_data_.tex_id_list.size() > 0)
				tex_sprite_data_.tex_id_list.pop_back();
		}

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Texture Sprite Save & Reset"))
	{
		ImGui::SetNextItemWidth(TEXT_WIDTH);
		ImGui::InputTextWithHint("sprite name", "Name", sprite_name, IM_ARRAYSIZE(sprite_name));

		if (ImGui::Button("Save"))
		{
			// TODO : 데이터 테이블을 통한 저장
			SaveTexSprite(sprite_name);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			texture_id = "";
			tex = nullptr;
			cur_frame = 1;
			memset(sprite_name, 0, sizeof(char) * strlen(sprite_name));

			tex_sprite_data_.tex_id_list.clear();
		}

		ImGui::TreePop();
	}
	
}

void WG_EffectWindow::EmitterBoard(Emitter& emitter)
{
	if (ImGui::TreeNode("Sprite"))
	{
		static int cur_frame = 1;
		static float timer = 1.0f;

		// Render Selected Sprite
		auto sprite = RESOURCE->UseResource<Sprite>(emitter.sprite_id);
		if (sprite)
		{
			Texture* tex = nullptr;
			ImVec2 img_size = { 200, 200 };
			// 스프라이트 타입에 따라 다르게 랜더링
			// UV Sprite
			if (sprite->type == UV_SPRITE)
			{
				UVSprite* uv_sprite = (UVSprite*)sprite;
				tex = RESOURCE->UseResource<Texture>(uv_sprite->tex_id);

				// 프레임 처리
				timer += TIMER->GetDeltaTime();
				if (timer > uv_sprite->uv_list.size() + 1)
					timer -= uv_sprite->uv_list.size();
				cur_frame = (int)timer;

				auto uv = uv_sprite->uv_list[min((int)cur_frame - 1, (int)uv_sprite->uv_list.size() - 1)];
				float tex_width = (float)tex->texture_desc.Width;
				float tex_height = (float)tex->texture_desc.Height;
				ImVec2 start(uv.first.x / tex_width, uv.first.y / tex_height);
				ImVec2 end(uv.second.x / tex_width, uv.second.y / tex_height);


				ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);
				ImGui::Image((void*)tex->srv.Get(), img_size, start, end);
			}
			// Texture Sprite
			else if (sprite->type == TEX_SPRITE)
			{
				TextureSprite* tex_sprite = (TextureSprite*)sprite;
				// 프레임 처리
				timer += TIMER->GetDeltaTime();
				if (timer > tex_sprite->tex_id_list.size() + 1)
					timer -= tex_sprite->tex_id_list.size();
				cur_frame = (int)timer;

				tex = RESOURCE->UseResource<Texture>(tex_sprite->tex_id_list[min((int)cur_frame - 1, (int)tex_sprite->tex_id_list.size() - 1)]);

				ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);
				ImGui::Image((void*)tex->srv.Get(), img_size);
			}
		}

		// Sprite_id or Something?
		SelectSprite(emitter.sprite_id);

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Attributes"))
	{
		if (ImGui::TreeNode("Emit Mode"))
		{
			static int mode = ONCE;
			mode = emitter.emit_type;
			if (ImGui::RadioButton("Once", mode == ONCE))
			{
				mode = ONCE;
				emitter.emit_type = ONCE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Per Second", mode == PER_SECOND))
			{
				mode = PER_SECOND;
				emitter.emit_type = PER_SECOND;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Per Period", mode == AFTER_TIME))
			{
				mode = AFTER_TIME;
				emitter.emit_type = AFTER_TIME;
			}

			switch (mode)
			{
			case ONCE:
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputInt("Emit Once", &emitter.emit_once);
				break;
			case PER_SECOND:
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputInt("Emit per Second", &emitter.emit_per_second);
				break;
			case AFTER_TIME:
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat("Emit per Period(sec)", &emitter.emit_time);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Life Time(sec)"))
		{
			ImGui::SetNextItemWidth(50.0f);
			ImGui::InputFloat("Life Time Min", &emitter.life_time[MIN]);
			ImGui::SetNextItemWidth(50.0f);
			ImGui::InputFloat("Life Time Max", &emitter.life_time[MAX]);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Particle Color"))
		{
			// Color
			static E_EmitterAttributeType color_mode = INITIAL_SET;
			color_mode = emitter.color_setting_type;
			if (ImGui::RadioButton("Initial Setting(COLOR)", color_mode == INITIAL_SET))
			{
				color_mode = INITIAL_SET;
				emitter.color_setting_type = INITIAL_SET;
			}
			if (ImGui::RadioButton("Set Per Lifetime(COLOR)", color_mode == SET_PER_LIFETIME))
			{
				color_mode = SET_PER_LIFETIME;
				emitter.color_setting_type = SET_PER_LIFETIME;
			}

			switch (color_mode)
			{
			case INITIAL_SET:
				ImGui::SetNextItemWidth(150.0f);
				ImGui::ColorEdit4("Initial Color", (float*)&emitter.initial_color);
				break;
			case SET_PER_LIFETIME:
				// VIEW
			{
				ImGui::Text("Color Timeline");
				float arr[EFFECT_TIMELINE_SIZE] = { 0, };
				// RED
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.color_timeline[i].x * 255;
				ImGui::PlotLines("RED", arr, IM_ARRAYSIZE(arr));

				// GREEN
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.color_timeline[i].y * 255;
				ImGui::PlotLines("GREEN", arr, IM_ARRAYSIZE(arr));

				// BLUE
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.color_timeline[i].z * 255;
				ImGui::PlotLines("BLUE", arr, IM_ARRAYSIZE(arr));

				// ALPHA
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.color_timeline[i].w * 255;
				ImGui::PlotLines("ALPHA", arr, IM_ARRAYSIZE(arr));
			}

			// ADD
			{
				// 타임라인에 추가할 시간
				static int color_lifetime = 0;
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderInt("Lifetime(%) (color)", &color_lifetime, 0, 100);
				// Color 
				static XMFLOAT4 color_to_add = { 1.0f, 1.0f, 1.0f, 1.0f };
				ImGui::SetNextItemWidth(300.0f);
				ImGui::ColorEdit4("Color to Add", (float*)&color_to_add);

				// ADD BUTTON
				if (ImGui::Button("Add Color to Timeline"))
				{
					emitter.color_timeline_map[color_lifetime] = color_to_add;
					ComputeColorTimeline(emitter.color_timeline_map, emitter.color_timeline);
				}
			}
			break;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Particle Size"))
		{
			// Size
			static E_EmitterAttributeType size_mode = INITIAL_SET;
			size_mode = emitter.size_setting_type;
			if (ImGui::RadioButton("Initial Setting(SIZE)", size_mode == INITIAL_SET))
			{
				size_mode = INITIAL_SET;
				emitter.size_setting_type = INITIAL_SET;
			}
			if (ImGui::RadioButton("Add per Lifetime(SIZE)", size_mode == ADD_PER_LIFETIME))
			{
				size_mode = ADD_PER_LIFETIME;
				emitter.size_setting_type = ADD_PER_LIFETIME;
			}
			if (ImGui::RadioButton("Set Per Lifetime(SIZE)", size_mode == SET_PER_LIFETIME))
			{
				size_mode = SET_PER_LIFETIME;
				emitter.size_setting_type = SET_PER_LIFETIME;
			}

			switch (size_mode)
			{
			case INITIAL_SET:
				ImGui::Text("Initial Size (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size Min", (float*)&emitter.initial_size[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size Max", (float*)&emitter.initial_size[MAX], "%.2f");
				break;
			case ADD_PER_LIFETIME:
				ImGui::Text("Initial Size (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size Min", (float*)&emitter.initial_size[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size Max", (float*)&emitter.initial_size[MAX], "%.2f");

				ImGui::Text("Size per lifetime (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size per Life Min", (float*)&emitter.add_size_per_lifetime[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size per Life Max", (float*)&emitter.add_size_per_lifetime[MAX], "%.2f");
				break;
			case SET_PER_LIFETIME:
				// VIEW
			{
				ImGui::Text("Size Timeline");
				float arr[EFFECT_TIMELINE_SIZE] = { 0, };
				// X
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.size_timeline[i].x;
				ImGui::PlotLines("Size X", arr, IM_ARRAYSIZE(arr));

				// Y
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.size_timeline[i].y;
				ImGui::PlotLines("Size Y", arr, IM_ARRAYSIZE(arr));

				// Z
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.size_timeline[i].z;
				ImGui::PlotLines("Size Z", arr, IM_ARRAYSIZE(arr));
			}

			// ADD
			{
				// 타임라인에 추가할 시간
				static int size_lifetime = 0;
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderInt("Lifetime(%) (Size)", &size_lifetime, 0, 100);
				// Color 
				static XMFLOAT3 size_to_add = { 0.0f, 0.0f, 0.0f };
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Size to Add", (float*)&size_to_add, "%.2f");

				// ADD BUTTON
				if (ImGui::Button("Add Size to Timeline"))
				{
					emitter.size_timeline_map[size_lifetime] = size_to_add;
					ComputeSizeTimeline(emitter.size_timeline_map, emitter.size_timeline);
				}
			}
			break;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Particle Rotation"))
		{
			// ROTATION
			static E_EmitterAttributeType rot_mode = INITIAL_SET;
			rot_mode = emitter.rotation_setting_type;
			if (ImGui::RadioButton("Initial Setting(ROT)", rot_mode == INITIAL_SET))
			{
				rot_mode = INITIAL_SET;
				emitter.rotation_setting_type = INITIAL_SET;
			}
			if (ImGui::RadioButton("Add per Lifetime(ROT)", rot_mode == ADD_PER_LIFETIME))
			{
				rot_mode = ADD_PER_LIFETIME;
				emitter.rotation_setting_type = ADD_PER_LIFETIME;
			}
			if (ImGui::RadioButton("Set Per Lifetime(ROT)", rot_mode == SET_PER_LIFETIME))
			{
				rot_mode = SET_PER_LIFETIME;
				emitter.rotation_setting_type = SET_PER_LIFETIME;
			}

			switch (rot_mode)
			{
			case INITIAL_SET:
				// Initial Rotation
				ImGui::Text("Initial Rotation (angle)");
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot Min", &emitter.initial_rotation[MIN]);
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot Max", &emitter.initial_rotation[MAX]);
				break;
			case ADD_PER_LIFETIME:
				// Initial Rotation
				ImGui::Text("Initial Rotation (angle)");
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot Min", &emitter.initial_rotation[MIN]);
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot Max", &emitter.initial_rotation[MAX]);

				// Rotation Per lifetime
				ImGui::Text("Rotation Per lifetime (Angle)");
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot per life Min", &emitter.add_rotation_per_lifetime[MIN]);
				ImGui::SetNextItemWidth(50.0f);
				ImGui::InputFloat("Rot per life Max", &emitter.add_rotation_per_lifetime[MAX]);
				break;
			case SET_PER_LIFETIME:
				// VIEW
			{
				ImGui::Text("Rotation Timeline");
				float arr[EFFECT_TIMELINE_SIZE] = { 0, };
				// Rot X
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.rotation_timeline[i];
				ImGui::PlotLines("Rotation", arr, IM_ARRAYSIZE(arr));
			}

			// ADD
			{
				// 타임라인에 추가할 시간
				static int rot_lifetime = 0.0f;
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderInt("Lifetime(%) (rot)", &rot_lifetime, 0, 100);
				// Color 
				static float rot_to_add = 0.0f;
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat("Rotation to Add", &rot_to_add, 0.0f, 0.0f, "%.2f");

				// ADD BUTTON
				if (ImGui::Button("Add Rotation to Timeline"))
				{
					emitter.rotation_timeline_map[rot_lifetime] = rot_to_add;
					ComputeRotationTimeline(emitter.rotation_timeline_map, emitter.rotation_timeline);
				}
			}
			break;
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Particle Velocity"))
		{
			// POSITION
			static E_EmitterAttributeType pos_mode = INITIAL_SET;
			pos_mode = emitter.position_setting_type;
			if (ImGui::RadioButton("Initial Setting(POS)", pos_mode == INITIAL_SET))
			{
				pos_mode = INITIAL_SET;
				emitter.position_setting_type = INITIAL_SET;
			}
			if (ImGui::RadioButton("Add per Lifetime(POS)", pos_mode == ADD_PER_LIFETIME))
			{
				pos_mode = ADD_PER_LIFETIME;
				emitter.position_setting_type = ADD_PER_LIFETIME;
			}
			if (ImGui::RadioButton("Set Per Lifetime(POS)", pos_mode == SET_PER_LIFETIME))
			{
				pos_mode = SET_PER_LIFETIME;
				emitter.position_setting_type = SET_PER_LIFETIME;
			}

			switch (pos_mode)
			{
			case INITIAL_SET:
				// Initial Position
				ImGui::Text("Initial Position (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Pos Min", (float*)&emitter.initial_position[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Pos Max", (float*)&emitter.initial_position[MAX], "%.2f");
				// Initial Velocity
				ImGui::Text("Initial Velocity (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Vel Min", (float*)&emitter.initial_velocity[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Vel Max", (float*)&emitter.initial_velocity[MAX], "%.2f");
				break;
			case ADD_PER_LIFETIME:
				// Initial Position
				ImGui::Text("Initial Position (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Pos Min", (float*)&emitter.initial_position[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Pos Max", (float*)&emitter.initial_position[MAX], "%.2f");
				
				// Initial Velocity
				ImGui::Text("Initial Velocity (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Vel Min", (float*)&emitter.initial_velocity[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Vel Max", (float*)&emitter.initial_velocity[MAX], "%.2f");

				// Velocity Per lifetime
				ImGui::Text("Accelation per lifetime (x,y,z)");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Acc per Life Min", (float*)&emitter.accelation_per_lifetime[MIN], "%.2f");
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Acc per Life Max", (float*)&emitter.accelation_per_lifetime[MAX], "%.2f");
				break;
			case SET_PER_LIFETIME:
				// VIEW
			{
				ImGui::Text("Velocity Timeline");
				float arr[EFFECT_TIMELINE_SIZE] = { 0, };
				// VelX
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.velocity_timeline[i].x;
				ImGui::PlotLines("Size X", arr, IM_ARRAYSIZE(arr));

				// VelY
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.velocity_timeline[i].y;
				ImGui::PlotLines("Size Y", arr, IM_ARRAYSIZE(arr));

				// VelZ
				for (int i = 0; i < EFFECT_TIMELINE_SIZE; i++)
					arr[i] = emitter.velocity_timeline[i].z;
				ImGui::PlotLines("Size Z", arr, IM_ARRAYSIZE(arr));
			}

			// ADD
			{
				// 타임라인에 추가할 시간
				static int vel_lifetime = 0.0f;
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderInt("Lifetime(%) (vel)", &vel_lifetime, 0, 100);
				// Color 
				static XMFLOAT3 vel_to_add = { 0.0f, 0.0f, 0.0f };
				ImGui::SetNextItemWidth(150.0f);
				ImGui::InputFloat3("Velocity to Add", (float*)&vel_to_add, "%.2f");

				// ADD BUTTON
				if (ImGui::Button("Add Vel to Timeline"))
				{
					emitter.velocity_timeline_map[vel_lifetime] = vel_to_add;
					ComputeVelocityTimeline(emitter.velocity_timeline_map, emitter.velocity_timeline);
				}
			}
			break;
			}

			ImGui::Checkbox("Gravity On / Off", &emitter.gravity_on_off);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Shader & Material"))
	{
		// Shader Selection
		SelectVertexShader(emitter.vs_id);
		SelectGeometryShader(emitter.geo_id);
		SelectMaterial(emitter.mat_id);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Blend State & Depth Stencil State"))
	{
		// BS, DS Selection
		SelectBSOptions(emitter.bs_state);
		SelectDSOptions(emitter.ds_state);

		ImGui::TreePop();
	}
}

void WG_EffectWindow::EffectBoard()
{
	if (ImGui::TreeNode("Emitters"))
	{
		// Emitter 리스트
		static int item_current_idx = 0;
		if (effect_data_.size() > 0)
			item_current_idx = min(item_current_idx, (int)effect_data_.size() - 1);
		else
			item_current_idx = 0;

		vector<string> emitter_vec;
		shared_ptr<Emitter> cur_emitter;
		{

			for (auto iter = effect_data_.begin(); iter != effect_data_.end(); ++iter)
			{
				emitter_vec.push_back(iter->first);
			}

			ImGui::SetNextItemWidth(LISTBOX_WIDTH);
			if (ImGui::BeginListBox("Emitters"))
			{
				for (int n = 0; n < emitter_vec.size(); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(emitter_vec[n].c_str(), is_selected))
						item_current_idx = n;

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();

				if (emitter_vec.size() > 0)
				{
					auto select = emitter_vec[item_current_idx];

					cur_emitter = effect_data_[select];
				}

			}
		}

		// Emitter 추가 / 제거
		{
			static ImGui::FileBrowser fileDialog;
			if (ImGui::Button("Add Emitter"))
			{
				fileDialog.Open();
				fileDialog.SetTypeFilters({ ".csv" });
			}

			fileDialog.Display();

			if (fileDialog.HasSelected())
			{
				auto emitter = make_shared<Emitter>();
				string path = fileDialog.GetSelected().string();
				LoadingEmitterData(path, *emitter.get());

				// 이름 파싱
				auto splited_str = split(path, '\\');
				auto strs = split(splited_str[max((int)splited_str.size() - 1, 0)], '.');
				auto name = strs[0];

				effect_data_.insert({ name, emitter });

				fileDialog.ClearSelected();
				fileDialog.Close();
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete Emitter"))
			{
				if (emitter_vec.size() > 0)
				{
					effect_data_.erase(emitter_vec[item_current_idx]);
				}

			}
		}

		// Emitter 수정
		if (cur_emitter != nullptr)
		{
			EmitterBoard(*cur_emitter.get());
			if (ImGui::Button("Emitter Render"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
				{
					scene->ResetEmitter();
					cur_emitter->particles.clear();
					scene->AddEmitter(cur_emitter);
				}

			}
		}
		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Effects Save & Rendering"))
	{
		// 세이브 / 랜더링
		static char effect_name[255] = { 0, };
		ImGui::SetNextItemWidth(TEXT_WIDTH);
		ImGui::InputTextWithHint("Effect Name", "Name", effect_name, IM_ARRAYSIZE(effect_name));
		if (ImGui::Button("Save Effect"))
		{
			SaveEffect(effect_name);
		}
		ImGui::SameLine();
		if (ImGui::Button("Render Effect"))
		{
			auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
			if (scene)
			{
				scene->ResetEmitter();
				for (auto pair : effect_data_)
				{
					pair.second->particles.clear();
					scene->AddEmitter(pair.second);
				}
			}

		}
		ImGui::TreePop();
	}	
}

// 세부 위젯들
void WG_EffectWindow::SelectBSOptions(E_EffectBS& bs_state)
{
	//static ImGuiComboFlags flags = 0;
	//ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);

	const char* items[] = { "DEFAULT_BS", "NO_BLEND", "ALPHA_BLEND", "DUALSOURCE_BLEND" };
	static int item_current_idx = 0; // Here we store our selection data as an index.
	item_current_idx = bs_state;
	const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
	if (ImGui::BeginCombo("Blend State", combo_preview_value))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(items[n], is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	bs_state = (E_EffectBS)item_current_idx;
}

void WG_EffectWindow::SelectDSOptions(E_EffectDS& ds_state)
{
	//static ImGuiComboFlags flags = 0;
	//ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);

	const char* items[] = { "DEFAULT_NONE", "DEPTH_COMP_NOWRITE", "DEPTH_COMP_WRITE" };
	static int item_current_idx = 0; // Here we store our selection data as an index.
	item_current_idx = ds_state;
	const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
	if (ImGui::BeginCombo("Depth Stencil State", combo_preview_value))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(items[n], is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ds_state = (E_EffectDS)item_current_idx;
}

void WG_EffectWindow::SelectFrame(int max_frame, int& cur_frame)
{
	max_frame= max(1, max_frame);

	// currentFrame 선택
	ImGui::SetNextItemWidth(200.0f);
	ImGui::SliderInt("Frame", &cur_frame, 1, max_frame);	

	static bool bPlay = false;
	static float timer = cur_frame;
	// Play 버튼
	if (ImGui::Button("Play"))
	{
		bPlay = true;
	}
	// Pause
	ImGui::SameLine();
	if (ImGui::Button("Pause"))
	{
		bPlay = false;
	}
	// Stop
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		bPlay = false;
		cur_frame = 1;
		timer = cur_frame;
	}

	if (bPlay)
	{
		timer += TIMER->GetDeltaTime();
		if (timer > max_frame + 1)
			timer -= max_frame;
		cur_frame = (int)timer;
	}

}

void WG_EffectWindow::SelectUV(vector<pair<POINT, POINT>>& list)
{
	static int start[2] = { 0, 0 };
	static int end[2] = { 0, 0 };
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("Start UV", start);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("End UV", end);

	ImGui::SameLine();
	if (ImGui::Button("Add UV"))
	{
		list.push_back({ {start[0], start[1]}, { end[0], end[1]} });
	}
	ImGui::SetNextItemWidth(LISTBOX_WIDTH + 100);
	static int item_current_idx = 0;
	if (ImGui::BeginListBox("UV per Frame"))
	{
		for (int n = 0; n < list.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			string str = "[" + to_string(n + 1) + "]"
				+ " Start (" + to_string(list[n].first.x) + ", " + to_string(list[n].first.y) + ")"
				+ " End (" + to_string(list[n].second.x) + ", " + to_string(list[n].second.y) + ")";
			if (ImGui::Selectable(str.c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}
	if (ImGui::Button("Delete Last Frame"))
	{
		if(list.size() > 0)
			list.pop_back();
	}
	
}

void WG_EffectWindow::SelectVertexShader(string& id)
{
	static int item_current_idx = 0;

	auto vs_set = RESOURCE->GetTotalVSID();
	vector<string> vs_vec(vs_set.begin(), vs_set.end());

	// 위에서 설정된 vs_id가 있다면 그 값으로 item_current_idx 변경
	for (int i = 0; i < vs_vec.size(); i++)
	{
		if (vs_vec[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Vertex Shader"))
	{
		for (int n = 0; n < vs_vec.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(vs_vec[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	id = vs_vec[item_current_idx];
}

void WG_EffectWindow::SelectGeometryShader(string& id)
{
	static int item_current_idx = 0;

	auto gs_set = RESOURCE->GetTotalGSID();
	vector<string> gs_vec(gs_set.begin(), gs_set.end());

	// 위에서 설정된 gs_id가 있다면 그 값으로 item_current_idx 변경
	for (int i = 0; i < gs_vec.size(); i++)
	{
		if (gs_vec[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Geometry Shader"))
	{
		for (int n = 0; n < gs_vec.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(gs_vec[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	id = gs_vec[item_current_idx];
}

void WG_EffectWindow::SelectMaterial(string& id)
{
	static int item_current_idx = 0;

	auto material_set = RESOURCE->GetTotalMATID();
	vector<string> mat_vec(material_set.begin(), material_set.end());

	// 위에서 설정된 ps_id가 있다면 그 값으로 item_current_idx 변경
	for (int i = 0; i < mat_vec.size(); i++)
	{
		if (mat_vec[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Pixel Shader"))
	{
		for (int n = 0; n < mat_vec.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(mat_vec[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	id = mat_vec[item_current_idx];
}

void WG_EffectWindow::SelectTexture(string& id)
{
	static int item_current_idx = 0;

	auto tex_set = RESOURCE->GetTotalTexID();
	vector<string> tex_vec(tex_set.begin(), tex_set.end());

	// 위에서 설정된 tex_id가 있다면 그 값으로 item_current_idx 변경
	for (int i = 0; i < tex_vec.size(); i++)
	{
		if (tex_vec[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Texture"))
	{
		for (int n = 0; n < tex_vec.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(tex_vec[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	id = tex_vec[item_current_idx];
}

void WG_EffectWindow::SelectSprite(string& id)
{
	static int item_current_idx = 0;

	// 가지고 있는 스프라이트 아이디를 리스트화 하기 위해 vector로 id 받아오기
	auto sprite_id_set = RESOURCE->GetTotalSpriteID();
	vector<string> sprite_id(sprite_id_set.begin(), sprite_id_set.end());

	// 위에서 설정된 tex_id가 있다면 그 값으로 item_current_idx 변경
	for (int i = 0; i < sprite_id.size(); i++)
	{
		if (sprite_id[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Sprite"))
	{
		for (int n = 0; n < sprite_id.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(sprite_id[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	if(sprite_id.size() > 0)
		id = sprite_id[item_current_idx];
}



// 세이브 / 로드 메소드들
void WG_EffectWindow::LoadingData()
{
	static ImGui::FileBrowser fileDialog;
	static int type = 0;
	if (ImGui::BeginMenu("Load Effect"))
	{
		if (ImGui::MenuItem("Load Sprite"))
		{
			type = 1;
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}
		if (ImGui::MenuItem("Load Emitter"))
		{
			type = 2;
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}
		if (ImGui::MenuItem("Load Effect"))
		{
			type = 3;
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}
		ImGui::EndMenu();
	}

	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		if (type == 1)
		{
			LoadingSpriteData(fileDialog.GetSelected().string());
		}
		else if (type == 2)
		{
			LoadingEmitterData(fileDialog.GetSelected().string(), emitter_data_);
			type_ = EMITTER;
		}
		else if (type == 3)
		{
			effect_data_.clear();
			LoadingEffectData(fileDialog.GetSelected().string());
			type_ = EFFECT;
		}
		fileDialog.ClearSelected();
		fileDialog.Close();
	}
}

void WG_EffectWindow::LoadingSpriteData(string path)
{
	auto strs1 = split(path, '\\');
	auto name = strs1[max((int)strs1.size() - 1, 0)];
	auto strs2 = split(name, '.');
	name = strs2[0];

	auto sprite = RESOURCE->UseResource<Sprite>(name);

	if (!sprite)
	{
		if (!RESOURCE->ImportSprite(path))
			return;
		sprite = RESOURCE->UseResource<Sprite>(name);
	}

	UVSprite* uv_sprite = nullptr;
	TextureSprite* tex_sprite = nullptr;

	switch (sprite->type)
	{
	case UV_SPRITE:
		type_ = UV_SPRITE;
		uv_sprite = (UVSprite*)sprite;
		if (uv_sprite)
			uv_sprite_data_ = *uv_sprite;
		break;
	case TEX_SPRITE:
		type_ = TEX_SPRITE;
		tex_sprite = (TextureSprite*)sprite;
		if (tex_sprite)
			tex_sprite_data_ = *tex_sprite;
		break;
	}
}

void WG_EffectWindow::LoadingEmitterData(string path, Emitter& emitter)
{
	auto strs1 = split(path, '\\');
	auto name = strs1[max((int)strs1.size() - 1, 0)];
	auto strs2 = split(name, '.');
	name = strs2[0];

	auto sheet = DATA->LoadSheet(name);

	if (sheet == NULL)
	{
		DATA->LoadSheetFile(path);
		sheet = DATA->LoadSheet(name);
	}

	if (sheet == NULL)
		return;

	auto effect = sheet->LoadItem(name);

	// type
	emitter.type = (E_EffectType)stoi(effect->GetValue("type"));

	// sprite_id
	emitter.sprite_id = effect->GetValue("sprite_id");

	// emit_type
	emitter.emit_type = (E_EmitType)stoi(effect->GetValue("emit_type"));
	// emit_once
	emitter.emit_once = stoi(effect->GetValue("emit_once"));
	// emit_per_second
	emitter.emit_per_second = stoi(effect->GetValue("emit_per_second"));
	// emit_time
	emitter.emit_time = stof(effect->GetValue("emit_time"));

	

	vector<string> splited_str;
	vector<string> splited_str2;

	// life_time
	{
		splited_str = split(effect->GetValue("life_time"), ' ');
		if (splited_str.size() < 2)
			return;
		emitter.life_time[0] = stof(splited_str[0]);
		emitter.life_time[1] = stof(splited_str[1]);
	}

	// SettingType
	{
		emitter.color_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("color_setting_type"));
		emitter.size_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("size_setting_type"));
		emitter.rotation_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("rotation_setting_type"));
		emitter.position_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("position_setting_type"));
	}

	// COLOR
	{
		// initial_color
		splited_str = split(effect->GetValue("initial_color"), ' ');
		if (splited_str.size() < 4)
			return;
		emitter.initial_color.x = stof(splited_str[0]);
		emitter.initial_color.y = stof(splited_str[1]);
		emitter.initial_color.z = stof(splited_str[2]);
		emitter.initial_color.w = stof(splited_str[3]);

		// color_timeline_map
		{
			string str_color_map = effect->GetValue("color_timeline_map");
			if (str_color_map.size())
			{
				splited_str = split(str_color_map, '~');
				for (auto value : splited_str)
				{
					auto splited_map_value = split(value, '-');

					if (splited_map_value.size() == 0)
						return;
					int time = stoi(splited_map_value[0]);

					auto splited_map_xyzw = split(splited_map_value[1], ' ');
					XMFLOAT4 color = { stof(splited_map_xyzw[0]), stof(splited_map_xyzw[1]), stof(splited_map_xyzw[2]), stof(splited_map_xyzw[3]) };

					emitter.color_timeline_map.insert({ time, color });
				}
				ComputeColorTimeline(emitter.color_timeline_map, emitter.color_timeline);
			}
		}

	}

	// SIZE
	{
		// initial_size
		{
			splited_str = split(effect->GetValue("initial_size"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_size[0].x = stof(splited_str2[0]);
			emitter.initial_size[0].y = stof(splited_str2[1]);
			emitter.initial_size[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_size[1].x = stof(splited_str2[0]);
			emitter.initial_size[1].y = stof(splited_str2[1]);
			emitter.initial_size[1].z = stof(splited_str2[2]);
		}
		// add_size_per_lifetime
		{
			splited_str = split(effect->GetValue("add_size_per_lifetime"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.add_size_per_lifetime[0].x = stof(splited_str2[0]);
			emitter.add_size_per_lifetime[0].y = stof(splited_str2[1]);
			emitter.add_size_per_lifetime[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.add_size_per_lifetime[1].x = stof(splited_str2[0]);
			emitter.add_size_per_lifetime[1].y = stof(splited_str2[1]);
			emitter.add_size_per_lifetime[1].z = stof(splited_str2[2]);
		}
		// size_timeline_map
		{
			string str_size_map = effect->GetValue("size_timeline_map");
			if (str_size_map.size())
			{
				splited_str = split(str_size_map, '~');
				for (auto value : splited_str)
				{
					auto splited_map_value = split(value, '-');

					if (splited_map_value.size() == 0)
						return;
					int time = stoi(splited_map_value[0]);

					auto splited_map_xyz = split(splited_map_value[1], ' ');
					XMFLOAT3 size = { stof(splited_map_xyz[0]), stof(splited_map_xyz[1]), stof(splited_map_xyz[2]) };

					emitter.size_timeline_map.insert({ time, size });
				}
				ComputeSizeTimeline(emitter.size_timeline_map, emitter.size_timeline);
			}
		}
		
		
	}
	
	// ROTATION
	{
		// initial_rotation
		{
			splited_str = split(effect->GetValue("initial_rotation"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter.initial_rotation[0] = stof(splited_str[0]);
			emitter.initial_rotation[1] = stof(splited_str[1]);
		}
		// add_rotation_per_lifetime
		{
			splited_str = split(effect->GetValue("add_rotation_per_lifetime"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter.add_rotation_per_lifetime[0] = stof(splited_str[0]);
			emitter.add_rotation_per_lifetime[1] = stof(splited_str[1]);
		}
		// rotation_timeline_map
		{
			string str_rot_map = effect->GetValue("rotation_timeline_map");
			if (str_rot_map.size())
			{
				splited_str = split(str_rot_map, '~');
				for (auto value : splited_str)
				{
					auto splited_map_value = split(value, '-');

					if (splited_map_value.size() == 0)
						return;
					int time = stoi(splited_map_value[0]);

					auto splited_map_xyz = split(splited_map_value[1], ' ');
					float rotation = stof(splited_map_xyz[0]);

					emitter.rotation_timeline_map.insert({ time, rotation });
				}
				ComputeRotationTimeline(emitter.rotation_timeline_map, emitter.rotation_timeline);
			}
		}
	}

	// VELOCITY
	{
		// initial_position
		{
			splited_str = split(effect->GetValue("initial_position"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_position[0].x = stof(splited_str2[0]);
			emitter.initial_position[0].y = stof(splited_str2[1]);
			emitter.initial_position[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_position[1].x = stof(splited_str2[0]);
			emitter.initial_position[1].y = stof(splited_str2[1]);
			emitter.initial_position[1].z = stof(splited_str2[2]);
		}
		// initial_velocity
		{
			splited_str = split(effect->GetValue("initial_velocity"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_velocity[0].x = stof(splited_str2[0]);
			emitter.initial_velocity[0].y = stof(splited_str2[1]);
			emitter.initial_velocity[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.initial_velocity[1].x = stof(splited_str2[0]);
			emitter.initial_velocity[1].y = stof(splited_str2[1]);
			emitter.initial_velocity[1].z = stof(splited_str2[2]);
		}
		// accelation_per_lifetime
		{
			splited_str = split(effect->GetValue("accelation_per_lifetime"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.accelation_per_lifetime[0].x = stof(splited_str2[0]);
			emitter.accelation_per_lifetime[0].y = stof(splited_str2[1]);
			emitter.accelation_per_lifetime[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter.accelation_per_lifetime[1].x = stof(splited_str2[0]);
			emitter.accelation_per_lifetime[1].y = stof(splited_str2[1]);
			emitter.accelation_per_lifetime[1].z = stof(splited_str2[2]);
		}
		// velocity_timeline_map
		{
			string str_vel_map = effect->GetValue("velocity_timeline_map");
			if (str_vel_map.size())
			{
				splited_str = split(str_vel_map, '~');
				for (auto value : splited_str)
				{
					auto splited_map_value = split(value, '-');

					if (splited_map_value.size() == 0)
						return;
					int time = stoi(splited_map_value[0]);

					auto splited_map_xyz = split(splited_map_value[1], ' ');
					XMFLOAT3 vel = { stof(splited_map_xyz[0]), stof(splited_map_xyz[1]), stof(splited_map_xyz[2]) };

					emitter.velocity_timeline_map.insert({ time, vel });
				}
				ComputeVelocityTimeline(emitter.velocity_timeline_map, emitter.velocity_timeline);
			}
		}
	}

	// GRAVITY
	emitter.gravity_on_off = stoi(effect->GetValue("gravity_on_off"));

	// BS
	emitter.bs_state = (E_EffectBS)stoi(effect->GetValue("BS"));

	// DS
	emitter.ds_state = (E_EffectDS)stoi(effect->GetValue("DS"));

	emitter.vs_id = effect->GetValue("vs_id");
	emitter.geo_id = effect->GetValue("geo_id");
	emitter.mat_id = effect->GetValue("mat_id");
}

void WG_EffectWindow::LoadingEffectData(string path)
{
	auto strs1 = split(path, '\\');
	auto name = strs1[max((int)strs1.size() - 1, 0)];
	auto strs2 = split(name, '.');
	name = strs2[0];

	auto sheet = DATA->LoadSheet(name);

	if (sheet == NULL)
	{
		DATA->LoadSheetFile(path);
		sheet = DATA->LoadSheet(name);
	}

	if (sheet == NULL)
		return;

	for (auto pair : sheet->resdic_item)
	{
		auto effect = pair.second;

		auto emitter = make_shared<Emitter>();


		// type
		emitter->type = (E_EffectType)stoi(effect->GetValue("type"));

		// sprite_id
		emitter->sprite_id = effect->GetValue("sprite_id");

		// emit_type
		emitter->emit_type = (E_EmitType)stoi(effect->GetValue("emit_type"));
		// emit_once
		emitter->emit_once = stoi(effect->GetValue("emit_once"));
		// emit_per_second
		emitter->emit_per_second = stoi(effect->GetValue("emit_per_second"));
		// emit_time
		emitter->emit_time = stof(effect->GetValue("emit_time"));



		vector<string> splited_str;
		vector<string> splited_str2;

		// life_time
		{
			splited_str = split(effect->GetValue("life_time"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter->life_time[0] = stof(splited_str[0]);
			emitter->life_time[1] = stof(splited_str[1]);
		}

		// SettingType
		{
			emitter->color_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("color_setting_type"));
			emitter->size_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("size_setting_type"));
			emitter->rotation_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("rotation_setting_type"));
			emitter->position_setting_type = (E_EmitterAttributeType)stoi(effect->GetValue("position_setting_type"));
		}

		// COLOR
		{
			// initial_color
			splited_str = split(effect->GetValue("initial_color"), ' ');
			if (splited_str.size() < 4)
				return;
			emitter->initial_color.x = stof(splited_str[0]);
			emitter->initial_color.y = stof(splited_str[1]);
			emitter->initial_color.z = stof(splited_str[2]);
			emitter->initial_color.w = stof(splited_str[3]);

			// color_timeline_map
			{
				string str_color_map = effect->GetValue("color_timeline_map");
				if (str_color_map.size())
				{
					splited_str = split(str_color_map, '~');
					for (auto value : splited_str)
					{
						auto splited_map_value = split(value, '-');

						if (splited_map_value.size() == 0)
							return;
						int time = stoi(splited_map_value[0]);

						auto splited_map_xyzw = split(splited_map_value[1], ' ');
						XMFLOAT4 color = { stof(splited_map_xyzw[0]), stof(splited_map_xyzw[1]), stof(splited_map_xyzw[2]), stof(splited_map_xyzw[3]) };

						emitter->color_timeline_map.insert({ time, color });
					}
					ComputeColorTimeline(emitter->color_timeline_map, emitter->color_timeline);
				}
			}

		}

		// SIZE
		{
			// initial_size
			{
				splited_str = split(effect->GetValue("initial_size"), '~');
				if (splited_str.size() < 2)
					return;
				// min
				splited_str2 = split(splited_str[0], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_size[0].x = stof(splited_str2[0]);
				emitter->initial_size[0].y = stof(splited_str2[1]);
				emitter->initial_size[0].z = stof(splited_str2[2]);
				// max
				splited_str2 = split(splited_str[1], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_size[1].x = stof(splited_str2[0]);
				emitter->initial_size[1].y = stof(splited_str2[1]);
				emitter->initial_size[1].z = stof(splited_str2[2]);
			}
			// add_size_per_lifetime
			{
				splited_str = split(effect->GetValue("add_size_per_lifetime"), '~');
				if (splited_str.size() < 2)
					return;
				// min
				splited_str2 = split(splited_str[0], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->add_size_per_lifetime[0].x = stof(splited_str2[0]);
				emitter->add_size_per_lifetime[0].y = stof(splited_str2[1]);
				emitter->add_size_per_lifetime[0].z = stof(splited_str2[2]);
				// max
				splited_str2 = split(splited_str[1], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->add_size_per_lifetime[1].x = stof(splited_str2[0]);
				emitter->add_size_per_lifetime[1].y = stof(splited_str2[1]);
				emitter->add_size_per_lifetime[1].z = stof(splited_str2[2]);
			}
			// size_timeline_map
			{
				string str_size_map = effect->GetValue("size_timeline_map");
				if (str_size_map.size())
				{
					splited_str = split(str_size_map, '~');
					for (auto value : splited_str)
					{
						auto splited_map_value = split(value, '-');

						if (splited_map_value.size() == 0)
							return;
						int time = stoi(splited_map_value[0]);

						auto splited_map_xyz = split(splited_map_value[1], ' ');
						XMFLOAT3 size = { stof(splited_map_xyz[0]), stof(splited_map_xyz[1]), stof(splited_map_xyz[2]) };

						emitter->size_timeline_map.insert({ time, size });
					}
					ComputeSizeTimeline(emitter->size_timeline_map, emitter->size_timeline);
				}
			}


		}

		// ROTATION
		{
			// initial_rotation
			{
				splited_str = split(effect->GetValue("initial_rotation"), ' ');
				if (splited_str.size() < 2)
					return;
				emitter->initial_rotation[0] = stof(splited_str[0]);
				emitter->initial_rotation[1] = stof(splited_str[1]);
			}
			// add_rotation_per_lifetime
			{
				splited_str = split(effect->GetValue("add_rotation_per_lifetime"), ' ');
				if (splited_str.size() < 2)
					return;
				emitter->add_rotation_per_lifetime[0] = stof(splited_str[0]);
				emitter->add_rotation_per_lifetime[1] = stof(splited_str[1]);
			}
			// rotation_timeline_map
			{
				string str_rot_map = effect->GetValue("rotation_timeline_map");
				if (str_rot_map.size())
				{
					splited_str = split(str_rot_map, '~');
					for (auto value : splited_str)
					{
						auto splited_map_value = split(value, '-');

						if (splited_map_value.size() == 0)
							return;
						int time = stoi(splited_map_value[0]);

						auto splited_map_xyz = split(splited_map_value[1], ' ');
						float rotation = stof(splited_map_xyz[0]);


						emitter->rotation_timeline_map.insert({ time, rotation });
					}
					ComputeRotationTimeline(
						emitter->rotation_timeline_map, emitter->rotation_timeline);
				}
			}
		}

		// VELOCITY
		{
			// initial_position
			{
				splited_str = split(effect->GetValue("initial_position"), '~');
				if (splited_str.size() < 2)
					return;
				// min
				splited_str2 = split(splited_str[0], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_position[0].x = stof(splited_str2[0]);
				emitter->initial_position[0].y = stof(splited_str2[1]);
				emitter->initial_position[0].z = stof(splited_str2[2]);
				// max
				splited_str2 = split(splited_str[1], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_position[1].x = stof(splited_str2[0]);
				emitter->initial_position[1].y = stof(splited_str2[1]);
				emitter->initial_position[1].z = stof(splited_str2[2]);
			}
			// initial_velocity
			{
				splited_str = split(effect->GetValue("initial_velocity"), '~');
				if (splited_str.size() < 2)
					return;
				// min
				splited_str2 = split(splited_str[0], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_velocity[0].x = stof(splited_str2[0]);
				emitter->initial_velocity[0].y = stof(splited_str2[1]);
				emitter->initial_velocity[0].z = stof(splited_str2[2]);
				// max
				splited_str2 = split(splited_str[1], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->initial_velocity[1].x = stof(splited_str2[0]);
				emitter->initial_velocity[1].y = stof(splited_str2[1]);
				emitter->initial_velocity[1].z = stof(splited_str2[2]);
			}
			// accelation_per_lifetime
			{
				splited_str = split(effect->GetValue("accelation_per_lifetime"), '~');
				if (splited_str.size() < 2)
					return;
				// min
				splited_str2 = split(splited_str[0], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->accelation_per_lifetime[0].x = stof(splited_str2[0]);
				emitter->accelation_per_lifetime[0].y = stof(splited_str2[1]);
				emitter->accelation_per_lifetime[0].z = stof(splited_str2[2]);
				// max
				splited_str2 = split(splited_str[1], ' ');
				if (splited_str2.size() < 3)
					return;
				emitter->accelation_per_lifetime[1].x = stof(splited_str2[0]);
				emitter->accelation_per_lifetime[1].y = stof(splited_str2[1]);
				emitter->accelation_per_lifetime[1].z = stof(splited_str2[2]);
			}
			// velocity_timeline_map
			{
				string str_vel_map = effect->GetValue("velocity_timeline_map");
				if (str_vel_map.size())
				{
					splited_str = split(str_vel_map, '~');
					for (auto value : splited_str)
					{
						auto splited_map_value = split(value, '-');

						if (splited_map_value.size() == 0)
							return;
						int time = stoi(splited_map_value[0]);

						auto splited_map_xyz = split(splited_map_value[1], ' ');
						XMFLOAT3 vel = { stof(splited_map_xyz[0]), stof(splited_map_xyz[1]), stof(splited_map_xyz[2]) };

						emitter->velocity_timeline_map.insert({ time, vel });
					}
					ComputeVelocityTimeline(emitter->velocity_timeline_map, emitter->velocity_timeline);
				}
			}
		}

		// GRAVITY
		emitter->gravity_on_off = stoi(effect->GetValue("gravity_on_off"));

		// BS
		emitter->bs_state = (E_EffectBS)stoi(effect->GetValue("BS"));

		// DS
		emitter->ds_state = (E_EffectDS)stoi(effect->GetValue("DS"));

		emitter->vs_id = effect->GetValue("vs_id");
		emitter->geo_id = effect->GetValue("geo_id");
		emitter->mat_id = effect->GetValue("mat_id");

		effect_data_.insert({ pair.first, emitter });
	}


}

void WG_EffectWindow::SaveUVSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);
	auto list = sheet->AddItem("uvList");
	
	sheet->AddCategory("tex_id");
	sheet->AddCategory("type");

	for (int i = 0; i < uv_sprite_data_.uv_list.size(); i++)
		sheet->AddCategory(to_string(i+1));

	effect->SetValue("tex_id", uv_sprite_data_.tex_id);
	effect->SetValue("type", to_string(UV_SPRITE));

	for (int i = 0; i < uv_sprite_data_.uv_list.size(); i++)
	{
		string uvStr = to_string(uv_sprite_data_.uv_list[i].first.x) + " " + to_string(uv_sprite_data_.uv_list[i].first.y) + " " 
			+ to_string(uv_sprite_data_.uv_list[i].second.x) + " " + to_string(uv_sprite_data_.uv_list[i].second.y);
		list->SetValue(to_string(i + 1), uvStr);
	}
		
	DATA->SaveSheetFile(sheetName);

	RESOURCE->SaveSprite(name, make_shared<UVSprite>(uv_sprite_data_));
}

void WG_EffectWindow::SaveTexSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);
	auto list = sheet->AddItem("texList");

	sheet->AddCategory("type");

	for (int i = 0; i < tex_sprite_data_.tex_id_list.size(); i++)
		sheet->AddCategory(to_string(i + 1));

	effect->SetValue("type", to_string(TEX_SPRITE));

	for (int i = 0; i < tex_sprite_data_.tex_id_list.size(); i++)
	{
		list->SetValue(to_string(i + 1), tex_sprite_data_.tex_id_list[i]);
	}

	DATA->SaveSheetFile(sheetName);

	// 세이브 한 스프라이트를 스프라이트 리스트에 추가한다.
	RESOURCE->SaveSprite(name, make_shared<TextureSprite>(tex_sprite_data_));
}

void WG_EffectWindow::SaveEmitter(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);

	// 카테고리 추가
	sheet->AddCategory("type");

	sheet->AddCategory("sprite_id");

	sheet->AddCategory("emit_type");
	sheet->AddCategory("emit_per_second");
	sheet->AddCategory("emit_once");
	sheet->AddCategory("emit_time");

	sheet->AddCategory("life_time");

	sheet->AddCategory("color_setting_type");
	sheet->AddCategory("size_setting_type");
	sheet->AddCategory("rotation_setting_type");
	sheet->AddCategory("position_setting_type");

	// COLOR
	sheet->AddCategory("initial_color");
	sheet->AddCategory("color_timeline_map");

	// SIZE
	sheet->AddCategory("initial_size");
	sheet->AddCategory("add_size_per_lifetime");
	sheet->AddCategory("size_timeline_map");

	// ROTATION
	sheet->AddCategory("initial_rotation");
	sheet->AddCategory("add_rotation_per_lifetime");
	sheet->AddCategory("rotation_timeline_map");

	// POSITION
	sheet->AddCategory("initial_position");
	sheet->AddCategory("initial_velocity");
	sheet->AddCategory("accelation_per_lifetime");
	sheet->AddCategory("velocity_timeline_map");

	// GRAVITY
	sheet->AddCategory("gravity_on_off");

	sheet->AddCategory("vs_id");
	sheet->AddCategory("geo_id");
	sheet->AddCategory("mat_id");

	sheet->AddCategory("BS");
	sheet->AddCategory("DS");

	// 값 추가

	// type
	effect->SetValue("type", to_string(EMITTER));

	// sprite_id
	effect->SetValue("sprite_id", emitter_data_.sprite_id);

	// emit_type
	effect->SetValue("emit_type", to_string(emitter_data_.emit_type));
	// emit_once
	effect->SetValue("emit_once", to_string(emitter_data_.emit_once));
	// emit_per_second
	effect->SetValue("emit_per_second", to_string(emitter_data_.emit_per_second));
	// emit_time
	effect->SetValue("emit_time", to_string(emitter_data_.emit_time));

	string fmt = "";

	// life_time
	fmt = to_string(emitter_data_.life_time[0]) + " " + to_string(emitter_data_.life_time[1]);
	effect->SetValue("life_time", fmt);

	// Setting Types
	effect->SetValue("color_setting_type", to_string(emitter_data_.color_setting_type));
	effect->SetValue("size_setting_type", to_string(emitter_data_.size_setting_type));
	effect->SetValue("rotation_setting_type", to_string(emitter_data_.rotation_setting_type));
	effect->SetValue("position_setting_type", to_string(emitter_data_.position_setting_type));

	// COLOR
	{
		// initial_color
		fmt = to_string(emitter_data_.initial_color.x) + " " + to_string(emitter_data_.initial_color.y) + " " + to_string(emitter_data_.initial_color.z) + " " + to_string(emitter_data_.initial_color.w);
		effect->SetValue("initial_color", fmt);
		// color_timeline_map
			// 10-x y z w~100-x y z w~
		fmt = "";
		for (auto& pair : emitter_data_.color_timeline_map)
			fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) + " " + to_string(pair.second.w) + "~";
		effect->SetValue("color_timeline_map", fmt);
	}
	
	// SIZE
	{
		// initial_size
		fmt = to_string(emitter_data_.initial_size[0].x) + " " + to_string(emitter_data_.initial_size[0].y) + " " + to_string(emitter_data_.initial_size[0].z) + "~"
			+ to_string(emitter_data_.initial_size[1].x) + " " + to_string(emitter_data_.initial_size[1].y) + " " + to_string(emitter_data_.initial_size[1].z);
		effect->SetValue("initial_size", fmt);
		// add_size_per_lifetime
		fmt = to_string(emitter_data_.add_size_per_lifetime[0].x) + " " + to_string(emitter_data_.add_size_per_lifetime[0].y) + " " + to_string(emitter_data_.add_size_per_lifetime[0].z) + "~"
			+ to_string(emitter_data_.add_size_per_lifetime[1].x) + " " + to_string(emitter_data_.add_size_per_lifetime[1].y) + " " + to_string(emitter_data_.add_size_per_lifetime[1].z);
		effect->SetValue("add_size_per_lifetime", fmt);
		// size_timeline_map
			// 10-x y z~100-x y z~
		fmt = "";
		for (auto& pair : emitter_data_.size_timeline_map)
			fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) + "~";
		effect->SetValue("size_timeline_map", fmt);
	}
	
	// ROTATION
	{
		// initial_rotation
		fmt = to_string(emitter_data_.initial_rotation[0]) + " " + to_string(emitter_data_.initial_rotation[1]);
		effect->SetValue("initial_rotation", fmt);
		// add_rotation_per_lifetime
		fmt = to_string(emitter_data_.add_rotation_per_lifetime[0]) + " " + to_string(emitter_data_.add_rotation_per_lifetime[1]);
		effect->SetValue("add_rotation_per_lifetime", fmt);
		// rotation_timeline_map	
			// 10-30~100-40~
		fmt = "";
		for (auto& pair : emitter_data_.rotation_timeline_map)
			fmt += to_string(pair.first) + "-" + to_string(pair.second) + "~";
		effect->SetValue("rotation_timeline_map", fmt);
	}
	
	// VELOCITY
	{
		// initial_position
		fmt = to_string(emitter_data_.initial_position[0].x) + " " + to_string(emitter_data_.initial_position[0].y) + " " + to_string(emitter_data_.initial_position[0].z) + "~"
			+ to_string(emitter_data_.initial_position[1].x) + " " + to_string(emitter_data_.initial_position[1].y) + " " + to_string(emitter_data_.initial_position[1].z);
		effect->SetValue("initial_position", fmt);
		// initial_velocity
		fmt = to_string(emitter_data_.initial_velocity[0].x) + " " + to_string(emitter_data_.initial_velocity[0].y) + " " + to_string(emitter_data_.initial_velocity[0].z) + "~"
			+ to_string(emitter_data_.initial_velocity[1].x) + " " + to_string(emitter_data_.initial_velocity[1].y) + " " + to_string(emitter_data_.initial_velocity[1].z);
		effect->SetValue("initial_velocity", fmt);
		// accelation_per_lifetime
		fmt = to_string(emitter_data_.accelation_per_lifetime[0].x) + " " + to_string(emitter_data_.accelation_per_lifetime[0].y) + " " + to_string(emitter_data_.accelation_per_lifetime[0].z) + "~"
			+ to_string(emitter_data_.accelation_per_lifetime[1].x) + " " + to_string(emitter_data_.accelation_per_lifetime[1].y) + " " + to_string(emitter_data_.accelation_per_lifetime[1].z);
		effect->SetValue("accelation_per_lifetime", fmt);
		// velocity_timeline_map
			// 10-x y z~100-x y z~
		fmt = "";
		for (auto& pair : emitter_data_.velocity_timeline_map)
			fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) +  "~";
		effect->SetValue("velocity_timeline_map", fmt);
	}
	
	// GRAVITY
	effect->SetValue("gravity_on_off", to_string(emitter_data_.gravity_on_off));


	// vs_id
	effect->SetValue("vs_id", emitter_data_.vs_id);
	// geo_id
	effect->SetValue("geo_id", emitter_data_.geo_id);
	// ps_id
	effect->SetValue("mat_id", emitter_data_.mat_id);


	// BS
	effect->SetValue("BS", to_string(emitter_data_.bs_state));
	// DS
	effect->SetValue("DS", to_string(emitter_data_.ds_state));


	DATA->SaveSheetFile(sheetName);
}

void WG_EffectWindow::SaveEffect(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	for (auto& pair : effect_data_)
	{
		auto effect = sheet->AddItem(pair.first);
		auto emitter = pair.second;

		// 카테고리 추가
		sheet->AddCategory("type");

		sheet->AddCategory("sprite_id");

		sheet->AddCategory("emit_type");
		sheet->AddCategory("emit_per_second");
		sheet->AddCategory("emit_once");
		sheet->AddCategory("emit_time");

		sheet->AddCategory("life_time");

		sheet->AddCategory("color_setting_type");
		sheet->AddCategory("size_setting_type");
		sheet->AddCategory("rotation_setting_type");
		sheet->AddCategory("position_setting_type");

		// COLOR
		sheet->AddCategory("initial_color");
		sheet->AddCategory("color_timeline_map");

		// SIZE
		sheet->AddCategory("initial_size");
		sheet->AddCategory("add_size_per_lifetime");
		sheet->AddCategory("size_timeline_map");

		// ROTATION
		sheet->AddCategory("initial_rotation");
		sheet->AddCategory("add_rotation_per_lifetime");
		sheet->AddCategory("rotation_timeline_map");

		// POSITION
		sheet->AddCategory("initial_position");
		sheet->AddCategory("initial_velocity");
		sheet->AddCategory("accelation_per_lifetime");
		sheet->AddCategory("velocity_timeline_map");

		// GRAVITY
		sheet->AddCategory("gravity_on_off");

		sheet->AddCategory("vs_id");
		sheet->AddCategory("geo_id");
		sheet->AddCategory("mat_id");

		sheet->AddCategory("BS");
		sheet->AddCategory("DS");

		// 값 추가

		// type
		effect->SetValue("type", to_string(EMITTER));

		// sprite_id
		effect->SetValue("sprite_id", emitter->sprite_id);

		// emit_type
		effect->SetValue("emit_type", to_string(emitter->emit_type));
		// emit_once
		effect->SetValue("emit_once", to_string(emitter->emit_once));
		// emit_per_second
		effect->SetValue("emit_per_second", to_string(emitter->emit_per_second));
		// emit_time
		effect->SetValue("emit_time", to_string(emitter->emit_time));

		string fmt = "";

		// life_time
		fmt = to_string(emitter->life_time[0]) + " " + to_string(emitter->life_time[1]);
		effect->SetValue("life_time", fmt);

		// Setting Types
		effect->SetValue("color_setting_type", to_string(emitter->color_setting_type));
		effect->SetValue("size_setting_type", to_string(emitter->size_setting_type));
		effect->SetValue("rotation_setting_type", to_string(emitter->rotation_setting_type));
		effect->SetValue("position_setting_type", to_string(emitter->position_setting_type));

		// COLOR
		{
			// initial_color
			fmt = to_string(emitter->initial_color.x) + " " + to_string(emitter->initial_color.y) + " " + to_string(emitter->initial_color.z) + " " + to_string(emitter->initial_color.w);
			effect->SetValue("initial_color", fmt);
			// color_timeline_map
				// 10-x y z w~100-x y z w~
			fmt = "";
			for (auto& pair : emitter->color_timeline_map)
				fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) + " " + to_string(pair.second.w) + "~";
			effect->SetValue("color_timeline_map", fmt);
		}

		// SIZE
		{
			// initial_size
			fmt = to_string(emitter->initial_size[0].x) + " " + to_string(emitter->initial_size[0].y) + " " + to_string(emitter->initial_size[0].z) + "~"
				+ to_string(emitter->initial_size[1].x) + " " + to_string(emitter->initial_size[1].y) + " " + to_string(emitter->initial_size[1].z);
			effect->SetValue("initial_size", fmt);
			// add_size_per_lifetime
			fmt = to_string(emitter->add_size_per_lifetime[0].x) + " " + to_string(emitter->add_size_per_lifetime[0].y) + " " + to_string(emitter->add_size_per_lifetime[0].z) + "~"
				+ to_string(emitter->add_size_per_lifetime[1].x) + " " + to_string(emitter->add_size_per_lifetime[1].y) + " " + to_string(emitter->add_size_per_lifetime[1].z);
			effect->SetValue("add_size_per_lifetime", fmt);
			// size_timeline_map
				// 10-x y z~100-x y z~
			fmt = "";
			for (auto& pair : emitter->size_timeline_map)
				fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) + "~";
			effect->SetValue("size_timeline_map", fmt);
		}

		// ROTATION
		{
			// initial_rotation
			fmt = to_string(emitter->initial_rotation[0]) + " " + to_string(emitter->initial_rotation[1]);
			effect->SetValue("initial_rotation", fmt);
			// add_rotation_per_lifetime
			fmt = to_string(emitter->add_rotation_per_lifetime[0]) + " " + to_string(emitter->add_rotation_per_lifetime[1]);
			effect->SetValue("add_rotation_per_lifetime", fmt);
			// rotation_timeline_map	
				// 10-30~100-40~
			fmt = "";
			for (auto& pair : emitter->rotation_timeline_map)
				fmt += to_string(pair.first) + "-" + to_string(pair.second) + "~";
			effect->SetValue("rotation_timeline_map", fmt);
		}

		// VELOCITY
		{
			// initial_position
			fmt = to_string(emitter->initial_position[0].x) + " " + to_string(emitter->initial_position[0].y) + " " + to_string(emitter->initial_position[0].z) + "~"
				+ to_string(emitter->initial_position[1].x) + " " + to_string(emitter->initial_position[1].y) + " " + to_string(emitter->initial_position[1].z);
			effect->SetValue("initial_position", fmt);
			// initial_velocity
			fmt = to_string(emitter->initial_velocity[0].x) + " " + to_string(emitter->initial_velocity[0].y) + " " + to_string(emitter->initial_velocity[0].z) + "~"
				+ to_string(emitter->initial_velocity[1].x) + " " + to_string(emitter->initial_velocity[1].y) + " " + to_string(emitter->initial_velocity[1].z);
			effect->SetValue("initial_velocity", fmt);
			// accelation_per_lifetime
			fmt = to_string(emitter->accelation_per_lifetime[0].x) + " " + to_string(emitter->accelation_per_lifetime[0].y) + " " + to_string(emitter->accelation_per_lifetime[0].z) + "~"
				+ to_string(emitter->accelation_per_lifetime[1].x) + " " + to_string(emitter->accelation_per_lifetime[1].y) + " " + to_string(emitter->accelation_per_lifetime[1].z);
			effect->SetValue("accelation_per_lifetime", fmt);
			// velocity_timeline_map
				// 10-x y z~100-x y z~
			fmt = "";
			for (auto& pair : emitter->velocity_timeline_map)
				fmt += to_string(pair.first) + "-" + to_string(pair.second.x) + " " + to_string(pair.second.y) + " " + to_string(pair.second.z) + "~";
			effect->SetValue("velocity_timeline_map", fmt);
		}

		// GRAVITY
		effect->SetValue("gravity_on_off", to_string(emitter->gravity_on_off));

		// vs_id
		effect->SetValue("vs_id", emitter->vs_id);
		// geo_id
		effect->SetValue("geo_id", emitter->geo_id);
		// ps_id
		effect->SetValue("mat_id", emitter->mat_id);


		// BS
		effect->SetValue("BS", to_string(emitter->bs_state));
		// DS
		effect->SetValue("DS", to_string(emitter->ds_state));

	}
	DATA->SaveSheetFile(sheetName);
}

// 타임라인 -> 배열 계산 메소드들
void WG_EffectWindow::ComputeColorTimeline(map<int, XMFLOAT4>& timeline, XMFLOAT4* arr)
{	
	// 1. 전체 배열을 map의 첫 값으로 초기화
	for (int i = 0; i < 101; i++)
	{
		arr[i] = timeline.begin()->second;
	}

	// map의 개수가 1개라면 메소드 종료
	if (timeline.size() == 1)
		return;

	// 2. 이후의 값들을 보정해서 계산
	int last_time = -1;
	for (const auto& pair : timeline)
	{
		// 첫 입력이면 last_time에만 저장하고 다음 배열로
		if (last_time == -1)
		{
			last_time = pair.first;
			continue;
		}

		// 아니라면 앞의 값과 보정
		int cur_time = pair.first;
		int time_dif = cur_time - last_time + 1;

		XMFLOAT4 value_dif_step = {
			(timeline[cur_time].x - timeline[last_time].x) / time_dif,
			(timeline[cur_time].y - timeline[last_time].y) / time_dif,
			(timeline[cur_time].z - timeline[last_time].z) / time_dif,
			(timeline[cur_time].w - timeline[last_time].w) / time_dif
		};

		for (int i = 0; i < time_dif; i++)
		{
			XMFLOAT4 add_value = { value_dif_step.x * i, value_dif_step.y * i, value_dif_step.z * i, value_dif_step.w * i };
			arr[last_time + i] = {
				timeline[last_time].x + add_value.x,
				timeline[last_time].y + add_value.y,
				timeline[last_time].z + add_value.z,
				timeline[last_time].w + add_value.w };
		}

		last_time = pair.first;
	}

	// 3. 마지막 값 이후에 값들은 마지막 값으로 초기화
	int timeline_last_time = last_time;
	for (int i = timeline_last_time; i < EFFECT_TIMELINE_SIZE; i++)
	{
		arr[i] = timeline[last_time];
	}
}

void WG_EffectWindow::ComputeSizeTimeline(map<int, XMFLOAT3>& timeline, XMFLOAT3* arr)
{
	// 1. 전체 배열을 map의 첫 값으로 초기화
	for (int i = 0; i < 101; i++)
	{
		arr[i] = timeline.begin()->second;
	}

	// map의 개수가 1개라면 메소드 종료
	if (timeline.size() == 1)
		return;

	// 2. 이후의 값들을 보정해서 계산
	int last_time = -1;
	for (const auto& pair : timeline)
	{
		// 첫 입력이면 last_time에만 저장하고 다음 배열로
		if (last_time == -1)
		{
			last_time = pair.first;
			continue;
		}

		// 아니라면 앞의 값과 보정
		int cur_time = pair.first;
		int time_dif = cur_time - last_time + 1;

		XMFLOAT3 value_dif_step = {
			(timeline[cur_time].x - timeline[last_time].x) / time_dif,
			(timeline[cur_time].y - timeline[last_time].y) / time_dif,
			(timeline[cur_time].z - timeline[last_time].z) / time_dif
		};

		for (int i = 0; i < time_dif; i++)
		{
			XMFLOAT3 add_value = { value_dif_step.x * i, value_dif_step.y * i, value_dif_step.z * i };
			arr[last_time + i] = {
				timeline[last_time].x + add_value.x,
				timeline[last_time].y + add_value.y,
				timeline[last_time].z + add_value.z
			};
		}

		last_time = pair.first;
	}

	// 3. 마지막 값 이후에 값들은 마지막 값으로 초기화
	int timeline_last_time = last_time;
	for (int i = timeline_last_time; i < EFFECT_TIMELINE_SIZE; i++)
	{
		arr[i] = timeline[last_time];
	}
}

void WG_EffectWindow::ComputeRotationTimeline(map<int, float>& timeline, float* arr)
{
	// 1. 전체 배열을 map의 첫 값으로 초기화
	for (int i = 0; i < 101; i++)
	{
		arr[i] = timeline.begin()->second;
	}

	// map의 개수가 1개라면 메소드 종료
	if (timeline.size() == 1)
		return;

	// 2. 이후의 값들을 보정해서 계산
	int last_time = -1;
	for (const auto& pair : timeline)
	{
		// 첫 입력이면 last_time에만 저장하고 다음 배열로
		if (last_time == -1)
		{
			last_time = pair.first;
			continue;
		}

		// 아니라면 앞의 값과 보정
		int cur_time = pair.first;
		int time_dif = cur_time - last_time + 1;

		float value_dif_step = (timeline[cur_time] - timeline[last_time]) / time_dif;

		for (int i = 0; i < time_dif; i++)
		{
			float add_value = value_dif_step * i;
			arr[last_time + i] = timeline[last_time] + add_value;
		}

		last_time = pair.first;
	}

	// 3. 마지막 값 이후에 값들은 마지막 값으로 초기화
	int timeline_last_time = last_time;
	for (int i = timeline_last_time; i < EFFECT_TIMELINE_SIZE; i++)
	{
		arr[i] = timeline[last_time];
	}
}

void WG_EffectWindow::ComputeVelocityTimeline(map<int, XMFLOAT3>& timeline, XMFLOAT3* arr)
{
	// 1. 전체 배열을 map의 첫 값으로 초기화
	for (int i = 0; i < 101; i++)
	{
		arr[i] = timeline.begin()->second;
	}

	// map의 개수가 1개라면 메소드 종료
	if (timeline.size() == 1)
		return;

	// 2. 이후의 값들을 보정해서 계산
	int last_time = -1;
	for (const auto& pair : timeline)
	{
		// 첫 입력이면 last_time에만 저장하고 다음 배열로
		if (last_time == -1)
		{
			last_time = pair.first;
			continue;
		}

		// 아니라면 앞의 값과 보정
		int cur_time = pair.first;
		int time_dif = cur_time - last_time + 1;

		XMFLOAT3 value_dif_step = {
			(timeline[cur_time].x - timeline[last_time].x) / time_dif,
			(timeline[cur_time].y - timeline[last_time].y) / time_dif,
			(timeline[cur_time].z - timeline[last_time].z) / time_dif
		};

		for (int i = 0; i < time_dif; i++)
		{
			XMFLOAT3 add_value = { value_dif_step.x * i, value_dif_step.y * i, value_dif_step.z * i };
			arr[last_time + i] = {
				timeline[last_time].x + add_value.x,
				timeline[last_time].y + add_value.y,
				timeline[last_time].z + add_value.z
			};
		}

		last_time = pair.first;
	}

	// 3. 마지막 값 이후에 값들은 마지막 값으로 초기화
	int timeline_last_time = last_time;
	for (int i = timeline_last_time; i < EFFECT_TIMELINE_SIZE; i++)
	{
		arr[i] = timeline[last_time];
	}
}