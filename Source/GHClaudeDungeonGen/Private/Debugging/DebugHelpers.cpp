// Fill out your copyright notice in the Description page of Project Settings.

#include "Debugging/DebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDebugHelpers::UDebugHelpers()
{
	// Disable ticking - debug visualization is triggered manually
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize debug visualization properties
	bEnableDebugDraw = false;
	bDrawGrid = true;
	bDrawCellStates = true;
	bDrawWalls = true;
	bDrawDoorways = true;
	bDrawForcedPlacements = true;
	
	DebugLineThickness = 2.0f;
	DebugDrawDuration = -1.0f; // Persistent by default

	// Initialize debug colors
	UnoccupiedCellColor = FColor::Green;
	OccupiedCellColor = FColor::Red;
	ReservedCellColor = FColor::Yellow;
	ExcludedCellColor = FColor::Black;
	WallColor = FColor::Blue;
	DoorwayColor = FColor::Cyan;
	ForcedPlacementColor = FColor::Magenta;
}

void UDebugHelpers::BeginPlay()
{
	Super::BeginPlay();
}

void UDebugHelpers::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDebugHelpers::DrawDebugGrid(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawGrid)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw grid lines
	for (const auto& CellPair : Grid)
	{
		const FGridCell& Cell = CellPair.Value;
		FVector CellWorldPos = GetWorldLocationForCell(Cell.GridCoordinates, CellSize);

		// Draw cell boundary as a box
		FVector BoxExtent(CellSize * 0.5f, CellSize * 0.5f, 5.0f);
		FVector BoxCenter = CellWorldPos + FVector(CellSize * 0.5f, CellSize * 0.5f, 0.0f);
		
		DrawDebugBox(World, BoxCenter, BoxExtent, FColor::White, false, DebugDrawDuration, 0, DebugLineThickness);
	}
}

void UDebugHelpers::DrawDebugCellStates(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawCellStates)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw color-coded cell states
	for (const auto& CellPair : Grid)
	{
		const FGridCell& Cell = CellPair.Value;
		FVector CellWorldPos = GetWorldLocationForCell(Cell.GridCoordinates, CellSize);
		FColor CellColor = GetColorForCellState(Cell.CellState);

		// Draw filled square slightly above ground
		FVector SquareCenter = CellWorldPos + FVector(CellSize * 0.5f, CellSize * 0.5f, 10.0f);
		FVector SquareExtent(CellSize * 0.4f, CellSize * 0.4f, 1.0f);
		
		DrawDebugSolidBox(World, SquareCenter, SquareExtent, CellColor, false, DebugDrawDuration);
	}
}

