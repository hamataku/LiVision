#!/bin/bash

set -euo pipefail

manifest="${1:-build/install_manifest.txt}"

if [[ ! -f "${manifest}" ]]; then
  echo "install_manifest.txt not found: ${manifest}" >&2
  echo "Usage: $0 [path/to/install_manifest.txt]" >&2
  exit 1
fi

while IFS= read -r file; do
  if [[ -e "${file}" || -L "${file}" ]]; then
    echo "Removing ${file}"
    rm -f "${file}"
  fi
  dir="$(dirname "${file}")"
  while [[ "${dir}" != "/" && -d "${dir}" ]]; do
    if rmdir "${dir}" 2>/dev/null; then
      echo "Removed empty dir ${dir}"
      dir="$(dirname "${dir}")"
    else
      break
    fi
  done
done < "${manifest}"
