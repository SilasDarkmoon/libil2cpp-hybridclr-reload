$src = "d:\MyFiles\gitroot\public\libil2cpp-hybridclr-reload\.codebuddy\ppt"
Copy-Item "$src\*.pptx" "$src\reload_share.pptx" -Force
$outDir = "$src\slides"
if (Test-Path $outDir) { Remove-Item $outDir -Recurse -Force }
New-Item -ItemType Directory -Path $outDir | Out-Null
$ppt = New-Object -ComObject PowerPoint.Application
try {
  $pres = $ppt.Presentations.Open("$src\reload_share.pptx", $true, $false, $false)
  $i = 1
  foreach ($slide in $pres.Slides) {
    $slide.Export("$outDir\slide-$('{0:d2}' -f $i).png", "PNG", 1600, 900)
    $i++
  }
  $pres.Close()
  Write-Output "Exported $($i-1) slides"
} finally {
  $ppt.Quit()
  [System.Runtime.Interopservices.Marshal]::ReleaseComObject($ppt) | Out-Null
}
Get-ChildItem $outDir | Select-Object Name, Length | Format-Table -AutoSize
