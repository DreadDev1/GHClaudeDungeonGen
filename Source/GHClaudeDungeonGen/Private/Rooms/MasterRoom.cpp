// Fill out your copyright notice in the Description page of Project Settings.


#include "Rooms/MasterRoom.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Debugging/DebugHelpers.h"
#include "Data/Room/RoomData.h"
#include "Data/Room/FloorData.h"
#include "Data/Room/WallData.h"
#include "Data/Room/CeilingData.h"
#include "Data/Room/DoorData.h"

AMasterRoom::AMasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize default values
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
	{
		GenerateRoom();
	}
}

#if WITH_EDITOR
void AMasterRoom::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-regenerate in editor when properties change
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		
		// Regenerate on key property changes
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
}

void AMasterRoom::ClearRoom()
{
	// Clear grid
	Grid.Empty();

	// Clear doorway tracking
	DoorwaySnapPoints.Empty();
	DoorwayDirections.Empty();

	// Destroy all spawned mesh components in containers
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

						Grid.Add(Cell);
					}
				}
			}
			break;
		}
		default:
			// L, T, U shapes - for now, treat as rectangles (can be enhanced later)
			UE_LOG(LogTemp, Warning, TEXT("AMasterRoom::GenerateGrid - L/T/U shapes not yet fully implemented, using rectangle"));
			// Create a simple rectangular shape using the base dimensions
			FRoomShapeDefinition RectShape;
			RectShape.ShapeType = ERoomShape::Rectangle;
			RectShape.RectWidth = FMath::Max(Shape.RectWidth, 5);
			RectShape.RectHeight = FMath::Max(Shape.RectHeight, 5);
			GenerateGrid(RectShape);
			break;
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
}
