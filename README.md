# Embedded Control Core (C)

A small, hardware-independent C99 toolkit for embedded control experiments and learning projects.

## Modules

- PID controller with integral and output limits, fixed-step timing, and derivative on measurement.
- First-order low-pass and moving-average filters.
- Setpoint ramping, rate limiting, and range clamping.
- A bounded serial command parser for parameter tuning and basic control commands.
- A host-side closed-loop example with a simulated plant.

## Build and run

Requires GCC and PowerShell on Windows.

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
powershell -ExecutionPolicy Bypass -File .\examples\run_example.ps1
```

The example simulates sensor and actuator behavior on a desktop. It does not demonstrate a connection to a physical board.

## Project layout

```text
src/       Reusable C modules
examples/  Desktop closed-loop example
tests/     Host-side test source and runner
```

The modules are not tied to an MCU or vendor HAL. Board startup, peripheral drivers, pin mappings, and hardware validation must be supplied by the integrating project.

## Scope

This repository contains only the general-purpose control modules, a generic example, and their host-side test source. Contest-specific documents, problem statements, board-specific scaffolding, temporary files, and generated binaries are excluded.

No license file is included, so this repository does not grant reuse or redistribution rights.

---

# 嵌入式控制核心（C）

一组与具体芯片无关的 C99 控制模块，适用于嵌入式控制练习和学习项目。

## 模块

- PID 控制器：支持积分限幅、输出限幅、固定周期参数和基于测量值的微分。
- 一阶低通滤波与滑动平均滤波。
- 目标斜坡、变化率限制和数值限幅。
- 用于参数调节及基本控制命令的串口文本解析器。
- 带模拟对象的桌面闭环示例。

## 编译与运行

需要 GCC 和 PowerShell。在 Windows 下执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
powershell -ExecutionPolicy Bypass -File .\examples\run_example.ps1
```

示例中的传感器和执行器均为桌面模拟，不代表已在实体开发板上验证。

本仓库只包含通用控制模块、桌面示例和主机端测试源码；不含比赛专用资料、题目文本、板级工程占位代码、临时文件或编译产物。接入具体 MCU 时，需要由集成工程补充启动流程、外设驱动、引脚映射和硬件验证。

当前未附带许可证文件；本仓库未授予代码复用或再分发许可。