void UDebugHelpers::DrawDebugWalls(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawWalls)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw walls on cell edges
	for (const auto& CellPair : Grid)
	{
		const FGridCell& Cell = CellPair.Value;
		FVector CellWorldPos = GetWorldLocationForCell(Cell.GridCoordinates, CellSize);

		// North wall (+Y)
		if (Cell.bHasNorthWall)
		{
			FVector WallStart = CellWorldPos + FVector(0.0f, CellSize, 0.0f);
			FVector WallEnd = CellWorldPos + FVector(CellSize, CellSize, 0.0f);
			FVector WallTop = FVector(0.0f, 0.0f, 200.0f); // Wall height
			
			DrawDebugLine(World, WallStart, WallEnd, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
			DrawDebugLine(World, WallStart + WallTop, WallEnd + WallTop, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
		}

		// East wall (+X)
		if (Cell.bHasEastWall)
		{
			FVector WallStart = CellWorldPos + FVector(CellSize, 0.0f, 0.0f);
			FVector WallEnd = CellWorldPos + FVector(CellSize, CellSize, 0.0f);
			FVector WallTop = FVector(0.0f, 0.0f, 200.0f);
			
			DrawDebugLine(World, WallStart, WallEnd, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
			DrawDebugLine(World, WallStart + WallTop, WallEnd + WallTop, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
		}

		// South wall (-Y)
		if (Cell.bHasSouthWall)
		{
			FVector WallStart = CellWorldPos;
			FVector WallEnd = CellWorldPos + FVector(CellSize, 0.0f, 0.0f);
			FVector WallTop = FVector(0.0f, 0.0f, 200.0f);
			
			DrawDebugLine(World, WallStart, WallEnd, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
			DrawDebugLine(World, WallStart + WallTop, WallEnd + WallTop, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
		}

		// West wall (-X)
		if (Cell.bHasWestWall)
		{
			FVector WallStart = CellWorldPos;
			FVector WallEnd = CellWorldPos + FVector(0.0f, CellSize, 0.0f);
			FVector WallTop = FVector(0.0f, 0.0f, 200.0f);
			
			DrawDebugLine(World, WallStart, WallEnd, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
			DrawDebugLine(World, WallStart + WallTop, WallEnd + WallTop, WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2.0f);
		}
	}
}

void UDebugHelpers::DrawDebugDoorways(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawDoorways)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw doorways on cell edges
	for (const auto& CellPair : Grid)
	{
		const FGridCell& Cell = CellPair.Value;
		FVector CellWorldPos = GetWorldLocationForCell(Cell.GridCoordinates, CellSize);

		// North doorway (+Y)
		if (Cell.bHasNorthDoorway)
		{
			FVector DoorCenter = CellWorldPos + FVector(CellSize * 0.5f, CellSize, 100.0f);
			DrawDebugSphere(World, DoorCenter, 20.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}

		// East doorway (+X)
		if (Cell.bHasEastDoorway)
		{
			FVector DoorCenter = CellWorldPos + FVector(CellSize, CellSize * 0.5f, 100.0f);
			DrawDebugSphere(World, DoorCenter, 20.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}

		// South doorway (-Y)
		if (Cell.bHasSouthDoorway)
		{
			FVector DoorCenter = CellWorldPos + FVector(CellSize * 0.5f, 0.0f, 100.0f);
			DrawDebugSphere(World, DoorCenter, 20.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}

		// West doorway (-X)
		if (Cell.bHasWestDoorway)
		{
			FVector DoorCenter = CellWorldPos + FVector(0.0f, CellSize * 0.5f, 100.0f);
			DrawDebugSphere(World, DoorCenter, 20.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}
	}
}

void UDebugHelpers::DrawDebugForcedPlacements(const TMap<FIntPoint, FMeshPlacementData>& ForcedPlacements, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawForcedPlacements)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Highlight forced placement areas
	for (const auto& Placement : ForcedPlacements)
	{
		const FIntPoint& BottomLeftCell = Placement.Key;
		const FMeshPlacementData& PlacementData = Placement.Value;

		FVector CellWorldPos = GetWorldLocationForCell(BottomLeftCell, CellSize);
		
		// Draw footprint area
		FVector FootprintExtent(
			PlacementData.CellsX * CellSize * 0.5f,
			PlacementData.CellsY * CellSize * 0.5f,
			50.0f
		);
		FVector FootprintCenter = CellWorldPos + FVector(
			PlacementData.CellsX * CellSize * 0.5f,
			PlacementData.CellsY * CellSize * 0.5f,
			50.0f
		);

		DrawDebugBox(World, FootprintCenter, FootprintExtent, ForcedPlacementColor, false, DebugDrawDuration, 0, DebugLineThickness * 3.0f);
	}
}

void UDebugHelpers::ToggleDebugDraw()
{
	bEnableDebugDraw = !bEnableDebugDraw;
}

void UDebugHelpers::UpdateDebugVisualization(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw)
	{
		return;
	}

	DrawDebugGrid(Grid, CellSize);
	DrawDebugCellStates(Grid, CellSize);
	DrawDebugWalls(Grid, CellSize);
	DrawDebugDoorways(Grid, CellSize);
}

FVector UDebugHelpers::GetWorldLocationForCell(const FIntPoint& GridCoord, float CellSize) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FVector WorldPos;
	WorldPos.X = GridCoord.X * CellSize;
	WorldPos.Y = GridCoord.Y * CellSize;
	WorldPos.Z = 0.0f;
	
	return Owner->GetActorLocation() + WorldPos;
}

FColor UDebugHelpers::GetColorForCellState(ECellState State) const
{
	switch (State)
	{
	case ECellState::Unoccupied:
		return UnoccupiedCellColor;
	case ECellState::Occupied:
		return OccupiedCellColor;
	case ECellState::Reserved:
		return ReservedCellColor;
	case ECellState::Excluded:
		return ExcludedCellColor;
	default:
		return FColor::White;
	}
}
