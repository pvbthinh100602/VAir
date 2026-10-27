# AI Coding Agent Instructions

This document provides guidelines for AI coding agents (including Cursor Cloud Agents, Copilot, and other AI assistants) working on the VAir project.

## Critical Rules

### 🌐 English-Only Requirement

**ALL content must be in English:**
- ✅ Code comments
- ✅ Documentation (README, docs/, etc.)
- ✅ Commit messages
- ✅ Issue descriptions and PR descriptions
- ✅ YAML comments in firmware configuration
- ✅ Variable names and function names
- ✅ Log messages and user-facing strings

**Exceptions** (Vietnamese text allowed):
- ❌ Shop names and addresses in `docs/hardware.md` (e.g., "Caka", "Lữ Gia", "Thủ Đức")
- ❌ Street addresses and location names in documentation

**Why**: The project is built for Vietnamese home use with planned Vietnamese **voice control**, but all development content is in English to facilitate international collaboration.

### 🔐 Never Commit Secrets

**Files that must NEVER be committed:**
- `firmware/esphome/secrets.yaml` (in `.gitignore`)
- Any file containing WiFi passwords, API keys, or credentials
- `.env` files with sensitive data

**Safe to commit:**
- `firmware/esphome/secrets.example.yaml` (template with placeholders)
- Documentation that references secret fields

**Why**: Secrets in git history cannot be fully removed and pose a security risk.

### 🔬 Hardware Verification Requirement

**Do NOT claim hardware behavior is verified without real hardware testing.**

Mark unverified behaviors clearly:
- ✅ "The configuration should work with..."
- ✅ "Suspected to use DAIKIN216 protocol (unconfirmed)"
- ✅ "GPIO mapping is proposed but not tested on hardware"
- ❌ "The ESP32 successfully controls the AC" (unless you actually tested it)
- ❌ "This circuit works" (unless someone ran it)

**Why**: The project is in early development. Most hardware interactions are theoretical until tested.

## Development Workflow

### 1. Initial Setup (First Time Only)

When starting work on VAir:

```bash
# Clone the repository
git clone https://github.com/pvbthinh100602/VAir.git
cd VAir

# Run automated setup
bash scripts/setup-dev.sh

# Activate environment
source .venv/bin/activate
```

**For Cursor Cloud Agents**: The `.cursor/environment.json` file automatically runs `scripts/setup-dev.sh` during VM bootstrap.

### 2. Before Making Changes

Always activate the environment first:

```bash
source .venv/bin/activate
```

### 3. Validate Before Committing

Run the validation script before every commit:

```bash
bash scripts/check.sh
```

This runs:
- ESPHome configuration validation
- YAML linting
- Language check (ensures no Vietnamese in code/docs)

**Do not commit if `scripts/check.sh` fails.**

### 4. Commit and Push

If checks pass:

```bash
git add .
git commit -m "Your descriptive commit message in English"
git push
```

## Repository Layout

```
VAir/
├── .cursor/
│   └── environment.json          # Cursor cloud agent bootstrap config
├── .github/
│   ├── ISSUE_TEMPLATE/           # Bug report and hardware question templates
│   └── workflows/
│       └── esphome-validate.yml  # CI validation (calls scripts/check.sh)
├── docs/
│   ├── hardware.md               # BOM with HCMC shops (Vietnamese names allowed)
│   ├── wiring.md                 # Circuit diagrams
│   ├── ir-capture.md             # IR signal capture guide
│   ├── roadmap.md                # Project phases and checklists
│   └── dev-setup.md              # Development environment setup guide
├── firmware/
│   ├── esphome/
│   │   ├── daikin-ftkb35.yaml    # ESPHome config (English comments only)
│   │   ├── secrets.example.yaml  # Template (commit this)
│   │   ├── secrets.yaml          # Real secrets (NEVER commit)
│   │   └── README.md             # ESPHome firmware guide
│   └── xiaozhi/
│       └── README.md             # Phase 2 placeholder (not implemented)
├── scripts/
│   ├── setup-dev.sh              # Automated dev environment setup
│   └── check.sh                  # Validation script (what CI runs)
├── server/
│   └── README.md                 # Phase 3 placeholder (not implemented)
├── .gitignore                    # Includes secrets.yaml, .esphome/, etc.
├── LICENSE                       # MIT License
├── THIRD_PARTY.md                # Third-party license documentation
├── AGENTS.md                     # This file
├── README.md                     # Project overview
└── requirements-dev.txt          # Pinned Python dependencies
```

### What to Edit

**Commonly edited files:**
- `firmware/esphome/daikin-ftkb35.yaml` - ESPHome configuration
- `docs/*.md` - Documentation updates
- `README.md` - Project overview

**Do not edit:**
- `firmware/esphome/secrets.yaml` - Contains user secrets
- `.esphome/` - Build artifacts (gitignored)
- `.venv/` - Virtual environment (gitignored)

**Phase 2/3 not implemented yet:**
- `firmware/xiaozhi/` - Placeholder only
- `server/` - Placeholder only

