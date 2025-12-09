// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/MasterRoom.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Room/RoomData.h"
#include "Debugging/DebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Debugging/DebugHelpers.h"
#include "Data/Room/RoomData.h"
#include "Data/Room/FloorData.h"
#include "Data/Room/WallData.h"
#include "Data/Room/CeilingData.h"
#include "Data/Room/DoorData.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

// Constants
static constexpr float SELECTION_WEIGHT_SCALE = 100.0f; // SelectionWeight is 0-100, normalized to 0-1

AMasterRoom::AMasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize default values
	GenerationSeed = 12345;
	GenerationSeed = 0;
	bUseRandomSeed = true;
	bOverrideShape = false;
	bIsGenerated = false;

	// Initialize component hierarchy
	InitializeComponents();
}

void AMasterRoom::InitializeComponents()
{
	// Create root scene component
	RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
	RootComponent = RoomRoot;
	// Create root component
	RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
	SetRootComponent(RoomRoot);

	// Create container components
	FloorContainer = CreateDefaultSubobject<USceneComponent>(TEXT("FloorContainer"));
	FloorContainer->SetupAttachment(RoomRoot);

	WallContainer = CreateDefaultSubobject<USceneComponent>(TEXT("WallContainer"));
	WallContainer->SetupAttachment(RoomRoot);

	CeilingContainer = CreateDefaultSubobject<USceneComponent>(TEXT("CeilingContainer"));
	CeilingContainer->SetupAttachment(RoomRoot);

	DoorwayContainer = CreateDefaultSubobject<USceneComponent>(TEXT("DoorwayContainer"));
	DoorwayContainer->SetupAttachment(RoomRoot);

	// Create debug helper component
	DebugHelper = CreateDefaultSubobject<UDebugHelpers>(TEXT("DebugHelper"));
}

void AMasterRoom::BeginPlay()
{
	Super::BeginPlay();
	
	// Auto-generate at runtime if RoomData is set
	if (!RoomData.IsNull() && !bIsGenerated)
	// Generate room at runtime if not already generated
	if (!bIsGenerated)
	{
		GenerateRoom();
	}
}

#if WITH_EDITOR
void AMasterRoom::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-regenerate in editor when properties change
	// Regenerate room when properties change in editor
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		
		// Regenerate on key property changes
		// Regenerate if relevant properties changed
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, RoomData) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, GenerationSeed) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, bUseRandomSeed) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, bOverrideShape) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, ShapeOverride))
		{
			if (!RoomData.IsNull())
			{
				GenerateRoom();
			}
		}
	}
}
			GenerateRoom();
		}
	}
}

void AMasterRoom::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Generate room preview in editor
	GenerateRoom();
}
#endif

void AMasterRoom::GenerateRoom()
{
	// Clear existing room content
	ClearRoom();

	// Check if RoomData is valid
	if (RoomData.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateRoom - RoomData is null"));
		return;
	}

	// Load RoomData if needed
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (LoadedRoomData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateRoom - Failed to load RoomData"));
		return;
	}

	// Initialize random stream
	if (bUseRandomSeed)
	{
		GenerationSeed = FMath::Rand();
	}
	RandomStream.Initialize(GenerationSeed);

	// Determine which shape to use
	FRoomShapeDefinition ShapeToUse;
	if (bOverrideShape)
	{
		ShapeToUse = ShapeOverride;
	}
	else if (LoadedRoomData->AllowedShapes.Num() > 0)
	{
		// Select random shape from allowed shapes
		int32 ShapeIndex = RandomStream.RandRange(0, LoadedRoomData->AllowedShapes.Num() - 1);
		ShapeToUse = LoadedRoomData->AllowedShapes[ShapeIndex];
	}
	else
	{
		// Default rectangular shape
		ShapeToUse.ShapeType = ERoomShape::Rectangle;
		ShapeToUse.RectWidth = 5;
		ShapeToUse.RectHeight = 5;
	}

	// Generate grid
	GenerateGrid(ShapeToUse);

	// Spawn meshes
	SpawnFloorMeshes();
	SpawnWallMeshes();
	SpawnCeilingMeshes();
	SpawnDoorwayMeshes();

	// Calculate doorway snap points
	CalculateDoorwaySnapPoints();

	// Apply forced placements
	ApplyForcedPlacements();

	bIsGenerated = true;
	
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::GenerateRoom - Room generated successfully with seed %d"), GenerationSeed);
	// Clear existing room
	ClearRoom();

	// Validate RoomData
	if (!RoomData.IsValid())
	{
		// Try to load if not already loaded
		if (!RoomData.IsNull())
		{
			RoomData.LoadSynchronous();
		}
		
		if (!RoomData.IsValid())
		{
			return; // Cannot generate without room data
		}
	}

	// Initialize random stream with effective seed
	const int32 EffectiveSeed = GetEffectiveSeed();
	RandomStream.Initialize(EffectiveSeed);

	// Initialize grid based on shape
	InitializeGrid();

	// Generate room components
	GenerateFloorMeshes();
	GenerateWallMeshes();
	GenerateCeilingMeshes();
	GenerateDoorways();

	// Mark as generated
	bIsGenerated = true;
}

