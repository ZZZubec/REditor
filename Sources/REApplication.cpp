#include "REApplication.h"

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
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/FileSystem.h>
#include "PugiXml/pugixml.hpp"
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Graphics/ModelView.h>

REApplication::REApplication(Urho3D::Context* context)
    : Application(context),
    yaw_(0.0f),
    pitch_(0.0f)
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

    CreateFigureBox();

    // Set the mouse mode to use in the sample
    InitMouseMode(Urho3D::MM_RELATIVE);

    // Pass console commands to file system.
    GetSubsystem<FileSystem>()->SetExecuteConsoleCommands(true);
    GetSubsystem<Console>()->RefreshInterpreters();
}

void REApplication::CreateFigureBox()
{
    _figure_mesh = new Redi::Figure(Redi::EFigureType::FT_QUAD);
    //forward
    {
        Redi::FVertex v1 {Vector3(1,0,0), Vector3(0,0,1), Vector2(1,1)};
        Redi::FVertex v2 {Vector3(1,1,0), Vector3(0,0,1), Vector2(1,0)};
        Redi::FVertex v3 {Vector3(0,1,0), Vector3(0,0,1), Vector2(0,0)};
        Redi::FVertex v4 {Vector3(0,0,0), Vector3(0,0,1), Vector2(0,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }
    
    //backward
    {
        Redi::FVertex v1 {Vector3(0,0,1), Vector3(0,0,-1), Vector2(0,1)};
        Redi::FVertex v2 {Vector3(0,1,1), Vector3(0,0,-1), Vector2(0,0)};
        Redi::FVertex v3 {Vector3(1,1,1), Vector3(0,0,-1), Vector2(1,0)};
        Redi::FVertex v4 {Vector3(1,0,1), Vector3(0,0,-1), Vector2(1,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }
    
    //left
    {
        Redi::FVertex v1 {Vector3(0,0,1), Vector3(-1,0,0), Vector2(0,1)};
        Redi::FVertex v2 {Vector3(0,1,1), Vector3(-1,0,0), Vector2(0,0)};
        Redi::FVertex v3 {Vector3(0,1,0), Vector3(-1,0,0), Vector2(1,0)};
        Redi::FVertex v4 {Vector3(0,0,0), Vector3(-1,0,0), Vector2(1,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }

    //right
    {
        Redi::FVertex v1 {Vector3(1,0,0), Vector3(1,0,0), Vector2(1,1)};
        Redi::FVertex v2 {Vector3(1,1,0), Vector3(1,0,0), Vector2(1,0)};
        Redi::FVertex v3 {Vector3(1,1,1), Vector3(1,0,0), Vector2(0,0)};
        Redi::FVertex v4 {Vector3(1,0,1), Vector3(1,0,0), Vector2(0,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }
    
    //top
    {
        Redi::FVertex v1 {Vector3(0,1,0), Vector3(0,1,0), Vector2(0,1)};
        Redi::FVertex v2 {Vector3(0,1,1), Vector3(0,1,0), Vector2(0,0)};
        Redi::FVertex v3 {Vector3(1,1,1), Vector3(0,1,0), Vector2(1,0)};
        Redi::FVertex v4 {Vector3(1,1,0), Vector3(0,1,0), Vector2(1,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }

    {
        Redi::FVertex v1 {Vector3(1,0,0), Vector3(0,-1,0), Vector2(1,1)};
        Redi::FVertex v2 {Vector3(1,0,1), Vector3(0,-1,0), Vector2(1,0)};
        Redi::FVertex v3 {Vector3(0,0,1), Vector3(0,-1,0), Vector2(0,0)};
        Redi::FVertex v4 {Vector3(0,0,0), Vector3(0,-1,0), Vector2(0,1)};
        _figure_mesh->AddFace(v1, v2, v3, v4);
    }
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
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(REApplication, OnUpdate));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(REApplication, HandleMouseModeRequest));

    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, during which we request
    // debug geometry
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(REApplication, HandlePostRenderUpdate));
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
    scene_->CreateComponent<DebugRenderer>();

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
    camera->SetFarClip(300.0f);
    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, -5.0f));
    cameraNode_->SetDirection(Vector3(-0.01f, -0.7f, 0.6f));

    boxNode_ = scene_->CreateChild("Box");
    boxNode_->SetPosition(Vector3(10,0, 10));
    const auto staticModel = boxNode_->CreateComponent<StaticModel>();
    staticModel->SetModel(cache_->GetResource<Model>("Models/Box.mdl"));
    staticModel->SetMaterial(cache_->GetResource<Material>("Materials/DefaultGrey.xml"));

    gizmo_ = MakeShared<Gizmo>(context_);

    //this->ReadFile("Scenes/RenderingShowcase_2_BakedDirect.xml");
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
        //
    }
}

void REApplication::OnUpdate(StringHash, VariantMap& eventData)
{
    float deltaTime = eventData[Update::P_TIMESTEP].GetFloat();

    MoveCamera(deltaTime);
    RenderUi(deltaTime);
}

void REApplication::SetEditorMode(Redi::EEditorMode editor_mode)
{
    _editor_mode = editor_mode;
}

void REApplication::MoveCamera(float deltaTime)
{
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    auto* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    if(useMouseMode_ == MouseMode::MM_RELATIVE)
    {
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
        cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0));
        

        // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
        if (input->GetKeyDown(KEY_W))
            cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * deltaTime);
        if (input->GetKeyDown(KEY_S))
            cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * deltaTime);
        if (input->GetKeyDown(KEY_A))
            cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * deltaTime);
        if (input->GetKeyDown(KEY_D))
            cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * deltaTime);

        // Toggle debug geometry with space
        if (input->GetKeyPress(KEY_SPACE))
            drawDebug_ = !drawDebug_;

        /*
        if (Raycast(250.0f))
        {
            if (input->GetKeyDown(KEY_E))
            {
                SetEditorMode(Redi::EEditorMode::EM_EXTRUDE);
            }
        }
        */
    }

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if ((ui->GetCursor() && !ui->GetCursor()->IsVisible()) || ui->GetElementAt(pos, true))
        return;

    current_node = nullptr;
    current_face = Redi::FFace();
    _indexes.clear();
    _vertices.clear();

    auto* graphics = GetSubsystem<Graphics>();
    auto* camera = cameraNode_->GetComponent<Camera>();
    Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    _figure_mesh->TraceLine(cameraRay.origin_, cameraRay.direction_, 250.0f);
}

