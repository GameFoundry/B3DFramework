#!/bin/bash

# -----------------------------------------------
# Banshee 3D Engine
# Copyright (c) 2025 Marko Pintera. All rights reserved.
# -----------------------------------------------
#
# B3DUploadBinaries.sh - Package and upload dependencies and data to FTP
#
# Usage:
#   ./B3DUploadBinaries.sh <package-name> [options]
#
# Options:
#   --no-upload       Create archive but skip FTP upload
#   --dry-run         Print actions without executing
#   --credentials     Path to FTP credentials file
#   --list            List available packages
#   --check-outdated  Check which packages need to be re-uploaded (slow)
#   --help            Show help message
#
# Environment variables (checked if no --credentials file):
#   B3D_FTP_URL      FTP server URL
#   B3D_FTP_USER     FTP username
#   B3D_FTP_PASS     FTP password

# Script directory
ScriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
FrameworkDir="$(cd "$ScriptDir/.." &> /dev/null && pwd)"
RootDir="$(cd "$FrameworkDir/.." &> /dev/null && pwd)"

# Platform detection
Platform="$OSTYPE"

# Default options
NoUpload=false
DryRun=false
CredentialsFile=""
ListPackages=false
CheckOutdated=false
PackageName=""

# -----------------------------------------------
# Usage/Help
# -----------------------------------------------
show_usage() {
	echo "B3DUploadBinaries - Package and upload dependencies and data to FTP"
	echo ""
	echo "Usage: $0 <package-name> [options]"
	echo ""
	echo "Options:"
	echo "  --no-upload       Create archive but skip FTP upload"
	echo "  --dry-run         Print actions without executing"
	echo "  --credentials     Path to FTP credentials file"
	echo "  --list            List available packages"
	echo "  --check-outdated  Check which packages need to be re-uploaded (slow)"
	echo "  --help            Show this help message"
	echo ""
	echo "Environment variables (checked if no --credentials file):"
	echo "  B3D_FTP_URL     FTP server URL"
	echo "  B3D_FTP_USER    FTP username"
	echo "  B3D_FTP_PASS    FTP password"
	echo ""
	echo "Examples:"
	echo "  $0 --list                       # List all available packages"
	echo "  $0 XShaderCompiler              # Upload dependency"
	echo "  $0 FrameworkData                # Upload Framework/Data"
	echo "  $0 XShaderCompiler --no-upload  # Create archive only"
}

# -----------------------------------------------
# Check if package is outdated
# Returns 0 if outdated, 1 if up-to-date
# -----------------------------------------------
is_package_outdated() {
	local packageDir="$1"
	local versionFile="$packageDir/.version"

	# No .version file means outdated
	if [ ! -f "$versionFile" ]; then
		return 0
	fi

	# Get .version file timestamp (seconds since epoch)
	local versionTime
	versionTime=$(stat -c %Y "$versionFile" 2>/dev/null || stat -f %m "$versionFile" 2>/dev/null)

	# Check if any file is newer than .version by more than 60 seconds
	while IFS= read -r -d '' file; do
		# Skip hidden files and the version file itself
		local basename
		basename=$(basename "$file")
		if [[ "$basename" == .* ]]; then
			continue
		fi

		local fileTime
		fileTime=$(stat -c %Y "$file" 2>/dev/null || stat -f %m "$file" 2>/dev/null)

		# If file is more than 60 seconds newer than .version
		if [ $((fileTime - versionTime)) -gt 60 ]; then
			return 0
		fi
	done < <(find "$packageDir" -type f -print0)

	return 1
}

