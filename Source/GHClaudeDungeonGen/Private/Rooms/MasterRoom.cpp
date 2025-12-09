// Fill out your copyright notice in the Description page of Project Settings.

#include "Rooms/MasterRoom.h"
#include "Data/Room/RoomData.h"
#include "Debugging/DebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AMasterRoom::AMasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize default values
	GenerationSeed = 12345;
	bUseRandomSeed = true;
	bOverrideShape = false;
	bIsGenerated = false;

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

	// Regenerate room when properties change in editor
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		
		// Regenerate if relevant properties changed
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, RoomData) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, GenerationSeed) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, bUseRandomSeed) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, bOverrideShape) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AMasterRoom, ShapeOverride))
		{
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

void AMasterRoom::RegenerateWithNewSeed()
{
	// Generate a new random seed
	GenerationSeed = FMath::Rand();
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
		return FMath::Rand();
	}
	return GenerationSeed;
}
