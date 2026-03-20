# Building for PortMaster (R36S / ArkOS / DarkOS)

Targets ARM64 devices running ArkOS, DarkOS, and similar RK3326 boards. The build
is intentionally pinned to a Buster userspace so the resulting binary stays at a
glibc level that remains compatible with the older ArkOS Ubuntu 19.10 base.
Bookworm ships a newer SDL2_mixer development package, but it also raises the
glibc baseline beyond what older ArkOS images can load.

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed and running

## Build

### Quick Build (All-in-One)

Run all steps at once from the project root:

```bash
docker build -t telmi-builder-portmaster -f Dockerfile.portmaster . && docker run --rm -v "$(pwd)":/workspace telmi-builder-portmaster bash portmaster/scripts/build_portmaster.sh && bash portmaster/scripts/package_portmaster.sh
```

### Step-by-Step Build

**1. Build the Docker image** (once, from the project root):

```bash
docker build -f Dockerfile.portmaster -t telmi-builder-portmaster .
```

**2. Compile the binary:**

```bash
docker run --rm -v "$(pwd)":/workspace telmi-builder-portmaster \
    bash portmaster/scripts/build_portmaster.sh
```

**3. Assemble the PortMaster package:**

```bash
bash portmaster/scripts/package_portmaster.sh
```

The ready-to-deploy package is output to `dist/portmaster/`.

## Deploy

Copy the contents of `dist/portmaster/` to `/roms/ports/` on your device SD card:

```
/roms/ports/
├── Telmi.sh               # PortMaster launcher entry point
└── telmi/
    ├── telmi_rk3326.aarch64
    ├── telmi.gptk
    ├── libs.aarch64/
    └── data/
        ├── res/           # fonts and UI assets
        ├── Music/         # music library
        ├── Stories/       # story content
        └── Saves/         # save states (written at runtime)
```

The launcher expects PortMaster's `DEVICE_ARCH`-specific library folder
convention (`libs.aarch64/`) so any bundled shared libraries stay isolated to
this port and do not affect the rest of the system.