void AMasterRoom::ClearRoom()
{
	// Clear grid
	Grid.Empty();

	// Clear doorway tracking
	DoorwaySnapPoints.Empty();
	DoorwayDirections.Empty();

	// Destroy all spawned mesh components in containers
	// Clear doorway data
	DoorwaySnapPoints.Empty();
	DoorwayDirections.Empty();

	// Destroy all mesh components in containers
	if (FloorContainer)
	{
		TArray<USceneComponent*> Children;
		FloorContainer->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			if (Child)
			{
				Child->DestroyComponent();
			}
		}
	}

	if (WallContainer)
	{
		TArray<USceneComponent*> Children;
		WallContainer->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			if (Child)
			{
				Child->DestroyComponent();
			}
		}
	}

	if (CeilingContainer)
	{
		TArray<USceneComponent*> Children;
		CeilingContainer->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			if (Child)
			{
				Child->DestroyComponent();
			}
		}
	}

	if (DoorwayContainer)
	{
		TArray<USceneComponent*> Children;
		DoorwayContainer->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			if (Child)
			{
				Child->DestroyComponent();
			}
		}
	}

	bIsGenerated = false;
}

void AMasterRoom::RegenerateRoom(int32 NewSeed)
{
	GenerationSeed = NewSeed;
	bUseRandomSeed = false;
	GenerateRoom();
}

void AMasterRoom::ApplyForcedPlacements()
{
	// Spawn forced props
	for (const TPair<FIntPoint, TSubclassOf<AActor>>& Placement : ForcedPropPlacements)
	{
		if (Placement.Value != nullptr)
		{
			// Find the grid cell at the specified coordinates
			FGridCell* Cell = Grid.FindByPredicate([&Placement](const FGridCell& GridCell)
			{
				return GridCell.GridCoordinates == Placement.Key;
			});

			if (Cell != nullptr)
			{
				// Spawn actor at cell world position
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Placement.Value, Cell->WorldPosition, FRotator::ZeroRotator, SpawnParams);
				
				if (SpawnedActor)
				{
					Cell->OccupyingActor = SpawnedActor;
				}
			}
		}
	}

	// Spawn forced furniture
	for (const TPair<FIntPoint, TSubclassOf<AActor>>& Placement : ForcedFurniturePlacements)
	{
		if (Placement.Value != nullptr)
		{
			FGridCell* Cell = Grid.FindByPredicate([&Placement](const FGridCell& GridCell)
			{
				return GridCell.GridCoordinates == Placement.Key;
			});

			if (Cell != nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Placement.Value, Cell->WorldPosition, FRotator::ZeroRotator, SpawnParams);
				
				if (SpawnedActor)
				{
					Cell->OccupyingActor = SpawnedActor;
				}
			}
		}
	}

	// Spawn forced enemies
	for (const TPair<FIntPoint, TSubclassOf<AActor>>& Placement : ForcedEnemyPlacements)
	{
		if (Placement.Value != nullptr)
		{
			FGridCell* Cell = Grid.FindByPredicate([&Placement](const FGridCell& GridCell)
			{
				return GridCell.GridCoordinates == Placement.Key;
			});

			if (Cell != nullptr)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Placement.Value, Cell->WorldPosition, FRotator::ZeroRotator, SpawnParams);
				
				if (SpawnedActor)
				{
					Cell->OccupyingActor = SpawnedActor;
				}
			}
		}
	}
}

