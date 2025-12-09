# Modular Dungeon Generator - Architecture Documentation

## Table of Contents
1. [Design Philosophy](#design-philosophy)
2. [Architecture Overview](#architecture-overview)
3. [Core Type Definitions](#core-type-definitions)
4. [Data Asset System](#data-asset-system)
5. [Pivot Point Conventions](#pivot-point-conventions)
6. [Usage Guide](#usage-guide)
7. [Future Phases Roadmap](#future-phases-roadmap)
8. [Glossary](#glossary)

---

## Design Philosophy

The Modular Dungeon Generator is built on the following core principles:

### 1. **Compositional Architecture**
Rather than monolithic room definitions, rooms are composed from separate, reusable asset packs:
- **Floor Data**: Tile meshes and materials for room floors
- **Wall Data**: Segments, corners, and doorway frames
- **Door Data**: Interactive doorways and door meshes
- **Ceiling Data**: Ceiling tiles and height configurations
- **Room Data**: Orchestrates all components into a complete room definition

**Benefit**: Artists can create asset packs independently, and designers can mix-and-match components to create variety without duplicating assets.

### 2. **Grid-Based Approach**
All dungeon elements align to a uniform grid system:
- Consistent cell size (default 100 units = 1 meter)
- Predictable world-space positioning
- Simplified collision detection and pathfinding
- Easy integration with AI navigation systems

**Benefit**: Ensures reliable generation, predictable gameplay, and simplified debugging.

### 3. **Data-Driven Generation**
All configuration resides in Data Assets, not code:
- No recompilation needed for content changes
- Blueprint-friendly workflow
- Non-programmers can create and modify dungeons
- Version control friendly (text-based assets)

**Benefit**: Rapid iteration and designer empowerment.

### 4. **Seed-Based Reproducibility**
Complete dungeon state can be saved and loaded via seed data:
- Multiplayer synchronization support
- Bug reproduction and testing
- Player-shareable dungeon codes
- Save/load system foundation

**Benefit**: Enables multiplayer, testing, and advanced features.

---

## Architecture Overview

### High-Level System Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    DungeonManager (Phase 2)                  │
│  - Grid management                                           │
│  - Room placement logic                                      │
│  - Doorway connection                                        │
└──────────────┬──────────────────────────────────────────────┘
               │
               │ References
               ▼
┌──────────────────────────────────────────────────────────────┐
│                        RoomData                               │
│  Identity: Name, Description                                  │
│  Grid Config: Cell size, shape, seed                          │
│  Shape Definitions: Rectangle, L, T, U, Custom                │
│  Dimensions: Min/Max size constraints                         │
│                                                               │
│  Compositional References:                                    │
│  ├─► FloorData                                               │
│  ├─► WallData                                                │
│  ├─► DoorData                                                │
│  └─► CeilingData                                             │
│                                                               │
│  Generation Settings: Doorways, weight, entry/exit flags     │
└──────────────────────────────────────────────────────────────┘
               │
               │ Composes
               ▼
┌────────────────────────────────────────────────────────┐
│  FloorData     │  WallData     │  DoorData    │  CeilingData │
│  - Asset pack  │  - Asset pack │  - Asset pack│  - Asset pack│
│  - Floor tiles │  - Segments   │  - Doorways  │  - Tiles     │
│  - Materials   │  - Corners    │  - Doors     │  - Height    │
│                │  - Frames     │  - Actors    │  - Materials │
│                │  - Materials  │  - Materials │              │
└────────────────────────────────────────────────────────┘
               │
               │ Uses
               ▼
┌──────────────────────────────────────────────────────────────┐
│                      Core Type System                         │
│                                                               │
│  GridTypes.h:                                                 │
│  - EMeshPivotType: CenterXY, BottomBackCenter, etc.          │
│  - ECellState: Unoccupied, Occupied, Reserved, Excluded      │
│  - EWallDirection: North(+Y), East(+X), South(-Y), West(-X)  │
│  - FGridCell: Coordinates, state, walls, doorways            │
│  - FGridConfiguration: Cell size, shape, seed                │
│  - FMeshPlacementData: Mesh, pivot, footprint, weight        │
│                                                               │
│  RoomShapeTypes.h:                                            │
│  - ERoomShape: Rectangle, LShape, TShape, UShape, Custom     │
│  - FRoomShapeDefinition: Shape type, dimensions, layout      │
│  - FShapeTemplate: Main/extension sections, attach points    │
│                                                               │
│  DungeonSeedData.h:                                           │
│  - FRoomSeedData: Room seed, location, rotation              │
│  - FFloorSeedData: Floor index, room/hallway/doorway seeds   │
│  - FDungeonSeedData: Master seed, timestamp, version         │
└──────────────────────────────────────────────────────────────┘
```

### Data Flow

```
1. Designer creates asset packs (Floor/Wall/Door/Ceiling Data)
                    ▼
2. Designer creates RoomData referencing asset packs
                    ▼
3. DungeonManager reads RoomData
                    ▼
4. Grid system allocates cells based on shape definition
                    ▼
5. Mesh placement system spawns actors from referenced assets
                    ▼
6. Materials applied based on data asset configuration
                    ▼
7. Seed data saved for reproduction
```

---

## Core Type Definitions

### GridTypes.h

#### **EMeshPivotType**
```cpp
enum class EMeshPivotType : uint8
{
    CenterXY,           // Centered on X and Y (floor, ceiling)
    BottomBackCenter,   // X center, Y back, Z bottom (walls, doorways)
    BottomCenter,       // X and Y center, Z bottom
    Custom              // User-defined offset
};
```

**Purpose**: Ensures proper mesh alignment when placing dungeon components. Different component types require different pivot conventions for correct positioning.

**Reasoning**: 
- **CenterXY**: Floor and ceiling tiles naturally align to cell centers
- **BottomBackCenter**: Walls align to cell edges (back = toward interior)
- **Custom**: Allows special cases without breaking convention

---

#### **ECellState**
```cpp
enum class ECellState : uint8
{
    Unoccupied,  // Empty, available for placement
    Occupied,    // Contains a room/structure
    Reserved,    // Set aside for future use (hallway planning)
    Excluded     // Blocked (obstacles, predefined areas)
};
```

**Purpose**: Track grid cell availability during generation.

**Reasoning**: 
- **Unoccupied**: Starting state, ready for room placement
- **Occupied**: Prevents overlap, used for collision detection
- **Reserved**: Enables multi-pass generation (place rooms, then hallways)
- **Excluded**: Supports designer-placed obstacles or pre-existing level geometry

---

#### **EWallDirection**
```cpp
enum class EWallDirection : uint8
{
    North,  // +Y direction
    East,   // +X direction
    South,  // -Y direction
    West    // -X direction
};
```

**Purpose**: Standardize wall and doorway orientation in Unreal's coordinate system.

**Reasoning**: 
- Matches Unreal Engine's coordinate conventions (+X = East, +Y = North)
- Enables directional wall/doorway placement logic
- Facilitates neighbor checking (which cells are adjacent in which direction)

---

#### **FGridCell**
```cpp
struct FGridCell
{
    FIntPoint GridCoordinates;      // (X, Y) grid position
    ECellState CellState;           // Occupancy state
    FVector WorldPosition;          // 3D world location
    
    // Wall flags (4 edges per cell)
    bool bHasNorthWall, bHasEastWall, bHasSouthWall, bHasWestWall;
    
    // Doorway flags (4 edges per cell)
    bool bHasNorthDoorway, bHasEastDoorway, bHasSouthDoorway, bHasWestDoorway;
    
    TWeakObjectPtr<AActor> OccupyingActor;  // Reference to placed actor
};
```

**Purpose**: Complete representation of a single grid cell.

**Reasoning**:
- **GridCoordinates**: Logical 2D position in grid space
- **WorldPosition**: Cached world location (avoids recalculation)
- **Wall/Doorway Flags**: Each cell knows its edge configuration (shared edges handled by checking neighbors)
- **OccupyingActor**: Enables lookup of what actor occupies this cell (TWeakObjectPtr prevents dangling references)

---

#### **FGridConfiguration**
```cpp
struct FGridConfiguration
{
    float CellSize;                    // World units per cell (default 100.0)
    FName ShapeDefinitionName;         // Reference to shape definition
    int32 GenerationSeed;              // Seed for random generation
    bool bUseRandomSeed;               // If true, ignore GenerationSeed
};
```

**Purpose**: Centralize grid settings for rooms.

**Reasoning**:
- **CellSize**: Allows different scale dungeons (small/large cells)
- **ShapeDefinitionName**: Decouples shape from room (one shape, many rooms)
- **Seed System**: Enables reproducible generation and testing

---

#### **FMeshPlacementData**
```cpp
struct FMeshPlacementData
{
    TSoftObjectPtr<UStaticMesh> Mesh;  // Async-loadable mesh reference
    EMeshPivotType PivotType;          // How to position mesh
    FVector CustomPivotOffset;         // Used if PivotType == Custom
    int32 CellsX, CellsY;              // Footprint size
    float SelectionWeight;             // Probability weight
    bool bAllowRotation;               // Can rotate 90°?
    bool bAllow180Rotation;            // Can rotate 180°?
};
```

**Purpose**: Self-contained mesh placement configuration.

**Reasoning**:
- **TSoftObjectPtr**: Enables async loading, reduces memory footprint
- **Pivot System**: Consistent positioning without manual offsets
- **Footprint**: Some tiles span multiple cells (2x1, 2x2, etc.)
- **SelectionWeight**: Artist control over variety distribution
- **Rotation Flags**: Some assets look good rotated, others don't

---

### RoomShapeTypes.h

#### **ERoomShape**
```cpp
enum class ERoomShape : uint8
{
    Rectangle,  // Standard box
    LShape,     // One extension
    TShape,     // Two extensions (opposite sides)
    UShape,     // Three extensions
    Custom      // User-defined cell array
};
```

**Purpose**: Define standard room silhouettes.

**Reasoning**:
- **Rectangle**: Most common, simplest to generate
- **L/T/U Shapes**: Adds visual variety, creates natural hallway connections
- **Custom**: Supports arbitrary shapes (artist-defined)

---

#### **FRoomShapeDefinition**
```cpp
struct FRoomShapeDefinition
{
    ERoomShape ShapeType;
    int32 RectWidth, RectHeight;               // For standard shapes
    TArray<int32> CustomCellLayout;            // For custom shapes (1=cell, 0=empty)
    int32 CustomLayoutWidth, CustomLayoutHeight;
};
```

**Purpose**: Complete shape specification.

**Reasoning**:
- Standard shapes use simple width/height
- Custom shapes use explicit cell array (1D array, indexed as [Y * Width + X])
- Both approaches stored in one struct for unified handling

---

#### **FShapeTemplate**
```cpp
struct FShapeTemplate
{
    int32 MainSectionWidth, MainSectionHeight;
    int32 ExtensionWidth, ExtensionHeight;
    int32 ExtensionAttachPoint;  // 0=start, 1=middle, 2=end
};
```

**Purpose**: Parameterize complex shapes without explicit arrays.

**Reasoning**: Enables procedural generation of L/T/U shapes with different proportions without manually defining cell arrays.

---

### DungeonSeedData.h

#### **FRoomSeedData**
```cpp
struct FRoomSeedData
{
    int32 RoomSeed;              // Room's generation seed
    FIntPoint Location;          // Grid position
    int32 Rotation;              // 0, 90, 180, 270
    FName RoomDataAssetName;     // Which RoomData was used
};
```

**Purpose**: Capture all data needed to recreate a specific room instance.

**Reasoning**: Minimal data set for exact reproduction.

---

#### **FFloorSeedData**
```cpp
struct FFloorSeedData
{
    int32 FloorIndex;                      // Which floor (multi-level support)
    int32 FloorSeed;                       // Floor's generation seed
    TArray<FRoomSeedData> RoomSeeds;       // All rooms on this floor
    TArray<FRoomSeedData> HallwaySeeds;    // All hallways (treated as rooms)
    TArray<FIntPoint> DoorwayPositions;    // All doorway locations
};
```

**Purpose**: Complete floor state.

**Reasoning**: Separates rooms/hallways for different generation logic.

---

#### **FDungeonSeedData**
```cpp
struct FDungeonSeedData
{
    int32 MasterSeed;                  // Top-level seed
    TArray<FFloorSeedData> FloorSeeds; // All floors
    FDateTime GenerationTimestamp;     // When created
    int32 SaveVersion;                 // For compatibility
};
```

**Purpose**: Complete dungeon state for save/load.

**Reasoning**: 
- Master seed enables single-value reproduction
- Timestamp for tracking/debugging
- SaveVersion for forward compatibility

---

## Data Asset System

### Asset Hierarchy

```
UDataAsset (Unreal Engine base class)
    │
    ├─► UFloorData
    ├─► UWallData
    ├─► UDoorData
    ├─► UCeilingData
    └─► URoomData (references above assets)
```

---

### FloorData

```cpp
class UFloorData : public UDataAsset
{
    FName AssetPackName;                                    // "DarkDungeon_Floor_Pack"
    TArray<FMeshPlacementData> FloorTiles;                  // All floor tile variants
    TSoftObjectPtr<UMaterialInterface> DefaultMaterial;     // Base material
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;  // Alt materials
    bool bRandomizeMaterials;                               // Apply variations?
};
```

**Usage**:
1. Create FloorData asset in Content Browser
2. Add floor tile meshes to FloorTiles array
3. Set selection weights for variety
4. Optionally add material variations for randomization

---

### WallData

```cpp
class UWallData : public UDataAsset
{
    FName AssetPackName;
    TArray<FMeshPlacementData> WallSegments;    // Straight walls
    TArray<FMeshPlacementData> InnerCorners;    // Concave corners
    TArray<FMeshPlacementData> OuterCorners;    // Convex corners
    TArray<FMeshPlacementData> DoorwayFrames;   // Archways/frames
    TSoftObjectPtr<UMaterialInterface> DefaultMaterial;
};
```

**Usage**:
1. Create WallData asset
2. Add meshes to appropriate arrays (segments, corners, frames)
3. System automatically chooses correct mesh type based on wall configuration

**Corner Detection Logic** (Phase 2):
- **Inner Corner**: Two adjacent walls meet, interior angle
- **Outer Corner**: Two adjacent walls meet, exterior angle
- Generator automatically selects from correct array

---

### DoorData

```cpp
class UDoorData : public UDataAsset
{
    FName AssetPackName;
    TArray<FMeshPlacementData> DoorwayMeshes;    // Open archways
    TArray<FMeshPlacementData> DoorMeshes;       // Actual doors
    TSubclassOf<AActor> DoorwayActorClass;       // Interactive actor
    FVector InteractionVolumeExtent;             // Trigger size
    TSoftObjectPtr<UMaterialInterface> DefaultMaterial;
};
```

**Usage**:
1. Create DoorData asset
2. Add doorway meshes (archways, no door)
3. Add door meshes (with door model)
4. Optionally specify DoorwayActorClass for interactivity (Phase 3)

---

### CeilingData

```cpp
class UCeilingData : public UDataAsset
{
    FName AssetPackName;
    TArray<FMeshPlacementData> CeilingTiles;
    float CeilingHeightOffset;                   // Height above floor (default 300.0)
    TSoftObjectPtr<UMaterialInterface> DefaultMaterial;
};
```

**Usage**:
1. Create CeilingData asset
2. Add ceiling tile meshes
3. Set height offset (standard 3 meters = 300 units)

---

### RoomData (Master Orchestrator)

```cpp
class URoomData : public UDataAsset
{
    // Identity
    FName RoomName;
    FText RoomDescription;
    
    // Grid
    FGridConfiguration GridConfig;
    
    // Shape
    TArray<FRoomShapeDefinition> AllowedShapes;
    FIntPoint MinDimensions;
    FIntPoint MaxDimensions;
    
    // Compositional References (THE KEY DESIGN PATTERN)
    TSoftObjectPtr<UFloorData> FloorData;
    TSoftObjectPtr<UWallData> WallData;
    TSoftObjectPtr<UDoorData> DoorData;
    TSoftObjectPtr<UCeilingData> CeilingData;
    
    // Generation
    int32 MinDoorways, MaxDoorways;
    float RoomSelectionWeight;
    bool bCanBeEntryRoom, bCanBeExitRoom;
};
```

**Usage**:
1. Create RoomData asset
2. Set identity (name, description)
3. Configure grid (cell size, seed)
4. Define allowed shapes (rectangle, L-shape, etc.)
5. **Reference existing Floor/Wall/Door/Ceiling Data assets**
6. Set generation parameters (doorway count, weight, flags)

**Why This Design**:
- **Reusability**: One FloorData asset used by many RoomData assets
- **Mix-and-Match**: Swap out WallData to reskin room without recreating
- **Artist Workflow**: Artists create asset packs, designers compose rooms
- **Memory Efficiency**: Assets loaded once, referenced by multiple rooms

---

## Pivot Point Conventions

### Overview

Proper pivot placement ensures meshes align correctly to the grid without manual offsets.

### Convention Table

| Asset Type       | Pivot Location                 | EMeshPivotType       | Reasoning                                      |
|------------------|--------------------------------|----------------------|------------------------------------------------|
| Floor Tiles      | Center X, Center Y, Bottom Z   | `CenterXY`           | Tiles placed at cell centers, sit on ground   |
| Ceiling Tiles    | Center X, Center Y, Bottom Z   | `CenterXY`           | Mirrors floor, offset upward by height         |
| Wall Segments    | Center X, Back Y, Bottom Z     | `BottomBackCenter`   | Back edge aligns to cell edge, centered on edge|
| Doorway Frames   | Center X, Back Y, Bottom Z     | `BottomBackCenter`   | Same as walls for consistent placement         |
| Inner Corners    | Custom (varies by geometry)    | `Custom`             | Complex geometry, artist-defined offset        |
| Outer Corners    | Custom (varies by geometry)    | `Custom`             | Complex geometry, artist-defined offset        |
| Door Meshes      | Center X, Back Y, Bottom Z     | `BottomBackCenter`   | Aligns with frame                              |

### Visual Diagram

```
Floor Tile (Top View):
    Grid Cell
    ┌─────────────┐
    │             │
    │      X      │  X = Pivot at cell center
    │             │
    └─────────────┘

Wall Segment (Side View):
         Wall
    ┌────────────┐
    │            │  
    │            │  X = Pivot at center of back edge, bottom
    └────X───────┘
         ↑
    (towards room interior)

Wall Segment (Top View):
    Room Interior
    ─────────────────
         ↑
         X ────────┐  X = Pivot at center of wall edge
         ↑        │
    (wall faces)  │  Wall
                  └
```

### Blender Export Guidelines

1. **Set Pivot Before Export**:
   - Select mesh
   - Set origin to desired pivot point
   - Export as FBX

2. **Floor/Ceiling Tiles**:
   - Origin: Object center (X/Y), bottom (Z)
   - Orientation: Flat on XY plane

3. **Walls/Doorways**:
   - Origin: Center of back edge (X), back edge (Y), bottom (Z)
   - Orientation: Facing +Y (toward room interior)

4. **Verification**:
   - Import to Unreal
   - Check pivot gizmo location
   - Adjust if needed using Pivot Painter or reimport

---

## Usage Guide

### Creating an Asset Pack (Artist Workflow)

#### Step 1: Create FloorData Asset
1. Content Browser → Right-click → Miscellaneous → Data Asset
2. Select `UFloorData` as parent class
3. Name: `DA_FloorPack_DarkDungeon`
4. Open asset:
   - Set AssetPackName: "DarkDungeon_Floor"
   - Add meshes to FloorTiles array:
     - Mesh: Select floor tile static mesh
     - PivotType: CenterXY
     - CellsX: 1, CellsY: 1
     - SelectionWeight: 1.0
   - Set DefaultMaterial: M_Floor_Dark
   - (Optional) Add MaterialVariations for randomization

#### Step 2: Create WallData Asset
1. Create UWallData asset: `DA_WallPack_DarkDungeon`
2. Configure:
   - AssetPackName: "DarkDungeon_Wall"
   - Add WallSegments (straight walls)
   - Add InnerCorners (concave corners)
   - Add OuterCorners (convex corners)
   - Add DoorwayFrames (archways)
   - Set DefaultMaterial

#### Step 3: Create DoorData Asset
1. Create UDoorData asset: `DA_DoorPack_DarkDungeon`
2. Configure:
   - AssetPackName: "DarkDungeon_Door"
   - Add DoorwayMeshes (open archways)
   - Add DoorMeshes (closed doors)
   - Set InteractionVolumeExtent: (100, 100, 250)

#### Step 4: Create CeilingData Asset
1. Create UCeilingData asset: `DA_CeilingPack_DarkDungeon`
2. Configure:
   - AssetPackName: "DarkDungeon_Ceiling"
   - Add CeilingTiles
   - CeilingHeightOffset: 300.0

---

### Creating a Room Definition (Designer Workflow)

#### Step 1: Create RoomData Asset
1. Create URoomData asset: `DA_Room_DarkTreasure`
2. Set Identity:
   - RoomName: "DarkTreasureRoom"
   - RoomDescription: "A dark treasure chamber with stone walls"

#### Step 2: Configure Grid
1. GridConfig:
   - CellSize: 100.0 (standard 1m cells)
   - GenerationSeed: 12345 (or check bUseRandomSeed)

#### Step 3: Define Allowed Shapes
1. Add entries to AllowedShapes array:
   - Shape 1: Rectangle, 5x5
   - Shape 2: LShape, 5x5 + 3x3 extension
2. Set dimensions:
   - MinDimensions: (3, 3)
   - MaxDimensions: (10, 10)

#### Step 4: Link Asset Packs (Composition)
1. Set compositional references:
   - FloorData: DA_FloorPack_DarkDungeon
   - WallData: DA_WallPack_DarkDungeon
   - DoorData: DA_DoorPack_DarkDungeon
   - CeilingData: DA_CeilingPack_DarkDungeon

#### Step 5: Configure Generation
1. Set generation parameters:
   - MinDoorways: 1
   - MaxDoorways: 3
   - RoomSelectionWeight: 1.0
   - bCanBeEntryRoom: false (treasure room not an entry)
   - bCanBeExitRoom: false

---

### Using Seed Data for Reproducibility

#### Saving Seed Data
```cpp
// Generate dungeon
DungeonManager->GenerateDungeon();

// Extract seed data
FDungeonSeedData SeedData = DungeonManager->GetSeedData();

// Save to file or replicate over network
SaveSeedDataToFile(SeedData);
```

#### Loading Seed Data
```cpp
// Load seed data
FDungeonSeedData SeedData = LoadSeedDataFromFile();

// Regenerate dungeon from seed
DungeonManager->GenerateFromSeedData(SeedData);

// Result: Identical dungeon layout
```

**Use Cases**:
- Multiplayer synchronization (host generates, sends seed to clients)
- Bug reporting (player shares seed code)
- Speedrunning (fixed seed for fair competition)
- Daily challenges (same seed for all players)

---

## Future Phases Roadmap

### Phase 2: MasterRoom Editor and Runtime Functionality ✅ COMPLETED
**Scope**: Implement room generation and editor visualization

**Components Implemented**:
1. **AMasterRoom Actor**:
   - Complete room generation system with grid initialization
   - EditAnywhere properties for designer configuration
   - CallInEditor functions for in-editor workflow
   - Forced placement system with conflict detection
   - Weighted multi-cell random placement algorithm
   - Proper pivot point calculations for mesh placement
   - Scene component hierarchy (Floor, Wall, Ceiling, Doorway containers)

2. **Grid Management**:
   - Dynamic grid allocation based on room shape
   - Cell state tracking (Unoccupied, Occupied, Reserved, Excluded)
   - Support for Rectangle and Custom shapes
   - Grid-to-world coordinate conversion
   - Multi-cell footprint validation

3. **Forced Placement System**:
   - Designer-configurable forced mesh placements
   - Bottom-left cell coordinate mapping
   - Overlap detection with warning logging
   - Rejection of conflicting placements (no auto-resolve)
   - Support for floor, wall, ceiling, and doorway forced placements

4. **Generation Algorithm**:
   - Two-pass floor tile generation:
     - Pass 1: Weighted multi-cell placement (largest to smallest)
     - Pass 2: Single-cell fill for remaining spaces
   - Deterministic random generation via FRandomStream
   - Respects forced placement cell reservations

5. **Debug Visualization (UDebugHelpers)**:
   - DrawGrid() - Green grid lines showing cell boundaries
   - DrawOccupiedCells() - Red highlighting of occupied cells
   - DrawUnoccupiedCells() - Blue highlighting of available cells
   - DrawForcedPlacements() - Yellow markers for forced mesh placements
   - Configurable visualization settings (toggles, line thickness, z-offset)

6. **CallInEditor Functions**:
   - GenerateRoom() - Generate room from current configuration
   - ClearRoom() - Remove all spawned meshes and reset state
   - RegenerateRoom() - Clear and regenerate in one action
   - UpdateDebugVisualization() - Refresh debug drawings

**Deliverables**:
- ✅ `AMasterRoom` actor with full generation logic
- ✅ Grid management system
- ✅ Forced placement algorithms with conflict detection
- ✅ Weighted multi-cell mesh spawning system
- ✅ Editor debug visualization tools
- ✅ CallInEditor workflow for designers

**Key Design Decisions**:
- FIntPoint keys in forced placement maps = bottom-left cell coordinate
- Overlapping forced placements are rejected and logged
- Random generation avoids cells occupied by forced placements
- GridConfig.CellSize drives all placement offset calculations
- Multi-cell placement uses weighted selection from largest to smallest
- Single-cell tiles fill remaining unoccupied spaces

**Usage Example**:
```cpp
// In Unreal Editor:
1. Place AMasterRoom actor in level
2. Assign RoomData asset in Details panel
3. (Optional) Configure forced placements in Details panel
4. Click "Generate Room" button to create the room
5. View debug visualization to inspect grid and cell states
6. Click "Clear Room" to remove generation
7. Adjust settings and click "Regenerate Room" to iterate
```

**Future Enhancements**:
- Wall/Corner detection and placement (placeholder implemented)
- Full L/T/U shape support (currently uses rectangles)
- Doorway snap point population
- Performance optimizations (coordinate mapping, bounds caching)

---

### Phase 3: Interactivity & Polish
**Scope**: Add gameplay features and visual polish

**Components**:
1. **Interactive Doors**:
   - `ADungeonDoorway` actor (referenced in DoorData)
   - Door opening/closing logic
   - Trigger volumes
   - Locked door system

2. **Room Content Spawning**:
   - Enemy spawn points
   - Loot placement
   - Furniture/props
   - Lighting fixtures

3. **Minimap System**:
   - Use FGridCell data to generate minimap
   - Fog-of-war (reveal as player explores)
   - Room icons (entry, exit, treasure, etc.)

4. **Visual Effects**:
   - Particle systems (torches, fog)
   - Dynamic lighting
   - Ambient sounds per room type

**Deliverables**:
- Interactive door system
- Content spawning framework
- Minimap/UI system
- VFX/SFX integration

---

### Phase 4: Advanced Features
**Scope**: Complex systems and optimization

**Components**:
1. **Multi-Floor Dungeons**:
   - Vertical connections (stairs, elevators)
   - FFloorSeedData utilization
   - Z-axis grid management

2. **Streaming & LOD**:
   - Room streaming (load/unload based on player position)
   - Mesh LODs for distant rooms
   - Occlusion culling

3. **Procedural Variations**:
   - Room modifiers (flooded, collapsed, overgrown)
   - Dynamic obstacles
   - Trap systems

4. **AI Integration**:
   - NavMesh generation from grid
   - Pathfinding integration
   - Enemy patrol routes

**Deliverables**:
- Multi-floor system
- Streaming infrastructure
- Procedural modifiers
- AI navigation integration

---

### Phase 5: Editor Tools
**Scope**: Designer-friendly Unreal Editor tools

**Components**:
1. **Dungeon Editor Window**:
   - Custom editor panel for dungeon configuration
   - Visual grid editor (paint rooms, preview layout)
   - Shape designer (custom room shape tool)

2. **Asset Pack Validator**:
   - Checks pivot points are correct
   - Validates mesh footprints
   - Reports missing materials

3. **Debug Visualization**:
   - In-editor grid overlay
   - Color-coded cell states
   - Wall/doorway gizmos

4. **Batch Operations**:
   - Bulk asset pack creation
   - Room template generation
   - Material override system

**Deliverables**:
- Custom Editor Mode
- Asset validation tools
- Debug visualization system
- Batch processing utilities

---

## Glossary

### Core Concepts

**Asset Pack**: A collection of related meshes and materials (e.g., "DarkDungeon_Floor" contains all floor tile variants for the dark dungeon theme).

**Cell**: A single grid square in the dungeon grid. Standard size is 100 units (1 meter).

**Cell State**: Occupancy status of a cell (Unoccupied, Occupied, Reserved, Excluded).

**Ceiling Height Offset**: Distance from floor level to ceiling placement (default 300 units = 3 meters).

**Compositional Reference**: A soft reference from RoomData to Floor/Wall/Door/CeilingData assets. Enables mix-and-match asset composition.

**Custom Pivot Offset**: User-defined offset vector for mesh pivot, used when EMeshPivotType is set to Custom.

**Doorway**: An opening in a room's wall that connects to another room or hallway. Tracked via bHasNorthDoorway, etc. flags on FGridCell.

**Extension**: Secondary section of a complex room shape (the "arms" of L/T/U shapes).

**Footprint**: The number of grid cells a mesh occupies (CellsX by CellsY).

**Grid Configuration**: Settings for a room's grid system (cell size, shape, seed).

**Grid Coordinates**: Logical 2D position in grid space (e.g., (5, 3) = 5th column, 3rd row).

**Hallway**: A narrow connecting space between rooms. Treated as a special type of room with restricted shapes.

**Master Seed**: Top-level seed value that determines all dungeon generation randomness. Sub-seeds derived from this.

**Pivot Point**: Reference point for mesh positioning. Different mesh types use different pivot conventions (CenterXY, BottomBackCenter, etc.).

**Room Shape**: Geometric outline of a room (Rectangle, LShape, TShape, UShape, Custom).

**Room Shape Definition**: Complete specification of a room's shape, including type and dimensions.

**Seed Data**: Collection of seeds and parameters that enable exact reproduction of a dungeon layout.

**Selection Weight**: Probability multiplier for random selection. Higher weight = more likely to be chosen.

**Shape Template**: Parameterized definition for complex shapes (L/T/U), specifying main and extension sections.

**Soft Object Pointer**: Unreal asset reference that loads on-demand (TSoftObjectPtr). Reduces memory footprint.

**Wall Direction**: Cardinal direction of a wall edge (North/+Y, East/+X, South/-Y, West/-X).

**World Position**: 3D coordinates in Unreal world space (e.g., (500.0, 300.0, 0.0)).

---

### Unreal Engine Specific Terms

**Data Asset**: Unreal's base class for designer-configurable data containers (UDataAsset).

**GENERATED_BODY()**: Macro required in all UCLASS/USTRUCT definitions. Enables reflection system.

**UPROPERTY()**: Macro that exposes variables to Unreal's reflection system, Blueprint, and editor.

**TArray**: Unreal's dynamic array container (similar to std::vector).

**TWeakObjectPtr**: Smart pointer that doesn't prevent garbage collection. Automatically nulls when object destroyed.

**TSoftObjectPtr**: Lazy-loading asset reference. Asset not loaded until explicitly accessed.

**BlueprintType**: Specifier that makes a type accessible in Blueprint visual scripting.

**EditAnywhere**: Specifier that makes a property editable in all editor contexts.

**BlueprintReadWrite**: Specifier that allows Blueprint read/write access to a property.

**meta = (ClampMin, ClampMax)**: Metadata that restricts numeric values in the editor UI.

**meta = (EditCondition)**: Metadata that conditionally shows/hides properties based on other property values.

---

### Generation Terms

**Attach Point**: Position where an extension connects to a main section (0=start, 1=middle, 2=end).

**Entry Room**: Room designated as dungeon starting point (bCanBeEntryRoom = true).

**Exit Room**: Room designated as dungeon exit/boss room (bCanBeExitRoom = true).

**Inner Corner**: Concave corner where two walls meet (interior angle).

**Outer Corner**: Convex corner where two walls meet (exterior angle).

**Reserved Cell**: Grid cell set aside for future use (e.g., hallway will be placed here).

**Room Selection Weight**: Probability weight for choosing this room type during generation.

---

## Questions & Troubleshooting

### Common Issues

**Q: Meshes are floating/buried when spawned**
A: Check pivot point. Floor tiles should use CenterXY, walls should use BottomBackCenter. Verify pivot in Blender before export.

**Q: Walls not aligning to grid**
A: Ensure CellSize matches your mesh dimensions. Default 100 units = 1 meter. Walls should be exactly CellSize units long.

**Q: Room generation overlaps existing rooms**
A: Check cell state tracking. Ensure cells are set to Occupied after placement. Verify collision detection in placement algorithm (Phase 2).

**Q: Materials not applying to meshes**
A: Verify DefaultMaterial is set in Data Asset. Check material slot names match between mesh and material assignment.

**Q: Custom room shape not generating correctly**
A: Verify CustomCellLayout array length = CustomLayoutWidth × CustomLayoutHeight. Check array is 1D (indexed as [Y * Width + X]).

**Q: Forced placement rejected with overlap warning (Phase 2)**
A: Check that forced placement coordinates don't conflict with other forced placements or existing occupied cells. The bottom-left coordinate (key) plus footprint size must fit within unoccupied cells. Forced placements are rejected if they overlap - there is no auto-resolve.

**Q: Debug visualization not appearing in editor (Phase 2)**
A: Ensure DebugHelper component's bShowDebugVisualization is enabled and specific visualization flags (bDrawGrid, bDrawOccupiedCells, etc.) are toggled on. Click "Update Debug Visualization" button after generating the room.

**Q: Floor tiles not filling all cells (Phase 2)**
A: Check that FloorData has both multi-cell and single-cell (1x1) tiles defined. The generator places large tiles first, then fills gaps with 1x1 tiles. If only large tiles exist, small gaps will remain unfilled.

**Q: GenerateRoom button does nothing (Phase 2)**
A: Verify RoomData asset is assigned in the Details panel. Check Output Log for error messages. Ensure RoomData has valid FloorData, GridConfig, and AllowedShapes configured.

---

## Credits & Contribution

**Architecture Design**: Phase 1 specification defines foundational systems.

**Future Contributors**: When implementing Phases 2-5, please maintain the architectural principles:
1. Keep data in Data Assets, not code
2. Respect compositional architecture (don't merge asset packs)
3. Preserve grid-based alignment
4. Maintain seed-based reproducibility

---

## Revision History

- **Version 1.0** (Phase 1): Initial type system and data asset architecture
- **Version 2.0** (Phase 2): ✅ MasterRoom editor and runtime functionality
  - AMasterRoom actor with grid initialization and generation
  - Forced placement system with conflict detection
  - Weighted multi-cell random placement algorithm
  - Debug visualization tools (grid, occupied/unoccupied cells, forced placements)
  - CallInEditor workflow for designers
- **Version 3.0** (Phase 3): [Future] Interactivity & polish
- **Version 4.0** (Phase 4): [Future] Advanced features
- **Version 5.0** (Phase 5): [Future] Editor tools

---

**End of Documentation**
