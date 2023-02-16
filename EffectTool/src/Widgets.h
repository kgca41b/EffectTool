#pragma once
#include "Engine_include.h"
#include "DataTypes.h"
#include "CharacterActor.h"
#include "CharacterData.h"

namespace KGCA41B
{
	class WG_MainMenu : public KGCA41B::GuiWidget
	{
	public:
		virtual void Update() override;
		virtual void Render() override;
	};

	class WG_CharacterWindow : public KGCA41B::GuiWidget
	{
	public:
		virtual void Update() override;
		virtual void Render() override;
	private:
		void	CharacterBoard();
		void	SelectBlendOptions();
		void	SelectFrame(int& max_frame, int& cur_frame);
		void	SelectUV(vector<pair<POINT, POINT>>& list, int& max_frame);
		void	SelectVertexShader(string& id);
		void	SelectPixelShader(string& id);
		void	SelectTexture(string& id);

	private:
		void SaveCharacterData(CharacterData& data);

	private:
		string loading_data_id_;
	public:
		void set_loading_data_id(string loading_data_id) { loading_data_id_ = loading_data_id; }
		void LoadCharacterData();
	public:
		CharacterData character_data;
	};

	class WG_DataViewer : public KGCA41B::GuiWidget
	{
		virtual void Update() override;
		virtual void Render() override;
	public:
		string	data_id_;
	};
}

