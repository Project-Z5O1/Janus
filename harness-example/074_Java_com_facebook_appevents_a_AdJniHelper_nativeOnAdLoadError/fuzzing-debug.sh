#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OMEN_PROFILE=debug exec "$SCRIPT_DIR/fuzzing.sh" "$@"
