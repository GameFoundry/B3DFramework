#!/bin/bash

# -----------------------------------------------
# Banshee 3D Engine
# Copyright (c) 2025 Marko Pintera. All rights reserved.
# -----------------------------------------------
#
# B3DUploadBinaries.sh - Package and upload dependencies and data
#
# Usage:
#   ./B3DUploadBinaries.sh <package-name> [options]
#
# Options:
#   --backend <name>  Upload backend: 'rclone' (default, Cloudflare R2) or 'ftp'
#   --no-upload       Create archive but skip upload
#   --no-bump         Re-upload current version without incrementing it
#   --dry-run         Print actions without executing
#   --credentials     Path to credentials file
#   --list            List available packages
#   --check-outdated  Check which packages need to be re-uploaded (slow)
#   --help            Show help message
#
# Credentials file format (auto-detected):
#   - key=value lines (preferred): both backends in one file, e.g.
#       B3D_FTP_URL=ftp://example.com
#       B3D_R2_ACCOUNT_ID=...
#   - Legacy 3-line FTP format (URL / user / pass) is still accepted.
#
# Environment variables (checked if not in credentials file):
#   FTP backend:
#     B3D_FTP_URL              FTP server URL
#     B3D_FTP_USER             FTP username
#     B3D_FTP_PASS             FTP password
#   rclone/R2 backend:
#     B3D_R2_ACCOUNT_ID        Cloudflare account ID
#     B3D_R2_ACCESS_KEY_ID     R2 access key ID
#     B3D_R2_SECRET_ACCESS_KEY R2 secret access key
#     B3D_R2_BUCKET            Target bucket name
#     B3D_R2_PATH              Optional prefix inside the bucket
#     B3D_R2_PUBLIC_URL        Optional public URL printed after upload

# Script directory
ScriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
FrameworkDir="$(cd "$ScriptDir/.." &> /dev/null && pwd)"
RootDir="$(cd "$FrameworkDir/.." &> /dev/null && pwd)"

# Platform detection
Platform="$OSTYPE"

# Default options
Backend="rclone"
NoUpload=false
NoBump=false
DryRun=false
CredentialsFile=""
ListPackages=false
CheckOutdated=false
PackageName=""

# Credentials (populated by load_credentials)
B3D_FTP_URL="${B3D_FTP_URL:-}"
B3D_FTP_USER="${B3D_FTP_USER:-}"
B3D_FTP_PASS="${B3D_FTP_PASS:-}"
B3D_R2_ACCOUNT_ID="${B3D_R2_ACCOUNT_ID:-}"
B3D_R2_ACCESS_KEY_ID="${B3D_R2_ACCESS_KEY_ID:-}"
B3D_R2_SECRET_ACCESS_KEY="${B3D_R2_SECRET_ACCESS_KEY:-}"
B3D_R2_BUCKET="${B3D_R2_BUCKET:-}"
B3D_R2_PATH="${B3D_R2_PATH:-}"
B3D_R2_PUBLIC_URL="${B3D_R2_PUBLIC_URL:-}"

