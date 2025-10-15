# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KRAFTON TechLab Week03/04 - Unreal Engine Style 3D Editor & Rendering System
A DirectX 11-based 3D rendering engine and editor that mimics Unreal Engine 4's architecture, built with C++ Actor-Component system.

## Build & Run

### Building the Project
- **Solution File**: `TL2.sln` (Visual Studio 2022)
- **Project**: `TL2\TL2.vcxproj`
- **Platform**: Windows 10/11, x64 (Debug/Release configurations available)
- **Build Command**: Open `TL2.sln` in Visual Studio and build (F7) or use MSBuild from command line

### Development Environment
- Visual Studio 2022 (v143 toolset)
- DirectX 11 SDK
- Windows SDK 10.0

## Architecture Overview

### Core Object System (Unreal Engine Style)

**UObject System** (`Object.h`, `ObjectFactory.h`)
- Base class for all engine objects with UUID-based identification
- Runtime Type Information (RTTI) using `UClass` descriptors
- Custom memory management via `CMemoryManager`
- Factory pattern for object creation using `ObjectFactory::NewObject<T>()`
- Macro-based class registration: `DECLARE_CLASS(ThisClass, SuperClass)` and `IMPLEMENT_CLASS(ThisClass)`

**FName System** (`Name.h`)
- String pooling for efficient memory usage and O(1) comparisons
- Case-insensitive comparisons ("Test" == "test")
- Uses `FNamePool` for centralized string storage

**Type Iteration** (`ObjectIterator.h`)
- `TObjectIterator<T>` for type-safe iteration over specific object types
- Automatically filters objects from `GUObjectArray` by class type

### Actor-Component Architecture

**AActor** (`Actor.h`)
- Base class for all scene objects
- Transform API: Location, Rotation (Quaternion), Scale
- Component composition via `USceneComponent` hierarchy
- `CreateDefaultSubobject<T>()` for component creation
- Tick system with `BeginPlay()`, `Tick()`, `EndPlay()`

**UActorComponent** (`ActorComponent.h`)
- Base component class with owner reference
- `USceneComponent` adds transform hierarchy
- Common components:
  - `UStaticMeshComponent` - Static mesh rendering
  - `UCameraComponent` - Camera view
  - `UGizmoComponent` - Editor gizmo manipulation
  - `UPrimitiveComponent` - Renderable geometry base
  - `UDecalComponent` - Projected decal rendering
  - `USpotlightComponent` - Spot light with cone angle
  - `UFireBallComponent` - Custom fireball effect rendering
  - `UHeightFogComponent` - Height-based fog effect
  - `UBillboardComponent` - 2D sprite billboarding
  - `UTextRenderComponent` - 3D text rendering
  - `UMovementComponent` - Base movement component
  - `URotationMovementComponent` - Automatic rotation
  - `UProjectileMovementComponent` - Physics-based projectile motion
  - `UAABoundingBoxComponent` - Axis-aligned bounding box
  - `UOBoundingBoxComponent` - Oriented bounding box

### World & Level Management

**UWorld** (`World.h`)
- Container for all actors in a level
- Manages main camera, grid, gizmo actors
- `SpawnActor<T>()` and `DestroyActor()` for actor lifecycle
- Scene graph with spatial data structures (Octree/BVH)
- PIE (Play In Editor) support

**UEngine & UEditorEngine** (`Engine.h`, `EditorEngine.h`)
- `UEngine` base class with world context management
- `UEditorEngine` adds PIE functionality (`StartPIE()`, `EndPIE()`)
- Dual-world system: Editor world vs PIE world
- `UGameEngine` for runtime execution

**ULevel** (`Level.h`)
- Scene serialization and deserialization
- Actor persistence across sessions

### Rendering System

**Renderer** (`Renderer.h`)
- DirectX 11 rendering abstraction
- Constant buffer management for transforms/materials
- Batch line rendering system (up to 10,000 lines per batch)
- View modes: Lit, Unlit, Wireframe
- Render state caching to minimize API calls

