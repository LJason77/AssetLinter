// Copyright (c) 2026, LJason. All Rights Reserved.

#pragma once

#include "EditorValidatorBase.h"
#include "AssetLinterNamingValidator.generated.h"

/**
 * 资产命名规范验证器。
 * 挂载于 Data Validation Subsystem，在保存资产时自动拦截并校验。
 */
UCLASS()
class UAssetLinterNamingValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

protected:
#pragma region Override
    /** 决定该验证器是否需要处理传入的资产 */
    virtual bool CanValidateAsset_Implementation(const FAssetData &InAssetData, UObject *InObject, FDataValidationContext &InContext) const override;

    /** 核心验证逻辑 */
    virtual EDataValidationResult
        ValidateLoadedAsset_Implementation(const FAssetData &InAssetData, UObject *InAsset, FDataValidationContext &Context) override;
#pragma endregion

public:
    UAssetLinterNamingValidator();
};