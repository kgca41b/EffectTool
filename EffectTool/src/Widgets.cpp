#include "Widgets.h"
#include "imfilebrowser.h"
#include "SceneMgr.h"
#include "EffectTool.h"
#include "DXStates.h"

using namespace KGCA41B;

#define LISTBOX_WIDTH 150.0f
#define TEXT_WIDTH 150.0f

#define GET_VARIABLE_NAME(n) #n

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
}

void WG_EffectWindow::Update()
{
	ImGui::SetCurrentContext(GUI->GetContext());
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void WG_EffectWindow::Render()
{
	ImGui::SetNextWindowSize(ImVec2(400, 600));
	ImGui::Begin("Effect Tool", &open_, ImGuiWindowFlags_MenuBar);
	{
		ImGui::BeginMenuBar();
		{
			if(loading_data_id_.size() != 0)
				LoadingSpriteData();

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
				if (ImGui::MenuItem("Particles"))
				{
					type_ = PARTICLES;
				}
				ImGui::EndMenu();
			}
			FileBrowser();
		}
		ImGui::EndMenuBar();

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
		case PARTICLES:
		{
			ParticlesBoard();
		}
		break;
		}
	}
	ImGui::End();
}

void WG_EffectWindow::FileBrowser()
{
	static ImGui::FileBrowser fileDialog;

	if (ImGui::BeginMenu("Load Effect"))
	{
		if (ImGui::MenuItem("Load Sprite"))
		{
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}
		if (ImGui::MenuItem("Load Particle"))
		{
			fileDialog.Open();
			fileDialog.SetTypeFilters({ ".csv" });
		}
		ImGui::EndMenu();
	}
		
	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		loading_data_id_ = fileDialog.GetSelected().string();
		fileDialog.ClearSelected();
		fileDialog.Close();
	}
}

void WG_EffectWindow::UVSpriteBoard()
{
	static string texture_id = "";
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static int max_frame = 5;
	static char sprite_name[255] = { 0, };

	ImVec2 img_size = { 200, 200 };
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 4);

	
	tex = RESOURCE->UseResource<Texture>(texture_id);

	if(tex)
		ImGui::Image((void*)tex->srv.Get(), img_size);

	if (uv_sprite_data.uv_list.size() > 0 && tex)
	{
		ImGui::SameLine();
		auto uv = uv_sprite_data.uv_list[min((int)cur_frame - 1, (int)uv_sprite_data.uv_list.size() - 1)];
		float tex_width = (float)tex->texture_desc.Width;
		float tex_height = (float)tex->texture_desc.Height;
		ImVec2 start(uv.first.x / tex_width, uv.first.y / tex_height);
		ImVec2 end(uv.second.x / tex_width, uv.second.y / tex_height);
		ImGui::Image((void*)tex->srv.Get(), img_size, start, end);
	}

	// ������ ����
	SelectFrame(max_frame, cur_frame);

	// �ؽ��� ����
	SelectTexture(texture_id);
	
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
		texture_id = "";
		tex = nullptr;
		cur_frame = 1;
		max_frame = 5;
		memset(sprite_name, 0, sizeof(char) * strlen(sprite_name));
	}
}

void WG_EffectWindow::TexSpriteBoard()
{
	static string texture_id = "";
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static int max_frame = 5;
	static char sprite_name[255] = { 0, };

	ImVec2 img_size = { 200, 200 };
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 4);
	

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
	SelectFrame(max_frame, cur_frame);

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
		max_frame = 5;
		memset(sprite_name, 0, sizeof(char) * strlen(sprite_name));
	}
	ImGui::SameLine();
	if (ImGui::Button("Render"))
	{
		
	}
}

void WG_EffectWindow::ParticlesBoard()
{
	static Texture* tex = nullptr;
	static int cur_frame = 1;
	static int max_frame = 5;
	static char particle_name[255] = { 0, };

	ImVec2 img_size = { 200, 200 };
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 2);
	ImGui::Image(nullptr, img_size);

	SelectVertexShader(emitter_data.vs_id);
	SelectGeometryShader(emitter_data.geo_id);
	SelectPixelShader(emitter_data.ps_id);

	ImGui::SetNextItemWidth(TEXT_WIDTH);
	ImGui::InputTextWithHint("sprite name", "Name", particle_name, IM_ARRAYSIZE(particle_name));
	if (ImGui::Button("Save"))
	{

	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{

	}
}

