import fs from "node:fs";
import { DUE_ITEMS_FILE } from "../config.js";

export function laodduedates(){
    if(fs.existsSync(DUE_ITEMS_FILE)){
        const raw = fs.readFileSync(DUE_ITEMS_FILE, "utf-8");
        const data = JSON.parse(raw);
        return data;
    }else{
        return [];
    }
}


export function saveduedates(duedates){
    try {
        fs.writeFileSync(DUE_ITEMS_FILE,JSON.stringify(duedates, null, 2))
    } catch (error) {
        console.log("due date: ",error.message)
    }
}

export function addduedate(lable, duedate){
    const prevdue = laodduedates();
    const newitem = {
        lable,
        duedate,
        remindwithindays:10,
        lastreminder:null
    } 
    prevdue.push(newitem);
    saveduedates(prevdue);
    return `Added ${lable}, due ${duedate}.`;

}

export function checkduedates(){
    const items = laodduedates();
    const messages = [];
    const todayStr = new Date().toISOString().split("T")[0];

    for (const item of items) {
        const due = new Date(item.duedate);
        const diff = due - new Date();
        const diffdays = Math.round(diff / 86_400_000);

        if (diffdays <= item.remindwithindays && item.lastreminder !== todayStr) {
            const message = `${item.lable}: ${diffdays} day(s) left`;
            messages.push(message);
            item.lastreminder = todayStr;
        }
    } 

    saveduedates(items);
    return messages;
} 