##################################################################################################################
# Formats this repo's C/C++ source files with clang-format, skipping generated, vendor, and archived test folders.
##################################################################################################################

$ErrorActionPreference = "Stop"

if (-not (Get-Command "clang-format" -ErrorAction SilentlyContinue)) {
	throw "Required tool 'clang-format' was not found on PATH."
}

$skipFolders = @(
	"build",
	"external",
	"tests_old"
)

$extensions = @(
	"*.c",
	"*.cc",
	"*.cpp",
	"*.cxx",
	"*.h",
	"*.hpp",
	"*.hxx"
)

$files = Get-ChildItem -Path $PSScriptRoot -Recurse -File -Include $extensions |
	Where-Object {
		$relativePath = Resolve-Path -Path $_.FullName -Relative
		foreach ($folder in $skipFolders) {
			if ($relativePath -like ".\$folder\*") {
				return $false
			}
		}
		return $true
	}

foreach ($file in $files) {
	clang-format -i $file.FullName
}

Write-Host "Formatted $($files.Count) C/C++ files."
