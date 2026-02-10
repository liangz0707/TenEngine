#!/bin/bash
# TenEngine-012-mesh 构建脚本
# 用途：配置、构建和测试 Mesh 模块

set -e

CLEAN=false
TEST=true
CONFIG="Release"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --no-test)
            TEST=false
            shift
            ;;
        --config)
            CONFIG="$2"
            shift 2
            ;;
        *)
            echo "未知参数: $1"
            exit 1
            ;;
    esac
done

echo "=== TenEngine-012-mesh 构建脚本 ==="

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 清理构建目录
if [ "$CLEAN" = true ]; then
    echo ""
    echo "清理构建目录..."
    if [ -d "build" ]; then
        rm -rf build
        echo "构建目录已清理"
    fi
fi

# 配置 CMake
echo ""
echo "配置 CMake..."
cmake -B build \
    -DTENENGINE_USE_ASSIMP=ON \
    -DTENENGINE_USE_FAST_OBJ=ON \
    -DTENENGINE_USE_CGLTF=ON

if [ $? -ne 0 ]; then
    echo "CMake 配置失败！"
    exit 1
fi
echo "CMake 配置成功"

# 构建项目
echo ""
echo "构建项目 (配置: $CONFIG)..."
cmake --build build --config "$CONFIG"

if [ $? -ne 0 ]; then
    echo "构建失败！"
    exit 1
fi
echo "构建成功"

# 验证构建产物
echo ""
echo "验证构建产物..."
if [ -f "build/$CONFIG/te_mesh.lib" ] || [ -f "build/$CONFIG/libte_mesh.a" ]; then
    echo "  ✓ te_mesh 库文件"
else
    echo "  ✗ te_mesh 库文件未找到"
fi

if [ -f "build/tests/unit/$CONFIG/test_mesh_api" ] || [ -f "build/tests/unit/$CONFIG/test_mesh_api.exe" ]; then
    echo "  ✓ 单元测试可执行文件"
else
    echo "  ✗ 单元测试可执行文件未找到"
fi

if [ -f "build/tests/integration/$CONFIG/test_mesh_import" ] || [ -f "build/tests/integration/$CONFIG/test_mesh_import.exe" ]; then
    echo "  ✓ 集成测试可执行文件"
else
    echo "  ✗ 集成测试可执行文件未找到"
fi

# 运行测试
if [ "$TEST" = true ]; then
    echo ""
    echo "运行测试..."
    cd build
    
    echo ""
    echo "运行单元测试..."
    ctest -C "$CONFIG" --test-dir tests/unit --output-on-failure
    
    echo ""
    echo "运行集成测试..."
    ctest -C "$CONFIG" --test-dir tests/integration --output-on-failure
    
    cd ..
fi

echo ""
echo "=== 构建完成 ==="
