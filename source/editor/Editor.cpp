#include "pch/pch.h"

#include "editor/Editor.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/NodeContextActions.h"
#include "editor/PropertyEditor.h"

#include "asset/AssetManager.h"
#include "asset/PodIncludes.h"
#include "platform/windows/Win32Window.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"
#include "system/Engine.h"
#include "system/EngineEvents.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/Node.h"
#include "world/nodes/RootNode.h"
#include "world/World.h"
#include "system/Input.h"
#include "world/NodeFactory.h"

#include "editor/imgui/ImguiUtil.h"

#include <iostream>
#include <set>


Editor::Editor()
{
	ImguiImpl::InitContext();
	m_assetWindow = std::make_unique<AssetWindow>();
	m_nodeContextActions = std::make_unique<NodeContextActions>();
	m_propertyEditor = std::make_unique<PropertyEditor>();
	m_onNodeRemoved.Bind([&](Node* node) {
		if (node == m_selectedNode) {
			m_selectedNode = nullptr;
		}
	});


	m_loadFileBrowser.SetTitle("Load Scene");

	MakeMainMenu();
}


void Editor::MakeMainMenu()
{
	ImMenu sceneMenu{ "Scene" };
	sceneMenu.AddEntry("Save", [&]() { m_sceneSave.OpenBrowser(); });
	sceneMenu.AddEntry("Load", [&]() { m_loadFileBrowser.Open(); });
	sceneMenu.AddSeperator();
	sceneMenu.AddEntry("Exit", []() { Engine::GetMainWindow()->Destroy(); });
	m_menus.emplace_back(sceneMenu);

	ImMenu debugMenu{ "Debug" };
	debugMenu.AddEntry("Open ImGui Demo", [&]() { m_showImguiDemo = true; });
	m_menus.emplace_back(debugMenu);

	ImMenu aboutMenu{ "About" };
	aboutMenu.AddEntry("Help", [&]() { m_showHelpWindow = true; });
	aboutMenu.AddEntry("About", [&]() { m_showAboutWindow = true; });
	m_menus.emplace_back(aboutMenu);
}


Editor::~Editor()
{
	ImguiImpl::CleanupContext();
}

void Editor::UpdateEditor()
{
	HandleInput();

	ImguiImpl::NewFrame();

	if (m_showImguiDemo) {
		ImGui::ShowDemoWindow();
	}

	if (m_showAboutWindow) {
		Run_AboutWindow();
	}

	if (m_showHelpWindow) {
		Run_HelpWindow();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	// Attempt to predict the viewport size for the first run, might be a bit off.
	ImGui::SetNextWindowSize(ImVec2(450, 1042), ImGuiCond_FirstUseEver);
	ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);

	Run_MenuBar();

	ImGui::Checkbox("Update World", &m_updateWorld);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
	if (ImGui::Button("Save")) {
		m_sceneSave.OpenBrowser();
	}
	ImGui::SameLine();

	if (ImGui::Button("Load")) {
		m_loadFileBrowser.SetTitle("Load World");
		m_loadFileBrowser.Open();
	}
	ImGui::PopStyleVar();


	m_sceneSave.Draw();
	m_loadFileBrowser.Display();

	if (m_loadFileBrowser.HasSelected()) {
		m_sceneToLoad = m_loadFileBrowser.GetSelected();
		m_loadFileBrowser.ClearSelected();
	}

	if (ImGui::BeginChild("EditorScrollable", ImVec2(0, -15.f))) {
		if (ImGui::CollapsingHeader("Outliner", ImGuiTreeNodeFlags_DefaultOpen)) {
			Outliner();
		}

		if (m_selectedNode) {
			if (ImGui::CollapsingHeader(
					refl::GetClass(m_selectedNode).GetNameStr().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				m_propertyEditor->Inject(m_selectedNode);
			}
		}

		if (ImGui::CollapsingHeader("Assets")) {
			Run_AssetView();
		}
	}

	ImGui::EndChild();

	std::string s = fmt::format("{:.1f} FPS | {}", Engine::GetFPS(), Engine::GetStatusLine());
	ImGui::Text(s.c_str());


	ImGui::End();

	if (m_showGltfWindow) {
		m_showGltfWindow = m_assetWindow->Draw();
	}


	ImguiImpl::EndFrame();

	for (auto& cmd : m_postDrawCommands) {
		cmd();
	}
	m_postDrawCommands.clear();
}

void Editor::Outliner()
{
	ImGui::BeginChild(
		"Outliner", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.f, 6.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 6.f));

	RecurseNodes(Engine::GetWorld()->GetRoot(), [&](Node* node, int32 depth) {
		auto str = std::string(depth * 6, ' ') + sceneconv::FilterNodeClassName(node->GetClass().GetName()) + "> "
				   + node->m_name;
		ImGui::PushID(node->GetUID());
		if (ImGui::Selectable(str.c_str(), node == m_selectedNode)) {
			m_selectedNode = node;
		}
		Run_ContextPopup(node);
		Run_OutlinerDropTarget(node);
		ImGui::PopID();
	});

	ImGui::PopStyleVar(2);
	ImGui::EndChild();
}

