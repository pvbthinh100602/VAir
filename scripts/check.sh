#!/usr/bin/env bash
# VAir Pre-commit Validation Script
# Runs all checks that CI runs: ESPHome config validation, YAML linting, language checks
#
# Usage: bash scripts/check.sh
# Run this before committing to catch issues early

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Repo root (parent of scripts/)
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo -e "${BLUE}=== VAir Pre-commit Validation ===${NC}"
echo

# Track overall status
ALL_PASSED=true

# Check if running in CI
if [[ "${CI:-false}" == "true" ]]; then
    echo "Running in CI environment"
    echo
fi

# Ensure we're in a venv or have esphome available
if ! command -v esphome &> /dev/null; then
    echo -e "${RED}ERROR: esphome command not found.${NC}"
    echo "Run 'bash scripts/setup-dev.sh' first, then activate the environment:"
    echo "  source .venv/bin/activate"
    exit 1
fi

# Check 1: ESPHome Configuration Validation
echo -e "${BLUE}[1/3] Validating ESPHome configuration...${NC}"

SECRETS_FILE="firmware/esphome/secrets.yaml"
if [[ ! -f "$SECRETS_FILE" ]]; then
    echo -e "${YELLOW}Warning: $SECRETS_FILE not found, creating temporary file...${NC}"
    TEMP_SECRETS=true
    
    # Create temporary secrets for validation
    API_KEY=$(python -c "import secrets, base64; print(base64.b64encode(secrets.token_bytes(32)).decode())" 2>/dev/null || echo "dGVtcG9yYXJ5X2FwaV9rZXlfZm9yX3ZhbGlkYXRpb25fb25seQ==")
    
    cat > "$SECRETS_FILE" <<EOF
wifi_ssid: "VALIDATION_SSID"
wifi_password: "validation_password"
ap_password: "validation_ap"
api_encryption_key: "$API_KEY"
EOF
else
    TEMP_SECRETS=false
fi

if esphome config firmware/esphome/daikin-ftkb35.yaml 2>&1 | tee /tmp/esphome-check.log; then
    # Check for actual errors (not warnings) - look for "Failed config" or error messages
    if grep -q "Failed config\|ERROR.*:" /tmp/esphome-check.log && ! grep -q "INFO Configuration is valid!" /tmp/esphome-check.log; then
        echo -e "${RED}✗ ESPHome configuration has errors${NC}"
        ALL_PASSED=false
    else
        echo -e "${GREEN}✓ ESPHome configuration is valid${NC}"
    fi
else
    echo -e "${RED}✗ ESPHome validation failed${NC}"
    ALL_PASSED=false
fi

# Cleanup temporary secrets
if [[ "$TEMP_SECRETS" == "true" ]]; then
    rm -f "$SECRETS_FILE"
fi

echo

# Check 2: YAML Linting
echo -e "${BLUE}[2/3] Linting YAML files...${NC}"

if command -v yamllint &> /dev/null; then
    # Create yamllint config if it doesn't exist
    if [[ ! -f ".yamllint" ]]; then
        cat > .yamllint <<EOF
extends: default
rules:
  line-length:
    max: 120
    level: warning
  comments:
    min-spaces-from-content: 1
  document-start: disable
  truthy:
    allowed-values: ['true', 'false', 'on', 'off']
EOF
    fi
    
    if yamllint firmware/esphome/*.yaml .github/workflows/*.yml; then
        echo -e "${GREEN}✓ YAML files are properly formatted${NC}"
    else
        echo -e "${YELLOW}⚠ YAML linting found issues (warnings only)${NC}"
        # Don't fail on yamllint warnings
    fi
else
    echo -e "${YELLOW}⚠ yamllint not found, skipping YAML validation${NC}"
fi

echo

# Check 3: Language Check (No Vietnamese characters except in allowed files)
echo -e "${BLUE}[3/3] Checking for Vietnamese characters in code/docs...${NC}"

# Files where Vietnamese text is explicitly allowed (shop names, addresses, example voice commands)
ALLOWED_FILES=(
    "docs/hardware.md"
    "server/README.md"
)

# The detector itself contains the diacritic class, so it must not be scanned.
SKIP_FILES=(
    "scripts/check.sh"
)

# Build grep exclude arguments
EXCLUDE_ARGS=""
for file in "${ALLOWED_FILES[@]}"; do
    EXCLUDE_ARGS="$EXCLUDE_ARGS --exclude=$file"
done

# Vietnamese Unicode ranges and common Vietnamese characters
# Check for characters with Vietnamese diacritics: ă, â, đ, ê, ô, ơ, ư and their uppercase variants
VIETNAMESE_PATTERN='[ăâđêôơưĂÂĐÊÔƠƯ]|[àáạảãâầấậẩẫăằắặẳẵ]|[èéẹẻẽêềếệểễ]|[ìíịỉĩ]|[òóọỏõôồốộổỗơờớợởỡ]|[ùúụủũưừứựửữ]|[ỳýỵỷỹ]|[ÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴ]|[ÈÉẸẺẼÊỀẾỆỂỄ]|[ÌÍỊỈĨ]|[ÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠ]|[ÙÚỤỦŨƯỪỨỰỬỮ]|[ỲÝỴỶỸ]'

VIETNAMESE_FOUND=false
VIOLATIONS=""

# Check all tracked files except binaries and allowed files
while IFS= read -r file; do
    # Skip binary files
    if file "$file" 2>/dev/null | grep -q "text"; then
        if grep -P "$VIETNAMESE_PATTERN" "$file" > /dev/null 2>&1; then
            # Check if this file is in the allowed list or is the detector itself
            IS_ALLOWED=false
            for allowed in "${ALLOWED_FILES[@]}" "${SKIP_FILES[@]}"; do
                if [[ "$file" == "$allowed" ]]; then
                    IS_ALLOWED=true
                    break
                fi
            done
            
            if [[ "$IS_ALLOWED" == "false" ]]; then
                VIETNAMESE_FOUND=true
                VIOLATIONS="$VIOLATIONS\n  - $file"
            fi
        fi
    fi
done < <(git ls-files)

if [[ "$VIETNAMESE_FOUND" == "true" ]]; then
    echo -e "${RED}✗ Vietnamese characters found in unexpected files:${NC}"
    echo -e "$VIOLATIONS"
    echo
    echo "Vietnamese text is only allowed in:"
    for allowed in "${ALLOWED_FILES[@]}"; do
        echo "  - $allowed"
    done
    echo
    echo "All code, comments, and documentation must be in English."
    echo "If this file legitimately needs Vietnamese text (e.g., shop names),"
    echo "add it to the ALLOWED_FILES array in scripts/check.sh"
    ALL_PASSED=false
else
    echo -e "${GREEN}✓ No Vietnamese characters found in code/docs (English-only verified)${NC}"
fi

echo
echo -e "${BLUE}=== Validation Complete ===${NC}"
echo

if [[ "$ALL_PASSED" == "true" ]]; then
    echo -e "${GREEN}✓ All checks passed!${NC}"
    echo "Your changes are ready to commit."
    exit 0
else
    echo -e "${RED}✗ Some checks failed.${NC}"
    echo "Please fix the issues above before committing."
    exit 1
fi
