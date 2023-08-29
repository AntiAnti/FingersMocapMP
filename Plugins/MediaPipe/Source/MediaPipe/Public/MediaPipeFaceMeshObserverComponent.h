// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0

#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediaPipeFaceMeshObserverComponent.generated.h"

USTRUCT(BlueprintType)
struct FMediaPipeVertex
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe Vertex")
	FVector Pos;

	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe Vertex")
	FVector2D TexCoord;
};

USTRUCT(BlueprintType)
struct FMediaPipeFaceMesh
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe Face Mesh")
	FTransform Pose;

	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe Face Mesh")
	TArray<FMediaPipeVertex> Vertices;

	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe Face Mesh")
	TArray<int> Indices;
};

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipeFaceMeshObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipeFaceMeshObserverComponent();

	// Config

	UPROPERTY(Category="MediaPipe|FaceMesh", BlueprintReadWrite, EditAnywhere)
	float UniformScale = 1.f;

	UPROPERTY(Category="MediaPipe|FaceMesh", BlueprintReadWrite, EditAnywhere)
	bool bFlipHorizontal = false;

	// Getters

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const FMediaPipeFaceMesh& GetMesh(int MeshId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const FTransform& GetMeshPose(int MeshId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const FMediaPipeVertex& GetMeshVertex(int MeshId, int VertexId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetMesh(int MeshId, FMediaPipeFaceMesh& Mesh);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetMeshPose(int MeshId, FTransform& Pose);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetMeshVertex(int MeshId, int VertexId, FMediaPipeVertex& Vertex);

	// Utils

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void DrawDebugMeshVertices(int MeshId, const FTransform& Transform, float PrimitiveScale = 1.0f, FLinearColor Color = FLinearColor(0, 1, 0, 1));

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;

	TArray<FMediaPipeFaceMesh> Meshes;
};