void Editor::LoadScene(const fs::path& scenefile)
{
	Engine::Get().CreateWorldFromFile("/" + fs::relative(scenefile).string());
	Engine::Get().SwitchRenderer(0);

	m_selectedNode = nullptr;
	Event::OnWindowResize.Broadcast(Engine::GetMainWindow()->GetWidth(), Engine::GetMainWindow()->GetHeight());
}

void Editor::Run_ContextPopup(Node* node)
{
	if (ImGui::BeginPopupContextItem("Outliner Context")) {
		for (auto& action : m_nodeContextActions->GetActions(node)) {
			if (!action.IsSplitter()) {
				if (ImGui::MenuItem(action.name)) {
					action.function(node);
				}
			}
			else {
				ImGui::Separator();
			}
		}

		if (ImGui::BeginMenu("New Node")) {
			Run_NewNodeMenu(node);
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}

void Editor::Run_NewNodeMenu(Node* underNode)
{
	auto factory = Engine::GetWorld()->m_nodeFactory;


	for (auto& entry : factory->m_nodeEntries) {
		if (ImGui::MenuItem(entry.first.c_str())) {

			auto cmd = [underNode, &entry]() {
				auto newNode = entry.second.newInstance();

				newNode->SetName(entry.first + "_new");
				Engine::GetWorld()->RegisterNode(newNode, underNode);

				DirtyFlagset temp;
				temp.set();

				newNode->SetDirtyMultiple(temp);
			};

			PushCommand(cmd);
		}
	}
}

void Editor::Run_AssetView()
{
	auto reloadAssetLambda = [](std::unique_ptr<PodEntry>& assetEntry) {
		auto l = [&assetEntry](auto tptr) {
			using PodType = std::remove_pointer_t<decltype(tptr)>;
			PodHandle<PodType> p;
			p.podId = assetEntry->uid;
			AssetManager::Reload(p);
		};

		podtools::VisitPodType(assetEntry->type, l);
	};


	ImGui::Indent(10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
	bool UnloadAll = ImGui::Button("Unload All");
	ImGui::SameLine();
	bool ReloadUnloaded = ImGui::Button("Reload Unloaded");
	ImGui::SameLine();
	bool ReloadAll = ImGui::Button("Reload All");
	ImGui::SameLine();
	if (ImGui::Button("Search for GLTFs")) {
		m_showGltfWindow = true;
	}

	// Static meh..
	static ImGuiTextFilter filter;
	filter.Draw("Filter Assets", ImGui::GetFontSize() * 16);

	ImGui::PopStyleVar();


	std::string text;
	for (auto& assetEntry : Engine::GetAssetManager()->m_pods) {
		if (!filter.PassFilter(assetEntry->path.c_str()) && !filter.PassFilter(assetEntry->name.c_str())) {
			continue;
		}

		ImGui::PushID(static_cast<int32>(assetEntry->uid));
		bool disabled = !(assetEntry->ptr);

		if (disabled) {
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0, 0.6f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0, 0.7f, 0.6f));
			if (ImGui::Button("Reload") || ReloadUnloaded) {
				reloadAssetLambda(assetEntry);
			}
			ImGui::PopStyleColor(3);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
		}
		else {
			if (ImGui::Button("Unload") || UnloadAll) {
				AssetManager::Unload(BasePodHandle{ assetEntry->uid });
			}
		}

		if (ReloadAll) {
			reloadAssetLambda(assetEntry);
		}

		ImGui::SameLine();
		ImGui::Text(assetEntry->path.c_str());
		if (disabled) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		Run_MaybeAssetTooltip(assetEntry.get());


		ImGui::PopID();
	}
}

void Editor::Run_MaybeAssetTooltip(PodEntry* entry)
{
	if (ImGui::IsItemHovered()) {
		std::string text = fmt::format("Path:\t{}\nName:\t{}\nType:\t{}\n Ptr:\t{}\n UID:\t{}", entry->path,
			entry->name, entry->type.name(), entry->ptr, entry->uid);

		if (!entry->ptr) {
			ImUtil::TextTooltipUtil(text);
			return;
		}

		text += "\n";
		text += "\n";
		text += "\n";

		text += refltools::PropertiesToText(entry->ptr.get());

		ImUtil::TextTooltipUtil(text);
	}
}

void Editor::Run_OutlinerDropTarget(Node* node)
{
	// Drag Source
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
		ImGui::SetDragDropPayload("WORLD_REORDER", &node, sizeof(Node**));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WORLD_REORDER")) {
			CLOG_ABORT(payload->DataSize != sizeof(Node**), "Incorrect drop operation.");
			Node** dropSource = reinterpret_cast<Node**>(payload->Data);
			Editor::MakeChildOf(node, *dropSource);
		}
		ImGui::EndDragDropTarget();
	}
}

void Editor::Run_MenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 3.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 6.f));

	if (ImGui::BeginMenuBar()) {
		for (auto& entry : m_menus) {
			entry.Draw();
		}
		ImGui::EndMenuBar();
	}
	ImGui::PopStyleVar(2);
}

