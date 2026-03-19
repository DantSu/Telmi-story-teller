# Building for PortMaster (R36S / ArkOS / DarkOS)

Targets ARM64 devices running ArkOS or DarkOS (R36S and similar RK3326 boards).

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed and running

## Build

**1. Build the Docker image** (once, from the project root):

```bash
docker build -f Dockerfile.r36s -t telmi-builder-r36s .
```

**2. Compile the binary:**

```bash
docker run --rm -v "$(pwd)":/workspace telmi-builder-r36s \
    bash scripts/build_r36s.sh
```

**3. Assemble the PortMaster package:**

```bash
bash scripts/package_portmaster.sh
```

The ready-to-deploy package is output to `dist/portmaster/`.

## Deploy

Copy the contents of `dist/portmaster/` to `/roms/ports/` on your device SD card:

```
/roms/ports/
├── telmi.sh               # PortMaster launcher entry point
└── telmi_app/
    ├── telmi_rk3326.aarch64
    ├── telmi.gptk
    └── data/
        ├── res/           # fonts and UI assets
        ├── Music/         # music library
        ├── Stories/       # story content
        └── Saves/         # save states (written at runtime)
```
