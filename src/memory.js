import fs from "node:fs"
import { MEMORY_FILE } from "./config.js"

export function loadmemory(){
    if(fs.existsSync(MEMORY_FILE)){
        const raw = fs.readFileSync(MEMORY_FILE,"utf-8");
        const data = JSON.parse(raw);
        return data;
    }else{
        return {facts:[]};
    }
}

export function savememory(memory){
    try {
        fs.writeFileSync(MEMORY_FILE,JSON.stringify(memory,null,2))
    } catch (error) {
        console.log("error in saving memory: ",error.message)
    }
}

export function addmemory(memory, fact){
    memory.facts.push(fact);
    savememory(memory);
}

export function buildmemorycontext(memory) {
  if (memory.facts.length === 0) return ""; 
  const factsStr = memory.facts.map((f) => `- ${f}`).join("\n");
  return `\n\nThings you remember about the user:\n${factsStr}`;
}