void Editor::Run_AboutWindow()
{
	constexpr float version = 1.f;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	if (ImGui::Begin("About", &m_showAboutWindow)) {
		auto str = fmt::format(R"(
Rayxen Engine:

Version: {}
Rayxen is a graphics/game engine focused on renderer extensibility.

Authors:
John Moschos - Founder, Programmer
Harry Katagis - Programmer
)",
			version);

		ImGui::Text(str.c_str());
		ImGui::Text("");
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::Run_HelpWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);

	if (ImGui::Begin("Help", &m_showHelpWindow)) {
		auto str = fmt::format(R"(
Help:

INSERT HELP HERE:
)");
		ImGui::Text(str.c_str());
		ImGui::Text("");
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::HandleInput()
{
}

void Editor::PushCommand(std::function<void()>&& func)
{
	Engine::GetEditor()->m_postDrawCommands.emplace_back(func);
}

void Editor::PreBeginFrame()
{
	if (!m_sceneToLoad.empty()) {
		LoadScene(m_sceneToLoad);
		m_sceneToLoad = "";
	}
}

void Editor::Duplicate(Node* node)
{
	PushCommand([node]() { Engine::GetWorld()->DeepDuplicateNode(node); });
}

void Editor::Delete(Node* node)
{
	PushCommand([node]() { Engine::GetWorld()->DeleteNode(node); });
}

void Editor::MoveChildUp(Node* node)
{
	auto& children = node->GetParent()->m_children;

	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt != begin(children)) {
		std::iter_swap(thisIt, thisIt - 1);
	}
}

void Editor::MoveChildDown(Node* node)
{
	auto& children = node->GetParent()->m_children;
	auto thisIt = std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

	// Should be impossible unless world ptrs are corrupted
	CLOG_ABORT(thisIt == end(children), "Attempting to move child not in its parent container.");

	if (thisIt + 1 != end(children)) {
		std::iter_swap(thisIt, thisIt + 1);
	}
	// WIP: decide what flag this sets
}

void Editor::MoveChildOut(Node* node)
{
	if (node->GetParent()->IsRoot()) {
		return;
	}
	auto lateCmd = [node]() {
		auto worldMatrix = node->GetWorldMatrix();
		node->GetParent()->SetDirty(Node::DF::Children);

		auto& children = node->GetParent()->m_children;
		std::vector<NodeUniquePtr>::iterator thisIt
			= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

		NodeUniquePtr src = std::move(*thisIt);

		children.erase(thisIt);

		Node* insertBefore = node->GetParent();
		auto& newChildren = insertBefore->GetParent()->m_children;

		auto insertAt = std::find_if(
			begin(newChildren), end(newChildren), [&insertBefore](auto& elem) { return insertBefore == elem.get(); });

		newChildren.emplace(insertAt, std::move(src));

		node->m_parent = insertBefore->GetParent();
		node->SetWorldMatrix(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}


void Editor::MoveSelectedUnder(Node* node)
{
	MakeChildOf(node, Engine::GetEditor()->m_selectedNode);
}


void Editor::MakeChildOf(Node* newParent, Node* node)
{
	if (!node || !newParent || node == newParent) {
		return;
	}

	// We cannot move a parent to a child. Start from node and iterate to root. If we find "selectedNode" we cannot
	// move.
	Node* pathNext = newParent->GetParent();
	while (pathNext) {
		if (pathNext == node) {
			LOG_INFO("Cannot move '{}' under '{}' because the second is a child of the first.", node->GetName(),
				newParent->GetName());
			return;
		}
		pathNext = pathNext->GetParent();
	}

	auto lateCmd = [newParent, node]() {
		auto worldMatrix = node->GetWorldMatrix();
		node->GetParent()->SetDirty(Node::DF::Children); // That parent is losing a child.

		auto& children = node->GetParent()->m_children;
		std::vector<NodeUniquePtr>::iterator thisIt
			= std::find_if(begin(children), end(children), [&node](auto& elem) { return node == elem.get(); });

		NodeUniquePtr src = std::move(*thisIt);

		children.erase(thisIt);

		newParent->m_children.emplace_back(std::move(src));

		node->m_parent = newParent;
		node->SetWorldMatrix(worldMatrix);
		node->SetDirty(Node::DF::Hierarchy);
		node->GetParent()->SetDirty(Node::DF::Children);
	};

	PushCommand(lateCmd);
}


void Editor::LookThroughThis(Node* node)
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera) {
		camera->SetWorldMatrix(node->GetWorldMatrix());
	}
}

void Editor::TeleportToCamera(Node* node)
{
	auto camera = Engine::GetWorld()->GetActiveCamera();
	if (camera) {
		node->SetWorldMatrix(camera->GetWorldMatrix());
	}
}

void Editor::MakeActiveCamera(Node* node)
{
	if (node->IsA<CameraNode>()) {
		Engine::GetWorld()->m_activeCamera = static_cast<CameraNode*>(node);
	}
}
