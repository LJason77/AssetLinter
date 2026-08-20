// Copyright (c) 2026, LJason. All Rights Reserved.


#include "AssetLinterBlueprintValidator.h"

#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"


UAssetLinterBlueprintValidator::UAssetLinterBlueprintValidator()
    : Name_ReceiveBeginPlay(TEXT("ReceiveBeginPlay"))
      , Name_ReceiveTick(TEXT("ReceiveTick"))
      , Name_ReceiveActorBeginOverlap(TEXT("ReceiveActorBeginOverlap"))
      , Name_LoadAsset_Blocking(TEXT("LoadAsset_Blocking"))
      , Name_LoadClassAsset_Blocking(TEXT("LoadClassAsset_Blocking"))
{
    bIsEnabled = true;
}

#pragma region Override
bool UAssetLinterBlueprintValidator::CanValidateAsset_Implementation(const FAssetData &InAssetData, UObject *InObject,
                                                                     FDataValidationContext &InContext) const
{
    // 仅拦截蓝图资产，过滤掉原生 C++ 类与普通数据资产
    return InObject != nullptr && InObject->IsA<UBlueprint>();
}

EDataValidationResult UAssetLinterBlueprintValidator::ValidateLoadedAsset_Implementation(const FAssetData &InAssetData, UObject *InAsset,
                                                                                         FDataValidationContext &Context)
{
    const UBlueprint *Blueprint = Cast<UBlueprint>(InAsset);
    if (!Blueprint) return EDataValidationResult::NotValidated;

    bool bHasError = false;

    // 获取该蓝图下所有的图表(涵盖 EventGraph、函数图表、宏等所有 AST 分支)
    TArray<UEdGraph *> AllGraphs;
    Blueprint->GetAllGraphs(AllGraphs);

    // 遍历 AST
    for (const UEdGraph *Graph : AllGraphs)
    {
        if (!Graph) continue;

        for (const UEdGraphNode *Node : Graph->Nodes)
        {
            if (!Node) continue;

            if (const UK2Node_Event *EventNode = Cast<UK2Node_Event>(Node))
            {
                ValidateEventNode(EventNode, Graph, InAsset, bHasError);
            }
            else if (const UK2Node_CallFunction *CallNode = Cast<UK2Node_CallFunction>(Node))
            {
                ValidateCallFunctionNode(CallNode, Graph, InAsset, bHasError);
            }
        }
    }

    if (bHasError) return EDataValidationResult::Invalid;

    AssetPasses(InAsset);
    return EDataValidationResult::Valid;
}
#pragma endregion

#pragma region Private
void UAssetLinterBlueprintValidator::ValidateEventNode(const UK2Node_Event *EventNode, const UEdGraph *Graph, const UObject *InAsset,
                                                       bool &bOutHasError)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName EventName = EventNode->EventReference.GetMemberName();
    if (EventName == Name_ReceiveTick || EventName == Name_ReceiveBeginPlay || EventName == Name_ReceiveActorBeginOverlap)
    {
        if (IsEmptyNode(EventNode))
        {
            // 将导致错误的具体节点名字暴露给用户，提升报错精准度
            const FText ErrorMsg = FText::Format(
                NSLOCTEXT("AssetLinter", "EmptyNode", "检测到未连接的空节点 [{0}]！这会导致毫无意义的虚拟机 (VM) 帧栈开销，请将其删除。图表位置: [{1}]"),
                FText::FromName(EventName),
                FText::FromString(Graph->GetName()));

            AssetFails(InAsset, ErrorMsg);
            bOutHasError = true;
        }
    }
}

void UAssetLinterBlueprintValidator::ValidateCallFunctionNode(
    const UK2Node_CallFunction *CallNode, const UEdGraph *Graph, const UObject *InAsset, bool &bOutHasError)
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName FunctionName = CallNode->FunctionReference.GetMemberName();
    if (FunctionName == Name_LoadAsset_Blocking || FunctionName == Name_LoadClassAsset_Blocking)
    {
        const FText ErrorMsg = FText::Format(
            NSLOCTEXT("AssetLinter", "BlockingLoad", "禁止使用同步阻塞加载节点 [{0}]！该操作会锁死操作系统主线程并引发掉帧。请重构为异步节点(Async Load Asset)。图表位置: [{1}]"),
            FText::FromName(FunctionName),
            FText::FromString(Graph->GetName()));

        AssetFails(InAsset, ErrorMsg);
        bOutHasError = true;
    }
}

bool UAssetLinterBlueprintValidator::IsEmptyNode(const UK2Node_Event *EventNode)
{
    const UEdGraphPin *ExecPin = EventNode->GetThenPin();
    return ExecPin && ExecPin->LinkedTo.IsEmpty();
}
#pragma endregion