void REApplication::RenderUi(float deltaTime)
{
    if (useMouseMode_ == Urho3D::MM_FREE)
    {
        gizmo_->ManipulateNode(cameraNode_->GetComponent<Camera>(), boxNode_);
    }

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
        
        ui::Text(cameraNode_->GetPosition().ToString().c_str());
        ui::Text(cameraNode_->GetDirection().ToString().c_str());

        gizmo_->RenderUI();
    }
    ui::End();
    if (metricsOpen_)
        ui::ShowMetricsWindow(&metricsOpen_);
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

    //SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(REApplication, HandleMouseModeRequest));
}

Redi::FFace REApplication::CreateFace(unsigned face_index)
{
    unsigned idx = face_index*3;
    Redi::FVertex v1 {_vertices[_indexes[idx + 0]] };
    Redi::FVertex v2 {_vertices[_indexes[idx + 1]] };
    Redi::FVertex v3 {_vertices[_indexes[idx + 2]] };

    Redi::FFace face;
    face.idx = face_index;
    face.vertices.push_back(v1);
    face.vertices.push_back(v2);
    face.vertices.push_back(v3);

    return face;
}

bool REApplication::Raycast(float maxDistance)
{
    hitDrawable = nullptr;

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if ((ui->GetCursor() && !ui->GetCursor()->IsVisible()) || ui->GetElementAt(pos, true))
        return false;

    current_node = nullptr;
    current_face = Redi::FFace();
    _indexes.clear();
    _vertices.clear();

    auto* graphics = GetSubsystem<Graphics>();
    auto* camera = cameraNode_->GetComponent<Camera>();
    Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    ea::vector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    scene_->GetComponent<Octree>()->RaycastSingle(query);
    if (results.size())
    {
        RayQueryResult& result = results[0];
        hitPos = result.position_;
        hitDrawable = result.drawable_;
        auto* geom = hitDrawable->GetLodGeometry(0, 0);
        IndexBuffer* index_buffer = geom->GetIndexBuffer();
        VertexBuffer* vertex_buffer = geom->GetVertexBuffer(0);
        const ea::vector<VertexElement>& vertex_elements = vertex_buffer->GetElements();
        max_faces_in_model = index_buffer->GetIndexCount()/3;
        ea::vector<unsigned> inds = index_buffer->GetUnpackedData();
        for(unsigned i=0; i<inds.size(); i++)
        {
            _indexes.push_back(inds[i]);
        }
        

        current_node = result.node_;
        result.position_ -= current_node->GetWorldPosition();

        const auto* vertexData = (const unsigned char*)vertex_buffer->Lock(0, vertex_buffer->GetVertexCount());
        if (vertexData)
        {
            unsigned numVertices = vertex_buffer->GetVertexCount();
            unsigned vertexSize = vertex_buffer->GetVertexSize();
            // Copy the original vertex positions
            for (unsigned i = 0; i < numVertices; ++i)
            {
                const Vector3& src = *reinterpret_cast<const Vector3*>(vertexData + i * vertexSize) * current_node->GetScale();
                _vertices.push_back(src);
            }
            vertex_buffer->Unlock();
        }

        float distance = 9999.0f;
        for(unsigned face_index=0; face_index < max_faces_in_model; face_index++)
        {
            unsigned idx = face_index * 3;
            Redi::FFace face = CreateFace(face_index);

            const Vector3 vec(
                result.position_.DistanceToPoint(face.vertices[0].position),
                result.position_.DistanceToPoint(face.vertices[1].position),
                result.position_.DistanceToPoint(face.vertices[2].position)
                );
            const float vec_length = vec.Length();

            if(face_index == 0)
            {
                distance = vec_length;
                current_face = face;
            }
            if (vec_length < distance)
            {
                distance = vec_length; 
                current_face = face;
            }
        }
        
        return true;
    }

    return false;
}

