param([Parameter(Mandatory=$true)][string]$SourceRoot,[Parameter(Mandatory=$true)][string]$OutputDir,[string]$QtVersion='6.11.1',[string]$Architecture='x86_64')
$ErrorActionPreference='Stop'
try {
 if($QtVersion -ne '6.11.1' -or $Architecture -ne 'x86_64'){throw '随包驱动适用于 Qt 6.11.1 MinGW 64 位；其他版本需重新构建 QMYSQL。'}
 $SourceRoot=[IO.Path]::GetFullPath($SourceRoot)
 $OutputDir=[IO.Path]::GetFullPath($OutputDir)
 $bundle=$null
 foreach($candidate in @((Join-Path (Split-Path $SourceRoot -Parent) '运行程序'),(Join-Path $SourceRoot '运行程序'))){
  if(Test-Path -LiteralPath (Join-Path $candidate 'sqldrivers/qsqlmysql.dll')){$bundle=$candidate;break}
 }
 if(-not $bundle){throw '找不到运行库，请将“源码”和“运行程序”保留在同一个目录。'}
 if([IO.Path]::GetFullPath($bundle).TrimEnd('\') -eq $OutputDir.TrimEnd('\')){throw '请不要直接在随包运行目录构建。'}
 New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
 # 不覆盖编译出的程序及用户已有数据库配置。
 foreach($f in Get-ChildItem -LiteralPath $bundle -File -Filter '*.dll'){Copy-Item -LiteralPath $f.FullName -Destination $OutputDir -Force}
 foreach($name in @('sqldrivers','platforms','styles','imageformats','iconengines','generic','networkinformation','tls','assets','licenses','tools','database')){
  $from=Join-Path $bundle $name
  if(Test-Path -LiteralPath $from){Copy-Item -LiteralPath $from -Destination $OutputDir -Recurse -Force}
 }
 foreach($name in @('运行演示.cmd','requirements.txt','database.ini.example')){
  $from=Join-Path $bundle $name
  if(Test-Path -LiteralPath $from){Copy-Item -LiteralPath $from -Destination $OutputDir -Force}
 }
 & (Join-Path $SourceRoot 'tools/configure_database.ps1') -SourceRoot $SourceRoot -OutputDir $OutputDir
 Write-Host ('MySQL 驱动和依赖已部署到：'+$OutputDir)
 Write-Host '首次演示请双击输出目录中的“运行演示.cmd”；连接自有数据库请先初始化和配置。'
}catch{Write-Error $_;exit 1}
