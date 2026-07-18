import readline from "node:readline/promises";
import { stdin as input, stdout as output } from "node:process";
import { SYSTEM_PROMPT } from "./config.js";
import { ask } from "./llm.js";
import {display} from "./display.js"
import {sechdulereminder } from "./tools/reminder.js"
import { getTime } from "./tools/time.js"
import {checkduedates, addduedate} from "./tools/duedate.js"

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
            // console.log(reply);
            await display(reply);
        }else{
        reply = await ask(text, SYSTEM_PROMPT);
        // console.log(reply);
        await display(reply);
        }
        }catch(e){
            console.log("got error: ",e.message);
        }
    }
}

main();