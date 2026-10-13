$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    New-Item -ItemType Directory -Force -Path "build" | Out-Null

    $exe = "build\test_control_core.exe"
    $sources = @(
        "tests\test_control_core.c",
        "src\control_pid.c",
        "src\control_filter.c",
        "src\control_ramp.c",
        "src\control_tune.c"
    )

    gcc -std=c99 -Wall -Wextra -Werror -I "src" @sources -lm -o $exe
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    & ".\$exe"
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
