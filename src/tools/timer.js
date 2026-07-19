import { display } from "../display.js";

function formatTime(totalSeconds) {
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = Math.floor(totalSeconds % 60);

  const pad = (n) => String(n).padStart(2, "0");
  return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
}

export function startCountdown(hours) {
  let remaining = hours * 3600;

  const intervalId = setInterval(async () => {
    if (remaining <= 0) {
      clearInterval(intervalId);
      await display("Time's up!");
      return;
    }

    await display(formatTime(remaining));
    remaining--;
  }, 1000);

  return `Countdown started for ${hours} hour(s).`;
}

export function startStopwatch() {
  let elapsed = 0;

  const intervalId = setInterval(async () => {
    await display(formatTime(elapsed));
    elapsed++;
  }, 1000);

  return "Stopwatch started.";
}