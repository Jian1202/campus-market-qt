param([Parameter(Mandatory=$true)][string]$SourceRoot,[Parameter(Mandatory=$true)][string]$OutputDir)
$ErrorActionPreference='Stop'
function Read-Ini([string]$File) {
 $values=@{}
 if(Test-Path -LiteralPath $File){
  foreach($line in [IO.File]::ReadAllLines($File)){
   if($line -match '^\s*([^;#\[=]+?)\s*=\s*(.*?)\s*$'){$values[$matches[1].Trim()]=$matches[2]}
  }
 }
 return $values
}
$config=Join-Path $OutputDir 'database.ini'
$existing=Read-Ini $config
$isSample=($existing.Count -eq 0) -or ($existing['user'] -eq 'campus_app' -and $existing['password'] -eq '请填写实验数据库密码')
if(-not $isSample){Write-Host '已保留现有数据库连接配置。';return}
$replacement=$null
foreach($bundle in @((Join-Path (Split-Path $SourceRoot -Parent) '运行程序'),(Join-Path $SourceRoot '运行程序'))){
 $from=Join-Path $bundle 'database.ini'
 $candidate=Read-Ini $from
 if($candidate['password'] -and $candidate['password'] -ne '请填写实验数据库密码'){
  $replacement=[IO.File]::ReadAllText($from);break
 }
}
if(-not $replacement){
 $publicRoot = if($env:PUBLIC){$env:PUBLIC}else{'C:\Users\Public'}
 $stateDir=Join-Path $publicRoot 'CampusMarketDemo'
 $stateFile=Join-Path $stateDir 'instance.json'
 if(Test-Path -LiteralPath $stateFile){
  $saved=Get-Content -LiteralPath $stateFile -Raw -Encoding UTF8 | ConvertFrom-Json
  if($saved.app_user -and $saved.app_password -and $saved.database -and $saved.port -and (Test-Path -LiteralPath (Join-Path $stateDir 'data'))){
   $replacement="[mysql]`nhost=127.0.0.1`nport=$($saved.port)`ndatabase=$($saved.database)`nuser=$($saved.app_user)`npassword=$($saved.app_password)`n"
  }
 }
}
if($replacement){
 if(Test-Path -LiteralPath $config){
  $archive=Join-Path (Join-Path $OutputDir '存档') (Get-Date -Format 'yyyyMMdd-HHmmss-fffffff')
  New-Item -ItemType Directory -Path $archive -Force | Out-Null
  Move-Item -LiteralPath $config -Destination (Join-Path $archive 'database.ini')
 }
 New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
 [IO.File]::WriteAllText($config,$replacement,(New-Object Text.UTF8Encoding($false)))
 Write-Host '已将旧样例配置更新为本机已初始化的项目数据库配置。'
}else{
 if(-not(Test-Path -LiteralPath $config)){Copy-Item -LiteralPath (Join-Path $SourceRoot 'database.ini.example') -Destination $config}
 Write-Host '尚未初始化数据库；请先运行输出目录中的“运行演示.cmd”。'
}
