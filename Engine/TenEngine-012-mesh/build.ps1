# TenEngine-012-mesh 构建脚本
# 用途：配置、构建和测试 Mesh 模块

param(
    [switch]$Clean = $false,
    [switch]$Test = $true,
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

Write-Host "=== TenEngine-012-mesh 构建脚本 ===" -ForegroundColor Cyan

# 获取脚本所在目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# 清理构建目录
if ($Clean) {
    Write-Host "`n清理构建目录..." -ForegroundColor Yellow
    if (Test-Path "build") {
        Remove-Item -Path "build" -Recurse -Force
        Write-Host "构建目录已清理" -ForegroundColor Green
    }
}

# 配置 CMake
Write-Host "`n配置 CMake..." -ForegroundColor Yellow
cmake -B build `
    -DTENENGINE_USE_ASSIMP=ON `
    -DTENENGINE_USE_FAST_OBJ=ON `
    -DTENENGINE_USE_CGLTF=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake 配置失败！" -ForegroundColor Red
    exit 1
}
Write-Host "CMake 配置成功" -ForegroundColor Green

# 构建项目
Write-Host "`n构建项目 (配置: $Config)..." -ForegroundColor Yellow
cmake --build build --config $Config

if ($LASTEXITCODE -ne 0) {
    Write-Host "构建失败！" -ForegroundColor Red
    exit 1
}
Write-Host "构建成功" -ForegroundColor Green

# 验证构建产物
Write-Host "`n验证构建产物..." -ForegroundColor Yellow
$libExists = Test-Path "build/$Config/te_mesh.lib"
$unitTestsExist = (Test-Path "build/tests/unit/$Config/test_mesh_api.exe") -and `
                  (Test-Path "build/tests/unit/$Config/test_mesh_resource.exe") -and `
                  (Test-Path "build/tests/unit/$Config/test_mesh_device.exe")
$integrationTestsExist = (Test-Path "build/tests/integration/$Config/test_mesh_import.exe") -and `
                         (Test-Path "build/tests/integration/$Config/test_mesh_load_unload.exe") -and `
                         (Test-Path "build/tests/integration/$Config/test_mesh_gpu_sync_async.exe")

if ($libExists) {
    Write-Host "  ✓ te_mesh.lib" -ForegroundColor Green
} else {
    Write-Host "  ✗ te_mesh.lib 未找到" -ForegroundColor Red
}

if ($unitTestsExist) {
    Write-Host "  ✓ 单元测试可执行文件" -ForegroundColor Green
} else {
    Write-Host "  ✗ 单元测试可执行文件未找到" -ForegroundColor Red
}

if ($integrationTestsExist) {
    Write-Host "  ✓ 集成测试可执行文件" -ForegroundColor Green
} else {
    Write-Host "  ✗ 集成测试可执行文件未找到" -ForegroundColor Red
}

# 运行测试
if ($Test) {
    Write-Host "`n运行测试..." -ForegroundColor Yellow
    Set-Location build
    
    Write-Host "`n运行单元测试..." -ForegroundColor Cyan
    ctest -C $Config --test-dir tests/unit --output-on-failure
    
    Write-Host "`n运行集成测试..." -ForegroundColor Cyan
    ctest -C $Config --test-dir tests/integration --output-on-failure
    
    Set-Location ..
}

Write-Host "`n=== 构建完成 ===" -ForegroundColor Cyan