**Shaders** (`.hlsl` files)
- `Primitive.hlsl` - Basic mesh rendering
- `StaticMeshShader.hlsl` - Static mesh with materials
- `ShaderLine.hlsl` - Batch line rendering
- `TextBillboard.hlsl` - Billboard text (UUID display)
- `Billboard.hlsl` - Billboard components

**Static Mesh System** (`StaticMesh.h`, `ObjManager.h`)
- OBJ file parsing with MTL material support
- Binary caching (`.obj` → `.bin` + `Mat.bin`) for fast loading
- Multiple material sections per mesh
- BVH (Bounding Volume Hierarchy) for ray tracing
- UV scrolling for texture animation

### Viewport & UI System

**Multi-Viewport Architecture** (`FViewport.h`, `SViewportWindow.h`)
- `FViewport` - D3D11 render target wrapper with input handling
- `FViewportClient` - Viewport behavior controller
- 4-split viewport layout: Perspective, Front, Side, Top
- Independent cameras and render targets per viewport

**Slate-Style Window System** (`SWindow.h`)
- `SWindow` - Base window class with rect-based layout
- `SSplitter` - Resizable split containers (horizontal/vertical)
- `SViewportWindow` - 3D scene viewport window
- `SMultiViewportWindow` - Multi-viewport container with layout switching

**Viewport Layout Switching** (`SMultiViewportWindow.h`)
- Animated transitions between layouts using ease-in-out interpolation
- `SwitchLayout()` - Switch between FourSplit and SingleMain modes
- `SwitchPanel()` - Maximize/restore individual viewports with smooth animation
- Layout state persisted in Editor.ini
- `ActiveViewport` tracking for proper input routing during drag operations

**Menu Bar System** (`MenuBarWidget.h`)
- UE-style menu bar with File, Edit, Window, Help menus
- Callbacks for menu actions (OnFileMenuAction, OnEditMenuAction, etc.)
- Integrates with SMultiViewportWindow for layout control
- ImGui-based rendering at top of editor window

**Show Flags** (`Enums.h: EEngineShowFlags`)
- Bit-flag based rendering toggles
- Controls visibility of primitives, bounding boxes, grid, text, etc.
- Per-viewport show flag configuration

### Selection & Interaction

**USelectionManager** (`SelectionManager.h`)
- Singleton for managing selected actors
- Multi-selection support with `TArray<AActor*>`
- Safe cleanup of invalid/deleted actors
- Synchronized between viewport and outliner

**Input System** (`InputManager.h`)
- Centralized keyboard/mouse input management
- Editor shortcuts (Ctrl+C/V for copy/paste)
- Alt+Drag for object duplication
- WASD + mouse for camera control

**Gizmo System** (`GizmoActor.h`, `GizmoComponent.h`)
- 3-axis manipulation gizmo (Translation, Rotation, Scale)
- Color-coded axes: X(Red), Y(Green), Z(Blue)
- Picking-based interaction

### Scene Management & Serialization

**Scene I/O** (`SceneLoader.h`, `Archive.h`)
- Binary serialization using `FArchive` abstraction
- Saves: Actor transforms, static mesh references, camera states, UUID state
- Scene files: `.scene` extension in `Data/` folder
- UUID synchronization on load (`UObject::SetNextUUID()`)

**Resource Management** (`ResourceManager.h`, `ObjManager.h`)
- Centralized asset loading and caching
- Automatic OBJ preloading from `Data/` folder
- Prevents duplicate resource loads
- Binary cache invalidation on source file changes

**Editor Configuration System** (`pch.h`)
- Global `EditorINI` (TMap<FString, FString>) stores editor preferences
- Persists splitter ratios, camera positions, and UI layout states
- Automatically saved on shutdown via `SMultiViewportWindow::OnShutdown()`
- Automatically loaded on startup via `SMultiViewportWindow::LoadSplitterConfig()`
- Common settings: "RootSplitter", "TopPanel", "LeftPanel", "BottomPanel", "LeftTop", "LeftBottom" ratios

### Coordinate System

**DirectX to UE Conversion**
- DirectX: Y-Up, Z-Depth → UE: Z-Up, X-Depth
- Transformation applied across camera, transforms, and rendering pipeline
- Handled in `SceneRotationUtils.h`

### Play In Editor (PIE)