# -----------------------------------------------
# List Available Packages
# -----------------------------------------------
list_packages() {
	echo "Available packages:"
	echo ""
	echo "=== Dependency Packages (platform-specific) ==="

	# List all directories in Dependencies folder
	if [ -d "$FrameworkDir/Dependencies" ]; then
		for dir in "$FrameworkDir/Dependencies"/*/; do
			if [ -d "$dir" ]; then
				name=$(basename "$dir")
				# Skip hidden directories
				if [[ "$name" != .* ]]; then
					echo "  $name"
				fi
			fi
		done
	else
		echo "  (No dependencies folder found)"
	fi

	echo ""
	echo "=== Data Packages (platform-independent) ==="
	printf "  %-22s -> %s\n" "FrameworkData" "Framework/Data/"
	printf "  %-22s -> %s\n" "FrameworkDataRaw" "Framework/Data/Raw/"
	printf "  %-22s -> %s\n" "FrameworkDocumentation" "Framework/Documentation/"
	printf "  %-22s -> %s\n" "ExampleData" "Framework/Examples/Data/"
	printf "  %-22s -> %s\n" "EditorData" "Data/"
	printf "  %-22s -> %s\n" "EditorDataRaw" "Data/Raw/"
}

# -----------------------------------------------
# Check Outdated Packages
# -----------------------------------------------
check_outdated() {
	echo "Checking for outdated packages (this may take a while)..."
	echo ""

	local foundOutdated=false

	echo "=== Dependency Packages ==="
	if [ -d "$FrameworkDir/Dependencies" ]; then
		# First pass: find longest name for alignment
		local maxLen=0
		for dir in "$FrameworkDir/Dependencies"/*/; do
			if [ -d "$dir" ]; then
				name=$(basename "$dir")
				if [[ "$name" != .* ]] && [ ${#name} -gt $maxLen ]; then
					maxLen=${#name}
				fi
			fi
		done

		# Second pass: check and print outdated
		for dir in "$FrameworkDir/Dependencies"/*/; do
			if [ -d "$dir" ]; then
				name=$(basename "$dir")
				if [[ "$name" != .* ]]; then
					if is_package_outdated "$dir"; then
						printf "  %-${maxLen}s  [OUTDATED]\n" "$name"
						foundOutdated=true
					fi
				fi
			fi
		done
	fi

	echo ""
	echo "=== Data Packages ==="
	if is_package_outdated "$FrameworkDir/Data"; then
		printf "  %-22s  [OUTDATED]\n" "FrameworkData"
		foundOutdated=true
	fi
	if is_package_outdated "$FrameworkDir/Data/Raw"; then
		printf "  %-22s  [OUTDATED]\n" "FrameworkDataRaw"
		foundOutdated=true
	fi
	if is_package_outdated "$FrameworkDir/Documentation"; then
		printf "  %-22s  [OUTDATED]\n" "FrameworkDocumentation"
		foundOutdated=true
	fi
	if is_package_outdated "$FrameworkDir/Examples/Data"; then
		printf "  %-22s  [OUTDATED]\n" "ExampleData"
		foundOutdated=true
	fi
	if is_package_outdated "$RootDir/Data"; then
		printf "  %-22s  [OUTDATED]\n" "EditorData"
		foundOutdated=true
	fi
	if is_package_outdated "$RootDir/Data/Raw"; then
		printf "  %-22s  [OUTDATED]\n" "EditorDataRaw"
		foundOutdated=true
	fi

	echo ""
	if [ "$foundOutdated" = false ]; then
		echo "All packages are up to date."
	fi
}

# -----------------------------------------------
# Parse Arguments
# -----------------------------------------------
while [[ $# -gt 0 ]]; do
	case "$1" in
		--no-upload)
			NoUpload=true
			shift
			;;
		--dry-run)
			DryRun=true
			shift
			;;
		--credentials)
			CredentialsFile="$2"
			shift 2
			;;
		--list)
			ListPackages=true
			shift
			;;
		--check-outdated)
			CheckOutdated=true
			shift
			;;
		--help|-h)
			show_usage
			exit 0
			;;
		-*)
			echo "[Error] Unknown option: $1"
			show_usage
			exit 1
			;;
		*)
			if [ -z "$PackageName" ]; then
				PackageName="$1"
			else
				echo "[Error] Multiple package names provided"
				show_usage
				exit 1
			fi
			shift
			;;
	esac
done

# If --list flag, show available packages
if [ "$ListPackages" = true ]; then
	list_packages
	exit 0
fi

# If --check-outdated flag, check for outdated packages
if [ "$CheckOutdated" = true ]; then
	check_outdated
	exit 0
fi

# If no package name, show help
if [ -z "$PackageName" ]; then
	show_usage
	exit 0
fi

# -----------------------------------------------
# Platform Detection
# -----------------------------------------------
if [[ "$Platform" == "win32" || "$Platform" == "msys" ]]; then
	PlatformSuffix="Win32"
elif [[ "$Platform" == "darwin"* ]]; then
	PlatformSuffix="macOS"
elif [[ "$Platform" == "linux-gnu"* ]]; then
	PlatformSuffix="Linux"
else
	echo "[Error] Unknown platform: $Platform"
	exit 1
fi

echo "Platform: $PlatformSuffix"

# -----------------------------------------------
# Package Resolution
# -----------------------------------------------
IsPlatformSpecific=true

case "$PackageName" in
	FrameworkData)
		PackageFolder="$FrameworkDir/Data"
		ArchivePrefix="FrameworkData"
		IsPlatformSpecific=false
		;;
	FrameworkDataRaw)
		PackageFolder="$FrameworkDir/Data/Raw"
		ArchivePrefix="FrameworkDataRaw"
		IsPlatformSpecific=false
		;;
	FrameworkDocumentation)
		PackageFolder="$FrameworkDir/Documentation"
		ArchivePrefix="FrameworkDocumentation"
		IsPlatformSpecific=false
		;;
	ExampleData)
		PackageFolder="$FrameworkDir/Examples/Data"
		ArchivePrefix="ExampleData"
		IsPlatformSpecific=false
		;;
	EditorData)
		PackageFolder="$RootDir/Data"
		ArchivePrefix="EditorData"
		IsPlatformSpecific=false
		;;
	EditorDataRaw)
		PackageFolder="$RootDir/Data/Raw"
		ArchivePrefix="EditorDataRaw"
		IsPlatformSpecific=false
		;;
	*)
		# Check if it's a dependency
		PackageFolder="$FrameworkDir/Dependencies/$PackageName"
		ArchivePrefix="$PackageName"
		if [ ! -d "$PackageFolder" ]; then
			echo "[Error] Package not found: $PackageName"
			echo ""
			echo "Run with --list to see available packages."
			exit 1
		fi
		;;
esac

echo "Package folder: $PackageFolder"

VersionFile="$PackageFolder/.version"
ReqVersionFile="$PackageFolder/.reqversion"
ManifestFile="$PackageFolder/DataPackageManifest.txt"

# -----------------------------------------------
# Version Management
# -----------------------------------------------
if [ -f "$ReqVersionFile" ]; then
	CurrentVersion=$(cat "$ReqVersionFile")
else
	CurrentVersion=0
fi

NewVersion=$((CurrentVersion + 1))

echo "Current version: $CurrentVersion"
echo "New version: $NewVersion"

# Update .version file (will be included in archive)
if [ "$DryRun" = false ]; then
	echo "$NewVersion" > "$VersionFile"
fi

# -----------------------------------------------
# File List Generation
# -----------------------------------------------
TempDir=$(mktemp -d)
FileListPath="$TempDir/filelist.txt"

PackageBaseName=$(basename "$PackageFolder")

if [ -f "$ManifestFile" ]; then
	echo "Using manifest file: $ManifestFile"
	# Read manifest (strip Windows line endings) and add to file list
	while IFS= read -r line || [ -n "$line" ]; do
		# Skip empty lines and comments
		if [ -n "$line" ] && [[ "$line" != \#* ]]; then
			echo "$line" >> "$FileListPath"
		fi
	done < <(tr -d '\r' < "$ManifestFile")
else
	echo "No manifest file found, packaging all files..."
	# Generate file list excluding dotfiles (except .version)
	cd "$PackageFolder"
	find . -type f ! -name ".*" ! -name "DataPackageManifest.txt" | sed 's|^\./||' >> "$FileListPath"
	find . -type f -name ".version" | sed 's|^\./||' >> "$FileListPath"
fi

if [ "$DryRun" = true ]; then
	echo ""
	echo "Files to be packaged:"
	cat "$FileListPath"
	echo ""
fi

# -----------------------------------------------
# Archive Creation
# -----------------------------------------------
if [ "$IsPlatformSpecific" = true ]; then
	ArchiveName="${ArchivePrefix}_${PlatformSuffix}_${NewVersion}.tar.gz"
else
	ArchiveName="${ArchivePrefix}_${NewVersion}.tar.gz"
fi

ArchivePath="$TempDir/$ArchiveName"

echo "Creating archive: $ArchiveName"

if [ "$DryRun" = false ]; then
	cd "$PackageFolder"
	tar -czf "$ArchivePath" -T "$FileListPath"

	if [ $? -ne 0 ]; then
		echo "[Error] Failed to create archive"
		# Revert version file
		echo "$CurrentVersion" > "$VersionFile"
		rm -rf "$TempDir"
		exit 1
	fi

	FullArchivePath=$(realpath "$ArchivePath")
	echo ""
	echo "Archive created successfully!"
	echo "  Path: $FullArchivePath"
	echo "  Size: $(du -h "$ArchivePath" | cut -f1)"
fi

# -----------------------------------------------
# FTP Upload
# -----------------------------------------------
if [ "$NoUpload" = true ]; then
	echo ""
	echo "Skipping upload (--no-upload specified)"
	echo "Archive location: $FullArchivePath"

	# Still update reqversion since archive was created successfully
	if [ "$DryRun" = false ]; then
		echo "$NewVersion" > "$ReqVersionFile"
	fi

	echo ""
	echo "Upload complete!"
	exit 0
fi

# Get FTP credentials
FtpUrl=""
FtpUser=""
FtpPass=""

if [ -n "$CredentialsFile" ]; then
	# Use provided credentials file
	if [ ! -f "$CredentialsFile" ]; then
		echo "[Error] Credentials file not found: $CredentialsFile"
		# Revert version file
		if [ "$DryRun" = false ]; then
			echo "$CurrentVersion" > "$VersionFile"
		fi
		rm -rf "$TempDir"
		exit 1
	fi

	mapfile -t creds < <(tr -d '\r' < "$CredentialsFile")
	FtpUrl="${creds[0]}"
	FtpUser="${creds[1]}"
	FtpPass="${creds[2]}"
elif [ -n "$B3D_FTP_URL" ] && [ -n "$B3D_FTP_USER" ] && [ -n "$B3D_FTP_PASS" ]; then
	# Use environment variables
	FtpUrl="$B3D_FTP_URL"
	FtpUser="$B3D_FTP_USER"
	FtpPass="$B3D_FTP_PASS"
else
	# Try default credentials file
	DefaultCredentialsFile="$ScriptDir/../../../ftp_credentials"
	if [ -f "$DefaultCredentialsFile" ]; then
		mapfile -t creds < <(tr -d '\r' < "$DefaultCredentialsFile")
		FtpUrl="${creds[0]}"
		FtpUser="${creds[1]}"
		FtpPass="${creds[2]}"
	else
		echo "[Error] No FTP credentials found."
		echo "Provide credentials via:"
		echo "  --credentials <file>"
		echo "  Environment variables: B3D_FTP_URL, B3D_FTP_USER, B3D_FTP_PASS"
		echo "  Or place ftp_credentials file at: $DefaultCredentialsFile"
		# Revert version file
		if [ "$DryRun" = false ]; then
			echo "$CurrentVersion" > "$VersionFile"
		fi
		rm -rf "$TempDir"
		exit 1
	fi
fi

echo ""
echo "Uploading to: $FtpUrl/$ArchiveName"

if [ "$DryRun" = false ]; then
	curl --upload-file "$ArchivePath" \
		--user "$FtpUser:$FtpPass" \
		--progress-bar \
		"$FtpUrl/$ArchiveName"

	if [ $? -ne 0 ]; then
		echo "[Error] Failed to upload archive"
		# Revert version file
		echo "$CurrentVersion" > "$VersionFile"
		rm -rf "$TempDir"
		exit 1
	fi
fi

# -----------------------------------------------
# Version Finalization
# -----------------------------------------------
if [ "$DryRun" = false ]; then
	echo "$NewVersion" > "$ReqVersionFile"
fi

# Cleanup
rm -rf "$TempDir"

echo ""
echo "======================================================================"
echo "Upload complete!"
echo "======================================================================"
echo ""
echo "Package: $PackageName"
echo "Archive: $ArchiveName"
echo "Version: $NewVersion"
