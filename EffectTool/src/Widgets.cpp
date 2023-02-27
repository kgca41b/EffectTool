#include "Widgets.h"
#include "imfilebrowser.h"
#include "SceneMgr.h"
#include "EffectTool.h"

using namespace KGCA41B;

#define LISTBOX_WIDTH 150.0f
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
				if (ImGui::MenuItem("Sprite Emitter"))
				{
					type_ = SPRITE_EMITTER;
				}
				if (ImGui::MenuItem("Point Emitter"))
				{
					type_ = POINT_EMITTER;
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
		case SPRITE_EMITTER:
		{
			SpriteEmitterBoard(sprite_emitter_data);

			ImGui::SetNextItemWidth(TEXT_WIDTH);
			ImGui::InputTextWithHint("sprite name", "Name", particle_name, IM_ARRAYSIZE(particle_name));
			if (ImGui::Button("Save"))
			{
				SaveSpriteEmitter(particle_name);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
					scene->ResetEmitter();
			}
			ImGui::SameLine();
			if (ImGui::Button("Render"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
					scene->AddEmitter(make_shared<SpriteEmitter>(sprite_emitter_data));
			}
		}break;
		case POINT_EMITTER:
		{
			PointEmitterBoard(point_emitter_data);

			ImGui::SetNextItemWidth(TEXT_WIDTH);
			ImGui::InputTextWithHint("sprite name", "Name", particle_name, IM_ARRAYSIZE(particle_name));
			if (ImGui::Button("Save"))
			{
				SaveSpriteEmitter(particle_name);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
					scene->AddEmitter(make_shared<SpriteEmitter>(sprite_emitter_data));
			}
			ImGui::SameLine();
			if (ImGui::Button("Render"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
					scene->ResetEmitter();
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
			LoadingEmitterData(fileDialog.GetSelected().string(), sprite_emitter_data);
			type_ = SPRITE_EMITTER;
		}
		else if (type == 3)
		{
			emitter_map.clear();
			LoadingEffectData(fileDialog.GetSelected().string());
			type_ = EFFECT;
		}
		fileDialog.ClearSelected();
		fileDialog.Close();
	}
}

void WG_EffectWindow::UVSpriteBoard()
{
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static char sprite_name[255] = { 0, };

	ImVec2 img_size = { 200, 200 };
	//ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);

	
	tex = RESOURCE->UseResource<Texture>(uv_sprite_data.tex_id);

	if(tex)
		ImGui::Image((void*)tex->srv.Get(), img_size);

	if (uv_sprite_data.uv_list.size() > 0 && tex)
	{
		ImGui::SameLine();
		auto uv = uv_sprite_data.uv_list[min(cur_frame - 1, (int)uv_sprite_data.uv_list.size() - 1)];
		float tex_width = (float)tex->texture_desc.Width;
		float tex_height = (float)tex->texture_desc.Height;
		ImVec2 start(uv.first.x / tex_width, uv.first.y / tex_height);
		ImVec2 end(uv.second.x / tex_width, uv.second.y / tex_height);
		ImGui::Image((void*)tex->srv.Get(), img_size, start, end);
	}

	// ������ ����
	SelectFrame(uv_sprite_data.max_frame, cur_frame);

	// �ؽ��� ����
	SelectTexture(uv_sprite_data.tex_id);
	
	// UV �� ����
	if (uv_sprite_data.uv_list.size() > uv_sprite_data.max_frame)
		uv_sprite_data.uv_list.resize(uv_sprite_data.max_frame);
	SelectUV(uv_sprite_data.uv_list, uv_sprite_data.max_frame);

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

		uv_sprite_data.tex_id = "";
		uv_sprite_data.uv_list.clear();
		uv_sprite_data.max_frame = 5;
	}
}

void WG_EffectWindow::TexSpriteBoard()
{
	static string texture_id = "";
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static char sprite_name[255] = { 0, };

	ImVec2 img_size = { 200, 200 };
	//ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);
	

	tex = RESOURCE->UseResource<Texture>(texture_id);

	if (tex)
		ImGui::Image((void*)tex->srv.Get(), img_size);

	
	if (tex_sprite_data.tex_id_list.size() > 0)
	{
		ImGui::SameLine();
		tex = RESOURCE->UseResource<Texture>(tex_sprite_data.tex_id_list[min((int)cur_frame - 1, (int)tex_sprite_data.tex_id_list.size() - 1)]);
		ImGui::Image((void*)tex->srv.Get(), img_size);
	}
		

	// ������ ����
	SelectFrame(tex_sprite_data.max_frame, cur_frame);

	// �ؽ��� ����
	SelectTexture(texture_id);

	if (tex_sprite_data.tex_id_list.size() > tex_sprite_data.max_frame)
		tex_sprite_data.tex_id_list.resize(tex_sprite_data.max_frame);

	// �ؽ��� �迭 �߰�
	if (ImGui::Button("Add"))
	{
		if (tex_sprite_data.tex_id_list.size() == tex_sprite_data.max_frame)
			tex_sprite_data.max_frame++;
		tex_sprite_data.tex_id_list.push_back(texture_id);
	}

	//ImGui::SameLine();
	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	static int item_current_idx = 0;
	if (ImGui::BeginListBox("Tex ID per Frame"))
	{
		for (int n = 0; n < tex_sprite_data.tex_id_list.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			string str = "[" + to_string(n + 1) + "] " + tex_sprite_data.tex_id_list[n];
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
		if (tex_sprite_data.tex_id_list.size() > 0)
			tex_sprite_data.tex_id_list.pop_back();
	}

	ImGui::SetNextItemWidth(TEXT_WIDTH);
	ImGui::InputTextWithHint("sprite name", "Name", sprite_name, IM_ARRAYSIZE(sprite_name));

	if (ImGui::Button("Save"))
	{
		// TODO : ������ ���̺��� ���� ����
		SaveTexSprite(sprite_name);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		texture_id = "";
		tex = nullptr;
		cur_frame = 1;
		memset(sprite_name, 0, sizeof(char) * strlen(sprite_name));

		tex_sprite_data.tex_id_list.clear();
		tex_sprite_data.max_frame = 5;
	}
	ImGui::SameLine();
	if (ImGui::Button("Render"))
	{
		
	}
}

void WG_EffectWindow::SpriteEmitterBoard(SpriteEmitter& emitter)
{
	static int cur_frame	= 1;
	static float timer		= 1.0f;

	// Render Selected Sprite
	auto sprite = RESOURCE->UseResource<Sprite>(emitter.sprite_id);
	if (sprite)
	{
		Texture* tex = nullptr;
		ImVec2 img_size = { 200, 200 };
		// ��������Ʈ Ÿ�Կ� ���� �ٸ��� ������
		// UV Sprite
		if (sprite->type == UV_SPRITE)
		{
			UVSprite* uv_sprite = (UVSprite*)sprite;
			tex = RESOURCE->UseResource<Texture>(uv_sprite->tex_id);

			// ������ ó��
			timer += TIMER->GetDeltaTime();
			if (timer > uv_sprite->max_frame + 1)
				timer -= uv_sprite->max_frame;
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
			// ������ ó��
			timer += TIMER->GetDeltaTime();
			if (timer > tex_sprite->max_frame + 1)
				timer -= tex_sprite->max_frame;
			cur_frame = (int)timer;

			tex = RESOURCE->UseResource<Texture>(tex_sprite->tex_id_list[min((int)cur_frame - 1, (int)tex_sprite->tex_id_list.size() - 1)]);

			ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);
			ImGui::Image((void*)tex->srv.Get(), img_size);
		}
	}
	

	// Sprite_id or Something?
	ImGui::Text("Sprite");
	SelectSprite(emitter.sprite_id);

	ImGui::Text("Emit Mode");
	static int mode = 0;
	mode = emitter.b_emit_once_or_per_second;
	if (ImGui::RadioButton("Once", mode == false))
	{ 
		mode = emitter.b_emit_once_or_per_second;
		emitter.b_emit_once_or_per_second = false;
	} 
	ImGui::SameLine();
	if (ImGui::RadioButton("Per Second", mode == true))
	{ 
		mode = emitter.b_emit_once_or_per_second;
		emitter.b_emit_once_or_per_second = true;
	}
	
	switch (mode)
	{
		case false:
			ImGui::SetNextItemWidth(150.0f);
			ImGui::InputInt("Emit Once", &emitter.emit_partice_once);
			break;
		case true:
			ImGui::SetNextItemWidth(150.0f);
			ImGui::InputInt("Emit per Second", &emitter.emit_per_second);
	}

	// Color
	ImGui::Text("Particle Color");
	static ImVec4 color;
	ImGui::ColorEdit3("Color", (float*)&emitter.color);
	emitter.color.w = 1.0f;

	// Life Time
	ImGui::Text("Life Time (seconds)");
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Life Time Min", &emitter.life_time[MIN]);
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Life Time Max", &emitter.life_time[MAX]);

	// Initial Size
	ImGui::Text("Initial Size (x,y,z)");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Size Min", (float*)&emitter.initial_size[MIN], "%.2f");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Size Max", (float*)&emitter.initial_size[MAX], "%.2f");

	// Initial Rotation
	ImGui::Text("Initial Rotation (angle)");
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Rot Min", &emitter.initial_rotation[MIN]);
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Rot Max", &emitter.initial_rotation[MAX]);

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


	// Size Per lifetime
	ImGui::Text("Size per lifetime (x,y,z)");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Size per Life Min", (float*)&emitter.size_per_lifetime[MIN], "%.2f");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Size per Life Max", (float*)&emitter.size_per_lifetime[MAX], "%.2f");


	// Rotation Per lifetime
	ImGui::Text("Rotation Per lifetime (Angle)");
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Rot per life Min", &emitter.rotation_per_lifetime[MIN]);
	ImGui::SetNextItemWidth(50.0f);
	ImGui::InputFloat("Rot per life Max", &emitter.rotation_per_lifetime[MAX]);

	// Velocity Per lifetime
	ImGui::Text("Accelation per lifetime (x,y,z)");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Acc per Life Min", (float*)&emitter.accelation_per_lifetime[MIN], "%.2f");
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputFloat3("Acc per Life Max", (float*)&emitter.accelation_per_lifetime[MAX], "%.2f");

	// Shader Selection
	SelectVertexShader(emitter.vs_id);
	SelectGeometryShader(emitter.geo_id);
	SelectMaterial(emitter.mat_id);

	// BS, DS Selection
	SelectBSOptions(emitter.bs_state);
	SelectDSOptions(emitter.ds_state);
}