**PIE Workflow**
- Editor world duplicated to PIE world with `Duplicate()` methods
- Separate `UGameEngine` instance for PIE execution
- State isolation between editor and game worlds
- Deferred cleanup on PIE shutdown (`bPIEShutdownRequested`)

## Key Data Structures

**FTransform** (`Vector.h`)
- Position: `FVector`
- Rotation: `FQuat` (Quaternion)
- Scale: `FVector`

**Containers** (`UEContainer.h`)
- `TArray<T>` - std::vector wrapper
- `TMap<K,V>` - std::unordered_map wrapper
- `TSet<T>` - std::unordered_set wrapper

## Common Development Patterns

### Creating a New Actor Class

```cpp
// MyActor.h
class AMyActor : public AActor
{
public:
    DECLARE_CLASS(AMyActor, AActor)
    AMyActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
};

// MyActor.cpp
IMPLEMENT_CLASS(AMyActor)
AMyActor::AMyActor() { /* constructor */ }

// IMPORTANT: Add to AllClassesRegistration.cpp
#include "MyActor.h"
IMPLEMENT_CLASS(AMyActor)
CLASS_META(AMyActor, Spawnable, "true")  // Optional: makes it spawnable in editor
```

**Class Registration** (`AllClassesRegistration.cpp`)
- ALL new classes MUST be registered in `AllClassesRegistration.cpp`
- Use `IMPLEMENT_CLASS(ClassName)` macro for basic registration
- Use `CLASS_META(ClassName, MetaKey, "MetaValue")` for additional metadata
- Common metadata:
  - `Spawnable, "true"` - Makes actor spawnable in Scene Manager
  - `CanSpawnInTransformWidget, "true"` - Allows component in transform widget

### Spawning Actors

```cpp
// In UWorld or any code with world access
AMyActor* NewActor = World->SpawnActor<AMyActor>();
NewActor->SetActorLocation(FVector(100, 200, 50));
```

### Iterating Objects by Type

```cpp
for (TObjectIterator<AActor> It; It; ++It)
{
    AActor* Actor = *It;
    // Process actor
}
```

### Safe Type Casting

```cpp
if (SomeObject->IsA<AActor>())
{
    AActor* Actor = static_cast<AActor*>(SomeObject);
}
```

### Adding Components

```cpp
// In actor constructor
UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
RootComponent = MeshComp; // Set as root
```

## Important Files & Locations

- **Assets**: `TL2/Data/` - OBJ models, textures, scene files
- **Shaders**: `TL2/*.hlsl` - DirectX 11 shaders
- **UI Widgets**: `TL2/UI/Widget/` - ImGui-based editor UI
- **Scene Manager**: `SceneManagerWidget.cpp` - World Outliner equivalent
- **All Classes Registration**: `AllClassesRegistration.cpp` - Central place where all IMPLEMENT_CLASS macros are included
- **Precompiled Header**: `pch.h` / `pch.cpp` - Contains common includes, global variables like `EditorINI` and `DeltaSeconds`

## Debugging & Testing

**Scene Manager (World Outliner)**
- Access via Outliner Window in editor
- Create actors: `+ Cube`, `+ Sphere`, `+ Triangle` buttons
- Search/filter actors by name
- Context menu: Focus, Reset Transform, Delete

**Show Flags Debugging**
- Toggle primitives, bounding boxes, grid, UUIDs
- Per-viewport configuration in View menu

**Picking & Selection**
- Click objects in 3D viewport or Scene Manager
- Selected objects show gizmo automatically
- Properties displayed in Details Panel

## Memory Management

- All `UObject` derivatives use custom allocator (`CMemoryManager`)
- Objects registered in global `GUObjectArray`
- Deletion only through `ObjectFactory::DeleteObject()`
- Cleanup via `ObjectFactory::DeleteAll()` at shutdown
- PIE duplicates objects with `Duplicate()` method

## Coordinate Space Reference

- **Local Space**: Relative to parent component
- **World Space**: Absolute position in world
- **View Space**: Relative to camera
- **Screen Space**: 2D viewport coordinates

Transform chain: Local → World → View → Projection → Screen
