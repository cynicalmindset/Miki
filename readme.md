# Miki

Miki is a terminal-based AI companion that runs a local LLM (via Ollama), talks back to you
through a physical ESP32-CAM device with a tiny OLED "face," and can recognize when you walk
into the room.

No cloud APIs, no voice input. You type in a terminal, Miki thinks locally, and replies show
up both in your terminal and on a physical screen sitting on your desk.

---

## Index

- [How it works (architecture)](#how-it-works-architecture)
- [Features](#features)
- [Project structure](#project-structure)
- [Setup](#setup)
  - [1. Prerequisites](#1-prerequisites)
  - [2. Node setup](#2-node-setup)
  - [3. ESP32 firmware setup](#3-esp32-firmware-setup)
  - [4. Point Node at your ESP32](#4-point-node-at-your-esp32)
  - [5. Face recognition setup](#5-face-recognition-setup)
- [Configuration](#configuration)
- [Known issues / rough edges](#known-issues--rough-edges)
- [Persona](#persona)

---

## How it works (architecture)

There are two separate programs that talk to each other over your local WiFi:

```
┌─────────────────────┐        HTTP         ┌──────────────────────────┐
│   Node.js CLI (PC)   │ <-----------------> │   ESP32-CAM (firmware)   │
│                      │                     │                          │
│  - REPL              │   POST /update      │  - OLED display driver   │
│  - Ollama calls       │   (text to show)    │  - Eye animations        │
│  - Tools (reminders,  │                     │  - Servo (nod/shake)     │
│    countdowns, etc)  │   GET  /capture      │  - Camera (JPEG frames)  │
│  - Face-check loop    │   (JPEG frame)      │  - Live MJPEG stream      │
│  - Memory + history   │                     │                          │
└─────────────────────┘                     └──────────────────────────┘
        │
        │  spawns
        v
┌──────────────────────┐
│  Python subprocess    │
│  face_check.py        │
│  (dlib / face_recog.) │
└──────────────────────┘
```

- **Node.js** is the "brain" — it runs the conversation loop, calls your local Ollama model,
  and decides what to show or do.
- **The ESP32-CAM** is a "dumb" peripheral — it only knows how to display text, animate eyes,
  wiggle a servo, and serve camera frames over WiFi. It has no intelligence of its own.
- **A small Python script** is shelled out to from Node for the one job Node isn't well
  suited for: comparing a camera frame against your enrolled face.

---

## Features

- **Local LLM chat** via Ollama (`qwen3:8b`), with real multi-turn conversation memory
  (not just single-shot prompts)
- **Long-term memory** — say `remember that <fact>` and Miki keeps it forever, and factors it
  into every future reply
- **Custom persona** — fully defined in `config.js`, currently a calm, manipulative,
  question-heavy character (easy to rewrite into anything else)
- **Tools**, each in its own file under `src/tools/`:
  - `time.js` — current date/time
  - `duedates.js` — track exam/assignment due dates, get day-count reminders
  - `reminders.js` — one-off "remind me to X in N minutes" reminders
  - `timer.js` — countdown timers and a stopwatch, both pushed live to the OLED
  - `facecheck.js` — face-presence detection via the camera
- **Physical OLED output** — every reply is mirrored to a small 128x64 screen, with automatic
  word-wrapping and paging for long messages
- **Idle eye animations** — static resting "eyes," a wake-up animation on boot, and periodic
  blinking, all non-blocking so the web server stays responsive
- **Servo head gestures** — `"servo yes"` nods, `"servo no"` shakes
- **Live camera stream** — `/stream` endpoint for watching the raw feed in a browser
- **Face recognition wake** — Miki notices when you walk in front of the camera and greets you
  automatically, once per appearance

---

## Project structure

```
miki/
├── package.json
├── reference.jpg          # your enrolled face photo (for face recognition)
├── face_check.py          # Python face-comparison script
├── memory.json            # auto-created: long-term facts
├── reminders.json         # auto-created: scheduled reminders
├── dueitems.json          # auto-created: exam/assignment due dates
└── src/
    ├── config.js           # all constants: model name, prompts, sleep hours, file paths
    ├── llm.js               # Ollama calls (both /api/generate and /api/chat)
    ├── memory.js             # long-term fact storage
    ├── display.js            # pushes text to the ESP32 OLED over HTTP
    ├── index.js               # the REPL — this is what you run
    └── tools/
        ├── index.js (routecommand)  # decides which tool (if any) matches user input
        ├── time.js
        ├── duedates.js
        ├── reminders.js
        ├── timer.js
        └── facecheck.js
```

The ESP32 firmware (a single `.ino` sketch) is a separate project, flashed independently onto
the physical device — it is not part of the Node codebase.

---

## Setup

### 1. Prerequisites

- [Ollama](https://ollama.com) installed and running (`ollama serve`), with a model pulled:
  ```bash
  ollama pull qwen3:8b
  ```
- Node.js 18+ (native `fetch` support required)
- Python 3.x with `pip`, for the face-recognition helper script
- Arduino IDE, for flashing the ESP32-CAM firmware
- An AI-Thinker-style ESP32-CAM board with an OLED (SSD1306, 128x64) and, optionally, a servo

### 2. Node setup

```bash
cd miki
npm install
node src/index.js
```

Type `exit` to quit. Type `remember that <fact>` any time to save something to long-term
memory.

### 3. ESP32 firmware setup

Flash the `.ino` sketch via Arduino IDE. Required libraries (Library Manager):
- `Adafruit_GFX`
- `Adafruit_SSD1306`
- `esp_camera` (bundled with ESP32 board support)

Update these two lines in the sketch with your own WiFi credentials before flashing:
```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

After flashing, open the Serial Monitor (115200 baud) to find the board's IP address — you'll
need this for the next step.

### 4. Point Node at your ESP32

Update the ESP32 IP address in:
- `src/display.js` (`ESP32_URL`)
- `src/tools/facecheck.js` (`ESP32_CAPTURE_URL`)

Router-assigned IPs can change after a power cycle — if things stop connecting, check the
Serial Monitor for the current IP first before assuming something is broken.

### 5. Face recognition setup

```bash
pip install face_recognition
```

Enroll your face once — capture a clear, well-lit frame from the camera and save it as
`reference.jpg` in the project root:

```bash
node -e "
import('fs').then(async (fs) => {
  const res = await fetch('http://<esp32-ip>/capture');
  const buffer = Buffer.from(await res.arrayBuffer());
  fs.writeFileSync('reference.jpg', buffer);
  console.log('saved reference.jpg');
});
"
```

Test the comparison script works before relying on it:
```bash
python face_check.py reference.jpg
```
This should print `match` (comparing the reference photo against itself).

---

## Configuration

Everything tunable lives in `src/config.js`:

| Constant | Purpose |
|---|---|
| `OLLAMA_URL` | Ollama's `/api/generate` endpoint |
| `MODEL` | which local model to use |
| `SLEEP_START_HOUR` / `SLEEP_END_HOUR` | hours during which Miki responds groggily |
| `DEFAULT_CITY` | fallback city for weather-style tools, if added |
| `MEMORY_FILE` / `REMINDERS_FILE` / `EXAMS_FILE` | where JSON data is persisted |
| `SYSTEM_PROMPT` | Miki's entire personality |
| `SLEEP_SYSTEM_PROMPT` | personality used during sleep hours |

`MAX_HISTORY` (in `index.js`) controls how many recent conversation turns are kept before
older ones are trimmed — keeps replies coherent and prevents the model from looping on old
phrasing.

---

## Known issues / rough edges

- **Face-check subprocess is slow.** Every check spins up a fresh Python process and reloads
  face-recognition models from scratch — expect 1–3+ seconds per check. If this becomes a
  bottleneck, a persistent Python process (instead of one-shot subprocess calls) would help.
- **No overlap guard on the face-check interval.** If a check takes longer than the poll
  interval, two checks could theoretically run concurrently.
- **ESP32 IP is hardcoded in two places** (`display.js`, `facecheck.js`) rather than pulled
  from a single shared config — worth consolidating.
- **MJPEG streaming blocks the ESP32's web server** for as long as the stream is open — no
  other requests (`/update`, `/capture`) are served while someone is watching `/stream`.
- **Model instruction-following isn't perfect** — the system prompt asks for things like "no
  emojis" and strict word limits, but qwen3:8b doesn't always obey perfectly. Emoji stripping
  is done in code (`stripEmojis` in `index.js`) as a guaranteed backstop.

---

## Persona

Miki's personality is fully defined as plain text in `SYSTEM_PROMPT` inside `src/config.js` —
no code changes needed to reshape it, just rewrite the prompt. The current persona is calm,
composed, quietly manipulative, and prone to asking pointed questions rather than answering
directly — a deliberate design choice, not a default.