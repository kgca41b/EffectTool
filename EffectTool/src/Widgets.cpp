#include "Widgets.h"
#include "SceneMgr.h"
#include "CharacterTool.h"

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
		if (ImGui::MenuItem("Character"))
		{
			GUI->AddWidget("CharacterTool", new WG_CharacterWindow);
		}
	}
	ImGui::EndMainMenuBar();
}

void WG_CharacterWindow::Update()
{
	ImGui::SetCurrentContext(GUI->GetContext());
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void WG_CharacterWindow::Render()
{
	ImGui::SetNextWindowSize(ImVec2(600, 600));
	ImGui::Begin("Character Tool", &open_, ImGuiWindowFlags_MenuBar);
	{
		string clicked = "";
		ImGui::BeginMenuBar();
		{
			if(loading_data_id_.size() != 0)
				LoadCharacterData();

			if (ImGui::BeginMenu("New Character"))
			{
				CharacterBoard();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Load Effect"))
			{
				if (ImGui::MenuItem("Loading From DataManager"))
				{
					auto widget = new WG_DataViewer;
					GUI->AddWidget("FileViewer", widget);
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenuBar();

		//switch (type_)
		//{
		//case NONE:
		//{

		//}
		//break;
		//case UV_SPRITE:
		//{
		//	UVSpriteBoard();
		//}
		//break;
		//case TEX_SPRITE:
		//{
		//	TexSpriteBoard();
		//}
		//break;
		//case PARTICLES:
		//{
		//	ParticlesBoard();
		//}
		//break;
		//}
	}
	ImGui::End();
}

void WG_CharacterWindow::CharacterBoard()
{
	ImVec2 img_size = { 200, 200 };
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcItemSize(img_size, img_size.x, img_size.y).x) / 4);

	// 프레임 선택
	SelectFrame(character_data.max_frame, character_data.cur_frame);

	// 텍스쳐 선택
	SelectTexture(character_data.texture_id);
	
	// VS 선택
	SelectVertexShader(character_data.vs_id);
	
	// PS 선택
	ImGui::SameLine();
	SelectPixelShader(character_data.ps_id);

	// 블랜딩 옵션들
	SelectBlendOptions();


	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		// TODO : 데이터 테이블을 통한 저장
		SaveCharacterData(character_data);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		character_data.max_frame = 10;
		character_data.cur_frame = 1;
		character_data.anim_id = "";
		character_data.texture_id = "";
		character_data.skeleton_id = "";
		character_data.vs_id = "";
	}
	ImGui::SameLine();
	if (ImGui::Button("Render"))
	{
		auto scene = SCENE->LoadScene("CharacterTool");
		CharacterTool* character_scene = dynamic_cast<CharacterTool*>(scene);
		if (character_scene)
			character_scene->character_actor.SetCharacterData(character_scene->reg_scene, character_data);
	}
}

void WG_CharacterWindow::SelectBlendOptions()
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

	//if(bZbufferComp && bZbufferWrite)
	//	DX11APP->GetDeviceContext()->OMSetDepthStencilState(DX11APP->GetCommonStates()->DepthDefault(), 0xff);
	//else if(bZbufferComp)
	//	DX11APP->GetDeviceContext()->OMSetDepthStencilState(DX11APP->GetCommonStates()->DepthRead(), 0xff);
	//else if (bZbufferWrite) // 아직 모름
	//	DX11APP->GetDeviceContext()->OMSetDepthStencilState(DX11APP->GetCommonStates()->DepthRead(), 0xff);
	//else
	//	DX11APP->GetDeviceContext()->OMSetDepthStencilState(DX11APP->GetCommonStates()->DepthNone(), 0xff);

	//if(bAlphaBlending && bAlphaTesting)
	//	DX11APP->GetDeviceContext()->OMSetBlendState(DX11APP->GetCommonStates()->Additive(), 0, -1);
	//else if(bAlphaBlending)
	//	DX11APP->GetDeviceContext()->OMSetBlendState(DX11APP->GetCommonStates()->AlphaBlend(), 0, -1);
	//else
	//	DX11APP->GetDeviceContext()->OMSetBlendState(DX11APP->GetCommonStates()->Opaque(), 0, -1);

}

void WG_CharacterWindow::SelectFrame(int& max_frame, int& cur_frame)
{
	// MaxFrame 선택
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt("Max Frame", &max_frame);

	// currentFrame 선택
	ImGui::SameLine();
	ImGui::SetNextItemWidth(200.0f);
	ImGui::SliderInt("Frame", &cur_frame, 1, max_frame);

	static bool bPlay = false;
	static float timer = cur_frame;
	// Play 버튼
	ImGui::SameLine();
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

void WG_CharacterWindow::SelectUV(vector<pair<POINT, POINT>>& list, int& max_frame)
{
	static int start[2] = { 0, 0 };
	static int end[2] = { 0, 0 };
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("Start UV", start);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputInt2("End UV", end);
	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{
		if (list.size() == max_frame)
			max_frame++;
		list.push_back({ {start[0], start[1]}, { end[0], end[1]} });
	}
	//ImGui::SameLine();
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
	ImGui::SameLine();
	if (ImGui::Button("Delete Last Frame"))
	{
		if(list.size() > 0)
			list.pop_back();
	}
	
}

void WG_CharacterWindow::SelectVertexShader(string& id)
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

	if (vs_vec.size() != 0) id = vs_vec[item_current_idx];
}

void WG_CharacterWindow::SelectPixelShader(string& id)
{
	static int item_current_idx = 0;

	auto ps_set = RESOURCE->GetTotalPSID();
	vector<string> ps_vec(ps_set.begin(), ps_set.end());

	// 위에서 설정된 ps_id가 있다면 그 값으로 item_current_idx 변경
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

	if (ps_vec.size() != 0) id = ps_vec[item_current_idx];
}

void WG_CharacterWindow::SelectTexture(string& id)
{
	int item_current_idx = 0;

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

	if (tex_vec.size() != 0)id = tex_vec[item_current_idx];
}

void KGCA41B::WG_CharacterWindow::SaveCharacterData(CharacterData& data)
{
	string anim_id;
	int max_frame, cur_frame;
	string skm_id;
	string skeleton_id;
	string vs_id;
	string texture_id;
	string ps_id;

	string sheetName = data.character_name;
	if (sheetName.size() == 0)
		return;
	sheetName += ".csv";

	auto sheet = DATA->AddNewSheet(sheetName);

	auto character = sheet->AddItem(data.character_name);

	sheet->AddCategory("anim_id");
	sheet->AddCategory("max_frame");
	sheet->AddCategory("cur_frame");
	sheet->AddCategory("skm_id");
	sheet->AddCategory("skeleton_id");
	sheet->AddCategory("vs_id");
	sheet->AddCategory("texture_id");
	sheet->AddCategory("ps_id");
	
	character->SetValue("anim_id", data.anim_id);
	character->SetValue("max_frame", to_string(data.max_frame));
	character->SetValue("cur_frame", to_string(data.cur_frame));
	character->SetValue("skm_id", data.skm_id);
	character->SetValue("skeleton_id", data.skeleton_id);
	character->SetValue("vs_id", data.vs_id);
	character->SetValue("texture_id", data.texture_id);
	character->SetValue("ps_id", data.ps_id);

	DATA->SaveSheetFile(sheetName);
}

void WG_CharacterWindow::LoadCharacterData()
{
	auto sheet = DATA->LoadSheet(loading_data_id_);

	auto strs = split(loading_data_id_, '.');
	auto name = strs[0];

	auto item = sheet->LoadItem(name);
	if (item == NULL)
		return;
	
	character_data.max_frame = stoi(item->GetValue("max_frame"));
	character_data.cur_frame = stoi(item->GetValue("cur_frame"));
	character_data.texture_id = item->GetValue("texture_id");
	character_data.skm_id = item->GetValue("skm_id");
	character_data.skeleton_id = item->GetValue("skeleton_id");
	character_data.vs_id = item->GetValue("vs_id");
	character_data.ps_id = item->GetValue("ps_id");

	loading_data_id_ = "";
}

void WG_DataViewer::Update()
{
	ImGui::SetCurrentContext(GUI->GetContext());
}

void WG_DataViewer::Render()
{
	ImGui::Begin("Load Character File", &open_);
	{
		static int item_current_idx = 0;

		DATA->LoadAllData();
		auto id_list = DATA->GetAllDataSheetID();
		if (ImGui::BeginListBox("Data File"))
		{
			for (int n = 0; n < id_list.size(); n++)
			{
				const bool is_selected = (item_current_idx == n);
				if (ImGui::Selectable(id_list[n].c_str(), is_selected))
					item_current_idx = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			if (id_list.size() == 0)
			{
			}
			else
			{
				auto tool_window = dynamic_cast<WG_CharacterWindow*>(GUI->FindWidget("CharacterTool"));
				tool_window->set_loading_data_id(id_list[item_current_idx]);
			}
			GUI->FindWidget("FileViewer")->open_ = false;
		}
	}
	ImGui::End();
}
