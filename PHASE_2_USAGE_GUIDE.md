# Phase 2 Usage Guide: AMasterRoom and UDebugHelpers

## Overview

Phase 2 implements the core room generation functionality through the AMasterRoom actor and UDebugHelpers component. This guide shows how to use these components in Unreal Engine.

## AMasterRoom - Room Generation Actor

### Quick Start

1. **Add AMasterRoom to your level:**
   - Drag AMasterRoom from the Content Browser into your level
   - Or use Blueprint: `Spawn Actor of Class -> MasterRoom`

2. **Configure the room:**
   - Set `RoomData` to reference your URoomData asset
   - Choose `bUseRandomSeed` or set a specific `GenerationSeed`
   - Optionally set `bUseShapeOverride` and configure `ShapeOverride`

3. **Generate the room:**
   - In editor: Call `GenerateRoom()` from the Details panel
   - At runtime: Call `GenerateRoom()` via Blueprint or C++
   - In Blueprint: `Get Master Room -> Generate Room`

### Core Properties

#### Generation Settings
- **RoomData**: Reference to URoomData asset that defines room configuration
- **bUseRandomSeed**: If true, generates new random seed each generation
- **GenerationSeed**: Fixed seed value (only used when bUseRandomSeed is false)
- **bUseShapeOverride**: Use custom shape instead of RoomData's shapes
- **ShapeOverride**: Custom room shape definition

#### Runtime State (Read-Only)
- **bIsGenerated**: True after successful generation
- **RuntimeGrid**: Current grid cell data (TMap<FIntPoint, FGridCell>)
- **Doorway Snap Points**: Arrays for each cardinal direction

### Forced Placement Workflow

Forced placements allow designers to place specific meshes at exact grid positions. They are applied before random generation and reject overlaps.

#### Setting Up Forced Placements

1. **Add to ForcedFloorPlacements map:**
   ```cpp
   FIntPoint BottomLeftCell(2, 3); // Grid coordinate
   FMeshPlacementData PlacementData;
   PlacementData.Mesh = MyStaticMesh;
   PlacementData.CellsX = 2;
   PlacementData.CellsY = 2;
   PlacementData.PivotType = EMeshPivotType::CenterXY;
   
   MasterRoom->ForcedFloorPlacements.Add(BottomLeftCell, PlacementData);
   ```

2. **Key points:**
   - Map key is the **bottom-left grid cell** of the mesh footprint
   - Footprint spans from (X, Y) to (X + CellsX - 1, Y + CellsY - 1)
   - Overlapping forced placements are rejected with warning logs
   - Forced placements take priority over random generation

3. **Use cases:**
   - Place specific treasure room features
   - Add unique pillars or statues
   - Create custom room layouts
   - Designer-controlled variation

### Blueprint API

All major functions are Blueprint-callable:

- **GenerateRoom()**: Main generation function
- **CleanupRoom()**: Removes all generated meshes
- **RefreshDebugVisualization()**: Update debug drawing
- **IsRoomGenerated()**: Check generation state
- **GetCurrentSeed()**: Get active seed value
- **GetGridCellCount()**: Get number of grid cells

### Multi-Cell Mesh Placement

The system supports meshes that span multiple grid cells:

1. **Set CellsX and CellsY in FMeshPlacementData:**
   ```cpp
   PlacementData.CellsX = 2; // Width in cells
   PlacementData.CellsY = 2; // Height in cells
   ```

2. **Weighted random selection:**
   - Larger footprints (4x4) are tried first
   - Falls back to smaller footprints (2x2, then 1x1)
   - Uses SelectionWeight for probability

3. **Footprint validation:**
   - System checks if all cells are available
   - Rejects placement if any cell is occupied/reserved
   - Marks all footprint cells as occupied after placement

### Room Shapes

Supports 5 shape types:

1. **Rectangle**: Simple WxH grid
2. **L-Shape**: Main section + one extension
3. **T-Shape**: Main section + two opposite extensions
4. **U-Shape**: Three-sided room
5. **Custom**: User-defined cell array

Set via RoomData's AllowedShapes or ShapeOverride.

### Component Organization

Generated meshes are organized in scene components:

