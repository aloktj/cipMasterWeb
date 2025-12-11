# UI Development Task Outline

This document translates the [CIP Originator Simulator UI Design](UI_DESIGN.md) into actionable development tasks. It is meant to help break down implementation work for the web front end.

## Foundational Setup
- Establish the three-panel layout (left sidebar, main workspace tabs) with responsive behavior for desktop and mobile.
- Add theme scaffolding (light/dark) and base typography/spacing consistent with the engineering-style visuals.

## Connectivity & Configuration
- Implement connection form for target IP and RPI selections with connect/disconnect control and status badges.
- Build XML loader UI with parsed assembly count and device identity summary.
- Wire connection status indicators to backend state (Connected/Connecting/Disconnected) and heartbeat pulse.

## Assembly Tabs & Navigation
- Create tabbed navigation for Input Assemblies, Output Assemblies, Config Assembly, Logs, and Diagnostics.
- Persist selected tab and remember last open assembly per session.

## Assembly View & Signal Table
- Render assembly header with name, instance, direction, size, last update, multicast address, and heartbeat indicator.
- Build signal table columns for Name, Value, Type, Offset (bit), Byte Index, and Actions.
- Support read-only live updates for incoming (T→O) assemblies, including brief change highlighting per RPI.
- Provide editable controls for outgoing (O→T) assemblies:
  - Toggles for BOOL
  - Numeric inputs for INT/REAL with validation
  - Length-limited text inputs for STRING
- Show "Pending" → "Sent" state around the RPI cycle when outbound values change.

## Raw Hex View
- Add collapsible side panel showing current T→O and O→T payloads with byte counts.
- Highlight differences versus previous cycle for quick visual diffing.

## Diagnostics
- Implement diagnostics page with heartbeat monitor, last update timer, expected vs. actual RPI, and connection status timeline.
- Display Forward_Open parameters, assemblies, transport type, RPI, vendor/device identifiers, timeout counters, and dropped packet metrics.
- Include a small graph of RPI jitter over the last ~100 cycles.

## Logs
- Create real-time log stream with timestamped entries for connection, Forward_Open, and I/O events.
- Add filters for CIP events, decoding, send/receive, and errors.
- Support auto-scroll toggle.

## Settings
- Provide theme toggle, log auto-scroll preference, UI update rate (throttling), XML validation mode (strict/lenient), and payload change highlighting toggle.

## Responsiveness & Accessibility
- Define mobile/tablet layouts: collapse signal tables into cards, stack logs vertically, and present tabs as horizontal chips.
- Ensure keyboard navigation for editing and clear color-coded statuses.

## Performance & UX Polish
- Throttle UI refresh to a user-friendly rate (10–20 FPS) while processing every RPI.
- Handle error states: missing assembly banner, XML parse errors with line details, CIP timeout disabling editing with reconnect option.
- Add subtle animations for updates and monospaced styling for offsets/hex values.

## Integration Notes
- Plan data flow between backend and UI for live updates (e.g., WebSockets) and outbound payload writes timed to the RPI.
- Ensure payload diffing and pending/sent states align with backend I/O cycle timing.
