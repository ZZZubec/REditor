#include "REApplication.h"

#include <Assimp/contrib/rapidjson/include/rapidjson/document.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Input/InputConstants.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/SystemUI/Console.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/Geometry.h>

#include "IO/FileSystem.h"
#include "PugiXml/pugixml.hpp"

#include "Structures.h"

REApplication::REApplication(Urho3D::Context* context)
    : Application(context)
{
}

void REApplication::Setup()
{
    engineParameters_[Urho3D::EP_WINDOW_TITLE] = "REditor";
    //engineParameters_[Urho3D::EP_LOG_NAME]     = GetSubsystem<Urho3D::FileSystem>()->GetAppPreferencesDir("rbfx", "samples") + GetTypeName() + ".log";
    engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
    engineParameters_[Urho3D::EP_HEADLESS]     = false;
    engineParameters_[Urho3D::EP_SOUND]        = true;
    engineParameters_[Urho3D::EP_HIGH_DPI]     = false;
    engineParameters_[Urho3D::EP_RESOURCE_PATHS] = "CoreData;Data";

    if (!engineParameters_.contains(Urho3D::EP_RESOURCE_PREFIX_PATHS))
        engineParameters_[Urho3D::EP_RESOURCE_PREFIX_PATHS] = ";..;../..";

}

void REApplication::Start()
{
    CreateConsoleAndDebugHud();
    
    // Create scene providing a colored background.
    CreateScene();

    // Finally subscribe to the update event. Note that by subscribing events at this point we have already missed some events
    // like the ScreenMode event sent by the Graphics subsystem when opening the application window. To catch those as well we
    // could subscribe in the constructor instead.
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    InitMouseMode(Urho3D::MM_FREE);

    // Pass console commands to file system.
    GetSubsystem<FileSystem>()->SetExecuteConsoleCommands(true);
    GetSubsystem<Console>()->RefreshInterpreters();
}

void REApplication::CreateConsoleAndDebugHud()
{
    // Create console
    Console* console = context_->GetSubsystem<Engine>()->CreateConsole();

    // Create debug HUD.
    DebugHud* debugHud = context_->GetSubsystem<Engine>()->CreateDebugHud();
}

void REApplication::Stop()
{
    // Only necessary so sample can be reopened. Under normal circumnstances applications do not need to do this.
    //context_->RemoveFactory<SimpleWindow>();
}

void REApplication::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(REApplication, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(REApplication, RenderUi));
}

void REApplication::CreateScene()
{
    ui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    ui::GetIO().BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    auto* cache_ = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
    // (-1000, -1000, -1000) to (1000, 1000, 1000)
    scene_->CreateComponent<Octree>();

    // Create a Zone component into a child scene node. The Zone controls ambient lighting and fog settings. Like the Octree,
    // it also defines its volume with a bounding box, but can be rotated (so it does not need to be aligned to the world X, Y
    // and Z axes.) Drawable objects "pick up" the zone they belong to and use it when rendering; several zones can exist
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    // Set same volume as the Octree, set a close bluish fog and some ambient light
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    cameraNode_ = scene_->CreateChild("Camera");
    auto camera = cameraNode_->CreateComponent<Camera>();
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

    boxNode_ = scene_->CreateChild("Box");
    boxNode_->SetPosition(cameraNode_->LocalToWorld(Vector3::FORWARD * 3.0f));
    const auto staticModel = boxNode_->CreateComponent<StaticModel>();
    staticModel->SetModel(cache_->GetResource<Model>("Models/Box.mdl"));
    staticModel->SetMaterial(cache_->GetResource<Material>("Materials/DefaultGrey.xml"));

    gizmo_ = MakeShared<Gizmo>(context_);

    //this->ReadFile("Scenes/RenderingShowcase_2_BakedDirect.xml");
}

void REApplication::RenderUi(StringHash eventType, VariantMap& eventData)
{
    gizmo_->ManipulateNode(cameraNode_->GetComponent<Camera>(), boxNode_);

    ui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_FirstUseEver);
    ui::SetNextWindowPos(ImVec2(200, 300), ImGuiCond_FirstUseEver);
    if (ui::Begin("Sample SystemUI", 0, ImGuiWindowFlags_NoSavedSettings))
    {
        if (messageBox_)
        {
            if (ui::Button("Close message box"))
                messageBox_ = nullptr;
        }
        else
        {
            if (ui::Button("Show message box"))
            {
                messageBox_ = new SystemMessageBox(context_, "Hello from SystemUI", "Sample Message Box");
                SubscribeToEvent(E_MESSAGEACK, [&](StringHash, VariantMap&) {
                    messageBox_ = nullptr;
                });
            }
        }

        if (ui::Button("Toggle console"))
            GetSubsystem<Console>()->Toggle();

        if (ui::Button("Toggle metrics window"))
            metricsOpen_ ^= true;

        gizmo_->RenderUI();
    }
    ui::End();
    if (metricsOpen_)
        ui::ShowMetricsWindow(&metricsOpen_);
}