void WG_EffectWindow::SelectBlendOptions()
{
	static bool bZbufferComp = false;
	static bool bZbufferWrite = false;
	static bool bAlphaBlending = false;
	static bool bAlphaTesting = false;
	static bool bWireFrame = false;
	ImGui::Checkbox("Z buffer Comp", &bZbufferComp);
	ImGui::SameLine();
	ImGui::Checkbox("Z buffer Write", &bZbufferWrite);
	ImGui::SameLine();
	ImGui::Checkbox("AlphaBlending", &bAlphaBlending);
	ImGui::SameLine();
	ImGui::Checkbox("AlphaTesting", &bAlphaTesting);
	ImGui::SameLine();
	ImGui::Checkbox("WireFrame", &bWireFrame);

	// Z ���� �� & Z ���� ����
	if (bZbufferComp && bZbufferWrite)
		DX11APP->GetDeviceContext()->OMSetDepthStencilState(DXStates::ds_defalut(), 1);
	else if (bZbufferComp)
		DX11APP->GetDeviceContext()->OMSetDepthStencilState(DXStates::ds_depth_enable_no_write(), 1);
	else
		DX11APP->GetDeviceContext()->OMSetDepthStencilState(DXStates::ds_depth_disable(), 1);

	// ���� ����
	if (bAlphaBlending)
		DX11APP->GetDeviceContext()->OMSetBlendState(DXStates::bs_default(), 0, -1);
	else
		DX11APP->GetDeviceContext()->OMSetBlendState(0, 0, -1);

	// ���� �׽���?

	// ���̾� ������ üũ
	if (bWireFrame)
		DX11APP->GetDeviceContext()->RSSetState(DXStates::rs_wireframe_cull_none());
	else
		DX11APP->GetDeviceContext()->RSSetState(DXStates::rs_solid_cull_none());

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

void WG_EffectWindow::SelectPixelShader(string& id)
{
	static int item_current_idx = 0;

	auto ps_set = RESOURCE->GetTotalPSID();
	vector<string> ps_vec(ps_set.begin(), ps_set.end());

	// ������ ������ ps_id�� �ִٸ� �� ������ item_current_idx ����
	for (int i = 0; i < ps_vec.size(); i++)
	{
		if (ps_vec[i] == id)
			item_current_idx = i;
	}

	ImGui::SetNextItemWidth(LISTBOX_WIDTH);
	if (ImGui::BeginListBox("Pixel Shader"))
	{
		for (int n = 0; n < ps_vec.size(); n++)
		{
			const bool is_selected = (item_current_idx == n);
			if (ImGui::Selectable(ps_vec[n].c_str(), is_selected))
				item_current_idx = n;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}

	id = ps_vec[item_current_idx];
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

void WG_EffectWindow::SaveUVSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;
	sheetName += ".csv";

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
}

void WG_EffectWindow::SaveTexSprite(string name)
{
	string sheetName = name;
	if (sheetName.size() == 0)
		return;
	sheetName += ".csv";

	auto sheet = DATA->AddNewSheet(sheetName);

	auto effect = sheet->AddItem(name);
	auto list = sheet->AddItem("texList");

	sheet->AddCategory("MaxFrame");
	sheet->AddCategory("vs_id");
	sheet->AddCategory("ps_id");
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
}

void WG_EffectWindow::SaveParticles()
{

}

void WG_EffectWindow::LoadingSpriteData()
{
	DATA->LoadSheetFile(loading_data_id_);
	auto sheet = DATA->LoadSheet(loading_data_id_);

	auto strs1 = split(loading_data_id_, '\\');
	auto name = strs1[max((int)strs1.size() - 1, 0)];

	auto strs2 = split(name, '.');
	name = strs2[0];

	auto item = sheet->LoadItem(name);
	if (item == NULL)
		return;

	string str_type = item->GetValue("type");

	if (str_type == "")
		return;

	type_ = (E_Effect)stoi(str_type);

	switch (type_)
	{
	case UV_SPRITE:
	{
		type_ = UV_SPRITE;
		uv_sprite_data.max_frame = stoi(item->GetValue("MaxFrame"));
		uv_sprite_data.tex_id = item->GetValue("tex_id");

		// TODO : UVList �Ľ�... ������ ���� �����ؾ��ҵ�
		auto uvListItem = sheet->LoadItem("uvList");
		// ����Ʈ���� ���� ���� �������� ���� �����´�.
		int max = 0;
		for (int i = 1; i <= uv_sprite_data.max_frame + 1; i++)
		{
			if (uvListItem->values[to_string(i)] == "")
			{
				max = i-1;
				break;
			}
				
		}
		// ���� ���� �����Ӻ��� ���� ���� �����ӱ��� �Ľ��ؼ� uv���� �־��ش�.
		uv_sprite_data.uv_list.clear();
		for (int i = 0; i < max; i++)
		{
			// 0 0 25 25 ����
			auto splited_str = split(uvListItem->values[to_string(i + 1)], ' ');
			uv_sprite_data.uv_list.push_back({ { stol(splited_str[0]), stol(splited_str[1]) }, { stol(splited_str[2]), stol(splited_str[3]) } });
		}
	} break;
	case TEX_SPRITE:
	{
		type_ = TEX_SPRITE;
		tex_sprite_data.max_frame = stoi(item->GetValue("MaxFrame"));

		// TODO : ������ ���� �����ؾ��ҵ�
		auto texListItem = sheet->LoadItem("texList");
		// ����Ʈ���� ���� ���� �������� ���� �����´�.
		int max = 0;
		for (int i = 1; i <= uv_sprite_data.max_frame + 1; i++)
		{
			if (texListItem->values[to_string(i + 1)] == "")
			{
				max = i-1;
				break;
			}
		}
		// ���� ���� �����Ӻ��� ���� ���� �����ӱ��� �Ľ��ؼ� tex_id���� �־��ش�.
		tex_sprite_data.tex_id_list.clear();
		for (int i = 0; i < max; i++)
		{
			tex_sprite_data.tex_id_list.push_back(texListItem->values[to_string(i + 1)]);
		}
	} break;
	case PARTICLES:
	{
		type_ = PARTICLES;
	} break;
	default:
	{
		
	} break;
	}

	loading_data_id_ = "";
}

