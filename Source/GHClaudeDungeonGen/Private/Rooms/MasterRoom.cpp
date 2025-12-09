// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/MasterRoom.h"
#include "Debugging/DebugHelpers.h"
#include "Data/Room/RoomData.h"
#include "Data/Room/FloorData.h"
#include "Data/Room/WallData.h"
#include "Data/Room/CeilingData.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AMasterRoom::AMasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// Create container components for organization
	FloorContainer = CreateDefaultSubobject<USceneComponent>(TEXT("FloorContainer"));
	FloorContainer->SetupAttachment(RootSceneComponent);

	WallContainer = CreateDefaultSubobject<USceneComponent>(TEXT("WallContainer"));
	WallContainer->SetupAttachment(RootSceneComponent);

	DoorContainer = CreateDefaultSubobject<USceneComponent>(TEXT("DoorContainer"));
	DoorContainer->SetupAttachment(RootSceneComponent);

	CeilingContainer = CreateDefaultSubobject<USceneComponent>(TEXT("CeilingContainer"));
	CeilingContainer->SetupAttachment(RootSceneComponent);

	// Create debug helpers component
	DebugHelpers = CreateDefaultSubobject<UDebugHelpers>(TEXT("DebugHelpers"));

	// Initialize properties
	bUseRandomSeed = true;
	GenerationSeed = 0;
	bUseShapeOverride = false;
	bIsGenerated = false;
}

void AMasterRoom::GenerateRoom()
{
	// Cleanup any existing generation
	CleanupRoom();

	// Validate RoomData
	if (!RoomData.IsValid() || RoomData.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::GenerateRoom - RoomData is null or invalid!"));
		return;
	}

	// Load RoomData if not already loaded
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData)
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::GenerateRoom - Failed to load RoomData!"));
		return;
	}

	// Initialize random stream
	if (bUseRandomSeed)
	{
		GenerationSeed = FMath::Rand();
	}
	RandomStream.Initialize(GenerationSeed);

	// Select shape definition
	FRoomShapeDefinition SelectedShape;
	if (bUseShapeOverride)
	{
		SelectedShape = ShapeOverride;
	}
	else if (LoadedRoomData->AllowedShapes.Num() > 0)
	{
		// Pick a random allowed shape
		int32 ShapeIndex = RandomStream.RandRange(0, LoadedRoomData->AllowedShapes.Num() - 1);
		SelectedShape = LoadedRoomData->AllowedShapes[ShapeIndex];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::GenerateRoom - No allowed shapes defined in RoomData!"));
		return;
	}

	// Initialize grid based on selected shape
	InitializeGrid(SelectedShape);

	// Apply forced placements first (they take priority)
	if (!ApplyForcedPlacements())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateRoom - Some forced placements were rejected due to overlaps"));
	}

	// Generate room components
	GenerateFloor();
	GenerateWalls();
	GenerateCeiling();

	bIsGenerated = true;
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::GenerateRoom - Room generation completed successfully"));
}

void AMasterRoom::CleanupRoom()
{
	// Destroy all child components in containers
	TArray<USceneComponent*> ContainersToClean = {FloorContainer, WallContainer, DoorContainer, CeilingContainer};
	
	for (USceneComponent* Container : ContainersToClean)
	{
		if (Container)
		{
			TArray<USceneComponent*> Children;
			Container->GetChildrenComponents(false, Children);
			
			for (USceneComponent* Child : Children)
			{
				if (Child)
				{
					Child->DestroyComponent();
				}
			}
		}
	}

	// Clear runtime grid
	RuntimeGrid.Empty();

	// Clear doorway snap points
	NorthDoorwaySnapPoints.Empty();
	EastDoorwaySnapPoints.Empty();
	SouthDoorwaySnapPoints.Empty();
	WestDoorwaySnapPoints.Empty();

	bIsGenerated = false;
}

