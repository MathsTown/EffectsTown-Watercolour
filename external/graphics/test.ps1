$ErrorActionPreference = "Stop"

function Require-Tool {
	param(
		[string]$Name
	)

	if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
		throw "Required tool '$Name' was not found on PATH."
	}
}

function Invoke-Checked {
	param(
		[string]$Label,
		[string]$Program,
		[string[]]$Arguments
	)

	Write-Host ""
	Write-Host "===== $Label ====="
	& $Program @Arguments
	if ($LASTEXITCODE -ne 0) {
		throw "$Label failed with exit code $LASTEXITCODE."
	}
}

function Invoke-TestBuild {
	param(
		[string]$Name,
		[string]$Generator,
		[string[]]$ConfigureArgs
	)

	$sourceDir = Join-Path $PSScriptRoot "tests"
	$buildDir = Join-Path $PSScriptRoot "build\new-tests-$Name"
	$cacheFile = Join-Path $buildDir "CMakeCache.txt"

	if (Test-Path $cacheFile) {
		$cachedGenerator = Select-String -Path $cacheFile -Pattern "^CMAKE_GENERATOR:INTERNAL=(.*)$" | Select-Object -First 1
		if ($cachedGenerator -and $cachedGenerator.Matches[0].Groups[1].Value -ne $Generator) {
			Remove-Item -LiteralPath $buildDir -Recurse -Force
		}
	}

	$baseConfigureArgs = @(
		"-S", $sourceDir,
		"-B", $buildDir,
		"-G", $Generator,
		"-DGRAPHICS_TEST_AVX2=ON"
	)

	Invoke-Checked "$Name configure" "cmake" ($baseConfigureArgs + $ConfigureArgs)
	Invoke-Checked "$Name build" "cmake" @("--build", $buildDir, "--parallel")
	Invoke-Checked "$Name ctest" "ctest" @("--test-dir", $buildDir, "--output-on-failure")
}

Require-Tool "cmake"
Require-Tool "ctest"
Require-Tool "cl"
Require-Tool "nmake"
Require-Tool "clang-cl"
Require-Tool "ninja"
Require-Tool "g++"
Require-Tool "mingw32-make"

Invoke-TestBuild "msvc" "Ninja" @("-DCMAKE_CXX_COMPILER=cl", "-DCMAKE_BUILD_TYPE=Debug")
Invoke-TestBuild "clangcl" "Ninja" @("-DCMAKE_CXX_COMPILER=clang-cl", "-DCMAKE_BUILD_TYPE=Debug")
Invoke-TestBuild "gcc" "MinGW Makefiles" @("-DCMAKE_CXX_COMPILER=g++", "-DCMAKE_BUILD_TYPE=Debug")

Write-Host ""
Write-Host "All tests passed."
