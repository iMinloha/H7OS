#!/usr/bin/env python3
"""
CubeMX 生成代码清理脚本
用法: python clean_cubemx.py [board_name]

自动删除 CubeMX 生成的冗余目录和文件:
  - Core/         (项目顶层已有)
  - FATFS/        (项目顶层已有)
  - Middlewares/   (项目顶层已有)
  - USB_DEVICE/    (项目顶层已有)
  - Utilities/     (项目顶层已有)
  - Drivers/CMSIS/.../Source/Templates/  (不需要的模板)

保留:
  - BSP/           (手写板级驱动)
  - Drivers/       (CMSIS + HAL 库)
  - Inc/  Src/  Startup/  (外设代码)
  - *.ioc  *.ld    (工程文件)
"""

import os
import sys
import shutil
from pathlib import Path

# ── 配置 ──────────────────────────────────────────────
# 项目顶层已存在、不需要在板级目录下重复的目录
REDUNDANT_TOP_DIRS = [
    "Core",
    "FATFS",
    "Middlewares",
    "USB_DEVICE",
    "Utilities",
]

# CMSIS 下不需要的目录
CMSIS_CLEAN_DIRS = [
    "Drivers/CMSIS/Device/ST/STM32H7xx/Source",
]

# CubeMX 可能生成但不需要的多余文件
REDUNDANT_FILES = [
    "Src/main.c",          # 项目入口在 Core/Src/main.c
    "Src/main.h",
    "MXTmpFiles",          # CubeMX 临时文件
]

# CubeMX 可能生成但不需要的目录模式
REDUNDANT_DIR_PATTERNS = [
    "MXTmpFiles",
]

# ── 清理逻辑 ───────────────────────────────────────────

def clean_board(board_path: Path, dry_run: bool = False) -> list[str]:
    """清理指定板级目录, 返回删除列表"""
    deleted = []

    # 1. 删除顶层冗余目录
    for d in REDUNDANT_TOP_DIRS:
        target = board_path / d
        if target.is_dir():
            if not dry_run:
                shutil.rmtree(target)
            deleted.append(f"[DIR]  {target.relative_to(board_path.parent)}")

    # 2. 删除 CMSIS 下的多余目录
    for d in CMSIS_CLEAN_DIRS:
        target = board_path / d
        if target.is_dir():
            if not dry_run:
                shutil.rmtree(target)
            deleted.append(f"[DIR]  {target.relative_to(board_path.parent)}")

    # 3. 删除多余文件
    for f in REDUNDANT_FILES:
        target = board_path / f
        if target.is_file():
            if not dry_run:
                target.unlink()
            deleted.append(f"[FILE] {target.relative_to(board_path.parent)}")

    # 4. 删除匹配模式的多余目录
    for pattern in REDUNDANT_DIR_PATTERNS:
        for target in board_path.rglob(pattern):
            if target.is_dir():
                if not dry_run:
                    shutil.rmtree(target)
                deleted.append(f"[DIR]  {target.relative_to(board_path.parent)}")

    return deleted


def main():
    # 确定 HAL 根目录 (脚本所在目录)
    hal_root = Path(__file__).resolve().parent

    # 获取板子名称
    if len(sys.argv) > 1:
        board_name = sys.argv[1]
    else:
        # 自动检测: 排除 _template 和非目录
        boards = [d.name for d in hal_root.iterdir()
                  if d.is_dir() and d.name != "_template"
                  and not d.name.startswith(".")
                  and not d.name.endswith("-file.md")]
        if not boards:
            print("错误: 未找到板级目录, 请指定板子名称")
            print(f"用法: python {Path(__file__).name} [board_name]")
            sys.exit(1)
        board_name = boards[0]

    board_path = hal_root / board_name
    if not board_path.is_dir():
        print(f"错误: 板级目录不存在: {board_path}")
        sys.exit(1)

    print(f"清理板级目录: {board_name}")
    print(f"路径: {board_path}")
    print()

    # 预览
    deleted = clean_board(board_path, dry_run=True)
    if not deleted:
        print("没有需要清理的冗余文件/目录")
        return

    print("将删除以下冗余内容:")
    for item in deleted:
        print(f"  {item}")
    print()

    # 确认
    response = input("确认清理? [y/N]: ").strip().lower()
    if response != 'y':
        print("已取消")
        return

    # 执行清理
    deleted = clean_board(board_path, dry_run=False)
    print(f"\n已清理 {len(deleted)} 项:")
    for item in deleted:
        print(f"  {item}")

    print(f"\n板级目录 {board_name} 清理完成 ✓")


if __name__ == "__main__":
    main()