# -----------------------------------------------
# Usage/Help
# -----------------------------------------------
show_usage() {
	echo "B3DUploadBinaries - Package and upload dependencies and data"
	echo ""
	echo "Usage: $0 <package-name> [options]"
	echo ""
	echo "Options:"
	echo "  --backend <name>  Upload backend: 'rclone' (default, R2) or 'ftp'"
	echo "  --no-upload       Create archive but skip upload"
	echo "  --no-bump         Re-upload current version without incrementing it"
	echo "  --dry-run         Print actions without executing"
	echo "  --credentials     Path to credentials file"
	echo "  --list            List available packages"
	echo "  --check-outdated  Check which packages need to be re-uploaded (slow)"
	echo "  --help            Show this help message"
	echo ""
	echo "Credentials file format:"
	echo "  - key=value lines (preferred); recognized keys are the env vars below"
	echo "  - Legacy 3-line FTP format (URL / user / pass) is still accepted"
	echo ""
	echo "Environment variables (checked if not in credentials file):"
	echo "  FTP backend:"
	echo "    B3D_FTP_URL              FTP server URL"
	echo "    B3D_FTP_USER             FTP username"
	echo "    B3D_FTP_PASS             FTP password"
	echo "  rclone/R2 backend:"
	echo "    B3D_R2_ACCOUNT_ID        Cloudflare account ID"
	echo "    B3D_R2_ACCESS_KEY_ID     R2 access key ID"
	echo "    B3D_R2_SECRET_ACCESS_KEY R2 secret access key"
	echo "    B3D_R2_BUCKET            Target bucket name"
	echo "    B3D_R2_PATH              Optional prefix inside the bucket"
	echo "    B3D_R2_PUBLIC_URL        Optional public URL printed after upload"
	echo ""
	echo "Examples:"
	echo "  $0 --list                              # List all available packages"
	echo "  $0 XShaderCompiler                     # Upload dependency to R2"
	echo "  $0 FrameworkData --backend ftp         # Upload via FTP instead"
	echo "  $0 XShaderCompiler --no-upload         # Create archive only"
	echo "  $0 XShaderCompiler --no-bump           # Re-upload current version"
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
# Credentials Loading
#
# Reads credentials from a key=value file into the B3D_FTP_* / B3D_R2_* shell
# variables. Existing values (e.g. set via real env vars) are preserved -
# the file only fills in keys that are currently empty.
#
# A credentials file with no '=' on its first non-comment line is treated as
# the legacy 3-line FTP format (URL / user / pass) for backward compatibility.
# -----------------------------------------------
load_credentials_file() {
	local credFile="$1"
	local line key value firstContent=""

	# Strip CR and find the first non-blank, non-comment line to detect format
	while IFS= read -r line || [ -n "$line" ]; do
		line="${line%$'\r'}"
		[[ -z "$line" ]] && continue
		[[ "$line" == \#* ]] && continue
		firstContent="$line"
		break
	done < "$credFile"

	if [[ "$firstContent" != *"="* ]]; then
		# Legacy 3-line FTP format
		mapfile -t creds < <(tr -d '\r' < "$credFile")
		[ -z "$B3D_FTP_URL" ]  && B3D_FTP_URL="${creds[0]:-}"
		[ -z "$B3D_FTP_USER" ] && B3D_FTP_USER="${creds[1]:-}"
		[ -z "$B3D_FTP_PASS" ] && B3D_FTP_PASS="${creds[2]:-}"
		return
	fi

	# key=value format
	while IFS= read -r line || [ -n "$line" ]; do
		line="${line%$'\r'}"
		[[ -z "$line" ]] && continue
		[[ "$line" == \#* ]] && continue
		[[ "$line" != *"="* ]] && continue

		key="${line%%=*}"
		value="${line#*=}"

		# Trim surrounding whitespace from key
		key="${key#"${key%%[![:space:]]*}"}"
		key="${key%"${key##*[![:space:]]}"}"

		# Strip optional surrounding quotes from value
		if [[ "$value" =~ ^\".*\"$ ]] || [[ "$value" =~ ^\'.*\'$ ]]; then
			value="${value:1:${#value}-2}"
		fi

		case "$key" in
			B3D_FTP_URL)              [ -z "$B3D_FTP_URL" ]              && B3D_FTP_URL="$value" ;;
			B3D_FTP_USER)             [ -z "$B3D_FTP_USER" ]             && B3D_FTP_USER="$value" ;;
			B3D_FTP_PASS)             [ -z "$B3D_FTP_PASS" ]             && B3D_FTP_PASS="$value" ;;
			B3D_R2_ACCOUNT_ID)        [ -z "$B3D_R2_ACCOUNT_ID" ]        && B3D_R2_ACCOUNT_ID="$value" ;;
			B3D_R2_ACCESS_KEY_ID)     [ -z "$B3D_R2_ACCESS_KEY_ID" ]     && B3D_R2_ACCESS_KEY_ID="$value" ;;
			B3D_R2_SECRET_ACCESS_KEY) [ -z "$B3D_R2_SECRET_ACCESS_KEY" ] && B3D_R2_SECRET_ACCESS_KEY="$value" ;;
			B3D_R2_BUCKET)            [ -z "$B3D_R2_BUCKET" ]            && B3D_R2_BUCKET="$value" ;;
			B3D_R2_PATH)              [ -z "$B3D_R2_PATH" ]              && B3D_R2_PATH="$value" ;;
			B3D_R2_PUBLIC_URL)        [ -z "$B3D_R2_PUBLIC_URL" ]        && B3D_R2_PUBLIC_URL="$value" ;;
		esac
	done < "$credFile"
}

