# AKSYN Audio Engine

Real-time audio streaming engine using **PortAudio** + **UDP sockets**.  
Captures voice from a phone (via WO Mic) and streams it to laptop speakers with live latency measurement.

---

## Requirements

- **WO Mic** app installed on your phone + WO Mic PC client connected
- **MinGW/GCC** on Windows (g++ in PATH)
- PortAudio DLL files in project folder (`libportaudio-64.dll`, `libportaudio.dll.a`)

---

## Files

| File | Purpose |
|------|---------|
| `sender_pa.cpp` | Captures WO Mic audio and sends over UDP |
| `receiver_pa.cpp` | Receives UDP packets, plays audio, logs latency to CSV |
| `latency_graph.html` | Interactive graph of the latency data |
| `discover_devices.cpp` | Lists all PortAudio device IDs (run once to find WO Mic) |

---

## Build

```powershell
# Build sender
g++ sender_pa.cpp -o sender_pa.exe -I. -L. -lportaudio -lws2_32

# Build receiver
g++ receiver_pa.cpp -o receiver_pa.exe -I. -L. -lportaudio -lws2_32
```

---

## Run

> ⚠️ Make sure WO Mic app on your phone shows **"Connected"** first.

**Terminal 1 — Start Receiver first:**
```powershell
.\receiver_pa.exe
```
Wait for: `[*] Jitter Buffer Active. Listening...`

**Terminal 2 — Start Sender:**
```powershell
.\sender_pa.exe
```
You'll see: `[*] Sender Active. Capturing from: Microphone (WO Mic Device) (ID 7)`

**Speak into your phone** — audio plays through laptop speakers with live latency printed:
```
[LATENCY] 1.23 ms
```

Latency is also saved to `latency_log.csv` automatically.

---

## View Latency Graph

Press **Enter** in both terminals to stop, then open:
```powershell
Start-Process latency_graph.html
```
Or just double-click `latency_graph.html` in File Explorer.

---

## Find WO Mic Device ID (if it changes)

```powershell
g++ discover_devices.cpp -o discover.exe -I. -L. -lportaudio
.\discover.exe
```
Look for `Microphone (WO Mic Device)` and update the device ID in `sender_pa.cpp`.

---

## Architecture

```
Phone Mic
   │
   ▼
WO Mic Driver (treats phone as local mic)
   │
   ▼
sender_pa.exe  ──UDP:5001──▶  receiver_pa.exe
(captures audio,              (jitter buffer,
 timestamps packets)           plays to speakers,
                               logs latency to CSV)
                                      │
                                      ▼
                              latency_graph.html
```
