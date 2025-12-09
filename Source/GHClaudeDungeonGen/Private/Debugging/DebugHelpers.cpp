// Fill out your copyright notice in the Description page of Project Settings.

#include "Debugging/DebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDebugHelpers::UDebugHelpers()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize default values
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
