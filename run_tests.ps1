param([string]$label = "")

$root = "c:\Users\athan\Desktop\CSD\CompilersOnGit"
$compiler = "$root\compiler.exe"
$vm       = "$root\vm.exe"
$binary   = "$root\binary.abc"
$testsDir = "$root\tests"

# Tests that don't require user input and should compile + run cleanly
$tests = @(
    "calc.asc",
    "basic_simple.asc",
    "Circle.asc",
    "queens.asc",
    "hercules.asc",
    "complete.asc",
    "delegation.asc",
    "ShadowedFunctions.asc",
    "tables1.asc",
    "tables2.asc",
    "tables3.asc"
)

# Error tests – should fail to compile (non-zero exit)
$errTests = @("err1.asc","err2.asc","err3.asc","err4.asc","err5.asc","err6.asc")

if ($label) { Write-Host "=== $label ===" -ForegroundColor Yellow }

$pass = 0; $fail = 0; $skip = 0

foreach ($t in $tests) {
    $f = Join-Path $testsDir $t
    if (-not (Test-Path $f)) { $skip++; continue }

    $ce = & $compiler $f 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL (compile) $t" -ForegroundColor Red
        $fail++; continue
    }
    $re = & $vm $binary 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL (runtime) $t" -ForegroundColor Red
        Write-Host "  $re" -ForegroundColor DarkRed
        $fail++
    } else {
        Write-Host "PASS          $t" -ForegroundColor Green
        $pass++
    }
}

foreach ($t in $errTests) {
    $f = Join-Path $testsDir $t
    if (-not (Test-Path $f)) { $skip++; continue }
    $ce = & $compiler $f 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL (compile) $t" -ForegroundColor Red; $fail++; continue
    }
    $re = & $vm $binary 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "PASS (runtime-err) $t" -ForegroundColor Green
        $pass++
    } else {
        Write-Host "FAIL (expected runtime error) $t" -ForegroundColor Red
        $fail++
    }
}

Write-Host ""
Write-Host "Results: $pass passed, $fail failed, $skip skipped" -ForegroundColor Cyan
