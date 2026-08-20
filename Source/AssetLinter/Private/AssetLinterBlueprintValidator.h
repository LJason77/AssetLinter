// Copyright (c) 2026, LJason. All Rights Reserved.

#pragma once

#include "EditorValidatorBase.h"
#include "AssetLinterBlueprintValidator.generated.h"

class UEdGraph;
class UK2Node_CallFunction;
class UK2Node_Event;

/**
 * 蓝图图谱静态分析验证器。
 * 深入蓝图 AST 拦截空 Tick 与同步阻塞加载等性能陷阱。
 */
UCLASS(DisplayName = "蓝图静态分析器")
class ASSETLINTER_API UAssetLinterBlueprintValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

    const FName Name_ReceiveBeginPlay;
    const FName Name_ReceiveTick;
    const FName Name_ReceiveActorBeginOverlap;
    const FName Name_LoadAsset_Blocking;
    const FName Name_LoadClassAsset_Blocking;

    void ValidateEventNode(const UK2Node_Event *EventNode, const UEdGraph *Graph, const UObject *InAsset, bool &bOutHasError);
    void ValidateCallFunctionNode(const UK2Node_CallFunction *CallNode, const UEdGraph *Graph, const UObject *InAsset, bool &bOutHasError);

    static bool IsEmptyNode(const UK2Node_Event *EventNode);

protected:
#pragma region Override
    virtual bool CanValidateAsset_Implementation(const FAssetData &InAssetData, UObject *InObject, FDataValidationContext &InContext) const override;

    virtual EDataValidationResult
        ValidateLoadedAsset_Implementation(const FAssetData &InAssetData, UObject *InAsset, FDataValidationContext &Context) override;
#pragma endregion

public:
    UAssetLinterBlueprintValidator();
};