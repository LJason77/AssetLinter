// Copyright (c) 2026, LJason. All Rights Reserved.


#include "AssetLinterSettings.h"

#include "UObject/UnrealType.h"


UAssetLinterSettings::UAssetLinterSettings()
{
    // 初始化贴图检查默认开启
    bCheckPowerOfTwo = true;
    bCheckNormalMapSRGB = true;

    // 默认预设 1: 贴图(Texture) 必须以 T_ 开头
    FAssetLinterNamingRule TextureRule;
    // 使用 FSoftObjectPath 避免在 CDO 阶段引起 Engine 模块的强制硬加载
    TextureRule.AssetClass = TSoftClassPtr(FSoftObjectPath(TEXT("/Script/Engine.Texture")));
    TextureRule.RequiredPrefix = TEXT("T_");
    NamingRules.Add(TextureRule);

    // 默认预设 2: 材质(Material) 必须以 M_ 开头
    FAssetLinterNamingRule MaterialRule;
    MaterialRule.AssetClass = TSoftClassPtr(FSoftObjectPath(TEXT("/Script/Engine.Material")));
    MaterialRule.RequiredPrefix = TEXT("M_");
    NamingRules.Add(MaterialRule);

    // 默认预设 3: 蓝图类 (Blueprint) 必须以 BP_ 开头
    FAssetLinterNamingRule BlueprintRule;
    BlueprintRule.AssetClass = TSoftClassPtr(FSoftObjectPath(TEXT("/Script/Engine.Blueprint")));
    BlueprintRule.RequiredPrefix = TEXT("BP_");
    NamingRules.Add(BlueprintRule);

    // 默认预设 4: 静态网格体 (StaticMesh) 必须以 SM_ 开头
    FAssetLinterNamingRule StaticMeshRule;
    StaticMeshRule.AssetClass = TSoftClassPtr(FSoftObjectPath(TEXT("/Script/Engine.StaticMesh")));
    StaticMeshRule.RequiredPrefix = TEXT("SM_");
    NamingRules.Add(StaticMeshRule);
}

#pragma region Override
#if WITH_EDITOR
void UAssetLinterSettings::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 如果修改的是 NamingRules 数组，执行 $O(N^2)$ 的暴力去重（配置表数据量极小，完全不会造成卡顿）
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UAssetLinterSettings, NamingRules))
    {
        TSet<FString> ExistingClasses;
        ExistingClasses.Reserve(NamingRules.Num());

        for (int32 Index = 0; Index < NamingRules.Num(); ++Index)
        {
            // 如果这个类在数组上方已经被定义过，则移除当前冗余项
            if (FString ClassPath = NamingRules[Index].AssetClass.ToString(); ExistingClasses.Contains(ClassPath))
            {
                NamingRules.RemoveAt(Index);
                // 由于删除了当前元素，数组发生前移，必须回退一次索引以防漏判
                --Index;
            }
            else
            {
                ExistingClasses.Add(ClassPath);
            }
        }
    }
}
#endif
#pragma endregion