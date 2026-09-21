const express = require("express");
const http = require("http");
const { WebSocketServer } = require("ws");
const { spawn } = require("child_process");
const path = require("path");

const PORT = process.env.PORT || 3000;
const ENGINE_PATH = path.join(__dirname, "..", "build", "chess");

const app = express();
app.use(express.static(path.join(__dirname, "..", "web")));

const server = http.createServer(app);
const wss = new WebSocketServer({ server });

wss.on("connection", (ws) => {
  console.log("[ws] client connected");

  const engine = spawn(ENGINE_PATH, [], { stdio: ["pipe", "pipe", "pipe"] });
  let buffer = "";

  engine.stdout.on("data", (data) => {
    buffer += data.toString();
    const lines = buffer.split("\n");
    buffer = lines.pop(); // keep incomplete line in buffer
    for (const line of lines) {
      const trimmed = line.trim();
      if (!trimmed) continue;
      console.log("[engine -> ws]", trimmed);
      ws.send(JSON.stringify({ type: "engine", line: trimmed }));
    }
  });

  engine.stderr.on("data", (data) => {
    console.error("[engine err]", data.toString().trim());
  });

  engine.on("close", (code) => {
    console.log("[engine] exited with code", code);
    ws.send(JSON.stringify({ type: "engine", line: "quit" }));
  });

  ws.on("message", (msg) => {
    const data = JSON.parse(msg.toString());
    if (data.type === "uci") {
      const cmd = data.line;
      console.log("[ws -> engine]", cmd);
      engine.stdin.write(cmd + "\n");
    }
  });

  ws.on("close", () => {
    console.log("[ws] client disconnected");
    engine.kill();
  });
});

server.listen(PORT, () => {
  console.log(`http://localhost:${PORT}`);
});