- **FloorContainer**: All floor tiles
- **WallContainer**: All wall segments
- **DoorContainer**: All door/doorway meshes
- **CeilingContainer**: All ceiling tiles

This allows easy management (hide/show, enable/disable collision, etc.).

## UDebugHelpers - Visualization Component

### Enabling Debug Visualization

1. **In the Details panel:**
   - Find the `DebugHelpers` component
   - Enable `bEnableDebugDraw`
   - Configure which elements to draw (Grid, Walls, etc.)

2. **Via Blueprint:**
   ```
   Get Debug Helpers -> Set Enable Debug Draw (true)
   Get Debug Helpers -> Update Debug Visualization
   ```

3. **Toggle at runtime:**
   ```
   Get Debug Helpers -> Toggle Debug Draw
   ```

### Visualization Options

#### What to Draw
- **bDrawGrid**: Grid cell outlines
- **bDrawCellStates**: Color-coded cell occupancy
- **bDrawWalls**: Wall edge lines
- **bDrawDoorways**: Doorway markers
- **bDrawForcedPlacements**: Forced placement highlights

#### Appearance
- **DebugLineThickness**: Line width (default: 2.0)
- **DebugDrawDuration**: How long to draw (-1 = persistent)
- **Colors**: Customize per element type
  - UnoccupiedCellColor (Green)
  - OccupiedCellColor (Red)
  - ReservedCellColor (Yellow)
  - ExcludedCellColor (Black)
  - WallColor (Blue)
  - DoorwayColor (Cyan)
  - ForcedPlacementColor (Magenta)

### Debugging Tips

1. **Grid alignment issues:**
   - Enable bDrawGrid and bDrawCellStates
   - Check if meshes align to cell centers

2. **Wall placement problems:**
   - Enable bDrawWalls
   - Verify walls appear on room boundaries

3. **Forced placement conflicts:**
   - Enable bDrawForcedPlacements
   - Check magenta boxes for overlap areas
   - Look for warning logs about rejected placements

4. **Footprint issues:**
   - Enable bDrawCellStates
   - Red = occupied, should match your mesh footprints

## Example Workflow

### Setting Up a Simple Room

1. Create a URoomData asset with:
   - FloorData, WallData, CeilingData references
   - GridConfig: CellSize = 100.0
   - AllowedShapes: Add a Rectangle shape (5x5)

2. Place AMasterRoom in level

3. Set RoomData reference

4. Enable debug visualization

5. Call GenerateRoom()

6. Observe the generated room with debug overlay

### Adding Forced Placements

1. Identify the grid coordinate for placement
   - Use debug grid to see coordinates
   - Bottom-left cell = anchor point

2. Add to ForcedFloorPlacements in editor or Blueprint

3. Regenerate room (call CleanupRoom then GenerateRoom)

4. Verify placement with debug visualization

## Performance Notes

- Debug visualization is manual (not automatic per frame)
- Call RefreshDebugVisualization() only when needed
- Large grids may have many debug draw calls
- Consider disabling debug draw in shipping builds

## Troubleshooting

### Room doesn't generate
- Check RoomData is set and valid
- Verify FloorData/WallData/CeilingData references
- Check log for error messages
- Ensure AllowedShapes has at least one shape

### Meshes not appearing
- Verify mesh references in Data Assets are valid
- Check mesh pivot points match EMeshPivotType
- Enable debug visualization to see grid state

### Forced placements rejected
- Check for overlap with other forced placements
- Verify grid coordinates are within room bounds
- Look for warning logs with rejection reason

### Debug visualization not showing
- Enable bEnableDebugDraw
- Call UpdateDebugVisualization() or RefreshDebugVisualization()
- Check that debug draw flags are enabled (bDrawGrid, etc.)
- Verify DebugDrawDuration is not 0

## Next Steps

After successful Phase 2 generation:
- Phase 3 will add doorway connections between rooms
- Phase 4 will add multi-floor support
- Phase 5 will add editor tools for visual editing

## Support

For issues or questions:
- Check console logs for detailed error messages
- Enable full debug visualization to diagnose problems
- Verify all Data Asset references are valid
- Review the main DUNGEON_GENERATOR_README.md for architecture details