# Selects which credentials file to read (CLI flag, then default locations) and
# loads it. Real environment variables always take precedence. Missing files
# at the default locations are not an error - the user may rely on env vars.
load_credentials() {
	if [ -n "$CredentialsFile" ]; then
		if [ ! -f "$CredentialsFile" ]; then
			echo "[Error] Credentials file not found: $CredentialsFile"
			revert_version
			rm -rf "$TempDir"
			exit 1
		fi
		load_credentials_file "$CredentialsFile"
		return
	fi

	# Default locations (canonical first, legacy fallback second)
	local defaultDir="$ScriptDir/../../.."
	if [ -f "$defaultDir/b3d_credentials" ]; then
		load_credentials_file "$defaultDir/b3d_credentials"
	elif [ -f "$defaultDir/ftp_credentials" ]; then
		load_credentials_file "$defaultDir/ftp_credentials"
	fi
}

# -----------------------------------------------
# Version Revert (used on upload failure)
# -----------------------------------------------
revert_version() {
	if [ "$NoBump" = true ]; then
		return
	fi
	if [ "$DryRun" = true ]; then
		return
	fi
	if [ -n "$VersionFile" ] && [ -n "$CurrentVersion" ]; then
		echo "$CurrentVersion" > "$VersionFile"
	fi
}

# -----------------------------------------------
# FTP Upload Backend
# -----------------------------------------------
upload_ftp() {
	if [ -z "$B3D_FTP_URL" ] || [ -z "$B3D_FTP_USER" ] || [ -z "$B3D_FTP_PASS" ]; then
		echo "[Error] Missing FTP credentials. Provide via:"
		echo "  --credentials <file>"
		echo "  Environment variables: B3D_FTP_URL, B3D_FTP_USER, B3D_FTP_PASS"
		echo "  Or a b3d_credentials / ftp_credentials file in the repo parent folder"
		return 1
	fi

	echo ""
	echo "Uploading to: $B3D_FTP_URL/$ArchiveName"

	if [ "$DryRun" = true ]; then
		return 0
	fi

	curl --upload-file "$ArchivePath" \
		--user "$B3D_FTP_USER:$B3D_FTP_PASS" \
		--progress-bar \
		"$B3D_FTP_URL/$ArchiveName"
}

