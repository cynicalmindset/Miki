import fs from "node:fs";

import { REMINDERS_FILE } from "../config.js";
// import { loadEnvFile } from "node:process";

export function laodreminders(){
    if(fs.existsSync(REMINDERS_FILE)){
        const raw = fs.readFileSync(REMINDERS_FILE, "utf-8");
const data = JSON.parse(raw);
return data;
    }else{
        return [];
    }
}

export function savereminders(reminders){
    try {
        fs.writeFileSync(REMINDERS_FILE,JSON.stringify(reminders, null, 2))
    } catch (error) {
        console.log("error in saving the reminder: ",error.message)
    }
}

export function sechdulereminder(minute,message){
    const fireAt = new Date(Date.now() + minute * 60_000).toISOString();
    const reminders = laodreminders();
    reminders.push({fireAt, message})
    savereminders(reminders);
    setTimeout(() => {
  console.log(`\n[REMINDER] ${message}\n`);
}, minute * 60_000);
return `Reminder set for ${minute} minutes from now: ${message}`;
}