#include "ModuleLogView.h"
#include "Log.h"
#include "imgui/imgui.h"

bool ModuleLogView::init()
{
	g_Log.addOutput(this);

	return true;
}

bool ModuleLogView::updateGUI()
{
	ImGui::Begin("Log View");

	if (ImGui::Button("Clear"))
	{
		allMessages.clear();
	}

	ImGui::Separator();

	ImGui::BeginChild(ImGui::GetID("Log View Child"));

	for (auto i = 0U; i < allMessages.size(); ++i)
	{
		int nattrs = 0;
		if (allMessages[i].find("<WARNING>") != std::string::npos) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
			nattrs = 1;
		} else if (allMessages[i].find("<ERROR>") != std::string::npos) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			nattrs = 1;
		} else if (allMessages[i].find("<DEBUG>") != std::string::npos) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
			nattrs = 1;
		}

		const char *line = allMessages[i].c_str();

		ImGui::TextWrapped("%s", line);

		ImGui::PopStyleColor(nattrs);
	}

	ImGui::SetScrollHere(1.0f);

	ImGui::EndChild();
	
	ImGui::End();

	return true;
}

void ModuleLogView::writeMessage(const std::string & message)
{
	allMessages.push_back(message);
}
