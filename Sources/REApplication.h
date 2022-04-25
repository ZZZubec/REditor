#pragma once

#include <Engine/Application.h>
#include <Scene/Scene.h>
#include <Input/Input.h>
#include <UI/Text.h>
#include <Graphics/Model.h>

#include <SystemUI/SystemMessageBox.h>
#include <SystemUI/Gizmo.h>

using namespace Urho3D;

class REApplication : public Application
{
    URHO3D_OBJECT(REApplication, Application);
public:
    /// Construct.
    explicit REApplication(Context* context);

    void Setup() override;
    /// Setup after engine initialization and before running the main loop.
    void Start() override;
    void CreateConsoleAndDebugHud();
    /// Tear down any state that would pollute next initialization of the sample.
    void Stop() override;

private:
    void CreateVertexBufferFromFloatArray(int step, Texture2D* Displacement, int w, int h);
    SharedPtr<Model> CreateModel(const SharedPtr<VertexBuffer>& vertex_buffer, const SharedPtr<IndexBuffer>& index_buffer, int w, int h, int vertexCount);

    /// Creates a scene. Only required to provide background color that is not black.
    void CreateScene();
    
    /// Subscribe to application-wide logic update events.
    void SubscribeToEvents();
    
    void ReadFile(ea::string Filename);
    void Dump(ea::string Path, const XMLElement element);
    /// Process key events like opening a console window.
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    
    /// Animate cube, handle keys.
    void OnUpdate(StringHash, VariantMap& eventData);

    void MoveCamera(float deltaTime);
    /// Assemble debug UI and handle UI events.
    void RenderUi(float deltaTime);

    void InitMouseMode(MouseMode mode);

    void HandleMouseModeRequest(StringHash /*eventType*/, VariantMap& eventData);
    void HandleMouseModeChange(StringHash /*eventType*/, VariantMap& eventData);

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

    /// Scene.
    SharedPtr<Scene> scene_;
    /// Camera scene node.
    SharedPtr<Node> cameraNode_;

    /// Reference holding message box.
    SharedPtr<SystemMessageBox> messageBox_;
    /// Box node.
    SharedPtr<Node> boxNode_;
    /// Box node.
    SharedPtr<Gizmo> gizmo_;
    /// Flag controlling display of imgui demo window.
    bool metricsOpen_ = false;

    MouseMode useMouseMode_;

    XMLFile* xml_file_;

    float yaw_;
    /// Camera pitch angle.
    float pitch_;

    bool drawDebug_;
};