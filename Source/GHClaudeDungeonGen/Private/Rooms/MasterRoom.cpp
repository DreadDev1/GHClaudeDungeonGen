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
	if (RoomData.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::GenerateRoom - RoomData is null!"));
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

	// Update debug visualization if enabled
	if (DebugHelpers && DebugHelpers->bEnableDebugDraw)
	{
		DebugHelpers->UpdateDebugVisualization(RuntimeGrid, GetCellSize());
	}

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

	// First, detect which cells need walls (cells at the edge of the room)
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		// Only consider occupied cells
		if (Cell.CellState != ECellState::Occupied)
		{
			continue;
		}

		// Check each direction to see if we need a wall
		FIntPoint NorthNeighbor(Cell.GridCoordinates.X, Cell.GridCoordinates.Y + 1);
		FIntPoint EastNeighbor(Cell.GridCoordinates.X + 1, Cell.GridCoordinates.Y);
		FIntPoint SouthNeighbor(Cell.GridCoordinates.X, Cell.GridCoordinates.Y - 1);
		FIntPoint WestNeighbor(Cell.GridCoordinates.X - 1, Cell.GridCoordinates.Y);

		// If neighbor doesn't exist or is unoccupied, we need a wall
		const FGridCell* NorthCell = RuntimeGrid.Find(NorthNeighbor);
		const FGridCell* EastCell = RuntimeGrid.Find(EastNeighbor);
		const FGridCell* SouthCell = RuntimeGrid.Find(SouthNeighbor);
		const FGridCell* WestCell = RuntimeGrid.Find(WestNeighbor);

		Cell.bHasNorthWall = !NorthCell || NorthCell->CellState == ECellState::Unoccupied;
		Cell.bHasEastWall = !EastCell || EastCell->CellState == ECellState::Unoccupied;
		Cell.bHasSouthWall = !SouthCell || SouthCell->CellState == ECellState::Unoccupied;
		Cell.bHasWestWall = !WestCell || WestCell->CellState == ECellState::Unoccupied;
	}

	float CellSize = GetCellSize();
	const FMeshPlacementData& DefaultWallSegment = WallDataAsset->WallSegments[0];

	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		// Skip unoccupied cells
		if (Cell.CellState != ECellState::Occupied)
		{
			continue;
		}

		// Place walls on each edge that needs one
		auto PlaceWall = [&](EWallDirection Direction, bool bHasWall, bool bHasDoorway, float RotationYaw) {
			if (!bHasWall || bHasDoorway)
			{
				return;
			}

			if (!DefaultWallSegment.Mesh.IsValid())
			{
				return;
			}

			UStaticMesh* Mesh = DefaultWallSegment.Mesh.LoadSynchronous();
			if (!Mesh)
			{
				return;
			}

			FString ComponentName = FString::Printf(TEXT("Wall_%d_%d_%d"), Cell.GridCoordinates.X, Cell.GridCoordinates.Y, (int32)Direction);
			UStaticMeshComponent* WallComponent = NewObject<UStaticMeshComponent>(this, FName(*ComponentName));
			if (!WallComponent)
			{
				return;
			}

			WallComponent->SetStaticMesh(Mesh);
			WallComponent->SetupAttachment(WallContainer);
			WallComponent->RegisterComponent();

			// Calculate wall position based on direction
			FVector BasePosition = GetWorldPositionForCell(Cell.GridCoordinates);
			FVector WallOffset = FVector::ZeroVector;

			switch (Direction)
			{
			case EWallDirection::North:
				WallOffset = FVector(CellSize * 0.5f, CellSize, 0.0f);
				break;
			case EWallDirection::East:
				WallOffset = FVector(CellSize, CellSize * 0.5f, 0.0f);
				break;
			case EWallDirection::South:
				WallOffset = FVector(CellSize * 0.5f, 0.0f, 0.0f);
				break;
			case EWallDirection::West:
				WallOffset = FVector(0.0f, CellSize * 0.5f, 0.0f);
				break;
			}

			WallComponent->SetWorldLocation(BasePosition + WallOffset);
			WallComponent->SetWorldRotation(FRotator(0.0f, RotationYaw, 0.0f));
		};

		// Place walls for each direction
		PlaceWall(EWallDirection::North, Cell.bHasNorthWall, Cell.bHasNorthDoorway, 0.0f);
		PlaceWall(EWallDirection::East, Cell.bHasEastWall, Cell.bHasEastDoorway, 90.0f);
		PlaceWall(EWallDirection::South, Cell.bHasSouthWall, Cell.bHasSouthDoorway, 180.0f);
		PlaceWall(EWallDirection::West, Cell.bHasWestWall, Cell.bHasWestDoorway, 270.0f);
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

	// Track which cells have been covered by ceiling tiles (important for multi-cell tiles)
	TSet<FIntPoint> ProcessedCells;

	// Sort ceiling tiles by footprint size (largest first)
	TArray<FMeshPlacementData> SortedTiles = CeilingDataAsset->CeilingTiles;
	SortedTiles.Sort([](const FMeshPlacementData& A, const FMeshPlacementData& B) {
		int32 AreaA = A.CellsX * A.CellsY;
		int32 AreaB = B.CellsX * B.CellsY;
		return AreaA > AreaB;
	});

	// Place ceiling tiles on all occupied floor cells
	for (auto& CellPair : RuntimeGrid)
	{
		FGridCell& Cell = CellPair.Value;
		
		// Only place ceiling on cells with floor tiles
		if (Cell.CellState != ECellState::Occupied)
		{
			continue;
		}

		// Skip if already processed by a multi-cell tile
		if (ProcessedCells.Contains(Cell.GridCoordinates))
		{
			continue;
		}

		// Try to place largest tile that fits
		bool bPlaced = false;
		for (const FMeshPlacementData& TileData : SortedTiles)
		{
			// Check if this tile would fit
			bool bCanPlace = true;
			for (int32 Y = 0; Y < TileData.CellsY && bCanPlace; ++Y)
			{
				for (int32 X = 0; X < TileData.CellsX && bCanPlace; ++X)
				{
					FIntPoint CheckCoord(Cell.GridCoordinates.X + X, Cell.GridCoordinates.Y + Y);
					const FGridCell* CheckCell = RuntimeGrid.Find(CheckCoord);
					if (!CheckCell || CheckCell->CellState != ECellState::Occupied || ProcessedCells.Contains(CheckCoord))
					{
						bCanPlace = false;
					}
				}
			}

			if (bCanPlace)
			{
				// Place the ceiling tile at height offset
				if (TileData.Mesh.IsValid())
				{
					UStaticMesh* Mesh = TileData.Mesh.LoadSynchronous();
					if (Mesh)
					{
						FString ComponentName = FString::Printf(TEXT("CeilingMesh_%d_%d"), Cell.GridCoordinates.X, Cell.GridCoordinates.Y);
						UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, FName(*ComponentName));
						if (MeshComponent)
						{
							MeshComponent->SetStaticMesh(Mesh);
							MeshComponent->SetupAttachment(CeilingContainer);
							MeshComponent->RegisterComponent();

							// Calculate position with pivot offset and ceiling height
							FVector BasePosition = GetWorldPositionForCell(Cell.GridCoordinates, CeilingHeight);
							FVector PivotOffset = CalculatePivotOffset(TileData);
							MeshComponent->SetWorldLocation(BasePosition + PivotOffset);

							// Mark cells as processed
							for (int32 Y = 0; Y < TileData.CellsY; ++Y)
							{
								for (int32 X = 0; X < TileData.CellsX; ++X)
								{
									FIntPoint ProcessedCoord(Cell.GridCoordinates.X + X, Cell.GridCoordinates.Y + Y);
									ProcessedCells.Add(ProcessedCoord);
								}
							}

							bPlaced = true;
							break;
						}
					}
				}
			}
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
				// Store the owning actor (this AMasterRoom) since OccupyingActor is TWeakObjectPtr<AActor>
				Cell->OccupyingActor = this;
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

	case ERoomShape::LShape:
		{
			// L-Shape: Main rectangle + one extension
			// Main section
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
			// Extension (simplified - extends from bottom-right)
			// In a full implementation, this would use FShapeTemplate
			// For now, extend by half the width to the right
			int32 ExtensionWidth = FMath::Max(1, ShapeDefinition.RectWidth / 2);
			int32 ExtensionHeight = FMath::Max(1, ShapeDefinition.RectHeight / 2);
			for (int32 Y = 0; Y < ExtensionHeight; ++Y)
			{
				for (int32 X = ShapeDefinition.RectWidth; X < ShapeDefinition.RectWidth + ExtensionWidth; ++X)
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

	case ERoomShape::TShape:
		{
			// T-Shape: Main rectangle + extensions on two opposite sides
			// Main section
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
			// Top extension
			int32 ExtensionWidth = FMath::Max(1, ShapeDefinition.RectWidth / 3);
			int32 ExtensionHeight = FMath::Max(1, ShapeDefinition.RectHeight / 3);
			int32 ExtensionStartX = (ShapeDefinition.RectWidth - ExtensionWidth) / 2;
			for (int32 Y = ShapeDefinition.RectHeight; Y < ShapeDefinition.RectHeight + ExtensionHeight; ++Y)
			{
				for (int32 X = ExtensionStartX; X < ExtensionStartX + ExtensionWidth; ++X)
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

	case ERoomShape::UShape:
		{
			// U-Shape: Main rectangle + extensions on three sides
			// Main section (bottom bar of U)
			for (int32 Y = 0; Y < ShapeDefinition.RectHeight / 3; ++Y)
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
			// Left vertical extension
			int32 ExtensionHeight = ShapeDefinition.RectHeight * 2 / 3;
			int32 ExtensionWidth = ShapeDefinition.RectWidth / 3;
			for (int32 Y = ShapeDefinition.RectHeight / 3; Y < ShapeDefinition.RectHeight; ++Y)
			{
				for (int32 X = 0; X < ExtensionWidth; ++X)
				{
					FIntPoint GridCoord(X, Y);
					FGridCell NewCell;
					NewCell.GridCoordinates = GridCoord;
					NewCell.CellState = ECellState::Unoccupied;
					NewCell.WorldPosition = GetWorldPositionForCell(GridCoord);
					RuntimeGrid.Add(GridCoord, NewCell);
				}
			}
			// Right vertical extension
			for (int32 Y = ShapeDefinition.RectHeight / 3; Y < ShapeDefinition.RectHeight; ++Y)
			{
				for (int32 X = ShapeDefinition.RectWidth - ExtensionWidth; X < ShapeDefinition.RectWidth; ++X)
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

	default:
		// Default to rectangle for any unknown shape type
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::InitializeGrid - Unknown shape type, defaulting to rectangle"));
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

void AMasterRoom::RefreshDebugVisualization()
{
	if (DebugHelpers && bIsGenerated)
	{
		DebugHelpers->UpdateDebugVisualization(RuntimeGrid, GetCellSize());
		
		// Also draw forced placements
		DebugHelpers->DrawDebugForcedPlacements(ForcedFloorPlacements, GetCellSize());
		DebugHelpers->DrawDebugForcedPlacements(ForcedWallPlacements, GetCellSize());
		DebugHelpers->DrawDebugForcedPlacements(ForcedCeilingPlacements, GetCellSize());
	}
}