void WG_EffectWindow::PointEmitterBoard(PointEmitter& emitter)
{
	
}

void WG_EffectWindow::EffectBoard()
{

	// Emitter ����Ʈ
	static int item_current_idx = 0;
	if (emitter_map.size() > 0)
		item_current_idx = min(item_current_idx, (int)emitter_map.size() - 1);
	else
		item_current_idx = 0;

	vector<string> emitter_vec;
	shared_ptr<Emitter> cur_emitter;
	{
		
		for (auto iter = emitter_map.begin(); iter != emitter_map.end(); ++iter)
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

				cur_emitter = emitter_map[select];
			}
			
		}
	}
	
	// Emitter �߰� / ����
	{
		static ImGui::FileBrowser fileDialog;
		if (ImGui::Button("Add"))
		{
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}

		fileDialog.Display();

		if (fileDialog.HasSelected())
		{
			auto emitter = make_shared<SpriteEmitter>();
			string path = fileDialog.GetSelected().string();
			LoadingEmitterData(path, *emitter.get());

			// �̸� �Ľ�
			auto splited_str = split(path, '\\');
			auto strs = split(splited_str[max((int)splited_str.size() - 1, 0)], '.');
			auto name = strs[0];

			emitter_map.insert({ name, emitter });

			fileDialog.ClearSelected();
			fileDialog.Close();
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			if (emitter_vec.size() > 0)
			{
				emitter_map.erase(emitter_vec[item_current_idx]);
			}
			
		}
	}
	
	// ���̺� / ������
	static char effect_name[255] = { 0, };
	ImGui::SetNextItemWidth(TEXT_WIDTH);
	ImGui::InputTextWithHint("Effect Name", "Name", effect_name, IM_ARRAYSIZE(effect_name));
	if (ImGui::Button("Effect Save"))
	{
		string sheetName = effect_name;
		if (sheetName.size() == 0)
			return;

		auto sheet = DATA->AddNewSheet(sheetName);

		for (auto pair : emitter_map)
		{
			auto effect = sheet->AddItem(pair.first);
			auto emitter = (SpriteEmitter*)pair.second.get();

			// ī�װ� �߰�
			sheet->AddCategory("type");

			sheet->AddCategory("sprite_id");

			sheet->AddCategory("b_emit_once_or_per_second");
			sheet->AddCategory("emit_partice_once");
			sheet->AddCategory("emit_per_second");

			sheet->AddCategory("color");

			sheet->AddCategory("life_time");

			sheet->AddCategory("initial_size");
			sheet->AddCategory("initial_rotation");
			sheet->AddCategory("initial_position");

			sheet->AddCategory("initial_velocity");

			sheet->AddCategory("size_per_lifetime");
			sheet->AddCategory("rotation_per_lifetime");
			sheet->AddCategory("accelation_per_lifetime");

			sheet->AddCategory("vs_id");
			sheet->AddCategory("geo_id");
			sheet->AddCategory("mat_id");

			sheet->AddCategory("BS");
			sheet->AddCategory("DS");

			// �� �߰�

			// type
			effect->SetValue("type", to_string(SPRITE_EMITTER));

			// sprite_id
			effect->SetValue("sprite_id", emitter->sprite_id);

			// b_emit_once_or_per_second
			effect->SetValue("b_emit_once_or_per_second", to_string(emitter->b_emit_once_or_per_second));
			// emit_per_second
			effect->SetValue("emit_partice_once", to_string(emitter->emit_partice_once));
			// emit_per_second
			effect->SetValue("emit_per_second", to_string(emitter->emit_per_second));

			string fmt = "";

			// color
			fmt = to_string(emitter->color.x) + " " + to_string(emitter->color.y) + " " + to_string(emitter->color.z) + " " + to_string(emitter->color.w);
			effect->SetValue("color", fmt);

			// life_time
			fmt = to_string(emitter->life_time[0]) + " " + to_string(emitter->life_time[1]);
			effect->SetValue("life_time", fmt);

			// initial_size
			fmt = to_string(emitter->initial_size[0].x) + " " + to_string(emitter->initial_size[0].y) + " " + to_string(emitter->initial_size[0].z) + "~"
				+ to_string(emitter->initial_size[1].x) + " " + to_string(emitter->initial_size[1].y) + " " + to_string(emitter->initial_size[1].z);
			effect->SetValue("initial_size", fmt);

			// initial_rotation
			fmt = to_string(emitter->initial_rotation[0]) + " " + to_string(emitter->initial_rotation[1]);
			effect->SetValue("initial_rotation", fmt);

			// initial_position
			fmt = to_string(emitter->initial_position[0].x) + " " + to_string(emitter->initial_position[0].y) + " " + to_string(emitter->initial_position[0].z) + "~"
				+ to_string(emitter->initial_position[1].x) + " " + to_string(emitter->initial_position[1].y) + " " + to_string(emitter->initial_position[1].z);
			effect->SetValue("initial_position", fmt);

			// initial_velocity
			fmt = to_string(emitter->initial_velocity[0].x) + " " + to_string(emitter->initial_velocity[0].y) + " " + to_string(emitter->initial_velocity[0].z) + "~"
				+ to_string(emitter->initial_velocity[1].x) + " " + to_string(emitter->initial_velocity[1].y) + " " + to_string(emitter->initial_velocity[1].z);
			effect->SetValue("initial_velocity", fmt);

			// size_per_lifetime
			fmt = to_string(emitter->size_per_lifetime[0].x) + " " + to_string(emitter->size_per_lifetime[0].y) + " " + to_string(emitter->size_per_lifetime[0].z) + "~"
				+ to_string(emitter->size_per_lifetime[1].x) + " " + to_string(emitter->size_per_lifetime[1].y) + " " + to_string(emitter->size_per_lifetime[1].z);
			effect->SetValue("size_per_lifetime", fmt);
			// rotation_per_lifetime
			fmt = to_string(emitter->rotation_per_lifetime[0]) + " " + to_string(emitter->rotation_per_lifetime[1]);
			effect->SetValue("rotation_per_lifetime", fmt);
			// accelation_per_lifetime
			fmt = to_string(emitter->accelation_per_lifetime[0].x) + " " + to_string(emitter->accelation_per_lifetime[0].y) + " " + to_string(emitter->accelation_per_lifetime[0].z) + "~"
				+ to_string(emitter->accelation_per_lifetime[1].x) + " " + to_string(emitter->accelation_per_lifetime[1].y) + " " + to_string(emitter->accelation_per_lifetime[1].z);
			effect->SetValue("accelation_per_lifetime", fmt);

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
	if (ImGui::Button("Effect Render"))
	{
		auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
		if (scene)
		{
			scene->ResetEmitter();
			for (auto pair : emitter_map)
			{
				pair.second->particles.clear();
				scene->AddEmitter(pair.second);
			}
		}

	}

	// Emitter ����
	if (cur_emitter != nullptr)
	{
		SpriteEmitter* sprite_emitter = nullptr;
		switch (cur_emitter->type)
		{
		case SPRITE_EMITTER:
		{
			sprite_emitter = (SpriteEmitter*)cur_emitter.get();
			SpriteEmitterBoard(*sprite_emitter);
			if (ImGui::Button("Emitter Render"))
			{
				auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
				if (scene)
				{
					scene->ResetEmitter();
					sprite_emitter->particles.clear();
					scene->AddEmitter(make_shared<SpriteEmitter>(*sprite_emitter));
				}

			}
		}
		break;
		case POINT_EMITTER:
		{

		}
		break;
		}
	}
}

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

void WG_EffectWindow::SelectFrame(int& max_frame, int& cur_frame)
{
	// currentFrame ����
	ImGui::SetNextItemWidth(200.0f);
	ImGui::SliderInt("Frame", &cur_frame, 1, max_frame);
	// MaxFrame ����
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt("Max Frame", &max_frame);
	

	static bool bPlay = false;
	static float timer = cur_frame;
	// Play ��ư
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

void WG_EffectWindow::SelectUV(vector<pair<POINT, POINT>>& list, int& max_frame)
{
	static int start[2] = { 0, 0 };
	static int end[2] = { 0, 0 };
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("Start UV", start);
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("End UV", end);

	if (ImGui::Button("Add"))
	{
		if (list.size() == max_frame)
			max_frame++;
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

	// ������ ������ vs_id�� �ִٸ� �� ������ item_current_idx ����
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

	// ������ ������ gs_id�� �ִٸ� �� ������ item_current_idx ����
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

	// ������ ������ ps_id�� �ִٸ� �� ������ item_current_idx ����
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

	// ������ ������ tex_id�� �ִٸ� �� ������ item_current_idx ����
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

	// ������ �ִ� ��������Ʈ ���̵� ����Ʈȭ �ϱ� ���� vector�� id �޾ƿ���
	auto sprite_id_set = RESOURCE->GetTotalSpriteID();
	vector<string> sprite_id(sprite_id_set.begin(), sprite_id_set.end());

	// ������ ������ tex_id�� �ִٸ� �� ������ item_current_idx ����
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

void WG_EffectWindow::RenderWireFrame()
{
	static bool bWireFrame = false;

	ImGui::Checkbox("WireFrame", &bWireFrame);

	auto scene = (EffectTool*)SCENE->LoadScene("EffectTool");
	scene->bWireFrame = bWireFrame;
}

void WG_EffectWindow::SaveUVSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);
	auto list = sheet->AddItem("uvList");
	
	sheet->AddCategory("MaxFrame");
	sheet->AddCategory("tex_id");
	sheet->AddCategory("type");

	for (int i = 0; i < uv_sprite_data.uv_list.size(); i++)
		sheet->AddCategory(to_string(i+1));

	effect->SetValue("MaxFrame", to_string(uv_sprite_data.max_frame));
	effect->SetValue("tex_id", uv_sprite_data.tex_id);
	effect->SetValue("type", to_string(UV_SPRITE));

	for (int i = 0; i < uv_sprite_data.uv_list.size(); i++)
	{
		string uvStr = to_string(uv_sprite_data.uv_list[i].first.x) + " " + to_string(uv_sprite_data.uv_list[i].first.y) + " " 
			+ to_string(uv_sprite_data.uv_list[i].second.x) + " " + to_string(uv_sprite_data.uv_list[i].second.y);
		list->SetValue(to_string(i + 1), uvStr);
	}
		
	DATA->SaveSheetFile(sheetName);

	RESOURCE->SaveSprite(name, make_shared<UVSprite>(uv_sprite_data));
}

void WG_EffectWindow::SaveTexSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);
	auto list = sheet->AddItem("texList");

	sheet->AddCategory("MaxFrame");
	sheet->AddCategory("type");

	for (int i = 0; i < tex_sprite_data.tex_id_list.size(); i++)
		sheet->AddCategory(to_string(i + 1));

	effect->SetValue("MaxFrame", to_string(tex_sprite_data.max_frame));
	effect->SetValue("type", to_string(TEX_SPRITE));

	for (int i = 0; i < tex_sprite_data.tex_id_list.size(); i++)
	{
		list->SetValue(to_string(i + 1), tex_sprite_data.tex_id_list[i]);
	}

	DATA->SaveSheetFile(sheetName);

	// ���̺� �� ��������Ʈ�� ��������Ʈ ����Ʈ�� �߰��Ѵ�.
	RESOURCE->SaveSprite(name, make_shared<TextureSprite>(tex_sprite_data));
}

