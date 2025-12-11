#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEBS=("${REPO_ROOT}/drogon_1.9.11-0_amd64.deb" "${REPO_ROOT}/eipscanner_1.3.0-1_amd64.deb")

if [[ ${EUID} -ne 0 ]]; then
  echo "This script needs to run as root (or under sudo) to install Debian packages." >&2
  exit 1
fi

apt-get update
apt-get install -y libjsoncpp-dev uuid-dev openssl libssl-dev zlib1g-dev

for pkg in "${DEBS[@]}"; do
  if [[ ! -f "${pkg}" ]]; then
    echo "Missing package: ${pkg}" >&2
    exit 1
  fi
  dpkg -i "${pkg}" || apt-get -f install -y
done

echo "Drogon and EIPScanner packages installed."
