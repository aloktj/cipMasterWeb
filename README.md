Here’s a first version of a **README.md** you can drop straight into your repo and tweak as you go.

---

````markdown
# TCMS CIP Simulator

A web-based EtherNet/IP / CIP **scanner** and **TCMS-side simulator** built with **C++**, **Drogon**, and **EIPScanner**.

The application runs on a **laptop** or **Raspberry Pi** and behaves like a **TCMS node** in a rail network.  
It can initiate both:

- **Class 3 (explicit)** CIP connections for configuration and diagnostics  
- **Class 1 (implicit)** CIP I/O connections for real-time data exchange  

against any conformant EtherNet/IP / CIP device (drives, I/O blocks, rail equipment, etc.).

The goal is to have a **vendor-neutral** test and simulation tool for TCMS–equipment communications.

For the planned user experience, see the [CIP Originator Simulator UI Design](docs/UI_DESIGN.md), which outlines the target layout, interactions, and diagnostics views for the web front end.

---

## Goals

- Act as a **CIP originator / scanner** (not an adapter)
- Support **Class 3 explicit messaging**:
  - Read and write arbitrary Class/Instance/Attribute paths
  - Browse common CIP objects (Identity, Assembly, Connection Manager, etc.)
- Support **Class 1 implicit I/O connections**:
  - Open point-to-point I/O connections to equipment
  - Map raw I/O bytes to named “signals” (bits, words, engineering values)
  - Visualize and drive I/O data from a web UI
- Run on **Linux** (x86-64) and **Raspberry Pi**
- Stay **general purpose and vendor-independent**:
  - No vendor-specific assumptions in the core
  - Optional templates / EDS-based profiles for nicer views

---

## High-Level Architecture