void WG_EffectWindow::SaveSpriteEmitter(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);

	// ī�װ� �߰�
	sheet->AddCategory("type");

	sheet->AddCategory("sprite_id");

	sheet->AddCategory("b_emit_once_or_per_second");
	sheet->AddCategory("emit_partice_once");
	sheet->AddCategory("emit_per_second");

	sheet->AddCategory("color");

	sheet->AddCategory("life_time");

	sheet->AddCategory("initial_size");
	sheet->AddCategory("initial_rotation");
	sheet->AddCategory("initial_position");

	sheet->AddCategory("initial_velocity");

	sheet->AddCategory("size_per_lifetime");
	sheet->AddCategory("rotation_per_lifetime");
	sheet->AddCategory("accelation_per_lifetime");

	sheet->AddCategory("vs_id");
	sheet->AddCategory("geo_id");
	sheet->AddCategory("mat_id");

	sheet->AddCategory("BS");
	sheet->AddCategory("DS");

	// �� �߰�

	// type
	effect->SetValue("type", to_string(SPRITE_EMITTER));

	// sprite_id
	effect->SetValue("sprite_id", sprite_emitter_data.sprite_id);

	// b_emit_once_or_per_second
	effect->SetValue("b_emit_once_or_per_second", to_string(sprite_emitter_data.b_emit_once_or_per_second));
	// emit_per_second
	effect->SetValue("emit_partice_once", to_string(sprite_emitter_data.emit_partice_once));
	// emit_per_second
	effect->SetValue("emit_per_second", to_string(sprite_emitter_data.emit_per_second));

	string fmt = "";

	// color
	fmt = to_string(sprite_emitter_data.color.x) + " " + to_string(sprite_emitter_data.color.y) + " " + to_string(sprite_emitter_data.color.z) + " " + to_string(sprite_emitter_data.color.w);
	effect->SetValue("color", fmt);

	// life_time
	fmt = to_string(sprite_emitter_data.life_time[0]) + " " + to_string(sprite_emitter_data.life_time[1]);
	effect->SetValue("life_time", fmt);

	// initial_size
	fmt = to_string(sprite_emitter_data.initial_size[0].x) + " " + to_string(sprite_emitter_data.initial_size[0].y) + " " + to_string(sprite_emitter_data.initial_size[0].z) + "~"
		+ to_string(sprite_emitter_data.initial_size[1].x) + " " + to_string(sprite_emitter_data.initial_size[1].y) + " " + to_string(sprite_emitter_data.initial_size[1].z);
	effect->SetValue("initial_size", fmt);

	// initial_rotation
	fmt = to_string(sprite_emitter_data.initial_rotation[0]) + " " + to_string(sprite_emitter_data.initial_rotation[1]);
	effect->SetValue("initial_rotation", fmt);

	// initial_position
	fmt = to_string(sprite_emitter_data.initial_position[0].x) + " " + to_string(sprite_emitter_data.initial_position[0].y) + " " + to_string(sprite_emitter_data.initial_position[0].z) + "~"
		+ to_string(sprite_emitter_data.initial_position[1].x) + " " + to_string(sprite_emitter_data.initial_position[1].y) + " " + to_string(sprite_emitter_data.initial_position[1].z);
	effect->SetValue("initial_position", fmt);

	// initial_velocity
	fmt = to_string(sprite_emitter_data.initial_velocity[0].x) + " " + to_string(sprite_emitter_data.initial_velocity[0].y) + " " + to_string(sprite_emitter_data.initial_velocity[0].z) + "~"
		+ to_string(sprite_emitter_data.initial_velocity[1].x) + " " + to_string(sprite_emitter_data.initial_velocity[1].y) + " " + to_string(sprite_emitter_data.initial_velocity[1].z);
	effect->SetValue("initial_velocity", fmt);

	// size_per_lifetime
	fmt = to_string(sprite_emitter_data.size_per_lifetime[0].x) + " " + to_string(sprite_emitter_data.size_per_lifetime[0].y) + " " + to_string(sprite_emitter_data.size_per_lifetime[0].z) + "~"
		+ to_string(sprite_emitter_data.size_per_lifetime[1].x) + " " + to_string(sprite_emitter_data.size_per_lifetime[1].y) + " " + to_string(sprite_emitter_data.size_per_lifetime[1].z);
	effect->SetValue("size_per_lifetime", fmt);
	// rotation_per_lifetime
	fmt = to_string(sprite_emitter_data.rotation_per_lifetime[0]) + " " + to_string(sprite_emitter_data.rotation_per_lifetime[1]);
	effect->SetValue("rotation_per_lifetime", fmt);
	// accelation_per_lifetime
	fmt = to_string(sprite_emitter_data.accelation_per_lifetime[0].x) + " " + to_string(sprite_emitter_data.accelation_per_lifetime[0].y) + " " + to_string(sprite_emitter_data.accelation_per_lifetime[0].z) + "~"
		+ to_string(sprite_emitter_data.accelation_per_lifetime[1].x) + " " + to_string(sprite_emitter_data.accelation_per_lifetime[1].y) + " " + to_string(sprite_emitter_data.accelation_per_lifetime[1].z);
	effect->SetValue("accelation_per_lifetime", fmt);

	// vs_id
	effect->SetValue("vs_id", sprite_emitter_data.vs_id);
	// geo_id
	effect->SetValue("geo_id", sprite_emitter_data.geo_id);
	// ps_id
	effect->SetValue("mat_id", sprite_emitter_data.mat_id);


	// BS
	effect->SetValue("BS", to_string(sprite_emitter_data.bs_state));
	// DS
	effect->SetValue("DS", to_string(sprite_emitter_data.ds_state));


	DATA->SaveSheetFile(sheetName);
}

