// Fill out your copyright notice in the Description page of Project Settings.

#include "Debugging/DebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDebugHelpers::UDebugHelpers()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize default values
	bDrawGrid = true;
	bDrawCellCoordinates = true;
	bDrawWalls = true;
	bDrawDoorways = true;
	bDrawCellStates = true;

	// Set default colors
	GridColor = FColor::White;
	WallColor = FColor::Red;
	DoorwayColor = FColor::Green;
	UnoccupiedCellColor = FColor(100, 100, 100, 50); // Gray with transparency
	OccupiedCellColor = FColor(0, 100, 200, 50); // Blue with transparency

	// Set default drawing parameters
	LineThickness = 2.0f;
	DebugDrawHeight = 5.0f;
}

UWorld* UDebugHelpers::GetDebugWorld() const
{
	return GetWorld();
}

void UDebugHelpers::DrawGridCell(const FGridCell& Cell, float CellSize, const FVector& Offset)
{
	UWorld* World = GetDebugWorld();
	if (!World)
	bShowDebugVisualization = true;
	bDrawGrid = true;
	bDrawOccupiedCells = true;
	bDrawUnoccupiedCells = true;
	bDrawForcedPlacements = true;
	DebugLineThickness = 2.0f;
	DebugVisualizationZOffset = 1.0f;
}

void UDebugHelpers::DrawGrid(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World)
{
	if (!bShowDebugVisualization || !bDrawGrid || !World)
	{
		return;
	}

	const FVector CellCenter = Cell.WorldPosition + Offset + FVector(0, 0, DebugDrawHeight);
	const float HalfCell = CellSize * 0.5f;

	// Draw grid outline if enabled
	if (bDrawGrid)
	{
		// Draw cell border
		const FVector Corners[4] = {
			CellCenter + FVector(-HalfCell, -HalfCell, 0),
			CellCenter + FVector(HalfCell, -HalfCell, 0),
			CellCenter + FVector(HalfCell, HalfCell, 0),
			CellCenter + FVector(-HalfCell, HalfCell, 0)
		};

		for (int32 i = 0; i < 4; i++)
		{
			DrawDebugLine(World, Corners[i], Corners[(i + 1) % 4], GridColor, false, -1.0f, 0, LineThickness);
		}
	}

	// Draw cell state color if enabled
	if (bDrawCellStates)
	{
		FColor StateColor = UnoccupiedCellColor;
		if (Cell.CellState == ECellState::Occupied)
		{
			StateColor = OccupiedCellColor;
		}

		// Draw a translucent box for the cell
		const FVector BoxExtent(HalfCell, HalfCell, 2.0f);
		DrawDebugBox(World, CellCenter, BoxExtent, StateColor, false, -1.0f, 0, 1.0f);
	}

	// Draw walls if enabled
	if (bDrawWalls)
	{
		DrawCellWalls(Cell, CellSize, Offset);
	}

	// Draw doorways if enabled
	if (bDrawDoorways)
	{
		DrawCellDoorways(Cell, CellSize, Offset);
	}

	// Draw coordinates if enabled
	if (bDrawCellCoordinates)
	{
		DrawCellCoordinate(Cell, Offset);
	}
}

void UDebugHelpers::DrawGridCells(const TArray<FGridCell>& Cells, float CellSize, const FVector& Offset)
{
	for (const FGridCell& Cell : Cells)
	{
		DrawGridCell(Cell, CellSize, Offset);
	}
}

void UDebugHelpers::DrawCellWalls(const FGridCell& Cell, float CellSize, const FVector& Offset)
{
	UWorld* World = GetDebugWorld();
	if (!World)
	// Find grid bounds
	FIntPoint MinCoord(INT_MAX, INT_MAX);
	FIntPoint MaxCoord(INT_MIN, INT_MIN);

	for (const FGridCell& Cell : Grid)
	{
		MinCoord.X = FMath::Min(MinCoord.X, Cell.GridCoordinates.X);
		MinCoord.Y = FMath::Min(MinCoord.Y, Cell.GridCoordinates.Y);
		MaxCoord.X = FMath::Max(MaxCoord.X, Cell.GridCoordinates.X);
		MaxCoord.Y = FMath::Max(MaxCoord.Y, Cell.GridCoordinates.Y);
	}

	// Draw grid lines (green)
	const FColor GridColor = FColor::Green;
	const float ZOffset = DebugVisualizationZOffset;

	// Draw vertical lines
	for (int32 X = MinCoord.X; X <= MaxCoord.X + 1; ++X)
	{
		FVector Start = OwnerLocation + FVector(X * CellSize, MinCoord.Y * CellSize, ZOffset);
		FVector End = OwnerLocation + FVector(X * CellSize, (MaxCoord.Y + 1) * CellSize, ZOffset);
		DrawDebugLine(World, Start, End, GridColor, true, -1.0f, 0, DebugLineThickness);
	}

	// Draw horizontal lines
	for (int32 Y = MinCoord.Y; Y <= MaxCoord.Y + 1; ++Y)
	{
		FVector Start = OwnerLocation + FVector(MinCoord.X * CellSize, Y * CellSize, ZOffset);
		FVector End = OwnerLocation + FVector((MaxCoord.X + 1) * CellSize, Y * CellSize, ZOffset);
		DrawDebugLine(World, Start, End, GridColor, true, -1.0f, 0, DebugLineThickness);
	}
}

