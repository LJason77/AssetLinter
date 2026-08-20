// Copyright (c) 2026, LJason. All Rights Reserved.


#include "AssetLinterTextureValidator.h"

#include "AssetLinterSettings.h"
#include "Engine/Texture2D.h"


UAssetLinterTextureValidator::UAssetLinterTextureValidator()
{
    bIsEnabled = true;
}

#pragma region Override
bool UAssetLinterTextureValidator::CanValidateAsset_Implementation(
    const FAssetData &InAssetData, UObject *InObject, FDataValidationContext &InContext) const
{
    // 仅拦截 2D 贴图资产
    return InObject != nullptr && InObject->IsA<UTexture2D>();
}

EDataValidationResult UAssetLinterTextureValidator::ValidateLoadedAsset_Implementation(
    const FAssetData &InAssetData, UObject *InAsset, FDataValidationContext &Context)
{
    const UTexture2D *Texture = Cast<UTexture2D>(InAsset);
    if (!Texture) return EDataValidationResult::NotValidated;

    const UAssetLinterSettings *Settings = GetDefault<UAssetLinterSettings>();
    if (!Settings) return EDataValidationResult::NotValidated;

    bool bHasError = false;

    // 非 2 的幂次方 (NPOT) 检查
    if (Settings->bCheckPowerOfTwo)
    {
        // 提取贴图的真实分辨率
        const int32 SizeX = Texture->GetSizeX();
        const int32 SizeY = Texture->GetSizeY();

        if (!IsPowerOfTwo(SizeX) || !IsPowerOfTwo(SizeY))
        {
            // 仅在发生错误时进行字符串拼接和内存分配
            const FText ErrorMsg = FText::Format(
                NSLOCTEXT("AssetLinter", "NPOT_Error",
                          "致命性能警告：该贴图的分辨率为 {0}x{1}，非 2 的幂次方(NPOT)！会导致 GPU 无法为其生成 Mipmap，严重浪费显存带宽。请将其修改为 512, 1024, 2048 等标准尺寸。"),
                FText::AsNumber(SizeX),
                FText::AsNumber(SizeY));

            AssetFails(InAsset, ErrorMsg);
            bHasError = true;
        }
    }

    // 法线贴图 sRGB 污染检查
    if (Settings->bCheckNormalMapSRGB)
    {
        // 判定依据 1: 压缩设置被标记为法线
        const bool bIsNormalCompression = Texture->CompressionSettings == TC_Normalmap;
        // 判定依据 2: LOD 组被标记为法线
        const bool bIsNormalLODGroup = Texture->LODGroup == TEXTUREGROUP_WorldNormalMap;

        // 如果这是一张 法线贴图，但却错误地开启了 sRGB
        if ((bIsNormalCompression || bIsNormalLODGroup) && Texture->SRGB)
        {
            const FText ErrorMsg = NSLOCTEXT(
                "AssetLinter", "SRGB_Error", "渲染管线错误：检测到法线贴图开启了 [sRGB] 选项！法线数据是数学向量空间 [-1, 1]，不参与人眼伽马校正，开启 sRGB 会导致光照计算彻底崩溃。请双击打开贴图，取消勾选 sRGB。");

            AssetFails(InAsset, ErrorMsg);
            bHasError = true;
        }
    }

    if (bHasError) return EDataValidationResult::Invalid;

    AssetPasses(InAsset);
    return EDataValidationResult::Valid;
}
#pragma endregion