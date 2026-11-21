param(
    [Parameter(Mandatory = $true)][string]$SourceRoot,
    [Parameter(Mandatory = $true)][string]$TargetRoot
)
$ErrorActionPreference = 'Stop'

function Copy-Folder([string]$relative) {
    $src = Join-Path $SourceRoot $relative
    $dst = Join-Path $TargetRoot $relative
    if (-not (Test-Path -LiteralPath $src -PathType Container)) {
        Write-Warning "Ogre media folder absent: $src"
        return
    }
    New-Item -ItemType Directory -Path $dst -Force | Out-Null
    Copy-Item (Join-Path $src '*') -Destination $dst -Recurse -Force
}

# Ogre HLMS needs its shader templates at runtime, in these relative paths.
Copy-Folder 'Hlms/Common'
Copy-Folder 'Hlms/Pbs'
Copy-Folder 'Hlms/Unlit'

# Media used by the current Sandbox cube and texture sample.
Copy-Folder 'packs/DebugPack'
Copy-Folder 'materials/textures'
Copy-Folder '2.0/scripts/materials/Common'
Write-Host "Ogre media copied to $TargetRoot"
