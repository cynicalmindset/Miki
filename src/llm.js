import {OLLAMA_URL, MODEL} from "./config.js"


export async function ask(prompt, systemprompt){
    const res = await fetch(OLLAMA_URL,{
        method:"POST",
        headers:{"Content-Type":"application/json"},
        body:JSON.stringify({
            model:MODEL,
            prompt:prompt,
            system:systemprompt,
            stream:false,
            think:false,
        })
    });
    if(!res.ok){
        throw new Error(`ollama fetching request failed hawww ${res.status}`)
    }

    const data = await res.json();
    return data.response;
}

export async function prase(raw, systemprompt){
    const instruction = `Tell the user this result in your own savage voice, 2-3 short words max, don't just repeat it robotically: ${rawResult}`;
    try {
  return await ask(instruction, systemPrompt);
} catch {
  return raw;
}
}