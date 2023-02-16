#include "CharacterTool.h"

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	ENGINE->OnInit(hinstance, L"CharacterTool", { 1600, 900 });

	KGCA41B::CharacterTool tool;

	ENGINE->Run(&tool);

	ENGINE->OnRelease();

	return 0;
}