void UDebugHelpers::DrawOccupiedCells(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World)
{
	if (!bShowDebugVisualization || !bDrawOccupiedCells || !World)
	{
		return;
	}

	const FVector CellCenter = Cell.WorldPosition + Offset + FVector(0, 0, DebugDrawHeight);
	const float HalfCell = CellSize * 0.5f;
	const float WallThickness = LineThickness * 2.0f;

	// Draw north wall (positive Y)
	if (Cell.bHasNorthWall)
	{
		const FVector Start = CellCenter + FVector(-HalfCell, HalfCell, 0);
		const FVector End = CellCenter + FVector(HalfCell, HalfCell, 0);
		DrawDebugLine(World, Start, End, WallColor, false, -1.0f, 0, WallThickness);
	}

	// Draw east wall (positive X)
	if (Cell.bHasEastWall)
	{
		const FVector Start = CellCenter + FVector(HalfCell, -HalfCell, 0);
		const FVector End = CellCenter + FVector(HalfCell, HalfCell, 0);
		DrawDebugLine(World, Start, End, WallColor, false, -1.0f, 0, WallThickness);
	}

	// Draw south wall (negative Y)
	if (Cell.bHasSouthWall)
	{
		const FVector Start = CellCenter + FVector(-HalfCell, -HalfCell, 0);
		const FVector End = CellCenter + FVector(HalfCell, -HalfCell, 0);
		DrawDebugLine(World, Start, End, WallColor, false, -1.0f, 0, WallThickness);
	}

	// Draw west wall (negative X)
	if (Cell.bHasWestWall)
	{
		const FVector Start = CellCenter + FVector(-HalfCell, -HalfCell, 0);
		const FVector End = CellCenter + FVector(-HalfCell, HalfCell, 0);
		DrawDebugLine(World, Start, End, WallColor, false, -1.0f, 0, WallThickness);
	}
}

void UDebugHelpers::DrawCellDoorways(const FGridCell& Cell, float CellSize, const FVector& Offset)
{
	UWorld* World = GetDebugWorld();
	if (!World)
	const FLinearColor OccupiedColor = FLinearColor::Red;
	const float ZOffset = DebugVisualizationZOffset;

	for (const FGridCell& Cell : Grid)
	{
		if (Cell.CellState == ECellState::Occupied)
		{
			FVector CellWorldPos = OwnerLocation + FVector(Cell.GridCoordinates.X * CellSize, Cell.GridCoordinates.Y * CellSize, ZOffset);
			DrawCellRect(CellWorldPos, CellSize, OccupiedColor, World);
		}
	}
}

void UDebugHelpers::DrawUnoccupiedCells(const TArray<FGridCell>& Grid, float CellSize, const FVector& OwnerLocation, UWorld* World)
{
	if (!bShowDebugVisualization || !bDrawUnoccupiedCells || !World)
	{
		return;
	}

	const FLinearColor UnoccupiedColor = FLinearColor::Blue;
	const float ZOffset = DebugVisualizationZOffset;

	for (const FGridCell& Cell : Grid)
	{
		if (Cell.CellState == ECellState::Unoccupied)
		{
			FVector CellWorldPos = OwnerLocation + FVector(Cell.GridCoordinates.X * CellSize, Cell.GridCoordinates.Y * CellSize, ZOffset);
			DrawCellRect(CellWorldPos, CellSize, UnoccupiedColor, World);
		}
	}
}

void UDebugHelpers::DrawForcedPlacements(const TMap<FIntPoint, FMeshPlacementData>& ForcedPlacements, float CellSize, const FVector& OwnerLocation, UWorld* World)
{
	if (!bShowDebugVisualization || !bDrawForcedPlacements || !World)
	{
		return;
	}

	const FVector CellCenter = Cell.WorldPosition + Offset + FVector(0, 0, DebugDrawHeight + 2.0f);
	const float HalfCell = CellSize * 0.5f;
	const float DoorwayIndicatorSize = CellSize * 0.3f;

	// Draw north doorway (positive Y)
	if (Cell.bHasNorthDoorway)
	{
		const FVector DoorwayPos = CellCenter + FVector(0, HalfCell, 0);
		DrawDebugSphere(World, DoorwayPos, DoorwayIndicatorSize * 0.5f, 8, DoorwayColor, false, -1.0f, 0, LineThickness);
	}

	// Draw east doorway (positive X)
	if (Cell.bHasEastDoorway)
	{
		const FVector DoorwayPos = CellCenter + FVector(HalfCell, 0, 0);
		DrawDebugSphere(World, DoorwayPos, DoorwayIndicatorSize * 0.5f, 8, DoorwayColor, false, -1.0f, 0, LineThickness);
	}

	// Draw south doorway (negative Y)
	if (Cell.bHasSouthDoorway)
	{
		const FVector DoorwayPos = CellCenter + FVector(0, -HalfCell, 0);
		DrawDebugSphere(World, DoorwayPos, DoorwayIndicatorSize * 0.5f, 8, DoorwayColor, false, -1.0f, 0, LineThickness);
	}

	// Draw west doorway (negative X)
	if (Cell.bHasWestDoorway)
	{
		const FVector DoorwayPos = CellCenter + FVector(-HalfCell, 0, 0);
		DrawDebugSphere(World, DoorwayPos, DoorwayIndicatorSize * 0.5f, 8, DoorwayColor, false, -1.0f, 0, LineThickness);
	}
}

