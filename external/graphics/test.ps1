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

function Ensure-SimdDependency {
	$externalDir = Join-Path $PSScriptRoot "external"
	$simdDir = Join-Path $externalDir "simd"
	$simdGitDir = Join-Path $simdDir ".git"
	$simdRepo = "https://github.com/MathsTown/simd.git"
	$simdBranch = "main"

	if ((Test-Path -LiteralPath $simdDir) -and -not (Test-Path -LiteralPath $simdGitDir)) {
		throw "external\simd exists but is not a Git checkout. Remove or move it so the test script can clone $simdRepo."
	}

	if (-not (Test-Path -LiteralPath $simdGitDir)) {
		New-Item -ItemType Directory -Force -Path $externalDir | Out-Null
		Invoke-Checked "simd clone" "git" @("clone", "--branch", $simdBranch, "--single-branch", $simdRepo, $simdDir)
		return
	}

	$status = & git -C $simdDir status --porcelain
	if ($LASTEXITCODE -ne 0) {
		throw "simd status failed with exit code $LASTEXITCODE."
	}
	if ($status) {
		throw "external\simd has local changes. This test dependency is expected to be read-only."
	}

	Invoke-Checked "simd fetch" "git" @("-C", $simdDir, "fetch", "--prune", "origin", $simdBranch)

	& git -C $simdDir checkout $simdBranch
	if ($LASTEXITCODE -ne 0) {
		Invoke-Checked "simd checkout" "git" @("-C", $simdDir, "checkout", "-B", $simdBranch, "origin/$simdBranch")
	}

	Invoke-Checked "simd update" "git" @("-C", $simdDir, "pull", "--ff-only", "origin", $simdBranch)
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

Require-Tool "git"
Require-Tool "cmake"
Require-Tool "ctest"
Require-Tool "cl"
Require-Tool "nmake"
Require-Tool "clang-cl"
Require-Tool "ninja"
Require-Tool "g++"
Require-Tool "mingw32-make"

Ensure-SimdDependency

Invoke-TestBuild "msvc" "Ninja" @("-DCMAKE_CXX_COMPILER=cl", "-DCMAKE_BUILD_TYPE=Debug")
Invoke-TestBuild "clangcl" "Ninja" @("-DCMAKE_CXX_COMPILER=clang-cl", "-DCMAKE_BUILD_TYPE=Debug")
Invoke-TestBuild "gcc" "MinGW Makefiles" @("-DCMAKE_CXX_COMPILER=g++", "-DCMAKE_BUILD_TYPE=Debug")

Write-Host ""
Write-Host "All tests passed."