void REApplication::ReadFile(const ea::string Filename)
{
    /*
    pugi::xml_document* doc = xml_file_->GetDocument();
    auto sc = doc->root().first_child();
    auto str_c = sc.child_value();
    URHO3D_LOGINFO("{}", str_c);
    */

    //xml_file_ = cache_->GetResource<XMLFile>("Scenes/RenderingShowcase_2_BakedDirect.xml");

    auto* cache_ = GetSubsystem<ResourceCache>();
    if(JSONFile* source = cache_->GetResource<JSONFile>("EditorStrings.json"))
    {
        URHO3D_LOGINFO("JSON");
        const auto jObj = source->GetRoot().GetObject();
        for (auto it = jObj.begin(); it != jObj.end(); it++)
        {
            ea::string unitString = it->first.trimmed();
            if(unitString == "Language")
            {
                ea::string name = it->second.GetString();
                const auto jLangValues = it->second.GetObject();
                for (auto it = jLangValues.begin(); it != jLangValues.end(); it++)
                {
                    ea::string unitString = it->first.trimmed();
                    ea::string v = it->second.GetString();
                    URHO3D_LOGINFO("{}={}", unitString, v);
                }

            }
        }
        //
        URHO3D_LOGINFO("JSON END");
    }
}

void REApplication::Dump(ea::string Path, const XMLElement element)
{
    URHO3D_LOGINFO("{}:{}, ats:{}", Path, element.GetName(), element.GetNumAttributes());
    for( const auto at : element.GetAttributeNames())
    {
        URHO3D_LOGINFO("at:{}", at);
    }
    if(const XMLElement child = element.GetNext())
    {
        Path += element.GetName() + "/";
        Dump(Path, child);
    }
}

void REApplication::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    if (eventData[KeyDown::P_KEY].GetUInt() == KEY_BACKQUOTE)
        GetSubsystem<Console>()->Toggle();

    auto* input = GetSubsystem<Input>();
    if(input->GetKeyPress(KEY_SPACE))
    {
        this->ReadFile("Scenes/RenderingShowcase_2_BakedDirect.xml");
        /*
        pugi::xml_document* doc = xml_file_->GetDocument();

        ea::string xml_node_name("");
        for (pugi::xml_node compElement = doc->root().child("scene"); compElement; compElement = compElement.next_sibling("component"))
        {
            URHO3D_LOGINFO("{}", compElement.name());
            xml_node_name.append("->" + (ea::string)compElement.name());
            for (const pugi::xml_attribute at : compElement.attributes())
            {
                ea::string at_name = at.name();
                ea::string at_value = at.value();
                URHO3D_LOGINFO("/{} {}={}", xml_node_name, at_name, at_value);
            }
        }
        */
        /*
        for (const pugi::xml_attribute at : xml_node.attributes())
        {
            ea::string at_name = at.name();
            ea::string at_value = at.value();
            URHO3D_LOGINFO("/{}->. {} = {}", xml_node.name(), at_name, at_value);
        }
        */

        /*
        auto source = xml_file_->GetRoot();
        XMLElement compElem = source.GetChild("component");
        while (compElem)
        {
            ea::string typeName = compElem.GetAttribute("type");
            unsigned compID = compElem.GetUInt("id");
            URHO3D_LOGINFO("/component->{}({})", compElem.GetName(), typeName);
            compElem = compElem.GetNext("component");
        }

        XMLElement childElem = source.GetChild("node");
        while (childElem)
        {
            unsigned nodeID = childElem.GetUInt("id");
            URHO3D_LOGINFO("/node->{}", compElem.GetName());
            childElem = childElem.GetNext("node");
        }
        */
    }
}

void REApplication::InitMouseMode(MouseMode mode)
{
    useMouseMode_ = mode;

    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();

    if (Urho3D::GetPlatform() != "Web")
    {
        if (useMouseMode_ == Urho3D::MM_FREE)
            input->SetMouseVisible(true);

        if (useMouseMode_ != Urho3D::MM_ABSOLUTE)
        {
            input->SetMouseMode(useMouseMode_);
#if URHO3D_SYSTEMUI
            Urho3D::Console* console = GetSubsystem<Urho3D::Console>();
            if (console && console->IsVisible())
                input->SetMouseMode(Urho3D::MM_ABSOLUTE, true);
#endif
        }
    }
    else
    {
        input->SetMouseVisible(true);
        //SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(REApplication, HandleMouseModeRequest));
        SubscribeToEvent(Urho3D::E_MOUSEMODECHANGED, URHO3D_HANDLER(REApplication, HandleMouseModeChange));
    }

    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(REApplication, HandleMouseModeRequest));
}

void REApplication::HandleMouseModeRequest(StringHash, VariantMap& eventData)
{
#if URHO3D_SYSTEMUI
    Urho3D::Console* console = GetSubsystem<Urho3D::Console>();
    if (console && console->IsVisible())
        return;
#endif
    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();

    unsigned buttonID = eventData["Button"].GetInt();
    if(buttonID == 4)
    {
        if(useMouseMode_ == Urho3D::MM_ABSOLUTE)
        {
            useMouseMode_ = Urho3D::MM_FREE;
        }
        else
        {
            useMouseMode_ = Urho3D::MM_ABSOLUTE;
        }
    }

    if (useMouseMode_ == Urho3D::MM_ABSOLUTE)
        input->SetMouseVisible(false);
    else if (useMouseMode_ == Urho3D::MM_FREE)
        input->SetMouseVisible(true);
    input->SetMouseMode(useMouseMode_);
}

void REApplication::HandleMouseModeChange(StringHash, VariantMap& eventData)
{
    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();
    bool mouseLocked = eventData[Urho3D::MouseModeChanged::P_MOUSELOCKED].GetBool();
    input->SetMouseVisible(!mouseLocked);
}


URHO3D_DEFINE_APPLICATION_MAIN(REApplication);