void UDebugHelpers::DrawCellCoordinate(const FGridCell& Cell, const FVector& Offset)
{
	UWorld* World = GetDebugWorld();
	if (!World)
	{
		return;
	}

	const FVector TextLocation = Cell.WorldPosition + Offset + FVector(0, 0, DebugDrawHeight + 10.0f);
	const FString CoordText = FString::Printf(TEXT("(%d,%d)"), Cell.GridCoordinates.X, Cell.GridCoordinates.Y);
	
	// Use finite duration to prevent accumulation of debug text
	DrawDebugString(World, TextLocation, CoordText, nullptr, FColor::Yellow, 0.1f, true, 1.0f);
}

void UDebugHelpers::DrawDoorwaySnapPoints(const TArray<FVector>& SnapPoints, float Radius)
{
	UWorld* World = GetDebugWorld();
	const FLinearColor ForcedPlacementColor = FLinearColor::Yellow;
	const float ZOffset = DebugVisualizationZOffset + 0.5f; // Slightly higher to stand out

	for (const TPair<FIntPoint, FMeshPlacementData>& Pair : ForcedPlacements)
	{
		const FIntPoint& BottomLeft = Pair.Key;
		const FMeshPlacementData& PlacementData = Pair.Value;

		// Draw a rectangle covering the entire footprint
		FVector StartPos = OwnerLocation + FVector(BottomLeft.X * CellSize, BottomLeft.Y * CellSize, ZOffset);
		float Width = PlacementData.CellsX * CellSize;
		float Height = PlacementData.CellsY * CellSize;

		FVector Corners[4] = {
			StartPos,
			StartPos + FVector(Width, 0.0f, 0.0f),
			StartPos + FVector(Width, Height, 0.0f),
			StartPos + FVector(0.0f, Height, 0.0f)
		};

		// Draw the rectangle
		for (int32 i = 0; i < 4; ++i)
		{
			FVector Start = Corners[i];
			FVector End = Corners[(i + 1) % 4];
			DrawDebugLine(World, Start, End, ForcedPlacementColor.ToFColor(true), true, -1.0f, 0, DebugLineThickness * 1.5f);
		}

		// Draw an X to mark the forced placement
		DrawDebugLine(World, Corners[0], Corners[2], ForcedPlacementColor.ToFColor(true), true, -1.0f, 0, DebugLineThickness);
		DrawDebugLine(World, Corners[1], Corners[3], ForcedPlacementColor.ToFColor(true), true, -1.0f, 0, DebugLineThickness);
	}
}

void UDebugHelpers::DrawCellRect(const FVector& CellWorldPosition, float CellSize, const FLinearColor& Color, UWorld* World)
{
	if (!World)
	{
		return;
	}

	for (const FVector& SnapPoint : SnapPoints)
	{
		DrawDebugSphere(World, SnapPoint + FVector(0, 0, DebugDrawHeight), Radius, 12, DoorwayColor, false, -1.0f, 0, LineThickness * 2.0f);
	}
}

void UDebugHelpers::ClearDebugDrawings()
{
	UWorld* World = GetDebugWorld();
	if (World)
	{
		FlushPersistentDebugLines(World);
	}
	// Draw a filled rectangle for the cell
	FVector Corners[4] = {
		CellWorldPosition,
		CellWorldPosition + FVector(CellSize, 0.0f, 0.0f),
		CellWorldPosition + FVector(CellSize, CellSize, 0.0f),
		CellWorldPosition + FVector(0.0f, CellSize, 0.0f)
	};

	// Draw lines to form a rectangle
	for (int32 i = 0; i < 4; ++i)
	{
		FVector Start = Corners[i];
		FVector End = Corners[(i + 1) % 4];
		DrawDebugLine(World, Start, End, Color.ToFColor(true), true, -1.0f, 0, DebugLineThickness);
	}

	// Draw diagonals for better visibility
	DrawDebugLine(World, Corners[0], Corners[2], Color.ToFColor(true), true, -1.0f, 0, DebugLineThickness * 0.5f);
	DrawDebugLine(World, Corners[1], Corners[3], Color.ToFColor(true), true, -1.0f, 0, DebugLineThickness * 0.5f);
}
