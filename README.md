# Asset Linter - 虚幻引擎资产验证管线

Asset Linter 是一款专为 Unreal Engine 5 构建的 C++ 静态分析与资产合规性验证插件。

它通过拦截编辑器中资产保存（`PreSave`）的底层生命周期，在数据落盘前强制执行项目规范与性能审查，彻底阻断不合规资产、蓝图性能黑洞以及 GPU 显存泄漏流入代码仓库。

## 核心防线

### 1. 命名规范

利用虚幻引擎底层的反射机制实现真正的 $O(1)$ 继承链匹配。

- **多态校验**：仅需对基类（如 `UTexture`）配置规则，所有派生类将自动继承约束。
- **防火墙式降维寻址**：采用自顶向下的匹配队列，支持父子类兜底机制。
- **零内存开销**：通过 `TSoftClassPtr` 软指针与动态哈希去重，彻底消除配置表的冗余驻留。

### 2. 蓝图 AST 抽象语法树分析

直接解包蓝图的底层节点图谱（`UEdGraph`），在字节码虚拟机编译前斩断性能毒瘤：

- **悬空 Tick 拦截**：精准定位未连接的空 `EventTick` 节点，切断 C++ 到 VM 的无效栈帧上下文切换，保护 CPU L1 指令缓存。
- **主线程 I/O 锁死拦截**：禁止使用 `LoadAsset_Blocking` 等同步阻塞节点，强制推动异步加载（Async Load）架构，消灭运行期卡顿。

### 3. GPU 渲染管线与显存审计

利用纯粹的汇编级位运算，在 ALU 寄存器层面秒杀贴图参数校验，保护渲染管线：

- **非 2 的幂(NPOT)拦截**：强制要求纹理分辨率符合 $2^n$，确保底层图形 API 能够生成完整的 Mipmap 金字塔，避免显存带宽被彻底击穿。
- **sRGB 污染隔离**：拦截开启了 sRGB 的法线贴图，防止硬件级的 Gamma 曲线扭曲破坏法线向量的纯线性光照点乘计算。

## 安装与部署

1. 将 `AssetLinter` 文件夹放置在你的项目 `Plugins/` 目录下，如：`YourProject/Plugins/AssetLinter/`
2. 右键点击项目的 `.uproject` 文件，选择 **Generate Visual Studio project files**。
3. 打开 IDE（Visual Studio / Rider）并编译项目。
4. 启动虚幻引擎编辑器，在 `编辑 -> 插件` 中确认 `Asset Linter` 已启用。

## 全局配置

Asset Linter 采用完全的模块化与数据驱动设计。所有的规则均可在引擎的 **项目设置(Project Settings)** 中进行可视化配置。

- **配置入口**：`编辑 -> 项目设置 -> 插件 -> Asset Linter 资产验证器`

### 配置项说明

- **全局命名规则列表**：在此处添加基于类的命名要求（例如 `UTexture2D` 必须以 `T_` 开头）。系统会在 CDO 初始化时注入默认的预设。
- **拦截非 2 的幂次方贴图**：默认开启。强烈建议不要关闭，守护 GPU 纹理缓存命中率。
- **拦截法线贴图 sRGB 污染**：默认开启。防止法线贴图被施加非线性的色彩空间校正。

## 架构与性能优势

- **Editor-Only 隔离**：本插件在 `.uplugin` 中被严格标记为 `Editor` 模块。它的任何验证逻辑都不会被打包进 Shipping 构建中，对 游戏运行时(Runtime) 性能做到 **0 开销**。
- **Data Validation Subsystem 集成**：原生无缝接入 UE5 的数据验证子系统，支持在 Content Browser 中批量右键选择 `Validate Assets` 执行全量工程扫描。
- **Zero Magic Statics**：所有 C++ 验证器核心循环中的反射字符串比较，均通过类成员提升与常量指针偏移完成，彻底消除了隐式的线程安全锁开销。

## 许可

[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
![GitHub](https://img.shields.io/github/license/LJason77/AssetLinter)
