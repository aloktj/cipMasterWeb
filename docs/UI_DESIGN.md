# CIP Originator Simulator – UI Design Specification

The following design describes the target user interface for the CIP Originator Simulator. It can be shared directly with front-end developers or used as internal documentation when implementing the UI.

## 1. Main Layout
The interface follows a three-panel structure:

```
┌──────────────────────────────┬──────────────────────────────────┐
│         Left Sidebar         │          Main Workspace          │
│  (Navigation & connection)   │      (Tabs for assemblies)       │
└──────────────────────────────┴──────────────────────────────────┘
```

## 2. Left Sidebar
### 2.1 Connection Controls
- **Target IP**: Text field
- **RPI (ms)**: Dropdown (5, 10, 20, 50, …)
- **Connect / Disconnect** button
- **Connection status badge**: 🟢 Connected, 🟡 Connecting, 🔴 Disconnected

### 2.2 XML Configuration
- **Load CIP XML**: File selector / upload button
- Displays the number of assemblies parsed and device identity info from the XML

### 2.3 Navigation Items
- Assemblies
- Input Assemblies (T→O)
- Output Assemblies (O→T)
- Config Assembly
- Logs
- Diagnostics
- Settings

## 3. Main Workspace
The workspace uses tabs, each representing a loaded assembly, such as:

`[ Input Assemblies ] [ Output Assemblies ] [ Config ] [ Logs ] [ Diagnostics ]`

## 4. Assembly View (Core Screen)
Each assembly is displayed as a table of signals.

### 4.1 Assembly Header
- Assembly name and instance (e.g., `ASSEMBLY_IN (Instance 0x65)`)
- Direction (T→O incoming or O→T outgoing)
- Size (e.g., `1120 bits / 140 bytes`)
- Last update (e.g., `14 ms ago`)
- Multicast address (e.g., `239.x.y.z`)
- Heartbeat indicator pulses every RPI

## 5. Signal Table
Columns: **Name**, **Value**, **Type**, **Offset (bit)**, **Byte Index**, **Actions**.

Example rows:

| Name    | Value  | Type    | Offset (bit) | Byte Index | Actions        |
|---------|--------|---------|--------------|------------|----------------|
| SignalA | 1      | BOOL    | 0            | 0          | —              |
| SignalB | 123    | UINT16  | 16           | 2          | —              |
| SignalC | false  | BOOL    | 32           | 4          | —              |
| SignalD | 45.60  | REAL32  | 64           | 8          | —              |
| SignalE | "ABCD" | STRING  | 128          | 16         | Edit (if O→T)  |

### 5.1 Incoming Assemblies (T→O)
- Values are read-only and update live each RPI
- Updated cells briefly highlight (e.g., yellow flash)

### 5.2 Outgoing Assemblies (O→T)
- Editable fields based on type:
  - **BOOL**: Toggle switch
  - **INT/REAL**: Numeric text box
  - **STRING**: Text input with length limit
- When a value changes:
  - Local O→T buffer updates immediately
  - Next cycle sends the updated payload
  - A green **Pending** badge appears until the next RPI cycle, then becomes **Sent**

## 6. Raw Hex View (Side Panel)
A collapsible panel shows raw payloads for debugging:
- **Incoming (T→O)** and **Outgoing (O→T)** payloads with byte counts
- Differences from the last cycle highlighted

## 7. Diagnostics Screen
Displays operational health:
- Heartbeat monitor and time since last update
- Expected RPI vs. actual cycle duration
- Connection status timeline
- Forward_Open parameters, assemblies used, transport type, RPI
- Vendor ID, Device Type, timeout counters, dropped packets
- Small graph of RPI jitter (last 100 cycles)

## 8. Logs Screen
Real-time, vertically scrolling logs with examples like:
- `[12:10:14.002] Connecting to 192.168.1.10`
- `[12:10:14.030] Forward_Open succeeded (ConnID 0x123456)`
- `[12:10:14.041] T→O update received (140 bytes)`
- `[12:10:14.052] O→T payload sent (142 bytes)`

Filters: CIP Events, Decoding Events, Send/Receive Events, Errors.

## 9. Settings Panel
- Theme (Light/Dark)
- Auto-scroll logs
- Update rate (if UI throttling is needed)
- Strict/Lenient XML validation mode
- Payload change highlighting (on/off)

## 10. UX Interaction Details
- Live updating at UI-friendly rate (e.g., 10–20 FPS) while the backend processes every RPI
- Error states: missing assembly banner, XML parsing failure with line details, CIP timeout disabling editing with reconnect option
- Accessibility: keyboard navigation and clear color-coded statuses

## 11. Mobile/Tablet Responsive Layout
- Signal table collapses to cards showing value, type, and offset
- Logs become a stacked view
- Tabs become horizontally scrolling chips

## 12. Visual Theme
- Modern engineering style with blue/teal accents
- Subtle animation on updates
- Monospaced font for hex/offsets
- Highlight values when changed

## 13. UX Goals
- **Transparent**: clearly show Class-1 I/O cycle activity
- **Controllable**: allow safe modification of outgoing signals
- **Debuggable**: provide low-level payload visibility
- **User-friendly**: clean UI with engineering precision
