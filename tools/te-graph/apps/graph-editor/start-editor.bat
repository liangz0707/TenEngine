@echo off
chcp 65001 >nul
REM 启动 TE Graph Editor（将 Cargo 加入 PATH）
set "CARGO_BIN=%USERPROFILE%\.cargo\bin"
set "GIT_BIN=C:\Program Files\Git\bin"
if exist "%CARGO_BIN%\cargo.exe" (
  set "PATH=%CARGO_BIN%;%GIT_BIN%;%PATH%"
  echo [OK] 使用 Cargo: %CARGO_BIN%\cargo.exe
) else (
  echo.
  echo [错误] 未找到 Cargo。Tauri 需要 Rust 工具链。
  echo.
  echo 请先安装 Rust:
  echo   1. 打开 https://rustup.rs/
  echo   2. 下载并运行 rustup-init.exe
  echo   3. 按提示安装（默认选项即可）
  echo   4. 安装完成后**关闭并重新打开**本窗口，再双击此脚本
  echo.
  echo 若已安装 Rust 但仍报错，请把 %%USERPROFILE%%\.cargo\bin 加入系统 PATH。
  echo 当前检查路径: %CARGO_BIN%
  echo.
  pause
  exit /b 1
)

cd /d "%~dp0"
echo Starting TE Graph Editor...
call npm run tauri dev
pause
