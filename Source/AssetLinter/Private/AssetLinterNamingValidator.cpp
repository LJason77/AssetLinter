// Copyright (c) 2026, LJason. All Rights Reserved.


#include "AssetLinterNamingValidator.h"

#include "AssetLinterSettings.h"
#include "Engine/Blueprint.h"


UAssetLinterNamingValidator::UAssetLinterNamingValidator()
{
    // 启用验证器
    bIsEnabled = true;
}

#pragma region Override
bool UAssetLinterNamingValidator::CanValidateAsset_Implementation(const FAssetData &InAssetData, UObject *InObject,
                                                                  FDataValidationContext &InContext) const
{
    // 过滤掉内存中游离的临时对象、蓝图生成的 Class Default Object(CDO) 或是未序列化的包
    return InObject != nullptr && InObject->IsAsset();
}

EDataValidationResult UAssetLinterNamingValidator::ValidateLoadedAsset_Implementation(const FAssetData &InAssetData, UObject *InAsset,
                                                                                      FDataValidationContext &Context)
{
    // 获取用户在 Project Settings 中配置的单例 (CDO)
    const UAssetLinterSettings *Settings = GetDefault<UAssetLinterSettings>();
    if (!Settings || Settings->NamingRules.IsEmpty())
    {
        // 如果没有配置任何规则，返回 NotValidated 将验证权交还给引擎其他验证器
        return EDataValidationResult::NotValidated;
    }

    // 提取资产的真实反射类(解包 UBlueprint 映射)
    const UClass *ActualAssetClass = InAsset->GetClass();
    if (const UBlueprint *Blueprint = Cast<UBlueprint>(InAsset))
    {
        if (Blueprint->GeneratedClass)
        {
            ActualAssetClass = Blueprint->GeneratedClass;
        }
        else if (Blueprint->ParentClass)
        {
            ActualAssetClass = Blueprint->ParentClass;
        }
    }

    const FString AssetName = InAsset->GetName();

    // 正向遍历规则数组
    for (const auto &[AssetClass, RequiredPrefix, RequiredSuffix] : Settings->NamingRules)
    {
        // 优先尝试 $O(1)$ 指针获取，避免无谓的路径解析开销
        const UClass *TargetClass = AssetClass.Get();
        if (!TargetClass)
        {
            // 内存未命中时才触发同步加载
            TargetClass = AssetClass.LoadSynchronous();
        }

        // 过滤空规则
        if (!TargetClass) continue;

        // 兼顾 UBlueprint 本身与解包后的真实业务类(IsChildOf)
        // 沿着虚函数表向上比对，判断当前资产是否属于该类或其子类
        if (InAsset->IsA(TargetClass) || ActualAssetClass->IsChildOf(TargetClass))
        {
            bool bIsValid = true;
            FString ErrorMsg;

            // 前缀检查：直接通过 FString 原生 API 执行汇编级字符集比对，比正则表达式快数倍
            if (!RequiredPrefix.IsEmpty() && !AssetName.StartsWith(RequiredPrefix))
            {
                bIsValid = false;
                ErrorMsg += FString::Printf(TEXT("必须以 [%s] 开头！"), *RequiredPrefix);
            }

            // 后缀检查
            if (!RequiredSuffix.IsEmpty() && !AssetName.EndsWith(RequiredSuffix))
            {
                bIsValid = false;
                ErrorMsg += FString::Printf(TEXT("必须以 [%s] 结尾！"), *RequiredSuffix);
            }

            // 提交校验结果
            if (bIsValid)
            {
                // 通知底层该资产通过验证
                AssetPasses(InAsset);
                return EDataValidationResult::Valid;
            }

            // 拼接中文错误详情，将其推入引擎验证错误日志池(会在 UI 弹出红色警告)
            const FText FinalError = FText::Format(
                NSLOCTEXT("AssetLinter", "NamingError", "资产 [{0}] 命名不规范! 目标类型: {1}。{2}"),
                FText::FromString(AssetName),
                FText::FromString(TargetClass->GetName()),
                FText::FromString(ErrorMsg));

            AssetFails(InAsset, FinalError);
            return EDataValidationResult::Invalid;
        }
    }

    // 没有命中任何自定义规则，返回不予验证
    return EDataValidationResult::NotValidated;
}
#pragma endregion