void REApplication::HandleMouseModeRequest(StringHash, VariantMap& eventData)
{
#if URHO3D_SYSTEMUI
    Urho3D::Console* console = GetSubsystem<Urho3D::Console>();
    if (console && console->IsVisible())
        return;
#endif
    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();

    unsigned buttonID = eventData[MouseButtonDown::P_BUTTON].GetInt();
    URHO3D_LOGINFO("mouse:{}", buttonID);
    if(buttonID == 4)
    {
        if(useMouseMode_ != Urho3D::MM_FREE)
        {
            useMouseMode_ = Urho3D::MM_FREE;
        }
        else
        {
            useMouseMode_ = Urho3D::MM_RELATIVE;
        }
        InitMouseMode(useMouseMode_);
    }
    /**/
}

void REApplication::HandleMouseModeChange(StringHash, VariantMap& eventData)
{
    Urho3D::Input* input = GetSubsystem<Urho3D::Input>();
    bool mouseLocked = eventData[Urho3D::MouseModeChanged::P_MOUSELOCKED].GetBool();
    input->SetMouseVisible(!mouseLocked);
}

Vector3 REApplication::MinVector(const Vector3& a, const Vector3& b)
{
    return Vector3(Min(a.x_, b.x_), Min(a.y_, b.y_), Min(a.z_, b.z_));
}

Vector3 REApplication::MaxVector(const Vector3& a, const Vector3& b)
{
    return Vector3(Max(a.x_, b.x_), Max(a.y_, b.y_), Max(a.z_, b.z_));
}

