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

const rl = readline.createInterface({ input, output });

export function routecommand(text){
    const t = text.toLowerCase().trim();
    if (t.includes("what time") || t.includes("current time") || t.startsWith("time")) {
         return getTime();
    }

    if(t.startsWith("due")){
        const message = checkduedates();
        if(message.length === 0){
            return "Nothing due soon"
        }
        return message.join(" | ");
    }

    if (t.startsWith("add date")) {
    const match = t.match(/add date (.+?) on (\d{4}-\d{1,2}-\d{1,2})/);
    if (match) {
        const label = match[1].trim();
        const dueDate = match[2].trim();
        return addduedate(label, dueDate);
    }
    return "Say it like: add date <name> on 2026-07-20.";
}

if (t.startsWith("countdown")) {
    const match = t.match(/countdown (\d+(?:\.\d+)?)\s*h/);
    if (match) {
        const hours = parseFloat(match[1]);
        return startCountdown(hours);
    }
    return "Say it like: countdown 2h.";
}

if (t.startsWith("stopwatch")) {
    return startStopwatch();
}

if (t.startsWith("remind me")) {
  const match = t.match(/remind me to (.+) in (\d+)\s*(minute|minutes|min)/);
  if (match) {
    const message = match[1].trim();
    const minutes = parseInt(match[2], 10);
    return sechdulereminder(minutes, message);
  }
  return "Say it like: remind me to <thing> in <N> minutes.";
}
    return null;
} 

function stripEmojis(text) {
  return text.replace(/[\u{1F300}-\u{1FAFF}\u{2600}-\u{27BF}\u{1F900}-\u{1F9FF}]/gu, "").trim();
}

export async function main(){
    const memrory = loadmemory();
    const history = [
            { role: "system", content: SYSTEM_PROMPT + buildmemorycontext(memrory) }
        ];
    while(true){
        const text = await rl.question("> ");
        // if(text=="exit"){
        //     break;
        // }
        const lower = text.toLowerCase();
        if(lower.startsWith("remember that")){
            const fact = text.split(/that/i).slice(1).join("that").trim();
            addmemory(memrory, fact);
            console.log(`Saved to memory: ${fact}`);
            continue;
        }
        try{
        const toolres = routecommand(text);
        let reply;
        const MAX_HISTORY = 10;
        if(toolres !== null){
            reply = toolres;
            // console.log(reply);
            await display(reply);
        }else{
        history[0].content = SYSTEM_PROMPT + buildmemorycontext(memrory);
        history.push({ role: "user", content: text });
        if (history.length > MAX_HISTORY + 1) {
             history.splice(1, history.length - 1 - MAX_HISTORY);
        }
        reply = stripEmojis(await askChat(history));
        history.push({ role: "assistant", content: reply });
        console.log(reply);
        await display(reply);
        }
        }catch(e){
            console.log("got error: ",e.message);
        }
    }
}

main();