```text
+-----------------------------+
|         Web Browser         |
|  (Operator / Developer UI)  |
+--------------+--------------+
               |
               | HTTP (Drogon)
               v
+-----------------------------+
|        Drogon Server        |
|  - Controllers (C++)        |
|  - HTML / JS views          |
|  - Device & I/O configs     |
+--------------+--------------+
               |
               | C++ API calls
               v
+-----------------------------+
|         EIPScanner          |
|  EtherNet/IP / CIP engine   |
|  - TCP 44818 (explicit)     |
|  - UDP 2222 (implicit I/O)  |
+--------------+--------------+
               |
               | Ethernet
               v
+-----------------------------+
|   CIP / EtherNet/IP Device  |
| (rail equipment, drives, IO)|
+-----------------------------+
````

* **Drogon** provides:

  * Routing, controllers, and the web UI
  * Storage of device configurations and signal mappings
* **EIPScanner** provides:

  * EtherNet/IP encapsulation and CIP message handling
  * Explicit messaging (Class 3)
  * I/O connections (Class 1) over UDP

---

## Features (planned / implemented)

### Device Management

* Add/delete/edit **CIP devices**
* Fields:

  * Device name (friendly name)
  * IP address / hostname
  * TCP port (default: 44818)
  * Request timeout (ms)
  * Optional: EDS file or internal template
* Test connection and show basic Identity Object info

### Class 3 – Explicit Messaging

* Generic **explicit message builder**:

  * Service Code (select common services or enter custom hex)
  * CIP Path (Class ID, Instance ID, Attribute ID)
  * Data type (UINT8/16/32, SINT, REAL, raw bytes)
  * Value / payload entry (number or hex)
* Send and show:

  * CIP general status / additional status
  * Response data (hex + decoded value)
* Optional: “Parameter table” view when EDS/template is available:

  * Named parameters with C/I/A path and data type
  * Read all / write selected

### Class 1 – Implicit I/O (I/O Connections)

* Configure **I/O connections** per device:

  * Connection name
  * Direction: input+output / input-only (T→O) / output-only (O→T)
  * Input Assembly instance (T→O)
  * Output Assembly instance (O→T)
  * Input/Output byte sizes
  * Requested Packet Interval (RPI) in ms
  * Transport: unicast or multicast (as supported by EIPScanner/device)
* Open/close connections via **Forward Open / Forward Close**
* Display connection state and statistics (packets, sequence, errors)

### Signal Mapping & Live I/O View

* For each I/O connection, define **signals** on top of assembly bytes:

  * Signal name
  * Direction (input or output relative to simulator)
  * Byte offset
  * Bit / word selection
  * Data type (BOOL, UINT8, UINT16, UINT32, SINT, REAL32)
  * Scaling factor and offset (engineering units)
  * Units string (%, bar, km/h, etc.)
  * Optional enum mapping (e.g. `0=Closed, 1=Open, 2=Fault`)
* Web UI widgets (per signal):

  * Outputs: checkboxes, sliders, numeric inputs, dropdowns
  * Inputs: LEDs, numeric labels, gauges/charts (later)
* Periodic polling or WebSocket updates to keep the I/O view live

---

## CIP Design Principles

To stay **general-purpose and vendor-neutral**:

* The core only assumes **standard CIP objects**:

  * Identity Object (0x01)
  * Assembly Object (0x04)
  * Connection Manager (0x06)
  * TCP/IP Interface (0xF5), Ethernet Link (0xF6) where present
* All **Class 3** logic is generic:

  * Service code + CIP path + raw payload in/out
  * No hard-coded vendor paths
* All **Class 1** logic is **assembly-centric**:

  * Input/output assembly instance IDs and sizes are configurable
  * No vendor-specific defaults
* Vendor / device “profiles” are optional:

  * Templates / EDS parsing can provide:

    * Default assembly IDs and sizes
    * Named parameters and signal layouts
  * But the tool must work with “manual” configuration only

---

## Technology Stack

* **Language**: C++17/20
* **Web framework**: [Drogon](https://github.com/drogonframework/drogon)
* **CIP / EtherNet/IP**: [EIPScanner](https://github.com/rideas/EIPScanner)
* **Build system**: CMake
* **Target platforms**:

  * Linux x86-64 (development laptop / PC)
  * Raspberry Pi (ARM)

---

## Building

### Prerequisites

* CMake (>= 3.16 recommended)
* A C++17/20 compiler (g++ or clang)
* OpenSSL and other Drogon dependencies
* EIPScanner (built and installed, or added as a submodule)

### Clone

```bash
git clone <this-repo-url> tcms-cip-simulator
cd tcms-cip-simulator
```

## Local build and smoke test (Ubuntu)

Install the packaged dependencies and development headers:

```bash
sudo dpkg -i drogon_1.9.11-0_amd64.deb eipscanner_1.3.0-1_amd64.deb
sudo apt-get update
sudo apt-get install -y libjsoncpp-dev libyaml-cpp-dev
```

Configure, build, and run the unit tests using the provided CMake preset:

```bash
cmake --preset linux-deb-packages
cmake --build --preset linux-deb-packages
ctest --preset default
```

Launch the server from the repository root and verify the health endpoint:

```bash
./build/deb/tcms_cip_sim &
curl http://localhost:8080/healthz
```

You should receive `{ "status": "ok" }` from the health check when the server is running.

### EIPScanner

Option 1 – Install system-wide:

```bash
git clone https://github.com/rideas/EIPScanner.git
cd EIPScanner
mkdir build && cd build
cmake ..
cmake --build . --target install
```

Option 2 – Add as a git submodule and reference it from this project’s CMake (to be documented once integrated).

### Build

```bash
cd tcms-cip-simulator
mkdir build && cd build
cmake ..
cmake --build .
```

On successful build, the main executable (e.g. `tcms_cip_sim`) will be placed in `build/`.

## Development environment setup

Prebuilt Debian packages for the core dependencies are stored in the repository root:

- `drogon_1.9.11-0_amd64.deb`
- `eipscanner_1.3.0-1_amd64.deb`

Install them into your build environment with:

```bash
sudo dpkg -i drogon_1.9.11-0_amd64.deb eipscanner_1.3.0-1_amd64.deb
```

If you prefer a scripted install, run `scripts/install_debian_dependencies.sh` (requires `sudo`).

### Build options

**Bundled packages (x86_64/PC)**

- Use the Debian packages above and configure via:
  ```bash
  cmake --preset linux-deb-packages
  cmake --build build/deb
  ctest --test-dir build/deb --output-on-failure
  ```

**Source build for EIPScanner (PC/RPi/arm64)**

- When the amd64 `.deb` is not usable (e.g., Raspberry Pi), build EIPScanner 1.3.0 from source:
  ```bash
  scripts/build_with_eipscanner_source.sh
  ```
- The preset `linux-eipscanner-src` tells CMake to fetch EIPScanner from Git while keeping Drogon from your package manager.

### CMake detection test

This repository includes a small CMake project that confirms the installed Drogon and EIPScanner packages are discoverable through `find_package` and can be linked together. Build it from the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/tests/library_detection
```