# -----------------------------------------------
# rclone / Cloudflare R2 Upload Backend
# -----------------------------------------------
upload_rclone() {
	if ! command -v rclone >/dev/null 2>&1; then
		echo "[Error] 'rclone' not found on PATH. Install it with:"
		case "$PlatformSuffix" in
			Win32) echo "          winget install Rclone.Rclone" ;;
			Linux) echo "          sudo -v ; curl https://rclone.org/install.sh | sudo bash" ;;
			MacOS) echo "          brew install rclone" ;;
			*)     echo "          See https://rclone.org/downloads/" ;;
		esac
		echo "        Or run with --backend ftp."
		return 1
	fi

	local missing=()
	[ -z "$B3D_R2_ACCOUNT_ID" ]        && missing+=("B3D_R2_ACCOUNT_ID")
	[ -z "$B3D_R2_ACCESS_KEY_ID" ]     && missing+=("B3D_R2_ACCESS_KEY_ID")
	[ -z "$B3D_R2_SECRET_ACCESS_KEY" ] && missing+=("B3D_R2_SECRET_ACCESS_KEY")
	[ -z "$B3D_R2_BUCKET" ]            && missing+=("B3D_R2_BUCKET")

	if [ ${#missing[@]} -gt 0 ]; then
		echo "[Error] Missing R2 credentials: ${missing[*]}"
		echo "        Provide via --credentials <file> or environment variables."
		return 1
	fi

	# Compose remote target path
	local remoteBase="$B3D_R2_BUCKET"
	if [ -n "$B3D_R2_PATH" ]; then
		# Trim leading/trailing slashes from the path prefix
		local trimmedPath="${B3D_R2_PATH#/}"
		trimmedPath="${trimmedPath%/}"
		[ -n "$trimmedPath" ] && remoteBase="$B3D_R2_BUCKET/$trimmedPath"
	fi
	local target="b3d_r2:$remoteBase/$ArchiveName"

	echo ""
	echo "Uploading to: $target (R2 endpoint https://${B3D_R2_ACCOUNT_ID}.r2.cloudflarestorage.com)"

	if [ "$DryRun" = true ]; then
		return 0
	fi

	# Write a temp rclone config (inside TempDir, cleaned up with the rest).
	# Subshell scope prevents the restrictive umask from leaking.
	local rcloneConfig="$TempDir/rclone.conf"
	(
		umask 077
		cat > "$rcloneConfig" <<EOF
[b3d_r2]
type = s3
provider = Cloudflare
access_key_id = $B3D_R2_ACCESS_KEY_ID
secret_access_key = $B3D_R2_SECRET_ACCESS_KEY
endpoint = https://${B3D_R2_ACCOUNT_ID}.r2.cloudflarestorage.com
acl = private
EOF
	)

	rclone --config "$rcloneConfig" copyto "$ArchivePath" "$target" \
		--progress --s3-no-check-bucket
	local status=$?

	if [ $status -eq 0 ] && [ -n "$B3D_R2_PUBLIC_URL" ]; then
		local publicBase="${B3D_R2_PUBLIC_URL%/}"
		local publicPath=""
		if [ -n "$B3D_R2_PATH" ]; then
			local trimmedPath="${B3D_R2_PATH#/}"
			trimmedPath="${trimmedPath%/}"
			[ -n "$trimmedPath" ] && publicPath="$trimmedPath/"
		fi
		echo ""
		echo "Public URL: $publicBase/$publicPath$ArchiveName"
	fi

	return $status
}

# -----------------------------------------------
# Parse Arguments
# -----------------------------------------------
while [[ $# -gt 0 ]]; do
	case "$1" in
		--backend)
			Backend="$2"
			shift 2
			;;
		--no-upload)
			NoUpload=true
			shift
			;;
		--no-bump)
			NoBump=true
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

# Validate backend selection
case "$Backend" in
	ftp|rclone) ;;
	*)
		echo "[Error] Unknown backend: $Backend (expected 'ftp' or 'rclone')"
		exit 1
		;;
esac

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
	PlatformSuffix="MacOS"
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

if [ "$NoBump" = true ]; then
	if [ "$CurrentVersion" -le 0 ]; then
		echo "[Error] --no-bump requires an existing .reqversion (current is 0 or missing)"
		exit 1
	fi
	NewVersion=$CurrentVersion
	echo "Current version: $CurrentVersion (re-uploading without bump)"
else
	NewVersion=$((CurrentVersion + 1))
	echo "Current version: $CurrentVersion"
	echo "New version: $NewVersion"
fi

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

	# GNU tar and BSD tar (macOS default) spell path rewriting differently
	if tar --version 2>&1 | grep -qi "bsdtar"; then
		tar -czf "$ArchivePath" -s ",^,${ArchivePrefix}/," -T "$FileListPath"
	else
		tar -czf "$ArchivePath" --transform "s,^,${ArchivePrefix}/," -T "$FileListPath"
	fi

	if [ $? -ne 0 ]; then
		echo "[Error] Failed to create archive"
		revert_version
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
# Upload
# -----------------------------------------------
if [ "$NoUpload" = true ]; then
	echo ""
	echo "Skipping upload (--no-upload specified)"
	echo "Archive location: $FullArchivePath"

	# Still update reqversion since archive was created successfully
	if [ "$DryRun" = false ] && [ "$NoBump" = false ]; then
		echo "$NewVersion" > "$ReqVersionFile"
	fi

	echo ""
	echo "Upload complete!"
	exit 0
fi

echo ""
echo "Backend: $Backend"

# Load credentials (file + env). On failure, revert version and exit.
load_credentials

case "$Backend" in
	ftp)    upload_ftp ;;
	rclone) upload_rclone ;;
esac
UploadStatus=$?

if [ $UploadStatus -ne 0 ]; then
	echo "[Error] Failed to upload archive"
	revert_version
	rm -rf "$TempDir"
	exit 1
fi

# -----------------------------------------------
# Version Finalization
# -----------------------------------------------
if [ "$DryRun" = false ] && [ "$NoBump" = false ]; then
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