void WG_EffectWindow::SavePointEmitter(string name)
{
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
		if(uv_sprite)
			uv_sprite_data = *uv_sprite;
		break;
	case TEX_SPRITE:
		type_ = TEX_SPRITE;
		tex_sprite = (TextureSprite*)sprite;
		if (tex_sprite)
			tex_sprite_data = *tex_sprite;
		break;
	}
	//DATA->LoadSheetFile(loading_data_id_);
	//auto sheet = DATA->LoadSheet(loading_data_id_);

	//auto strs1 = split(loading_data_id_, '\\');
	//auto name = strs1[max((int)strs1.size() - 1, 0)];

	//auto strs2 = split(name, '.');
	//name = strs2[0];

	//auto item = sheet->LoadItem(name);
	//if (item == NULL)
	//	return;

	//string str_type = item->GetValue("type");

	//if (str_type == "")
	//	return;

	//E_Effect type = (E_Effect)stoi(str_type);

	//switch (type)
	//{
	//case UV_SPRITE:
	//{
	//	type_ = UV_TAB;
	//	uv_sprite_data.max_frame = stoi(item->GetValue("MaxFrame"));
	//	uv_sprite_data.tex_id = item->GetValue("tex_id");

	//	// TODO : UVList �Ľ�... ������ ���� �����ؾ��ҵ�
	//	auto uvListItem = sheet->LoadItem("uvList");
	//	// ����Ʈ���� ���� ���� �������� ���� �����´�.
	//	int max = 0;
	//	for (int i = 1; i <= uv_sprite_data.max_frame + 1; i++)
	//	{
	//		if (uvListItem->values[to_string(i)] == "")
	//		{
	//			max = i-1;
	//			break;
	//		}
	//			
	//	}
	//	// ���� ���� �����Ӻ��� ���� ���� �����ӱ��� �Ľ��ؼ� uv���� �־��ش�.
	//	uv_sprite_data.uv_list.clear();
	//	for (int i = 0; i < max; i++)
	//	{
	//		// 0 0 25 25 ����
	//		auto splited_str = split(uvListItem->values[to_string(i + 1)], ' ');
	//		uv_sprite_data.uv_list.push_back({ { stol(splited_str[0]), stol(splited_str[1]) }, { stol(splited_str[2]), stol(splited_str[3]) } });
	//	}
	//	
	//	// �ε��� ��������Ʈ�� ����Ʈ�� �ִ´�.
	//	sprite_list.insert({ name, make_shared<UVSprite>(uv_sprite_data) });
	//} break;
	//case TEX_SPRITE:
	//{
	//	type_ = TEX_TAB;
	//	tex_sprite_data.max_frame = stoi(item->GetValue("MaxFrame"));

	//	// TODO : ������ ���� �����ؾ��ҵ�
	//	auto texListItem = sheet->LoadItem("texList");
	//	// ����Ʈ���� ���� ���� �������� ���� �����´�.
	//	int max = 0;
	//	for (int i = 1; i <= tex_sprite_data.max_frame + 1; i++)
	//	{
	//		if (texListItem->values[to_string(i)] == "")
	//		{
	//			max = i-1;
	//			break;
	//		}
	//	}
	//	// ���� ���� �����Ӻ��� ���� ���� �����ӱ��� �Ľ��ؼ� tex_id���� �־��ش�.
	//	tex_sprite_data.tex_id_list.clear();
	//	for (int i = 0; i < max; i++)
	//	{
	//		tex_sprite_data.tex_id_list.push_back(texListItem->values[to_string(i + 1)]);
	//	}
	//	// �ε��� ��������Ʈ�� ����Ʈ�� �ִ´�.
	//	sprite_list.insert({ name, make_shared<TextureSprite>(tex_sprite_data) });
	//} break;
	//case PARTICLES:
	//{
	//	type_ = PARTICLE_TAB;
	//} break;
	//default:
	//{
	//	
	//} break;
	//}

	//loading_data_id_ = "";
}