ea::vector<Vector3> REApplication::GetVerticesRect(const Redi::FFace& face, const Redi::FFace& next_face)
{
    ea::vector<Vector3> positions;
    positions.push_back(current_face.vertices[0].position);
    positions.push_back(current_face.vertices[1].position);
    positions.push_back(current_face.vertices[2].position);
            
    for(unsigned i=0; i<next_face.vertices.size(); i++)
    {
        bool find = false;
        for(unsigned j=0; j<current_face.vertices.size(); j++)
        {
            if(next_face.vertices[i].position.DistanceToPoint(current_face.vertices[j].position) < 0.001f)
            {
                find = true;
                break;
            }
        }
        if(!find)
        {
            positions.push_back(next_face.vertices[i].position);
            break;
        }
    }

    const float eps = 0.1f;
    Redi::FNormalRect rect;
    rect.min = positions[0];
    rect.max = positions[0];
    for(unsigned i=1; i<positions.size(); i++)
    {
        rect.min = MinVector(rect.min, positions[i]);
        rect.max = MaxVector(rect.max, positions[i]);
    }

    ea::vector<Vector3> rect_pos(4);
    
    if(Equals(positions[0].z_, positions[1].z_, eps) && Equals(positions[0].z_, positions[2].z_, eps))
    {
        rect.direction = Redi::ENormalDirection::ND_Z;
        for(unsigned i=0; i<positions.size(); i++)
        {
            if(positions[i].Equals(Vector3(rect.min.x_, rect.min.y_, rect.min.z_), eps))
            {
                rect_pos[0] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.min.x_, rect.max.y_, rect.min.z_), eps))
            {
                rect_pos[1] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.max.x_, rect.max.y_, rect.min.z_), eps))
            {
                rect_pos[2] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.max.x_, rect.min.y_, rect.min.z_), eps))
            {
                rect_pos[3] = positions[i];
            }
        }
    }
    else if(Equals(positions[0].x_, positions[1].x_, eps) && Equals(positions[0].x_, positions[2].x_, eps))
    {
        rect.direction = Redi::ENormalDirection::ND_X;
        for(unsigned i=0; i<positions.size(); i++)
        {
            if(positions[i].Equals(Vector3(rect.min.x_, rect.min.y_, rect.min.z_)))
            {
                rect_pos[0] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.min.x_, rect.min.y_, rect.max.z_), eps))
            {
                rect_pos[1] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.min.x_, rect.max.y_, rect.max.z_), eps))
            {
                rect_pos[2] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.min.x_, rect.max.y_, rect.min.z_), eps))
            {
                rect_pos[3] = positions[i];
            }
        }
    }
    else
    {
        rect.direction = Redi::ENormalDirection::ND_Y;
        for(unsigned i=0; i<positions.size(); i++)
        {
            if(positions[i].Equals(Vector3(rect.min.x_, rect.min.y_, rect.min.z_), eps))
            {
                rect_pos[0] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.min.x_, rect.min.y_, rect.max.z_), eps))
            {
                rect_pos[1] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.max.x_, rect.min.y_, rect.max.z_), eps))
            {
                rect_pos[2] = positions[i];
            }
            else if(positions[i].Equals(Vector3(rect.max.x_, rect.min.y_, rect.min.z_), eps))
            {
                rect_pos[3] = positions[i];
            }
        }
    }
    for(unsigned i=0; i<rect_pos.size(); i++)
    {
        rect_pos[i] = current_node->GetWorldPosition() + current_node->GetWorldRotation() * rect_pos[i];
    }

    return rect_pos;
}

Vector3 REApplication::RotateVector(const Vector3& origin, const Vector3& axis, float angle)
{
    Vector3 vxp = axis.CrossProduct(origin);
    Vector3 vxvxp = axis.CrossProduct(vxp);
    return origin + Sin(angle) * vxp + (1 - Cos(angle)) * vxvxp;
}
Vector3 REApplication::RotateAboutPoint(const Vector3& origin, const Vector3& pivot, const Vector3& axis, float angle)
{
    return pivot + RotateVector((origin - pivot), axis, angle);
}

void REApplication::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    // If draw debug mode is enabled, draw viewport debug geometry. This time use depth test, as otherwise the result becomes
    // hard to interpret due to large object count
    if (drawDebug_)
        GetSubsystem<Renderer>()->DrawDebugGeometry(true);

    DebugRenderer* dbgRenderer = scene_->GetComponent<DebugRenderer>();
    if(dbgRenderer)
    {
        for( unsigned x = 0; x <= 10; ++x)
        {
            for( unsigned z = 0; z <= 10; ++z)
            {
                dbgRenderer->AddQuad(Vector3(0.5f*x, 0, 0.5f*z), 1.0f, 1.0f, Color::BLACK, true);
            }
        }

        _figure_mesh->render(dbgRenderer);

        if (current_face.idx >= 0 && current_face.idx < max_faces_in_model)
        {
            unsigned next_face_id = 0;
            if(current_face.idx % 2 == 0)
            {
                next_face_id = current_face.idx + 1;
            }
            else
            {
                next_face_id = current_face.idx - 1;
            }

            Redi::FFace next_face = CreateFace(next_face_id);
            ea::vector<Vector3> rect_pos = GetVerticesRect(current_face, next_face);

            dbgRenderer->AddPolygon(rect_pos[0], rect_pos[1], rect_pos[2], rect_pos[3], Color::GRAY, false);
        }
    }
}


URHO3D_DEFINE_APPLICATION_MAIN(REApplication);

