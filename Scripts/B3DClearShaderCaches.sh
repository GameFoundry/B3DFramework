#!/usr/bin/env bash
#
# B3DClearShaderCaches.sh
#
# Clears the on-disk driver pipeline/shader caches for the Vulkan ICDs used by
# B3D so that the next run recompiles every pipeline cold. This is required to
# get meaningful pipeline-compile timings, because drivers transparently cache
# compiled pipeline ISA keyed by SPIR-V hash and a warm cache returns in well under 1 ms.
#
# Caches cleared (under %LOCALAPPDATA%):
#   nVidia : NVIDIA/GLCache              (nVidia stores Vulkan pipeline binaries here too)
#   AMD    : AMD/VkCache, AMD/VkCache_cb (amdvlk/AMD Vulkan pipeline binary caches)
#
# Usage: B3DClearShaderCaches.sh [--nvidia] [--amd] [--engine] [--dry-run] [--help]
#        (with no vendor flag, both nVidia and AMD caches are cleared)

set -u

do_nvidia=0
do_amd=0
do_engine=0
dry_run=0
vendor_specified=0

usage() {
	cat <<'EOF'
Usage: B3DClearShaderCaches.sh [--nvidia] [--amd] [--engine] [--dry-run] [--help]

Clears the Vulkan driver pipeline/shader caches so the next run compiles cold.

  --nvidia    Clear only the nVidia cache (NVIDIA/GLCache).
  --amd       Clear only the AMD caches (AMD/VkCache, AMD/VkCache_cb).
  --engine    Also clear the engine's persistent RendererMaterialShaders cache
              (forces a BSL->SPIR-V recompile in addition to the driver compile).
  --dry-run   Report what would be cleared without deleting anything.
  --help      Show this help and exit.

With no vendor flag, both nVidia and AMD driver caches are cleared.
EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		--nvidia) do_nvidia=1; vendor_specified=1; shift ;;
		--amd)    do_amd=1;    vendor_specified=1; shift ;;
		--engine) do_engine=1; shift ;;
		--dry-run) dry_run=1; shift ;;
		-h|--help) usage; exit 0 ;;
		*) echo "Unknown argument: $1" >&2; usage >&2; exit 2 ;;
	esac
done

# Default: clear both vendors when none was named explicitly.
if [[ "$vendor_specified" -eq 0 ]]; then
	do_nvidia=1
	do_amd=1
fi

# Resolve %LOCALAPPDATA% to a unix-style path (cygpath under git-bash/MSYS).
if [[ -z "${LOCALAPPDATA:-}" ]]; then
	echo "LOCALAPPDATA is not set; this script targets Windows (git-bash/MSYS)." >&2
	exit 1
fi
if command -v cygpath >/dev/null 2>&1; then
	localappdata="$(cygpath -u "$LOCALAPPDATA")"
else
	localappdata="$LOCALAPPDATA"
fi

cleared_any=0

# Deletes the contents of a cache directory (keeping the directory itself).
# Args: label, directory
clear_dir() {
	local label="$1"
	local dir="$2"

	if [[ ! -d "$dir" ]]; then
		printf '  %-28s (not present, skipped)\n' "$label"
		return
	fi

	local count size
	count="$(find "$dir" -type f 2>/dev/null | wc -l | tr -d ' ')"
	size="$(du -sh "$dir" 2>/dev/null | cut -f1)"

	if [[ "$dry_run" -eq 1 ]]; then
		printf '  %-28s WOULD CLEAR %s files, %s\n' "$label" "$count" "$size"
		return
	fi

	# Remove directory contents but keep the directory so the driver can repopulate it.
	find "$dir" -mindepth 1 -delete 2>/dev/null || rm -rf "${dir:?}/"* 2>/dev/null || true
	printf '  %-28s cleared (%s files, %s)\n' "$label" "$count" "$size"
	cleared_any=1
}

echo "Clearing Vulkan shader caches under: $localappdata"
echo ""

if [[ "$do_nvidia" -eq 1 ]]; then
	echo "nVidia:"
	clear_dir "NVIDIA/GLCache" "$localappdata/NVIDIA/GLCache"
fi

if [[ "$do_amd" -eq 1 ]]; then
	echo "AMD:"
	clear_dir "AMD/VkCache" "$localappdata/AMD/VkCache"
	clear_dir "AMD/VkCache_cb" "$localappdata/AMD/VkCache_cb"
fi

if [[ "$do_engine" -eq 1 ]]; then
	echo "Engine (persistent shader-bytecode cache):"
	# B3D editor app dir; examples may use a different app name but share this layout.
	clear_dir "Banshee3D RendererMaterialShaders" "$localappdata/Banshee3D/PersistentCache/RendererMaterialShaders"
fi

echo ""
if [[ "$dry_run" -eq 1 ]]; then
	echo "(dry run - nothing deleted)"
fi
