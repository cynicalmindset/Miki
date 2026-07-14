import readline from "node:readline/promises";
import { stdin as input, stdout as output } from "node:process";
import { SYSTEM_PROMPT } from "./config.js";
import { ask } from "./llm.js";
import {sechdulereminder } from "./tools/reminder.js"
import { getTime } from "./tools/time.js"

const rl = readline.createInterface({ input, output });

export function routecommand(text){
    const t = text.toLowerCase().trim();
    if (t.includes("what time") || t.includes("current time") || t.startsWith("time")) {
  return getTime();
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


export async function main(){
    while(true){
        const text = await rl.question("> ");
        // if(text=="exit"){
        //     break;
        // }
        try{
        const toolres = routecommand(text);
        let reply;
        if(toolres !== null){
            reply = toolres;
            console.log(reply);
        }else{
        reply = await ask(text, SYSTEM_PROMPT);
        console.log(reply)
        }
        }catch(e){
            console.log("got error: ",e.message);
        }
    }
}

main();