// Fill out your copyright notice in the Description page of Project Settings.


#include "Debugging/DebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDebugHelpers::UDebugHelpers()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Start disabled, enable when debug is needed

	// Initialize default debug settings
	bShowGrid = true;
	bShowCellCoordinates = false;
	bShowCellStates = true;
	bShowWalls = true;
	bShowDoorways = true;
	bShowSnapPoints = true;
	DebugDrawDuration = 0.0f;
	DebugLineThickness = 2.0f;
	CachedCellSize = 100.0f;
}

void UDebugHelpers::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only draw if any debug flag is enabled and we have cached data
	if (IsAnyDebugEnabled() && CachedGridCells.Num() > 0)
	{
		DrawRoomDebug(CachedGridCells, CachedSnapPoints, CachedCellSize);
	}
}

void UDebugHelpers::DrawGrid(const TArray<FGridCell>& GridCells, float CellSize)
{
	if (!bShowGrid || GridCells.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const float HalfCell = CellSize / 2.0f;
	const FColor GridColor = FColor::Green;

	for (const FGridCell& Cell : GridCells)
	{
		const FVector CellWorldPos = OwnerLocation + Cell.WorldPosition;
		
		// Draw cell boundaries as a square
		const FVector TopLeft = CellWorldPos + FVector(-HalfCell, HalfCell, 0);
		const FVector TopRight = CellWorldPos + FVector(HalfCell, HalfCell, 0);
		const FVector BottomRight = CellWorldPos + FVector(HalfCell, -HalfCell, 0);
		const FVector BottomLeft = CellWorldPos + FVector(-HalfCell, -HalfCell, 0);

		DrawDebugLine(World, TopLeft, TopRight, GridColor, false, DebugDrawDuration, 0, DebugLineThickness);
		DrawDebugLine(World, TopRight, BottomRight, GridColor, false, DebugDrawDuration, 0, DebugLineThickness);
		DrawDebugLine(World, BottomRight, BottomLeft, GridColor, false, DebugDrawDuration, 0, DebugLineThickness);
		DrawDebugLine(World, BottomLeft, TopLeft, GridColor, false, DebugDrawDuration, 0, DebugLineThickness);
	}
}

void UDebugHelpers::DrawCellCoordinates(const TArray<FGridCell>& GridCells)
{
	if (!bShowCellCoordinates || GridCells.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();

	for (const FGridCell& Cell : GridCells)
	{
		const FVector TextLocation = OwnerLocation + Cell.WorldPosition + FVector(0, 0, 10);
		const FString CoordText = FString::Printf(TEXT("(%d,%d)"), Cell.GridCoordinates.X, Cell.GridCoordinates.Y);
		
		DrawDebugString(World, TextLocation, CoordText, nullptr, FColor::White, DebugDrawDuration, true);
	}
}

void UDebugHelpers::DrawCellStates(const TArray<FGridCell>& GridCells, float CellSize)
{
	if (!bShowCellStates || GridCells.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const float HalfCell = CellSize / 2.0f;

	for (const FGridCell& Cell : GridCells)
	{
		const FVector CellWorldPos = OwnerLocation + Cell.WorldPosition;
		const FColor StateColor = GetCellStateColor(Cell.CellState);
		
		// Draw filled square for cell state
		const FVector Center = CellWorldPos + FVector(0, 0, 5);
		DrawDebugSolidBox(World, Center, FVector(HalfCell * 0.8f, HalfCell * 0.8f, 2.0f), StateColor, false, DebugDrawDuration);
	}
}

void UDebugHelpers::DrawWalls(const TArray<FGridCell>& GridCells, float CellSize)
{
	if (!bShowWalls || GridCells.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const float HalfCell = CellSize / 2.0f;
	const FColor WallColor = FColor::Red;
	const float WallHeight = 50.0f;

	for (const FGridCell& Cell : GridCells)
	{
		const FVector CellWorldPos = OwnerLocation + Cell.WorldPosition;
		
		// Draw walls as vertical lines
		if (Cell.bHasNorthWall)
		{
			const FVector Start = CellWorldPos + FVector(-HalfCell, HalfCell, 0);
			const FVector End = CellWorldPos + FVector(HalfCell, HalfCell, 0);
			DrawDebugLine(World, Start, Start + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, End, End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, Start + FVector(0, 0, WallHeight), End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
		}
		
		if (Cell.bHasEastWall)
		{
			const FVector Start = CellWorldPos + FVector(HalfCell, -HalfCell, 0);
			const FVector End = CellWorldPos + FVector(HalfCell, HalfCell, 0);
			DrawDebugLine(World, Start, Start + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, End, End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, Start + FVector(0, 0, WallHeight), End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
		}
		
		if (Cell.bHasSouthWall)
		{
			const FVector Start = CellWorldPos + FVector(-HalfCell, -HalfCell, 0);
			const FVector End = CellWorldPos + FVector(HalfCell, -HalfCell, 0);
			DrawDebugLine(World, Start, Start + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, End, End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, Start + FVector(0, 0, WallHeight), End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
		}
		
		if (Cell.bHasWestWall)
		{
			const FVector Start = CellWorldPos + FVector(-HalfCell, -HalfCell, 0);
			const FVector End = CellWorldPos + FVector(-HalfCell, HalfCell, 0);
			DrawDebugLine(World, Start, Start + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, End, End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
			DrawDebugLine(World, Start + FVector(0, 0, WallHeight), End + FVector(0, 0, WallHeight), WallColor, false, DebugDrawDuration, 0, DebugLineThickness * 2);
		}
	}
}

void UDebugHelpers::DrawDoorways(const TArray<FGridCell>& GridCells, float CellSize)
{
	if (!bShowDoorways || GridCells.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const float HalfCell = CellSize / 2.0f;
	const FColor DoorwayColor = FColor::Cyan;
	const float DoorwayHeight = 50.0f;

	for (const FGridCell& Cell : GridCells)
	{
		const FVector CellWorldPos = OwnerLocation + Cell.WorldPosition;
		
		// Draw doorways as cyan markers
		if (Cell.bHasNorthDoorway)
		{
			const FVector Location = CellWorldPos + FVector(0, HalfCell, DoorwayHeight / 2);
			DrawDebugSphere(World, Location, 15.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}
		
		if (Cell.bHasEastDoorway)
		{
			const FVector Location = CellWorldPos + FVector(HalfCell, 0, DoorwayHeight / 2);
			DrawDebugSphere(World, Location, 15.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}
		
		if (Cell.bHasSouthDoorway)
		{
			const FVector Location = CellWorldPos + FVector(0, -HalfCell, DoorwayHeight / 2);
			DrawDebugSphere(World, Location, 15.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}
		
		if (Cell.bHasWestDoorway)
		{
			const FVector Location = CellWorldPos + FVector(-HalfCell, 0, DoorwayHeight / 2);
			DrawDebugSphere(World, Location, 15.0f, 8, DoorwayColor, false, DebugDrawDuration, 0, DebugLineThickness);
		}
	}
}

void UDebugHelpers::DrawSnapPoints(const TArray<FVector>& SnapPoints)
{
	if (!bShowSnapPoints || SnapPoints.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FColor SnapPointColor = FColor::Yellow;

	for (const FVector& SnapPoint : SnapPoints)
	{
		const FVector WorldSnapPoint = OwnerLocation + SnapPoint;
		DrawDebugSphere(World, WorldSnapPoint, 20.0f, 12, SnapPointColor, false, DebugDrawDuration, 0, DebugLineThickness);
		DrawDebugCoordinateSystem(World, WorldSnapPoint, FRotator::ZeroRotator, 30.0f, false, DebugDrawDuration, 0, DebugLineThickness);
	}
}

void UDebugHelpers::DrawRoomDebug(const TArray<FGridCell>& GridCells, const TArray<FVector>& SnapPoints, float CellSize)
{
	// Cache values for continuous drawing
	CachedGridCells = GridCells;
	CachedSnapPoints = SnapPoints;
	CachedCellSize = CellSize;

	// Enable tick only if debug is needed
	SetComponentTickEnabled(IsAnyDebugEnabled() && CachedGridCells.Num() > 0);

	// Draw all debug visuals
	DrawGrid(GridCells, CellSize);
	DrawCellCoordinates(GridCells);
	DrawCellStates(GridCells, CellSize);
	DrawWalls(GridCells, CellSize);
	DrawDoorways(GridCells, CellSize);
	DrawSnapPoints(SnapPoints);
}

void UDebugHelpers::ClearDebugDrawings()
{
	CachedGridCells.Empty();
	CachedSnapPoints.Empty();

	UWorld* World = GetWorld();
	if (World)
	{
		FlushDebugStrings(World);
		FlushPersistentDebugLines(World);
	}
}

FColor UDebugHelpers::GetCellStateColor(ECellState State) const
{
	switch (State)
	{
		case ECellState::Unoccupied:
			return FColor(100, 100, 100, 128); // Gray (semi-transparent)
		case ECellState::Occupied:
			return FColor(0, 255, 0, 128); // Green (semi-transparent)
		case ECellState::Reserved:
			return FColor(255, 255, 0, 128); // Yellow (semi-transparent)
		case ECellState::Excluded:
			return FColor(255, 0, 0, 128); // Red (semi-transparent)
		default:
			return FColor::White;
	}
}

bool UDebugHelpers::IsAnyDebugEnabled() const
{
	return bShowGrid || bShowCellCoordinates || bShowCellStates || 
	       bShowWalls || bShowDoorways || bShowSnapPoints;
}
