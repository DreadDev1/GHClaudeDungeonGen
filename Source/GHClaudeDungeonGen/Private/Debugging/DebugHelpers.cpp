// Fill out your copyright notice in the Description page of Project Settings.

#include "Debugging/DebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
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

void UDebugHelpers::DrawDebugGrid(const TMap<FIntPoint, FGridCell>& Grid, float CellSize)
{
	if (!bEnableDebugDraw || !bDrawGrid)
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
	for (const FVector& SnapPoint : SnapPoints)
	{
		DrawDebugSphere(World, SnapPoint + FVector(0, 0, DebugDrawHeight), Radius, 12, DoorwayColor, false, -1.0f, 0, LineThickness * 2.0f);
	}
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