The `library_detection` binary will print a short confirmation message if both libraries were found and linked correctly.

---

## Running

Build the project and run the Drogon server:

```bash
cmake -S . -B build
cmake --build build
./build/tcms_cip_sim
```

Runtime configuration lives in `config/config.json`. Update the `listeners` block to change the HTTP port or bind address and adjust the `log` section to tune file/console logging.

Once the server is running, verify it is reachable with the built-in health check:

* `http://localhost:8080/healthz` – basic liveness probe returning `{ "status": "ok" }`

Additional routes will be added as device CRUD and CIP features come online.

### Deployment (PC / Raspberry Pi)

**Systemd service**

1. Copy the built binary and configuration to `/opt/tcms-cip-sim`.
2. Create a service account (e.g., `tcms`) with permission to read the config and write logs.
3. Install `packaging/systemd/tcms-cip-sim.service` to `/etc/systemd/system/` and adjust paths if needed.
4. Enable and start:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable --now tcms-cip-sim
   sudo systemctl status tcms-cip-sim
   ```

**Container image (multi-arch PC/RPi)**

The multi-stage Dockerfile in `packaging/container/Dockerfile` builds EIPScanner from source so it can target both amd64 and arm64.

```bash
# For local amd64 builds
docker build -f packaging/container/Dockerfile -t tcms-cip-sim:local .

# For multi-arch (requires Docker Buildx)
docker buildx build \
  --platform linux/amd64,linux/arm64 \
  -f packaging/container/Dockerfile \
  -t ghcr.io/<org>/tcms-cip-sim:latest .
```

Run the container with a bind-mounted config if you need to override defaults:

```bash
docker run -p 8080:8080 -v $(pwd)/config:/opt/tcms-cip-sim/config tcms-cip-sim:local
```

---

## Typical Workflow

1. **Add a device**

   * Go to `/devices`
   * Click “Add device”
   * Enter name, IP, port (44818), timeout, and optional EDS/template

2. **Check Identity**

   * Open the device page
   * Use the explicit messaging tab to read:

     * Class 0x01, Instance 1, Attributes (Vendor ID, Product Code, Revision, etc.)

3. **Configure Class 1 I/O**

   * From the I/O tab:

     * Enter input/output Assembly instance IDs and sizes (from device manual or EDS)
     * Set an RPI (e.g. 10–20 ms)
     * Choose unicast or multicast
   * Click “Open Connection”

4. **Define signals**

   * Add signal mappings on top of assembly bytes
   * For each signal, specify name, offset, type, scaling, units
   * Save the mapping as a device profile

5. **Simulate TCMS behavior**

   * Use the live I/O view to:

     * Toggle outputs (commands to equipment)
     * Monitor inputs (feedback from equipment)
   * Use explicit messaging for deeper diagnostics and configuration

---

## Rail TCMS Context

This tool is intended for **train control and monitoring system (TCMS)** integration and lab testing.
Typical use cases:

* Emulate the **TCMS side** of a CIP-based equipment network
* Commission and debug rail subsystems (doors, HVAC, brakes, traction) that expose CIP/EtherNet/IP interfaces
* Experiment with **Class 1 + Class 3** communication patterns similar to those used in other rail communication stacks (e.g. TRDP PD/MD) while using CIP as the underlying protocol

The design stays generic so it can be used outside rail as a general EtherNet/IP / CIP scanner and simulator.

---

## Roadmap / TODO

* [ ] Integrate EIPScanner as a submodule and wrap it in a clean C++ interface
* [ ] Implement basic device CRUD and Identity Object read
* [ ] Implement generic Class 3 explicit message builder in the web UI
* [ ] Implement Class 1 I/O connection management (Forward Open/Close)
* [ ] Implement signal mapping UI and live I/O visualization
* [ ] Support JSON/YAML export/import of device templates
* [ ] Optional: EDS parser for auto-populating assemblies and parameters
* [ ] Optional: simple trend charts for selected input signals
* [ ] Optional: TRDP ↔ CIP bridge modes for mixed lab setups

---

## License

This project is licensed under the MIT License (same as EIPScanner).
See `LICENSE` for details.

```