void AMasterRoom::GenerateFloor()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->FloorData.IsValid())
	{
		return;
	}

	UFloorData* FloorDataAsset = LoadedRoomData->FloorData.LoadSynchronous();
	if (!FloorDataAsset || FloorDataAsset->FloorTiles.Num() == 0)
	{
		return;
	}

	// Sort floor tiles by footprint size (largest first) for weighted multi-cell placement
	TArray<FMeshPlacementData> SortedTiles = FloorDataAsset->FloorTiles;
	SortedTiles.Sort([](const FMeshPlacementData& A, const FMeshPlacementData& B) {
		int32 AreaA = A.CellsX * A.CellsY;
		int32 AreaB = B.CellsX * B.CellsY;
		return AreaA > AreaB; // Largest first
	});

	// Attempt multi-cell placement first (descending size order)
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		// Skip if cell is already occupied or reserved
		if (Cell.CellState != ECellState::Unoccupied)
		{
			continue;
		}

		// Try to place largest possible tile that fits
		bool bPlaced = false;
		for (const FMeshPlacementData& TileData : SortedTiles)
		{
			// Skip single-cell tiles in multi-cell pass
			if (TileData.CellsX == 1 && TileData.CellsY == 1)
			{
				continue;
			}

			if (TryPlaceMultiCellMesh(Cell.GridCoordinates, TileData, FloorContainer))
			{
				bPlaced = true;
				break;
			}
		}
	}

	// Fill remaining single cells
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		if (Cell.CellState != ECellState::Unoccupied)
		{
			continue;
		}

		// Select a random single-cell tile
		TArray<FMeshPlacementData> SingleCellTiles;
		for (const FMeshPlacementData& TileData : FloorDataAsset->FloorTiles)
		{
			if (TileData.CellsX == 1 && TileData.CellsY == 1)
			{
				SingleCellTiles.Add(TileData);
			}
		}

		if (SingleCellTiles.Num() > 0)
		{
			// Weight-based selection
			float TotalWeight = 0.0f;
			for (const FMeshPlacementData& Tile : SingleCellTiles)
			{
				TotalWeight += Tile.SelectionWeight;
			}

			float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
			float AccumulatedWeight = 0.0f;

			for (const FMeshPlacementData& TileData : SingleCellTiles)
			{
				AccumulatedWeight += TileData.SelectionWeight;
				if (RandomValue <= AccumulatedWeight)
				{
					TryPlaceMultiCellMesh(Cell.GridCoordinates, TileData, FloorContainer);
					break;
				}
			}
		}
	}
}

void AMasterRoom::GenerateWalls()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->WallData.IsValid())
	{
		return;
	}

	UWallData* WallDataAsset = LoadedRoomData->WallData.LoadSynchronous();
	if (!WallDataAsset || WallDataAsset->WallSegments.Num() == 0)
	{
		return;
	}

	// For Phase 2, we'll place basic wall segments on cell edges
	// More sophisticated corner detection will come in later iterations
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		// Process each wall direction
		if (Cell.bHasNorthWall && !Cell.bHasNorthDoorway)
		{
			// Place north wall (for now, just use first wall segment)
			if (WallDataAsset->WallSegments.Num() > 0)
			{
				const FMeshPlacementData& WallData = WallDataAsset->WallSegments[0];
				// Wall placement logic will be refined in future iterations
			}
		}
		
		// Similar logic for East, South, West walls...
	}
}

void AMasterRoom::GenerateCeiling()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->CeilingData.IsValid())
	{
		return;
	}

	UCeilingData* CeilingDataAsset = LoadedRoomData->CeilingData.LoadSynchronous();
	if (!CeilingDataAsset || CeilingDataAsset->CeilingTiles.Num() == 0)
	{
		return;
	}

	float CeilingHeight = CeilingDataAsset->CeilingHeightOffset;

	// Similar multi-cell placement logic as floor
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		if (Cell.CellState != ECellState::Occupied)
		{
			continue;
		}

		// Select and place ceiling tile with height offset
		if (CeilingDataAsset->CeilingTiles.Num() > 0)
		{
			const FMeshPlacementData& TileData = CeilingDataAsset->CeilingTiles[0];
			// Ceiling placement with height offset
		}
	}
}

