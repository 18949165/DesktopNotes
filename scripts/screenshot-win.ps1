# Capture a specific window by process name WITHOUT stealing focus.
# Uses PrintWindow API (PW_RENDERFULLCONTENT) for off-screen rendering.

param([string]$ProcessName = "StickyNotes",
      [string]$OutPath = "D:\DPracProj\software\build\shot.png")

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

public class WinCap {
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }
}
"@ -ReferencedAssemblies System.Drawing,System.Windows.Forms

$proc = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue |
        Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $proc) { Write-Output "no window for $ProcessName"; exit 1 }
$h = $proc.MainWindowHandle
Write-Output ("hwnd={0} title={1}" -f $h, $proc.MainWindowTitle)

$rect = New-Object WinCap+RECT
[WinCap]::GetWindowRect($h, [ref]$rect) | Out-Null
$w  = $rect.Right  - $rect.Left
$ht = $rect.Bottom - $rect.Top
Write-Output ("rect {0},{1} {2} x {3}" -f $rect.Left,$rect.Top,$w,$ht)
if ($w -lt 1 -or $ht -lt 1) { Write-Output "bad size"; exit 2 }

$bmp = New-Object System.Drawing.Bitmap $w, $ht
$g   = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
$ok  = [WinCap]::PrintWindow($h, $hdc, 0x2)
$g.ReleaseHdc($hdc)
$g.Dispose()
if (-not $ok) { Write-Output "PrintWindow returned false" }

$bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output ("Saved: {0}" -f $OutPath)
