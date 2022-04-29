#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Model.h>

#include <Urho3D/SystemUI/SystemMessageBox.h>
#include <Urho3D/SystemUI/Gizmo.h>

#include "Figure.h"
#include "Structures.h"

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
    void CreateFigureBox();
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

    void SetEditorMode(Redi::EEditorMode editor_mode);
    void MoveCamera(float deltaTime);
    /// Assemble debug UI and handle UI events.
    void RenderUi(float deltaTime);

    void InitMouseMode(MouseMode mode);

    Redi::FFace CreateFace(unsigned face_index);
    bool Raycast(float maxDistance);
    void HandleMouseModeRequest(StringHash /*eventType*/, VariantMap& eventData);
    void HandleMouseModeChange(StringHash /*eventType*/, VariantMap& eventData);
    Vector3 MinVector(const Vector3& a, const Vector3& b);
    Vector3 MaxVector(const Vector3& a, const Vector3& b);

    ea::vector<Vector3> GetVerticesRect(const Redi::FFace& face, const Redi::FFace& next_face);
    Vector3 RotateVector(const Vector3& origin, const Vector3& axis, float angle);
    Vector3 RotateAboutPoint(const Vector3& origin, const Vector3& pivot, const Vector3& axis, double angle);
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
    float pitch_;

    bool drawDebug_;

    Node* current_node{nullptr};
    Redi::FFace current_face;
    unsigned max_faces_in_model{0};

    Vector3 hitPos{Vector3::ZERO};
    Drawable* hitDrawable{nullptr};
    
    ea::vector<Vector3> _vertices{};
    ea::vector<unsigned> _indexes{};
    Redi::EEditorMode _editor_mode;

    Redi::Figure* _figure_mesh;
};