// const now = new Date();

export function getTime() {
  const now = new Date();

  const options = {
    hour: "2-digit",
    minute: "2-digit",
    hour12: true,
    weekday: "long",
    day: "2-digit",
    month: "long",
    year: "numeric",
  };

  const formatted = now.toLocaleString("en-US", options);
  return `its ${formatted}.`;
}   

// export 