void AMasterRoom::GenerateGrid(const FRoomShapeDefinition& Shape)
{
	Grid.Empty();

	// Get cell size from RoomData
	float CellSize = 100.0f; // Default
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (LoadedRoomData)
	{
		CellSize = LoadedRoomData->GridConfig.CellSize;
	}

	// Generate grid based on shape type
	switch (Shape.ShapeType)
	{
		case ERoomShape::Rectangle:
		{
			// Generate rectangular grid
			for (int32 Y = 0; Y < Shape.RectHeight; ++Y)
			{
				for (int32 X = 0; X < Shape.RectWidth; ++X)
				{
					FGridCell Cell;
					Cell.GridCoordinates = FIntPoint(X, Y);
					Cell.CellState = ECellState::Occupied;
					
					// Calculate world position (centered on origin)
					Cell.WorldPosition = FVector(
						X * CellSize - (Shape.RectWidth * CellSize) / 2.0f + CellSize / 2.0f,
						Y * CellSize - (Shape.RectHeight * CellSize) / 2.0f + CellSize / 2.0f,
						0.0f
					);

					// Set perimeter walls
					Cell.bHasNorthWall = (Y == Shape.RectHeight - 1);
					Cell.bHasSouthWall = (Y == 0);
					Cell.bHasEastWall = (X == Shape.RectWidth - 1);
					Cell.bHasWestWall = (X == 0);

void AMasterRoom::RegenerateWithNewSeed()
{
	// Generate a new random seed based on current time
	GenerationSeed = static_cast<int32>(FDateTime::Now().GetTicks());
	bUseRandomSeed = false;
	
	// Regenerate room
	GenerateRoom();
}

float AMasterRoom::GetCellSize() const
{
	if (RoomData.IsValid())
	{
		return RoomData->GridConfig.CellSize;
	}
	return 100.0f; // Default cell size
}

FRoomShapeDefinition AMasterRoom::GetActiveShapeDefinition() const
{
	if (bOverrideShape)
	{
		return ShapeOverride;
	}

	if (RoomData.IsValid() && RoomData->AllowedShapes.Num() > 0)
	{
		// For now, use the first shape (can be randomized later)
		return RoomData->AllowedShapes[0];
	}

	// Return default rectangle shape
	FRoomShapeDefinition DefaultShape;
	DefaultShape.ShapeType = ERoomShape::Rectangle;
	DefaultShape.RectWidth = 5;
	DefaultShape.RectHeight = 5;
	return DefaultShape;
}

void AMasterRoom::InitializeGrid()
{
	Grid.Empty();

	const FRoomShapeDefinition ShapeDefinition = GetActiveShapeDefinition();
	const float CellSize = GetCellSize();

	// For now, implement simple rectangular grid
	// TODO: Implement L, T, U, and Custom shapes in future iterations
	if (ShapeDefinition.ShapeType == ERoomShape::Rectangle)
	{
		const int32 Width = ShapeDefinition.RectWidth;
		const int32 Height = ShapeDefinition.RectHeight;

		for (int32 Y = 0; Y < Height; Y++)
		{
			for (int32 X = 0; X < Width; X++)
			{
				FGridCell Cell;
				Cell.GridCoordinates = FIntPoint(X, Y);
				Cell.CellState = ECellState::Occupied;
				Cell.WorldPosition = GridToWorldPosition(FIntPoint(X, Y));

				// Set walls on edges
				Cell.bHasNorthWall = (Y == Height - 1);
				Cell.bHasEastWall = (X == Width - 1);
				Cell.bHasSouthWall = (Y == 0);
				Cell.bHasWestWall = (X == 0);

				Grid.Add(Cell);
			}
		}
	}
}

void AMasterRoom::GenerateFloorMeshes()
{
	// Placeholder implementation
	// TODO: Load floor meshes from RoomData->FloorData and spawn them
	// For now, just ensure we have the container ready
}

void AMasterRoom::GenerateWallMeshes()
{
	// Placeholder implementation
	// TODO: Load wall meshes from RoomData->WallData and spawn them
	// For now, just ensure we have the container ready
}

void AMasterRoom::GenerateCeilingMeshes()
{
	// Placeholder implementation
	// TODO: Load ceiling meshes from RoomData->CeilingData and spawn them
	// For now, just ensure we have the container ready
}

void AMasterRoom::GenerateDoorways()
{
	// Placeholder implementation
	// TODO: Analyze grid, place doorways, and create snap points
	// For now, just ensure we have the container ready
}

UStaticMeshComponent* AMasterRoom::SpawnMeshComponent(USceneComponent* Parent, UStaticMesh* Mesh, const FTransform& Transform)
{
	if (!Parent || !Mesh)
void AMasterRoom::GenerateRoom()
{
	// Clear any existing generation first
	if (bIsGenerated)
	{
		ClearRoom();
	}

	// Load and validate room data
	if (!LoadRoomData())
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::GenerateRoom - Failed to load RoomData"));
		return;
	}

	// Initialize random stream
	if (bUseRandomSeed)
	{
		RandomStream.Initialize(FMath::Rand());
	}
	else
	{
		RandomStream.Initialize(GenerationSeed);
	}

	// Initialize the grid
	InitializeGrid();

	// Apply forced placements first (highest priority)
	ApplyForcedPlacements();

	// Generate floor tiles
	GenerateFloorTiles();

	// Generate walls
	GenerateWalls();

	// Generate ceiling
	GenerateCeiling();

	// Generate doorways
	GenerateDoorways();

	bIsGenerated = true;

	// Update debug visualization
	UpdateDebugVisualization();

	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::GenerateRoom - Room generated successfully"));
}

void AMasterRoom::ClearRoom()
{
	// Destroy all spawned mesh components
	TArray<USceneComponent*> ChildComponents;
	
	// Get all children of container components
	if (FloorContainer)
	{
		FloorContainer->GetChildrenComponents(true, ChildComponents);
	}
	if (WallContainer)
	{
		WallContainer->GetChildrenComponents(true, ChildComponents);
	}
	if (CeilingContainer)
	{
		CeilingContainer->GetChildrenComponents(true, ChildComponents);
	}
	if (DoorwayContainer)
	{
		DoorwayContainer->GetChildrenComponents(true, ChildComponents);
	}

	// Destroy each child component
	for (USceneComponent* Child : ChildComponents)
	{
		if (Child)
		{
			Child->DestroyComponent();
		}
	}

	// Clear runtime state
	Grid.Empty();
	DoorwaySnapPoints.Empty();
	DoorwayDirections.Empty();
	bIsGenerated = false;

	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::ClearRoom - Room cleared successfully"));
}

void AMasterRoom::RegenerateRoom()
{
	ClearRoom();
	GenerateRoom();
}

void AMasterRoom::UpdateDebugVisualization()
{
	if (!DebugHelper)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::UpdateDebugVisualization - DebugHelper is null"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::UpdateDebugVisualization - World is null"));
		return;
	}

	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::UpdateDebugVisualization - RoomData is null"));
		return;
	}

	const float CellSize = LoadedRoomData->GridConfig.CellSize;
	const FVector OwnerLocation = GetActorLocation();

	// Draw grid lines
	DebugHelper->DrawGrid(Grid, CellSize, OwnerLocation, World);

	// Draw occupied cells
	DebugHelper->DrawOccupiedCells(Grid, CellSize, OwnerLocation, World);

	// Draw unoccupied cells
	DebugHelper->DrawUnoccupiedCells(Grid, CellSize, OwnerLocation, World);

	// Draw forced placements
	DebugHelper->DrawForcedPlacements(ForcedFloorPlacements, CellSize, OwnerLocation, World);

	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::UpdateDebugVisualization - Debug visualization updated"));
}

void AMasterRoom::InitializeGrid()
{
	// Get the active room shape
	FRoomShapeDefinition ActiveShape = GetActiveRoomShape();

	// Clear existing grid
	Grid.Empty();

	// Get cell size from RoomData
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData)
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::InitializeGrid - RoomData is null"));
		return;
	}

	const float CellSize = LoadedRoomData->GridConfig.CellSize;

	// Generate grid based on shape type
	switch (ActiveShape.ShapeType)
	{
		case ERoomShape::Rectangle:
		{
			// Simple rectangular grid
			for (int32 Y = 0; Y < ActiveShape.RectHeight; ++Y)
			{
				for (int32 X = 0; X < ActiveShape.RectWidth; ++X)
				{
					FGridCell Cell;
					Cell.GridCoordinates = FIntPoint(X, Y);
					Cell.CellState = ECellState::Unoccupied;
					Cell.WorldPosition = GetActorLocation() + FVector(X * CellSize, Y * CellSize, 0.0f);
					Grid.Add(Cell);
				}
			}
			break;
		}
		case ERoomShape::Custom:
		{
			// Generate custom shape grid
			for (int32 Y = 0; Y < Shape.CustomLayoutHeight; ++Y)
			{
				for (int32 X = 0; X < Shape.CustomLayoutWidth; ++X)
				{
					int32 Index = Y * Shape.CustomLayoutWidth + X;
					if (Index < Shape.CustomCellLayout.Num() && Shape.CustomCellLayout[Index] == 1)
					{
						FGridCell Cell;
						Cell.GridCoordinates = FIntPoint(X, Y);
						Cell.CellState = ECellState::Occupied;
						
						Cell.WorldPosition = FVector(
							X * CellSize - (Shape.CustomLayoutWidth * CellSize) / 2.0f + CellSize / 2.0f,
							Y * CellSize - (Shape.CustomLayoutHeight * CellSize) / 2.0f + CellSize / 2.0f,
							0.0f
						);

						// Check neighbors for walls
						int32 NorthIndex = (Y + 1) * Shape.CustomLayoutWidth + X;
						int32 SouthIndex = (Y - 1) * Shape.CustomLayoutWidth + X;
						int32 EastIndex = Y * Shape.CustomLayoutWidth + (X + 1);
						int32 WestIndex = Y * Shape.CustomLayoutWidth + (X - 1);

						Cell.bHasNorthWall = (Y == Shape.CustomLayoutHeight - 1 || NorthIndex >= Shape.CustomCellLayout.Num() || Shape.CustomCellLayout[NorthIndex] == 0);
						Cell.bHasSouthWall = (Y == 0 || SouthIndex < 0 || Shape.CustomCellLayout[SouthIndex] == 0);
						Cell.bHasEastWall = (X == Shape.CustomLayoutWidth - 1 || EastIndex >= Shape.CustomCellLayout.Num() || Shape.CustomCellLayout[EastIndex] == 0);
						Cell.bHasWestWall = (X == 0 || WestIndex < 0 || Shape.CustomCellLayout[WestIndex] == 0);

			// Custom shape based on layout array
			if (ActiveShape.CustomCellLayout.Num() != ActiveShape.CustomLayoutWidth * ActiveShape.CustomLayoutHeight)
			{
				UE_LOG(LogTemp, Error, TEXT("AMasterRoom::InitializeGrid - Custom layout array size mismatch"));
				return;
			}

			for (int32 Y = 0; Y < ActiveShape.CustomLayoutHeight; ++Y)
			{
				for (int32 X = 0; X < ActiveShape.CustomLayoutWidth; ++X)
				{
					int32 Index = Y * ActiveShape.CustomLayoutWidth + X;
					if (ActiveShape.CustomCellLayout[Index] == 1)
					{
						FGridCell Cell;
						Cell.GridCoordinates = FIntPoint(X, Y);
						Cell.CellState = ECellState::Unoccupied;
						Cell.WorldPosition = GetActorLocation() + FVector(X * CellSize, Y * CellSize, 0.0f);
						Grid.Add(Cell);
					}
				}
			}
			break;
		}
		case ERoomShape::LShape:
		case ERoomShape::TShape:
		case ERoomShape::UShape:
		{
			// L, T, U shapes - for now, treat as rectangles (can be enhanced later)
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateGrid - L/T/U shapes not yet fully implemented, generating simple rectangle"));
			
			// Generate rectangular grid directly without recursion
			const int32 Width = FMath::Max(Shape.RectWidth, 5);
			const int32 Height = FMath::Max(Shape.RectHeight, 5);
			
			for (int32 Y = 0; Y < Height; ++Y)
			{
				for (int32 X = 0; X < Width; ++X)
				{
					FGridCell Cell;
					Cell.GridCoordinates = FIntPoint(X, Y);
					Cell.CellState = ECellState::Occupied;
					
					// Calculate world position (centered on origin)
					Cell.WorldPosition = FVector(
						X * CellSize - (Width * CellSize) / 2.0f + CellSize / 2.0f,
						Y * CellSize - (Height * CellSize) / 2.0f + CellSize / 2.0f,
						0.0f
					);

					// Set perimeter walls
					Cell.bHasNorthWall = (Y == Height - 1);
					Cell.bHasSouthWall = (Y == 0);
					Cell.bHasEastWall = (X == Width - 1);
					Cell.bHasWestWall = (X == 0);

		default:
		{
			// For L, T, U shapes - implement as simple rectangles for now
			// These would require more complex shape generation logic
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::InitializeGrid - L/T/U shapes not yet implemented, using rectangle"));
			for (int32 Y = 0; Y < ActiveShape.RectHeight; ++Y)
			{
				for (int32 X = 0; X < ActiveShape.RectWidth; ++X)
				{
					FGridCell Cell;
					Cell.GridCoordinates = FIntPoint(X, Y);
					Cell.CellState = ECellState::Unoccupied;
					Cell.WorldPosition = GetActorLocation() + FVector(X * CellSize, Y * CellSize, 0.0f);
					Grid.Add(Cell);
				}
			}
			break;
		}
	}
}

void AMasterRoom::SpawnFloorMeshes()
{
	// Placeholder - actual mesh spawning to be implemented with asset loading
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::SpawnFloorMeshes - Placeholder"));
}

void AMasterRoom::SpawnWallMeshes()
{
	// Placeholder - actual mesh spawning to be implemented with asset loading
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::SpawnWallMeshes - Placeholder"));
}

void AMasterRoom::SpawnCeilingMeshes()
{
	// Placeholder - actual mesh spawning to be implemented with asset loading
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::SpawnCeilingMeshes - Placeholder"));
}

void AMasterRoom::SpawnDoorwayMeshes()
{
	// Placeholder - actual mesh spawning to be implemented with asset loading
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::SpawnDoorwayMeshes - Placeholder"));
}

void AMasterRoom::CalculateDoorwaySnapPoints()
{
	DoorwaySnapPoints.Empty();
	DoorwayDirections.Empty();

	// Calculate snap points from grid cells with doorways
	for (const FGridCell& Cell : Grid)
	{
		if (Cell.bHasNorthDoorway)
		{
			DoorwaySnapPoints.Add(Cell.WorldPosition + FVector(0, 50.0f, 0)); // Offset to edge
			DoorwayDirections.Add(EWallDirection::North);
		}
		if (Cell.bHasEastDoorway)
		{
			DoorwaySnapPoints.Add(Cell.WorldPosition + FVector(50.0f, 0, 0));
			DoorwayDirections.Add(EWallDirection::East);
		}
		if (Cell.bHasSouthDoorway)
		{
			DoorwaySnapPoints.Add(Cell.WorldPosition + FVector(0, -50.0f, 0));
			DoorwayDirections.Add(EWallDirection::South);
		}
		if (Cell.bHasWestDoorway)
		{
			DoorwaySnapPoints.Add(Cell.WorldPosition + FVector(-50.0f, 0, 0));
			DoorwayDirections.Add(EWallDirection::West);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::InitializeGrid - Grid initialized with %d cells"), Grid.Num());
}

void AMasterRoom::ApplyForcedPlacements()
{
	// Apply forced floor placements
	for (const TPair<FIntPoint, FMeshPlacementData>& Pair : ForcedFloorPlacements)
	{
		const FIntPoint& BottomLeft = Pair.Key;
		const FMeshPlacementData& PlacementData = Pair.Value;

		// Check if placement is valid
		if (!CanPlaceMeshAt(BottomLeft, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced floor placement at (%d, %d) with size (%dx%d) overlaps occupied cells or is out of bounds. Rejecting."),
				BottomLeft.X, BottomLeft.Y, PlacementData.CellsX, PlacementData.CellsY);
			continue;
		}

		// Mark cells as occupied
		MarkCellsOccupied(BottomLeft, PlacementData.CellsX, PlacementData.CellsY);

		// Spawn the mesh
		if (PlacementData.Mesh.IsValid())
		{
			UStaticMesh* Mesh = PlacementData.Mesh.LoadSynchronous();
			if (Mesh)
			{
				FVector WorldPosition = CalculateWorldPositionForCell(BottomLeft, PlacementData);
				SpawnMeshAtPosition(WorldPosition, Mesh, PlacementData, FloorContainer);
			}
		}
	}

	// Apply forced wall placements (similar logic)
	for (const TPair<FIntPoint, FMeshPlacementData>& Pair : ForcedWallPlacements)
	{
		const FIntPoint& Position = Pair.Key;
		const FMeshPlacementData& PlacementData = Pair.Value;

		// Check if placement is valid
		if (!CanPlaceMeshAt(Position, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced wall placement at (%d, %d) overlaps occupied cells or is out of bounds. Rejecting."),
				Position.X, Position.Y);
			continue;
		}

		// Mark cells as occupied/reserved
		MarkCellsOccupied(Position, PlacementData.CellsX, PlacementData.CellsY);

		// Spawn the mesh
		if (PlacementData.Mesh.IsValid())
		{
			UStaticMesh* Mesh = PlacementData.Mesh.LoadSynchronous();
			if (Mesh)
			{
				FVector WorldPosition = CalculateWorldPositionForCell(Position, PlacementData);
				SpawnMeshAtPosition(WorldPosition, Mesh, PlacementData, WallContainer);
			}
		}
	}

	// Apply forced ceiling placements
	for (const TPair<FIntPoint, FMeshPlacementData>& Pair : ForcedCeilingPlacements)
	{
		const FIntPoint& BottomLeft = Pair.Key;
		const FMeshPlacementData& PlacementData = Pair.Value;

		// Check if placement is valid
		if (!CanPlaceMeshAt(BottomLeft, PlacementData.CellsX, PlacementData.CellsY))
		{
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::ApplyForcedPlacements - Forced ceiling placement at (%d, %d) overlaps occupied cells or is out of bounds. Rejecting."),
				BottomLeft.X, BottomLeft.Y);
			continue;
		}

		// Mark cells as occupied
		MarkCellsOccupied(BottomLeft, PlacementData.CellsX, PlacementData.CellsY);

		// Spawn the mesh
		if (PlacementData.Mesh.IsValid())
		{
			UStaticMesh* Mesh = PlacementData.Mesh.LoadSynchronous();
			if (Mesh)
			{
				FVector WorldPosition = CalculateWorldPositionForCell(BottomLeft, PlacementData);
				SpawnMeshAtPosition(WorldPosition, Mesh, PlacementData, CeilingContainer);
			}
		}
	}

	// Apply forced doorway placements
	for (const TPair<FIntPoint, FMeshPlacementData>& Pair : ForcedDoorwayPlacements)
	{
		const FIntPoint& Position = Pair.Key;
		const FMeshPlacementData& PlacementData = Pair.Value;

		// Spawn the mesh (doorways don't necessarily occupy cells)
		if (PlacementData.Mesh.IsValid())
		{
			UStaticMesh* Mesh = PlacementData.Mesh.LoadSynchronous();
			if (Mesh)
			{
				FVector WorldPosition = CalculateWorldPositionForCell(Position, PlacementData);
				SpawnMeshAtPosition(WorldPosition, Mesh, PlacementData, DoorwayContainer);
			}
		}
	}
}

void AMasterRoom::GenerateFloorTiles()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->FloorData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateFloorTiles - No floor data available"));
		return;
	}

	UFloorData* FloorData = LoadedRoomData->FloorData.LoadSynchronous();
	if (!FloorData || FloorData->FloorTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateFloorTiles - Floor data has no tiles"));
		return;
	}

	// Sort floor tiles by footprint size (largest first)
	TArray<FMeshPlacementData> SortedTiles = FloorData->FloorTiles;
	SortedTiles.Sort([](const FMeshPlacementData& A, const FMeshPlacementData& B)
	{
		return (A.CellsX * A.CellsY) > (B.CellsX * B.CellsY);
	});

	// First pass: Place multi-cell tiles (largest to smallest)
	for (const FMeshPlacementData& TileData : SortedTiles)
	{
		if (TileData.CellsX == 1 && TileData.CellsY == 1)
		{
			continue; // Skip single-cell tiles for now
		}

		// Try to place this tile type in available locations
		for (FGridCell& Cell : Grid)
		{
			if (Cell.CellState == ECellState::Unoccupied)
			{
				// Check if we can place this multi-cell tile here
				if (CanPlaceMeshAt(Cell.GridCoordinates, TileData.CellsX, TileData.CellsY))
				{
					// Use weighted random selection based on SelectionWeight
					if (RandomStream.FRand() <= (TileData.SelectionWeight / SELECTION_WEIGHT_SCALE))
					{
						// Place the tile
						MarkCellsOccupied(Cell.GridCoordinates, TileData.CellsX, TileData.CellsY);

						if (TileData.Mesh.IsValid())
						{
							UStaticMesh* Mesh = TileData.Mesh.LoadSynchronous();
							if (Mesh)
							{
								FVector WorldPosition = CalculateWorldPositionForCell(Cell.GridCoordinates, TileData);
								SpawnMeshAtPosition(WorldPosition, Mesh, TileData, FloorContainer);
							}
						}
					}
				}
			}
		}
	}

	// Second pass: Fill remaining cells with single-cell tiles
	TArray<FMeshPlacementData> SingleCellTiles;
	for (const FMeshPlacementData& TileData : FloorData->FloorTiles)
	{
		if (TileData.CellsX == 1 && TileData.CellsY == 1)
		{
			SingleCellTiles.Add(TileData);
		}
	}

	if (SingleCellTiles.Num() > 0)
	{
		// Calculate total weight
		float TotalWeight = 0.0f;
		for (const FMeshPlacementData& TileData : SingleCellTiles)
		{
			TotalWeight += TileData.SelectionWeight;
		}

		// Fill remaining unoccupied cells
		for (FGridCell& Cell : Grid)
		{
			if (Cell.CellState == ECellState::Unoccupied)
			{
				// Weighted random selection
				float RandomValue = RandomStream.FRand() * TotalWeight;
				float CurrentWeight = 0.0f;

				for (const FMeshPlacementData& TileData : SingleCellTiles)
				{
					CurrentWeight += TileData.SelectionWeight;
					if (RandomValue <= CurrentWeight)
					{
						// Place this tile
						Cell.CellState = ECellState::Occupied;

						if (TileData.Mesh.IsValid())
						{
							UStaticMesh* Mesh = TileData.Mesh.LoadSynchronous();
							if (Mesh)
							{
								FVector WorldPosition = CalculateWorldPositionForCell(Cell.GridCoordinates, TileData);
								SpawnMeshAtPosition(WorldPosition, Mesh, TileData, FloorContainer);
							}
						}
						break;
					}
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
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateWalls - No wall data available"));
		return;
	}

	UWallData* WallDataAsset = LoadedRoomData->WallData.LoadSynchronous();
	if (!WallDataAsset)
	{
		return;
	}

	// Wall generation logic would analyze grid cells and place walls at boundaries
	// For now, this is a placeholder that would need full implementation
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::GenerateWalls - Wall generation placeholder"));
}

void AMasterRoom::GenerateCeiling()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->CeilingData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateCeiling - No ceiling data available"));
		return;
	}

	UCeilingData* CeilingDataAsset = LoadedRoomData->CeilingData.LoadSynchronous();
	if (!CeilingDataAsset || CeilingDataAsset->CeilingTiles.Num() == 0)
	{
		return;
	}

	// Generate ceiling tiles similar to floor tiles but at ceiling height
	const float CeilingHeight = CeilingDataAsset->CeilingHeightOffset;

	// Use first ceiling tile for all cells (simple approach)
	if (CeilingDataAsset->CeilingTiles.Num() > 0)
	{
		const FMeshPlacementData& TileData = CeilingDataAsset->CeilingTiles[0];
		
		if (TileData.Mesh.IsValid())
		{
			UStaticMesh* Mesh = TileData.Mesh.LoadSynchronous();
			if (Mesh)
			{
				for (const FGridCell& Cell : Grid)
				{
					if (Cell.CellState == ECellState::Occupied || Cell.CellState == ECellState::Unoccupied)
					{
						FVector WorldPosition = CalculateWorldPositionForCell(Cell.GridCoordinates, TileData);
						WorldPosition.Z += CeilingHeight;
						SpawnMeshAtPosition(WorldPosition, Mesh, TileData, CeilingContainer);
					}
				}
			}
		}
	}
}

void AMasterRoom::GenerateDoorways()
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData || !LoadedRoomData->DoorData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateDoorways - No door data available"));
		return;
	}

	UDoorData* DoorDataAsset = LoadedRoomData->DoorData.LoadSynchronous();
	if (!DoorDataAsset)
	{
		return;
	}

	// Doorway generation logic would analyze room boundaries and place doorways
	// For now, this is a placeholder
	UE_LOG(LogTemp, Log, TEXT("AMasterRoom::GenerateDoorways - Doorway generation placeholder"));
}

bool AMasterRoom::CanPlaceMeshAt(const FIntPoint& BottomLeft, int32 CellsX, int32 CellsY) const
{
	// Check each cell in the footprint
	for (int32 Y = 0; Y < CellsY; ++Y)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			FIntPoint CheckCoord = BottomLeft + FIntPoint(X, Y);
			
			// Find the cell in the grid
			const FGridCell* FoundCell = Grid.FindByPredicate([CheckCoord](const FGridCell& Cell)
			{
				return Cell.GridCoordinates == CheckCoord;
			});

			// If cell doesn't exist or is not unoccupied, placement is invalid
			if (!FoundCell || FoundCell->CellState != ECellState::Unoccupied)
			{
				return false;
			}
		}
	}

	return true;
}

void AMasterRoom::MarkCellsOccupied(const FIntPoint& BottomLeft, int32 CellsX, int32 CellsY)
{
	for (int32 Y = 0; Y < CellsY; ++Y)
	{
		for (int32 X = 0; X < CellsX; ++X)
		{
			FIntPoint TargetCoord = BottomLeft + FIntPoint(X, Y);
			
			// Find and mark the cell as occupied
			for (FGridCell& Cell : Grid)
			{
				if (Cell.GridCoordinates == TargetCoord)
				{
					Cell.CellState = ECellState::Occupied;
					break;
				}
			}
		}
	}
}

UStaticMeshComponent* AMasterRoom::SpawnMeshAtPosition(const FVector& WorldPosition, UStaticMesh* Mesh, const FMeshPlacementData& PlacementData, USceneComponent* ParentComponent)
{
	if (!Mesh || !ParentComponent)
	{
		return nullptr;
	}

	// Create new static mesh component
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this);
	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetupAttachment(Parent);
		MeshComponent->SetRelativeTransform(Transform);
		MeshComponent->RegisterComponent();
	// Create a new static mesh component
	FString ComponentName = FString::Printf(TEXT("SpawnedMesh_%s"), *Mesh->GetName());
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, *ComponentName);
	
	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetupAttachment(ParentComponent);
		MeshComponent->RegisterComponent();
		MeshComponent->SetWorldLocation(WorldPosition);
		
		// Handle rotation if allowed
		if (PlacementData.bAllowRotation)
		{
			// Random rotation in 90-degree increments
			int32 RotationSteps = RandomStream.RandRange(0, 3);
			FRotator Rotation(0.0f, RotationSteps * 90.0f, 0.0f);
			MeshComponent->SetWorldRotation(Rotation);
		}
	}

	return MeshComponent;
}