void WG_EffectWindow::LoadingEmitterData(string path, SpriteEmitter& emitter)
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

	// sprite_id
	emitter.sprite_id			= effect->GetValue("sprite_id");

	// b_emit_once_or_per_second
	emitter.b_emit_once_or_per_second = stoi(effect->GetValue("b_emit_once_or_per_second"));
	// emit_partice_once
	emitter.emit_partice_once = stoi(effect->GetValue("emit_partice_once"));
	// emit_per_second
	emitter.emit_per_second	= stoi(effect->GetValue("emit_per_second"));

	vector<string> splited_str;
	vector<string> splited_str2;

	// color
	{
		splited_str = split(effect->GetValue("color"), ' ');
		if (splited_str.size() < 4)
			return;
		emitter.color.x = stof(splited_str[0]);
		emitter.color.y = stof(splited_str[1]);
		emitter.color.z = stof(splited_str[2]);
		emitter.color.w = stof(splited_str[3]);
	}

	// life_time
	{
		splited_str = split(effect->GetValue("life_time"), ' ');
		if (splited_str.size() < 2)
			return;
		emitter.life_time[0] = stof(splited_str[0]);
		emitter.life_time[1] = stof(splited_str[1]);
	}

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

	// initial_rotation
	{
		splited_str = split(effect->GetValue("initial_rotation"), ' ');
		if (splited_str.size() < 2)
			return;
		emitter.initial_rotation[0] = stof(splited_str[0]);
		emitter.initial_rotation[1] = stof(splited_str[1]);
	}

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

	// size_per_lifetime
	{
		splited_str = split(effect->GetValue("size_per_lifetime"), '~');
		if (splited_str.size() < 2)
			return;
		// min
		splited_str2 = split(splited_str[0], ' ');
		if (splited_str2.size() < 3)
			return;
		emitter.size_per_lifetime[0].x = stof(splited_str2[0]);
		emitter.size_per_lifetime[0].y = stof(splited_str2[1]);
		emitter.size_per_lifetime[0].z = stof(splited_str2[2]);
		// max
		splited_str2 = split(splited_str[1], ' ');
		if (splited_str2.size() < 3)
			return;
		emitter.size_per_lifetime[1].x = stof(splited_str2[0]);
		emitter.size_per_lifetime[1].y = stof(splited_str2[1]);
		emitter.size_per_lifetime[1].z = stof(splited_str2[2]);
	}

	// rotation_per_lifetime
	{
		splited_str = split(effect->GetValue("rotation_per_lifetime"), ' ');
		if (splited_str.size() < 2)
			return;
		emitter.rotation_per_lifetime[0] = stof(splited_str[0]);
		emitter.rotation_per_lifetime[1] = stof(splited_str[1]);
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

	// BS
	emitter.bs_state = (E_EffectBS)stoi(effect->GetValue("BS"));

	// DS
	emitter.ds_state = (E_EffectDS)stoi(effect->GetValue("DS"));

	emitter.vs_id		= effect->GetValue("vs_id");
	emitter.geo_id		= effect->GetValue("geo_id");
	emitter.mat_id		= effect->GetValue("mat_id");
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

		auto emitter = make_shared<SpriteEmitter>();

		// sprite_id
		emitter->sprite_id = effect->GetValue("sprite_id");

		// b_emit_once_or_per_second
		emitter->b_emit_once_or_per_second = stoi(effect->GetValue("b_emit_once_or_per_second"));
		// emit_partice_once
		emitter->emit_partice_once = stoi(effect->GetValue("emit_partice_once"));
		// emit_per_second
		emitter->emit_per_second = stoi(effect->GetValue("emit_per_second"));

		vector<string> splited_str;
		vector<string> splited_str2;

		// color
		{
			splited_str = split(effect->GetValue("color"), ' ');
			if (splited_str.size() < 4)
				return;
			emitter->color.x = stof(splited_str[0]);
			emitter->color.y = stof(splited_str[1]);
			emitter->color.z = stof(splited_str[2]);
			emitter->color.w = stof(splited_str[3]);
		}

		// life_time
		{
			splited_str = split(effect->GetValue("life_time"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter->life_time[0] = stof(splited_str[0]);
			emitter->life_time[1] = stof(splited_str[1]);
		}

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

		// initial_rotation
		{
			splited_str = split(effect->GetValue("initial_rotation"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter->initial_rotation[0] = stof(splited_str[0]);
			emitter->initial_rotation[1] = stof(splited_str[1]);
		}

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

		// size_per_lifetime
		{
			splited_str = split(effect->GetValue("size_per_lifetime"), '~');
			if (splited_str.size() < 2)
				return;
			// min
			splited_str2 = split(splited_str[0], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter->size_per_lifetime[0].x = stof(splited_str2[0]);
			emitter->size_per_lifetime[0].y = stof(splited_str2[1]);
			emitter->size_per_lifetime[0].z = stof(splited_str2[2]);
			// max
			splited_str2 = split(splited_str[1], ' ');
			if (splited_str2.size() < 3)
				return;
			emitter->size_per_lifetime[1].x = stof(splited_str2[0]);
			emitter->size_per_lifetime[1].y = stof(splited_str2[1]);
			emitter->size_per_lifetime[1].z = stof(splited_str2[2]);
		}

		// rotation_per_lifetime
		{
			splited_str = split(effect->GetValue("rotation_per_lifetime"), ' ');
			if (splited_str.size() < 2)
				return;
			emitter->rotation_per_lifetime[0] = stof(splited_str[0]);
			emitter->rotation_per_lifetime[1] = stof(splited_str[1]);
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

		// BS
		emitter->bs_state = (E_EffectBS)stoi(effect->GetValue("BS"));

		// DS
		emitter->ds_state = (E_EffectDS)stoi(effect->GetValue("DS"));


		emitter->vs_id = effect->GetValue("vs_id");
		emitter->geo_id = effect->GetValue("geo_id");
		emitter->mat_id = effect->GetValue("mat_id");

		emitter_map.insert({ pair.first, emitter });
	}
	

}
