$commonDir = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$outputDir = Join-Path $commonDir "Common\generated"
$headerFile = Join-Path $outputDir "build_info.h"

$gitCommand = Get-Command git -ErrorAction SilentlyContinue
$appBuildId = "unknown"

if ($gitCommand) {
    $gitHash = (& git -C $commonDir rev-parse --short=8 HEAD 2>$null)

    if ($LASTEXITCODE -eq 0 -and $gitHash) {
        $gitStatus = @(& git -C $commonDir status --porcelain)
        $dirtySuffix = if ($gitStatus.Count -gt 0) { ".dirty" } else { "" }
        $appBuildId = "g$($gitHash.Trim())$dirtySuffix"
    }
}

$content = "#pragma once`n`n#define APP_BUILD_ID `"$appBuildId`"`n"

New-Item -ItemType Directory -Path $outputDir -Force | Out-Null

$currentContent = if (Test-Path $headerFile) {
    [System.IO.File]::ReadAllText($headerFile)
} else {
    ""
}

if ($currentContent -ne $content) {
    [System.IO.File]::WriteAllText(
        $headerFile,
        $content,
        [System.Text.UTF8Encoding]::new($false)
    )
}