FVector AMasterRoom::GridToWorldPosition(const FIntPoint& GridCoord) const
{
	const float CellSize = GetCellSize();
	
	// Calculate world position (centered on cell)
	const float WorldX = GridCoord.X * CellSize;
	const float WorldY = GridCoord.Y * CellSize;
	
	return FVector(WorldX, WorldY, 0.0f);
}

int32 AMasterRoom::GetEffectiveSeed() const
{
	if (bUseRandomSeed)
	{
		// Use system time for random seed generation
		// This provides consistent cross-platform behavior
		return static_cast<int32>(FDateTime::Now().GetTicks());
	}
	return GenerationSeed;
FVector AMasterRoom::CalculateWorldPositionForCell(const FIntPoint& CellCoord, const FMeshPlacementData& PlacementData) const
{
	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData)
	{
		return FVector::ZeroVector;
	}

	const float CellSize = LoadedRoomData->GridConfig.CellSize;
	FVector BasePosition = GetActorLocation() + FVector(CellCoord.X * CellSize, CellCoord.Y * CellSize, 0.0f);

	// Adjust for pivot type
	switch (PlacementData.PivotType)
	{
		case EMeshPivotType::CenterXY:
		{
			// Center the mesh on the cell(s)
			float OffsetX = (PlacementData.CellsX * CellSize) / 2.0f;
			float OffsetY = (PlacementData.CellsY * CellSize) / 2.0f;
			BasePosition += FVector(OffsetX, OffsetY, 0.0f);
			break;
		}
		case EMeshPivotType::BottomBackCenter:
		{
			// Center on X, back on Y
			float OffsetX = (PlacementData.CellsX * CellSize) / 2.0f;
			BasePosition += FVector(OffsetX, 0.0f, 0.0f);
			break;
		}
		case EMeshPivotType::BottomCenter:
		{
			// Center on X and Y, bottom on Z
			float OffsetX = (PlacementData.CellsX * CellSize) / 2.0f;
			float OffsetY = (PlacementData.CellsY * CellSize) / 2.0f;
			BasePosition += FVector(OffsetX, OffsetY, 0.0f);
			break;
		}
		case EMeshPivotType::Custom:
		{
			BasePosition += PlacementData.CustomPivotOffset;
			break;
		}
	}

	return BasePosition;
}

FRoomShapeDefinition AMasterRoom::GetActiveRoomShape() const
{
	if (bOverrideShape)
	{
		return ShapeOverride;
	}

	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (LoadedRoomData && LoadedRoomData->AllowedShapes.Num() > 0)
	{
		// Use the first allowed shape
		return LoadedRoomData->AllowedShapes[0];
	}

	// Return default rectangular shape
	FRoomShapeDefinition DefaultShape;
	DefaultShape.ShapeType = ERoomShape::Rectangle;
	DefaultShape.RectWidth = 5;
	DefaultShape.RectHeight = 5;
	return DefaultShape;
}

bool AMasterRoom::LoadRoomData()
{
	if (!RoomData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::LoadRoomData - RoomData is not set"));
		return false;
	}

	URoomData* LoadedRoomData = RoomData.LoadSynchronous();
	if (!LoadedRoomData)
	{
		UE_LOG(LogTemp, Error, TEXT("AMasterRoom::LoadRoomData - Failed to load RoomData"));
		return false;
	}

	return true;
}
