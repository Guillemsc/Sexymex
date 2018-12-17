#include "ModuleMainMenu.h"
#include "ModuleAgentContainer.h"
#include "ModuleNodeCluster.h"
#include "ModuleYellowPages.h"
#include "ModuleTextures.h"
#include "Application.h"
#include "Log.h"
#include "imgui/imgui.h"

#include <d3d9.h>

bool ModuleMainMenu::updateGUI()
{
	ImGui::Begin("Main menu");

	D3DSURFACE_DESC surfaceDesc;
	auto texture = App->modTextures->banner;
	texture->GetLevelDesc(0, &surfaceDesc);
	const float width = 0.7f * surfaceDesc.Width;
	const float height = 0.7f * surfaceDesc.Height;
	ImGui::Image(texture, ImVec2(width, height));
	//ImGui::ImageButton(texture, ImVec2(width, height));

	if (ImGui::Button("Node cluster"))
	{
		setEnabled(false);
		App->agentContainer->setEnabled(true);
		App->modNodeCluster->setEnabled(true);
	}

	if (ImGui::Button("Yellow Pages"))
	{
		setEnabled(false);
		App->modYellowPages->setEnabled(true);
	}

	ImGui::End();

	return true;
}
