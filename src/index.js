import readline from "node:readline/promises";
import { stdin as input, stdout as output } from "node:process";
import { SYSTEM_PROMPT } from "./config.js";
import { ask, askChat } from "./llm.js";
import {display} from "./display.js"
import {sechdulereminder } from "./tools/reminder.js"
import { getTime } from "./tools/time.js"
import {checkduedates, addduedate} from "./tools/duedate.js"
import {loadmemory, buildmemorycontext,addmemory} from "./memory.js"
import {startCountdown, startStopwatch} from "./tools/timer.js"
import { checkFace } from "./tools/facecheck.js";
import { routecommand } from "./router.js";
import express from "express";
import { fileURLToPath } from "node:url";
import path from "node:path";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const CLIENT_DIR = path.join(__dirname, "..", "client");


const memrory = loadmemory();
const history = [
  { role: "system", content: SYSTEM_PROMPT + buildmemorycontext(memrory) }
];
const MAX_HISTORY = 10;

async function handleMessage(text) {
  const lower = text.toLowerCase();

  if (lower.startsWith("remember that")) {
    const fact = text.split(/that/i).slice(1).join("that").trim();
    addmemory(memrory, fact);
    return `Saved to memory: ${fact}`;
  }

  const toolres = await routecommand(text);
  let reply;

  if (toolres !== null) {
    reply = toolres;
  } else {
    history[0].content = SYSTEM_PROMPT + buildmemorycontext(memrory);
    history.push({ role: "user", content: text });
    if (history.length > MAX_HISTORY + 1) {
      history.splice(1, history.length - 1 - MAX_HISTORY);
    }
    reply = stripEmojis(await askChat(history));
    history.push({ role: "assistant", content: reply });
  }

  await display(reply);
  return reply;
}

const rl = readline.createInterface({ input, output });

// let ispresent = false;

// export async function routecommand(text){
//     const t = text.toLowerCase().trim();
//     if (t.includes("what time") || t.includes("current time") || t.startsWith("time")) {
//          return getTime();
//     }

//     if(t.startsWith("due")){
//         const message = checkduedates();
//         if(message.length === 0){
//             return "Nothing due soon"
//         }
//         return message.join(" | ");
//     }

//     if (t.startsWith("add date")) {
//     const match = t.match(/add date (.+?) on (\d{4}-\d{1,2}-\d{1,2})/);
//     if (match) {
//         const label = match[1].trim();
//         const dueDate = match[2].trim();
//         return addduedate(label, dueDate);
//     }
//     return "Say it like: add date <name> on 2026-07-20.";
// }

// if (t.startsWith("countdown")) {
//     const match = t.match(/countdown (\d+(?:\.\d+)?)\s*h/);
//     if (match) {
//         const hours = parseFloat(match[1]);
//         return startCountdown(hours);
//     }
//     return "Say it like: countdown 2h.";
// }

// if (t.startsWith("stopwatch")) {
//     return startStopwatch();
// }

// if (t.startsWith("remind me")) {
//   const match = t.match(/remind me to (.+) in (\d+)\s*(minute|minutes|min)/);
//   if (match) {
//     const message = match[1].trim();
//     const minutes = parseInt(match[2], 10);
//     return sechdulereminder(minutes, message);
//   }
//   return "Say it like: remind me to <thing> in <N> minutes.";
// }

// if (t === "servo yes" || t === "servo no") {
//     await display(t);
//     return "ok.";
// }

//     return null;
// }

function stripEmojis(text) {
  return text.replace(/[\u{1F300}-\u{1FAFF}\u{2600}-\u{27BF}\u{1F900}-\u{1F9FF}]/gu, "").trim();
}



async function detectUser(history) {
    const faceDetected = await checkFace();

    if (!faceDetected) {
        console.log("face not found");
        return;       
    }

    const greeting = stripEmojis(
        await askChat([
            ...history,
            {
                role: "user",
                content: "the user just walked in and you noticed them, greet them"
            }
        ])
    );

    console.log(greeting);
    await display(greeting);
    await display("servo yes");
}

const app = express();
app.use(express.json());
app.use(express.static(CLIENT_DIR));

app.post("/chat", async (req, res) => {
  try {
    const reply = await handleMessage(req.body.text || "");
    res.json({ reply });
  } catch (e) {
    res.json({ reply: `error: ${e.message}` });
  }
});

app.listen(3000, () => console.log("web UI running at http://localhost:3000"));

export async function main(){
    // const memrory = loadmemory();
    // // detectUser()
    // const history = [
    //         { role: "system", content: SYSTEM_PROMPT + buildmemorycontext(memrory) }
    //     ];

    await detectUser(history); 

    // let isPresent = false;

  
    // const faceDetected = await checkFace();

    // if (faceDetected && !isPresent) {
    //     isPresent = true;
    //     const greeting = stripEmojis(await askChat([
    //     ...history,
    //     { role: "user", content: "the user just walked in and you noticed them, greet them" }
    //     ]));
    //     console.log(greeting);
    //     await display(greeting);
    //     await display("servo yes");
    // } else if (!faceDetected) {
    //     isPresent = false;
    // }
    

     while (true) {
    const text = await rl.question("> ");
    try {
      const reply = await handleMessage(text);
      console.log(reply);
    } catch (e) {
      console.log("got error: ", e.message);
    }
  }
}

main();