bool AMasterRoom::ApplyForcedPlacements()
{
	bool bAllPlacementsSucceeded = true;

	// Apply forced floor placements
	for (const auto& Placement : ForcedFloorPlacements)
	{
		const FIntPoint& BottomLeftCell = Placement.Key;
		const FMeshPlacementData& PlacementData = Placement.Value;

		// Check for overlaps
		if (CheckFootprintOverlap(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced floor placement at (%d, %d) overlaps existing placement. Rejecting."),
				BottomLeftCell.X, BottomLeftCell.Y);
			bAllPlacementsSucceeded = false;
			continue;
		}

		// Reserve cells and place mesh
		if (ReserveCellsForFootprint(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			TryPlaceMultiCellMesh(BottomLeftCell, PlacementData, FloorContainer);
		}
	}

	// Apply forced wall placements
	for (const auto& Placement : ForcedWallPlacements)
	{
		const FIntPoint& BottomLeftCell = Placement.Key;
		const FMeshPlacementData& PlacementData = Placement.Value;

		if (CheckFootprintOverlap(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced wall placement at (%d, %d) overlaps existing placement. Rejecting."),
				BottomLeftCell.X, BottomLeftCell.Y);
			bAllPlacementsSucceeded = false;
			continue;
		}

		if (ReserveCellsForFootprint(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			TryPlaceMultiCellMesh(BottomLeftCell, PlacementData, WallContainer);
		}
	}

	// Apply forced ceiling placements
	for (const auto& Placement : ForcedCeilingPlacements)
	{
		const FIntPoint& BottomLeftCell = Placement.Key;
		const FMeshPlacementData& PlacementData = Placement.Value;

		if (CheckFootprintOverlap(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced ceiling placement at (%d, %d) overlaps existing placement. Rejecting."),
				BottomLeftCell.X, BottomLeftCell.Y);
			bAllPlacementsSucceeded = false;
			continue;
		}

		if (ReserveCellsForFootprint(BottomLeftCell, PlacementData.CellsX, PlacementData.CellsY))
		{
			TryPlaceMultiCellMesh(BottomLeftCell, PlacementData, CeilingContainer);
		}
	}

	return bAllPlacementsSucceeded;
}

FVector AMasterRoom::GetWorldPositionForCell(const FIntPoint& GridCoord, float ZOffset) const
{
	float CellSize = GetCellSize();
	
	// Calculate world position from grid coordinates
	// Grid origin is at actor location
	FVector WorldPos;
	WorldPos.X = GridCoord.X * CellSize;
	WorldPos.Y = GridCoord.Y * CellSize;
	WorldPos.Z = ZOffset;
	
	return GetActorLocation() + WorldPos;
}

bool AMasterRoom::IsValidGridPosition(const FIntPoint& GridCoord) const
{
	return RuntimeGrid.Contains(GridCoord);
}

FVector AMasterRoom::CalculatePivotOffset(const FMeshPlacementData& PlacementData) const
{
	float CellSize = GetCellSize();
	FVector Offset = FVector::ZeroVector;

	switch (PlacementData.PivotType)
	{
	case EMeshPivotType::CenterXY:
		// Center of the footprint
		Offset.X = (PlacementData.CellsX * CellSize) * 0.5f;
		Offset.Y = (PlacementData.CellsY * CellSize) * 0.5f;
		break;

	case EMeshPivotType::BottomBackCenter:
		// Center X, back Y (toward room interior)
		Offset.X = (PlacementData.CellsX * CellSize) * 0.5f;
		Offset.Y = 0.0f;
		break;

	case EMeshPivotType::BottomCenter:
		// Center X and Y, bottom Z
		Offset.X = (PlacementData.CellsX * CellSize) * 0.5f;
		Offset.Y = (PlacementData.CellsY * CellSize) * 0.5f;
		break;

	case EMeshPivotType::Custom:
		Offset = PlacementData.CustomPivotOffset;
		break;
	}

	return Offset;
}

bool AMasterRoom::TryPlaceMultiCellMesh(const FIntPoint& BottomLeftCell, const FMeshPlacementData& PlacementData, USceneComponent* ParentContainer)
{
	// Validate placement
	if (!IsValidGridPosition(BottomLeftCell))
	{
		return false;
	}

	// Check if footprint fits
	for (int32 Y = 0; Y < PlacementData.CellsY; ++Y)
	{
		for (int32 X = 0; X < PlacementData.CellsX; ++X)
		{
			FIntPoint CheckCoord(BottomLeftCell.X + X, BottomLeftCell.Y + Y);
			if (!IsValidGridPosition(CheckCoord))
			{
				return false;
			}

			FGridCell* Cell = RuntimeGrid.Find(CheckCoord);
			if (!Cell || Cell->CellState != ECellState::Unoccupied)
			{
				return false;
			}
		}
	}

	// Load mesh
	if (!PlacementData.Mesh.IsValid())
	{
		return false;
	}

	UStaticMesh* Mesh = PlacementData.Mesh.LoadSynchronous();
	if (!Mesh)
	{
		return false;
	}

	// Create static mesh component
	FString ComponentName = FString::Printf(TEXT("Mesh_%d_%d"), BottomLeftCell.X, BottomLeftCell.Y);
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, FName(*ComponentName));
	if (!MeshComponent)
	{
		return false;
	}

	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetupAttachment(ParentContainer);
	MeshComponent->RegisterComponent();

	// Calculate position with pivot offset
	FVector BasePosition = GetWorldPositionForCell(BottomLeftCell);
	FVector PivotOffset = CalculatePivotOffset(PlacementData);
	MeshComponent->SetWorldLocation(BasePosition + PivotOffset);

	// Mark cells as occupied
	for (int32 Y = 0; Y < PlacementData.CellsY; ++Y)
	{
		for (int32 X = 0; X < PlacementData.CellsX; ++X)
		{
			FIntPoint CellCoord(BottomLeftCell.X + X, BottomLeftCell.Y + Y);
			if (FGridCell* Cell = RuntimeGrid.Find(CellCoord))
			{
				Cell->CellState = ECellState::Occupied;
				Cell->OccupyingActor = MeshComponent;
			}
		}
	}

	return true;
}

bool AMasterRoom::ReserveCellsForFootprint(const FIntPoint& BottomLeftCell, int32 FootprintX, int32 FootprintY)
{
	// Check all cells in footprint
	for (int32 Y = 0; Y < FootprintY; ++Y)
	{
		for (int32 X = 0; X < FootprintX; ++X)
		{
			FIntPoint CellCoord(BottomLeftCell.X + X, BottomLeftCell.Y + Y);
			if (!IsValidGridPosition(CellCoord))
			{
				return false;
			}

			FGridCell* Cell = RuntimeGrid.Find(CellCoord);
			if (!Cell || Cell->CellState != ECellState::Unoccupied)
			{
				return false;
			}
		}
	}

	// Reserve all cells
	for (int32 Y = 0; Y < FootprintY; ++Y)
	{
		for (int32 X = 0; X < FootprintX; ++X)
		{
			FIntPoint CellCoord(BottomLeftCell.X + X, BottomLeftCell.Y + Y);
			if (FGridCell* Cell = RuntimeGrid.Find(CellCoord))
			{
				Cell->CellState = ECellState::Reserved;
			}
		}
	}

	return true;
}

bool AMasterRoom::CheckFootprintOverlap(const FIntPoint& BottomLeftCell, int32 FootprintX, int32 FootprintY) const
{
	for (int32 Y = 0; Y < FootprintY; ++Y)
	{
		for (int32 X = 0; X < FootprintX; ++X)
		{
			FIntPoint CellCoord(BottomLeftCell.X + X, BottomLeftCell.Y + Y);
			
			if (!IsValidGridPosition(CellCoord))
			{
				return true; // Out of bounds = overlap
			}

			const FGridCell* Cell = RuntimeGrid.Find(CellCoord);
			if (!Cell || Cell->CellState != ECellState::Unoccupied)
			{
				return true; // Occupied or reserved = overlap
			}
		}
	}

	return false; // No overlap
}

void AMasterRoom::InitializeGrid(const FRoomShapeDefinition& ShapeDefinition)
{
	RuntimeGrid.Empty();

	float CellSize = GetCellSize();

	// Generate grid based on shape type
	switch (ShapeDefinition.ShapeType)
	{
	case ERoomShape::Rectangle:
		{
			// Simple rectangular grid
			for (int32 Y = 0; Y < ShapeDefinition.RectHeight; ++Y)
			{
				for (int32 X = 0; X < ShapeDefinition.RectWidth; ++X)
				{
					FIntPoint GridCoord(X, Y);
					FGridCell NewCell;
					NewCell.GridCoordinates = GridCoord;
					NewCell.CellState = ECellState::Unoccupied;
					NewCell.WorldPosition = GetWorldPositionForCell(GridCoord);
					
					RuntimeGrid.Add(GridCoord, NewCell);
				}
			}
		}
		break;

	case ERoomShape::Custom:
		{
			// Custom shape from layout array
			if (ShapeDefinition.CustomCellLayout.Num() == ShapeDefinition.CustomLayoutWidth * ShapeDefinition.CustomLayoutHeight)
			{
				for (int32 Y = 0; Y < ShapeDefinition.CustomLayoutHeight; ++Y)
				{
					for (int32 X = 0; X < ShapeDefinition.CustomLayoutWidth; ++X)
					{
						int32 Index = Y * ShapeDefinition.CustomLayoutWidth + X;
						if (ShapeDefinition.CustomCellLayout[Index] == 1)
						{
							FIntPoint GridCoord(X, Y);
							FGridCell NewCell;
							NewCell.GridCoordinates = GridCoord;
							NewCell.CellState = ECellState::Unoccupied;
							NewCell.WorldPosition = GetWorldPositionForCell(GridCoord);
							
							RuntimeGrid.Add(GridCoord, NewCell);
						}
					}
				}
			}
		}
		break;

	// L, T, U shapes would be implemented similarly
	default:
		// For now, default to rectangle
		for (int32 Y = 0; Y < ShapeDefinition.RectHeight; ++Y)
		{
			for (int32 X = 0; X < ShapeDefinition.RectWidth; ++X)
			{
				FIntPoint GridCoord(X, Y);
				FGridCell NewCell;
				NewCell.GridCoordinates = GridCoord;
				NewCell.CellState = ECellState::Unoccupied;
				NewCell.WorldPosition = GetWorldPositionForCell(GridCoord);
				
				RuntimeGrid.Add(GridCoord, NewCell);
			}
		}
		break;
	}
}

float AMasterRoom::GetCellSize() const
{
	if (RoomData.IsValid())
	{
		URoomData* LoadedRoomData = RoomData.LoadSynchronous();
		if (LoadedRoomData)
		{
			return LoadedRoomData->GridConfig.CellSize;
		}
	}
	
	// Default cell size
	return 100.0f;
}
