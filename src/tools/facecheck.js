import fs from "node:fs";
import { execFile } from "node:child_process";
import { promisify } from "node:util";

const execFileAsync = promisify(execFile);

const ESP32_CAPTURE_URL = "http://10.143.78.112/capture";
const FRAME_PATH = "frame.jpg";
const FACE_CHECK_SCRIPT = "face_check.py";

export async function checkFace() {
  try {
    const res = await fetch(ESP32_CAPTURE_URL);
    const buffer = Buffer.from(await res.arrayBuffer());
    fs.writeFileSync(FRAME_PATH, buffer);

    const result = await execFileAsync("python", [FACE_CHECK_SCRIPT, FRAME_PATH]);
    return result.stdout.trim() === "match";
  } catch (e) {
    console.log("face check failed:", e.message);
    return false;
  }
}