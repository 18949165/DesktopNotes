# scripts/package-portable.ps1
# 单目录绿色版打包脚本
# 用法: powershell -ExecutionPolicy Bypass -File scripts\package-portable.ps1

$ErrorActionPreference = "Stop"
$root   = Resolve-Path (Join-Path $PSScriptRoot "..")
$build  = Join-Path $root "build-release"
$dist   = Join-Path $root "dist\portable\StickyNotes-1.0.0"

# 1) Release 构建
Write-Host "==> [1/5] Release 构建..." -ForegroundColor Cyan
if (-not (Test-Path $build)) { cmake -B $build -DCMAKE_BUILD_TYPE=Release | Out-Null }
cmake --build $build --config Release --target StickyNotes | Out-Null
if ($LASTEXITCODE -ne 0) { throw "构建失败" }

# 2) 清空旧产物
Write-Host "==> [2/5] 准备输出目录..." -ForegroundColor Cyan
if (Test-Path $dist) { Remove-Item -Recurse -Force $dist }
New-Item -ItemType Directory -Path $dist | Out-Null

# 3) 拷贝 exe
Write-Host "==> [3/5] 拷贝可执行文件..." -ForegroundColor Cyan
$exe = Get-ChildItem -Path $build -Recurse -Filter "StickyNotes.exe" |
       Where-Object { $_.DirectoryName -match "Release" } | Select-Object -First 1
if (-not $exe) { throw "找不到 StickyNotes.exe" }
Copy-Item $exe.FullName $dist

# 4) windeployqt 拉 Qt 运行时
Write-Host "==> [4/5] windeployqt 部署 Qt 依赖..." -ForegroundColor Cyan
$qtBin = (Get-Command windeployqt.exe -ErrorAction SilentlyContinue).Source
if (-not $qtBin) {
    # 常见 Qt 安装路径兜底
    $candidates = @(
        "C:\Qt\6.7.3\msvc2022_64\bin\windeployqt6.exe",
        "C:\Qt\6.7.2\msvc2022_64\bin\windeployqt6.exe",
        "C:\Qt\6.6.0\msvc2022_64\bin\windeployqt6.exe"
    )
    foreach ($c in $candidates) { if (Test-Path $c) { $qtBin = $c; break } }
}
if (-not $qtBin) {
    Write-Warning "找不到 windeployqt。请把 Qt bin 加入 PATH,或运行 vcvars64.bat 后再试。"
} else {
    Write-Host "    using $qtBin"
    & $qtBin --release --qmldir $root --no-translations --no-system-d3d-compiler $dist\StickyNotes.exe | Out-Null
}

# 5) 拷贝 ElaWidgetTools 的 DLL + plugins
Write-Host "==> [5/5] 拷贝 ElaWidgetTools 运行时..." -ForegroundColor Cyan
$ela = Join-Path $build "_deps\ela-src"   # FetchContent 解压目录
$elaPlugins = Join-Path $ela "ElaWidgetTools\Plugins"
if (Test-Path $elaPlugins) {
    Copy-Item -Recurse -Force $elaPlugins (Join-Path $dist "plugins")
}
# 找 ElaWidgetTools.dll（Release 版）
$elaDll = Get-ChildItem -Path $build -Recurse -Filter "ElaWidgetTools.dll" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($elaDll) {
    Copy-Item $elaDll.FullName $dist
} else {
    Write-Warning "找不到 ElaWidgetTools.dll,运行时可能缺库"
}

Write-Host ""
Write-Host "==> 打包完成: $dist" -ForegroundColor Green
Write-Host "    双击 StickyNotes.exe 运行(无需安装)"
Get-ChildItem $dist | Select-Object Name, Length | Format-Table
