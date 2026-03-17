// Stack Up! - StackBlock.cpp

#include "StackBlock.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AStackBlock::AStackBlock()
{
	PrimaryActorTick.bCanEverTick = true;

	BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
	RootComponent = BlockMesh;
	BlockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockMesh->SetCastShadow(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		BlockMesh->SetStaticMesh(CubeFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		BaseMaterial = MatFinder.Object;
	}

	BlockWidth    = 300.f;
	BlockDepth    = 300.f;
	bIsMoving     = false;
	bMoveAlongX   = true;
	MoveSpeed     = 200.f;
	MoveRange     = 400.f;
	MoveDir       = 1;
	bIsFalling    = false;
	FallVelocity  = FVector::ZeroVector;
}

void AStackBlock::BeginPlay()
{
	Super::BeginPlay();
}

void AStackBlock::InitBlock(float InWidth, float InDepth, FVector InPosition, FLinearColor InColor)
{
	BlockWidth = InWidth;
	BlockDepth = InDepth;

	// The engine Cube mesh is 100x100x100 cm; scale to match our desired dimensions.
	BlockMesh->SetWorldScale3D(FVector(BlockWidth / 100.f, BlockDepth / 100.f, BlockHeight / 100.f));
	SetActorLocation(InPosition);

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		// BasicShapeMaterial exposes a "Color" vector parameter that maps to BaseColor.
		DynMat->SetVectorParameterValue(TEXT("Color"), InColor);
		BlockMesh->SetMaterial(0, DynMat);
	}
}

void AStackBlock::StartMoving(bool bInX, float InSpeed, float InRange, FVector InOrigin)
{
	bIsMoving   = true;
	bMoveAlongX = bInX;
	MoveSpeed   = InSpeed;
	MoveRange   = InRange;
	MoveOrigin  = InOrigin;
	MoveDir     = 1;

	// Spawn block at one edge of the oscillation range
	FVector StartPos = InOrigin;
	if (bInX)
		StartPos.X = InOrigin.X - InRange;
	else
		StartPos.Y = InOrigin.Y - InRange;

	SetActorLocation(StartPos);
}

void AStackBlock::StopMoving()
{
	bIsMoving = false;
}

void AStackBlock::StartFalling(FVector InVelocity)
{
	bIsMoving    = false;
	bIsFalling   = true;
	FallVelocity = InVelocity;
}

void AStackBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		FVector Pos = GetActorLocation();

		if (bMoveAlongX)
		{
			Pos.X += MoveSpeed * MoveDir * DeltaTime;
			if (Pos.X >= MoveOrigin.X + MoveRange)
			{
				Pos.X   = MoveOrigin.X + MoveRange;
				MoveDir = -1;
			}
			else if (Pos.X <= MoveOrigin.X - MoveRange)
			{
				Pos.X   = MoveOrigin.X - MoveRange;
				MoveDir = 1;
			}
		}
		else
		{
			Pos.Y += MoveSpeed * MoveDir * DeltaTime;
			if (Pos.Y >= MoveOrigin.Y + MoveRange)
			{
				Pos.Y   = MoveOrigin.Y + MoveRange;
				MoveDir = -1;
			}
			else if (Pos.Y <= MoveOrigin.Y - MoveRange)
			{
				Pos.Y   = MoveOrigin.Y - MoveRange;
				MoveDir = 1;
			}
		}

		SetActorLocation(Pos);
	}

	if (bIsFalling)
	{
		// Simple gravity integration
		FallVelocity.Z -= 980.f * DeltaTime;
		SetActorLocation(GetActorLocation() + FallVelocity * DeltaTime);

		// Auto-destroy once out of view
		if (GetActorLocation().Z < -3000.f)
		{
			Destroy();
		}
	}
}
