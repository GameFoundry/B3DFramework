#!/usr/bin/env bash
#
# B3DUpdateCopyright.sh
#
# Updates the copyright year notice in the Framework and BansheeEngine
# editor source headers. Handles the two distinct header styles used by the
# two codebases and preserves overall line width (124 chars) by padding or
# trimming the '*' characters on either side of the year token.
#
# Usage: B3DUpdateCopyright.sh [--mode single|range] [--year YYYY] [--dry-run] [--help]

set -u

mode="range"
year="$(date +%Y)"
dry_run=0

usage() {
	cat <<'EOF'
Usage: B3DUpdateCopyright.sh [--mode single|range] [--year YYYY] [--dry-run] [--help]

Updates copyright year headers across the Framework (Framework/Source,
Framework/Tools) and the BansheeEngine editor (Source/).

  --mode single   Replace the year notice with just the target year.
                  E.g. "2018" or "2018-2025" both become "YEAR".
  --mode range    Keep the original start year, set end to target year.
                  "2018"      becomes "2018-YEAR".
                  "2018-2025" becomes "2018-YEAR".
                  Default.
  --year YYYY     Override the target year (default: current calendar year).
  --dry-run       Report changes without writing anything.
  --help          Show this help and exit.
EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		--mode)
			mode="${2:-}"
			shift 2
			;;
		--year)
			year="${2:-}"
			shift 2
			;;
		--dry-run)
			dry_run=1
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		*)
			echo "Unknown argument: $1" >&2
			usage >&2
			exit 2
			;;
	esac
done

if [[ "$mode" != "single" && "$mode" != "range" ]]; then
	echo "--mode must be 'single' or 'range' (got '$mode')" >&2
	exit 2
fi

if ! [[ "$year" =~ ^[0-9]{4}$ ]]; then
	echo "--year must be a 4-digit year (got '$year')" >&2
	exit 2
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"

target_width=124

scanned=0
updated=0
skipped=0
no_header=0

make_stars() {
	local n="$1"
	if (( n <= 0 )); then
		printf ''
		return
	fi
	printf '%*s' "$n" '' | tr ' ' '*'
}

# Rebuild a header line to exactly target_width characters, centering the
# middle text by splitting the asterisk padding symmetrically on either side.
# Args: middle_text
# Layout: "//" + N*"*" + " " + middle + " " + M*"*" + "//"
rebuild_line() {
	local middle="$1"
	local middle_len=${#middle}
	local total_stars=$(( target_width - middle_len - 6 ))
	if (( total_stars < 2 )); then
		total_stars=2
	fi
	local left_stars=$(( total_stars / 2 ))
	local right_stars=$(( total_stars - left_stars ))
	local left_str right_str
	left_str="$(make_stars "$left_stars")"
	right_str="$(make_stars "$right_stars")"
	printf '//%s %s %s//' "$left_str" "$middle" "$right_str"
}

compute_new_token() {
	local old_token="$1"
	local start_year end_year
	if [[ "$old_token" == *-* ]]; then
		start_year="${old_token%-*}"
		end_year="${old_token#*-}"
	else
		start_year="$old_token"
		end_year="$old_token"
	fi

	if [[ "$mode" == "single" ]]; then
		printf '%s' "$year"
		return
	fi

	if [[ "$start_year" == "$year" ]]; then
		printf '%s' "$year"
	else
		printf '%s-%s' "$start_year" "$year"
	fi
}

rx_fw_l1='^//(\*+) B3D Framework - Copyright ([0-9]{4}(-[0-9]{4})?) Marko Pintera (\*+)//$'
rx_fw_l2='^//(\*+) Licensed under the MIT license\. See LICENSE\.md for full terms\. This notice is not to be removed\. (\*+)//$'
rx_ed_l1_new='^//(\*+) Banshee Engine (\*+)//$'
rx_ed_l1_old='^//(\*+) Banshee Engine \(www\.banshee3d\.com\) (\*+)//$'
rx_ed_l2='^//(\*+) Copyright \(c\) ([0-9]{4}(-[0-9]{4})?) Marko Pintera \(marko\.pintera@gmail\.com\)\. All rights reserved\. (\*+)//$'

process_file() {
	local file="$1"
	scanned=$(( scanned + 1 ))

	local line1 line2
	if ! { IFS= read -r line1 && IFS= read -r line2; } < "$file"; then
		no_header=$(( no_header + 1 ))
		return
	fi

	local eol=""
	if [[ "$line1" == *$'\r' ]]; then
		eol=$'\r'
	fi
	local l1="${line1%$'\r'}"
	local l2="${line2%$'\r'}"

	local kind=""
	local old_token=""

	if [[ "$l1" =~ $rx_fw_l1 ]]; then
		kind="framework"
		old_token="${BASH_REMATCH[2]}"
	elif [[ "$l2" =~ $rx_ed_l2 ]]; then
		kind="editor"
		old_token="${BASH_REMATCH[2]}"
	else
		no_header=$(( no_header + 1 ))
		return
	fi

	local new_token
	new_token="$(compute_new_token "$old_token")"

	local new_l1 new_l2
	if [[ "$kind" == "framework" ]]; then
		new_l1="$(rebuild_line "B3D Framework - Copyright $new_token Marko Pintera")"
		new_l2="$(rebuild_line "Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed.")"
	else
		new_l1="$(rebuild_line "Banshee Engine")"
		new_l2="$(rebuild_line "Copyright (c) $new_token Marko Pintera (marko.pintera@gmail.com). All rights reserved.")"
	fi

	if [[ "$new_l1" == "$l1" && "$new_l2" == "$l2" ]]; then
		skipped=$(( skipped + 1 ))
		return
	fi

	if [[ "$dry_run" -eq 1 ]]; then
		printf 'WOULD UPDATE %s   (%s -> %s)\n' "$file" "$old_token" "$new_token"
		updated=$(( updated + 1 ))
		return
	fi

	local tmp
	tmp="$(mktemp)"
	{
		printf '%s%s\n' "$new_l1" "$eol"
		printf '%s%s\n' "$new_l2" "$eol"
		tail -n +3 "$file"
	} > "$tmp"
	# Preserve original permissions
	chmod --reference="$file" "$tmp" 2>/dev/null || true
	mv "$tmp" "$file"

	printf 'UPDATE %s   (%s -> %s)\n' "$file" "$old_token" "$new_token"
	updated=$(( updated + 1 ))
}

walk() {
	local root="$1"
	if [[ ! -d "$root" ]]; then
		return
	fi
	while IFS= read -r -d '' file; do
		process_file "$file"
	done < <(
		find "$root" -type f \( \
			-name '*.h' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.c' -o \
			-name '*.cs' -o -name '*.m' -o -name '*.mm' \
		\) \
		-not -path '*/Dependencies/*' \
		-not -path '*/ThirdParty/*' \
		-not -path '*/Generated/*' \
		-not -path '*/Intermediate/*' \
		-not -path '*/build/*' \
		-not -path '*/Build/*' \
		-not -path '*/.git/*' \
		-print0
	)
}

walk "$repo_root/Framework/Source"
walk "$repo_root/Framework/Tools"
walk "$repo_root/Source"

echo ""
echo "--- Summary ---"
echo "Mode:                         $mode"
echo "Target year:                  $year"
echo "Scanned:                      $scanned"
echo "Updated:                      $updated"
echo "Skipped (already up to date): $skipped"
echo "No recognized header:         $no_header"
if [[ "$dry_run" -eq 1 ]]; then
	echo "(dry run — no files written)"
fi
