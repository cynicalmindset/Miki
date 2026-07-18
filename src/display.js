export async function display(text){
    const ESP32_URL = "http://10.143.78.112/update";

    await fetch(ESP32_URL, {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `msg=${encodeURIComponent(text)}`
    });
}