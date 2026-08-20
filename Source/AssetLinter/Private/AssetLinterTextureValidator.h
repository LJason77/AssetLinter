// Copyright (c) 2026, LJason. All Rights Reserved.

#pragma once

#include "EditorValidatorBase.h"
#include "AssetLinterTextureValidator.generated.h"

/**
 * 贴图底层参数静态分析验证器。
 * 拦截 NPOT 与法线 sRGB 污染，保护 GPU 显存与渲染流水线。
 */
UCLASS(DisplayName = "贴图参数分析器")
class UAssetLinterTextureValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

    /** 利用汇编级位运算，极速判定整数是否为 2 的幂次方 */
    FORCEINLINE static bool IsPowerOfTwo(const int32 Value)
    {
        // 任何 2 的幂次方，在二进制中必然只有 1 个 bit 为 1（例如 $1024 = 10000000000_2$）
        // 它减去 1 后（$1023 = 01111111111_2$），两者 按位与(AND) 必定为 0
        return Value > 0 && (Value & Value - 1) == 0;
    }

protected:
#pragma region Override
    virtual bool CanValidateAsset_Implementation(const FAssetData &InAssetData, UObject *InObject, FDataValidationContext &InContext) const override;

    virtual EDataValidationResult
        ValidateLoadedAsset_Implementation(const FAssetData &InAssetData, UObject *InAsset, FDataValidationContext &Context) override;
#pragma endregion

public:
    UAssetLinterTextureValidator();
};