## Common Tasks for AI Agents

### Adding a New ESPHome Feature

1. Activate environment: `source .venv/bin/activate`
2. Edit `firmware/esphome/daikin-ftkb35.yaml`
3. Validate: `esphome config firmware/esphome/daikin-ftkb35.yaml`
4. Test compile: `esphome compile firmware/esphome/daikin-ftkb35.yaml`
5. Run checks: `bash scripts/check.sh`
6. Commit with English commit message

### Updating Documentation

1. Edit the relevant `.md` file in `docs/`
2. Ensure all text is in English (except shop names in `hardware.md`)
3. Run checks: `bash scripts/check.sh`
4. Commit changes

### Adding Dependencies

1. Edit `requirements-dev.txt` with pinned version: `package==1.2.3`
2. Update environment: `pip install -r requirements-dev.txt`
3. Test that everything still works
4. Update `docs/dev-setup.md` if needed
5. Commit both `requirements-dev.txt` and documentation

### Troubleshooting CI Failures

If CI fails:

1. Run locally: `bash scripts/check.sh`
2. Fix any errors reported
3. Re-run checks until they pass
4. Push the fix

**Remember**: Local checks and CI checks are identical (both use `scripts/check.sh`).

## License Compliance

The VAir project uses MIT License, but depends on components with different licenses:

- **IRremoteESP8266**: LGPL-2.1 (requires source availability for modifications)
- **xiaozhi-esp32**: MIT License
- **ESPHome**: MIT License

**When adding dependencies:**

1. Check the license
2. Update `THIRD_PARTY.md` with:
   - Component name and link
   - License type
   - How it's used
   - Any special obligations (e.g., LGPL requirements)
3. Ensure compatibility with MIT License

See [THIRD_PARTY.md](THIRD_PARTY.md) for complete information.

## Python Version Requirements

**Current Setup (ESPHome 2026.10.0):**
- Requires: Python 3.12, 3.13, or 3.14
- Pinned in: `requirements-dev.txt`

**Legacy (ESPHome 2025.7.5):**
- Requires: Python <3.12
- To use: Edit `requirements-dev.txt` and change version

The `scripts/setup-dev.sh` script detects the Python version and fails with clear instructions if incompatible.

## Testing Limitations

**Without hardware, you can test:**
- ✅ YAML validation: `esphome config`
- ✅ Firmware compilation: `esphome compile`
- ✅ YAML linting: `yamllint`
- ✅ Language checks: `scripts/check.sh`
- ✅ Documentation accuracy

**With hardware required:**
- ❌ IR LED transmission
- ❌ IR signal capture from remote
- ❌ WiFi connectivity
- ❌ AC control verification
- ❌ OTA updates

**Always mark untested hardware behavior as "proposed" or "unverified".**

## Known Unconfirmed Items

When working with VAir, be aware these are **not yet confirmed**:

1. **Daikin ARC486A56 Protocol**: Suspected DAIKIN216, but needs IR capture to verify
2. **Phase 2 GPIO Mapping**: Proposed but not tested on hardware
3. **ESP32-S3 N8R2 PSRAM**: Cheaper board with 2MB PSRAM may be insufficient (unverified)
4. **IRremoteIDF on ESP-IDF 6.0**: Build compatibility not tested
5. **Phase 2 and Phase 3**: Entire sections are planned but not implemented

**Always check `docs/roadmap.md` for current status of each phase.**

## Getting Help

- **Documentation**: Start with [docs/dev-setup.md](docs/dev-setup.md)
- **Issues**: Search existing issues on GitHub
- **Roadmap**: Check [docs/roadmap.md](docs/roadmap.md) for project status
- **Scripts**: Run `bash scripts/setup-dev.sh` or `bash scripts/check.sh` for guided workflows

## Cursor Cloud Agent Specifics

If you are a Cursor Cloud Agent:

1. **Bootstrap**: `.cursor/environment.json` automatically runs `scripts/setup-dev.sh`
2. **Environment**: `.venv` should be created automatically
3. **Activation**: Run `source .venv/bin/activate` if commands fail
4. **Validation**: Always run `bash scripts/check.sh` before committing
5. **PR Updates**: If working on an existing PR branch, push updates to the same branch

## Summary Checklist

Before committing, verify:

- [ ] All text is in English (except shop names in `docs/hardware.md`)
- [ ] No secrets committed (`secrets.yaml` is gitignored)
- [ ] `bash scripts/check.sh` passes all checks
- [ ] ESPHome config validates: `esphome config firmware/esphome/daikin-ftkb35.yaml`
- [ ] YAML is properly formatted (yamllint passes)
- [ ] Commit message is in English and descriptive
- [ ] Unverified hardware behavior is marked as "proposed" or "unconfirmed"
- [ ] New dependencies are documented in `THIRD_PARTY.md` if applicable

---

**For human developers**: See [docs/dev-setup.md](docs/dev-setup.md) for the full development guide.

**Last Updated**: October 26, 2026
