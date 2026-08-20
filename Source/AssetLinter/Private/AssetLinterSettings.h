// Copyright (c) 2026, LJason. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "AssetLinterSettings.generated.h"

/**
 * 资产命名规则单体定义 (数据驱动模式)
 */
USTRUCT(BlueprintType, DisplayName = "资产命名规则条目")
struct FAssetLinterNamingRule
{
    GENERATED_BODY()

    /**
     * 目标资产类。
     * 使用 TSoftClassPtr 避免将目标类强行加载进内存，同时支持项目中的所有自定义 UObject 派生类。
     */
    UPROPERTY(EditAnywhere, Config, DisplayName = "目标资产类", Category = "命名规则", meta = (AllowAbstract = "true"))
    TSoftClassPtr<UObject> AssetClass;

    /** 强制要求的前缀(例如 "T_") */
    UPROPERTY(EditAnywhere, Config, DisplayName = "强制前缀", Category = "命名规则")
    FString RequiredPrefix;

    /** 强制要求的后缀(例如 "_Inst")，留空则不检查 */
    UPROPERTY(EditAnywhere, Config, DisplayName = "强制后缀", Category = "命名规则")
    FString RequiredSuffix;
};

/**
 * AssetLinter 全局验证配置。
 */
UCLASS(DisplayName = "Asset Linter 资产验证器", Config = Editor, DefaultConfig)
class UAssetLinterSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UAssetLinterSettings();

#pragma region Override
    /** 获取设置在 Project Settings 中的分类名称 */
    virtual FName GetCategoryName() const override { return FName("Plugins"); }
    /** 获取设置在 Project Settings 中的分节名称 */
    virtual FName GetSectionName() const override { return FName("AssetLinterSettings"); }

#if WITH_EDITOR
    /** 拦截编辑器面板的属性修改事件，用于数据清洗与去重 */
    virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif
#pragma endregion

    /**
     * 全局命名规则列表。
     * 自顶向下匹配，命中即退出。具体子类应置于父类上方。
     */
    UPROPERTY(EditAnywhere, Config, DisplayName = "全局命名规则列表", Category = "命名规范", meta = (TitleProperty = "RequiredPrefix"))
    TArray<FAssetLinterNamingRule> NamingRules;

    /**
     * 是否拦截非 2 的幂次方 (NPOT) 贴图。
     * NPOT 贴图无法生成 Mipmap，会导致严重的 GPU 纹理缓存(T-Cache) 未命中与带宽浪费。
     */
    UPROPERTY(EditAnywhere, Config, DisplayName = "拦截非 2 的幂次方贴图", Category = "贴图与渲染管线")
    bool bCheckPowerOfTwo;

    /**
     * 是否拦截开启了 sRGB 的法线贴图。
     *
     * 法线贴图的 RGB 通道存储的是法线向量的 XYZ 坐标。
     * GPU 在进行 dot(N, L)（法线点乘光照向量）时，必须使用纯粹的线性数值。
     * 如果开启了 sRGB，引擎在采样这层贴图时，底层硬件会自动执行一次 $Color^{2.2}$ 的伽马曲线(Gamma Curve)扭曲。
     * 这会导致原本平滑的法线向量被强行掰弯，模型表面会出现极度不自然的黑色死角和高光接缝。
     */
    UPROPERTY(EditAnywhere, Config, DisplayName = "拦截法线贴图 sRGB 污染", Category = "贴图与渲染管线")
    bool bCheckNormalMapSRGB;
};