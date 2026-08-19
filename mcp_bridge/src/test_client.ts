import * as net from "net";
import * as fs from "fs";

class TestClient {
  private client: net.Socket;
  private buffer: Buffer = Buffer.alloc(0);
  private pendingRequests = new Map<
    string,
    { resolve: (value: any) => void; reject: (reason?: any) => void }
  >();
  public isConnected = false;

  constructor(private port: number) {
    this.client = new net.Socket();

    this.client.on("data", (data: Buffer) => {
      console.log("Received bytes:", data.length);
      this.buffer = Buffer.concat([this.buffer, data]);
      this.processBuffer();
    });

    this.client.on("error", (err) => {
      console.error(`IPC Connection error: ${err.message}`);
    });

    this.client.on("close", () => {
      this.isConnected = false;
    });
  }

  async connect(): Promise<void> {
    if (this.isConnected) return;
    return new Promise((resolve, reject) => {
      this.client.connect(this.port, "127.0.0.1", () => {
        this.isConnected = true;
        resolve();
      });
      this.client.once("error", reject);
    });
  }

  private processBuffer() {
    while (this.buffer.length >= 8) {
      const magic = this.buffer.readInt32LE(0);
      const size = this.buffer.readInt32LE(4);
      console.log("Parsed magic:", magic.toString(16), "size:", size);

      if (this.buffer.length >= 8 + size) {
        const payload = this.buffer.subarray(8, 8 + size);
        this.buffer = this.buffer.subarray(8 + size);

        const jsonStr = payload.toString("utf8");
        try {
          const json = JSON.parse(jsonStr);
          const pending = this.pendingRequests.get(json.msg_id);
          if (pending) {
            if (json.status === "ok") pending.resolve(json.data);
            else pending.reject(new Error(json.data));
            this.pendingRequests.delete(json.msg_id);
          }
        } catch (e) {
          console.error("Failed to parse JSON response:", e, jsonStr);
        }
      } else {
        break;
      }
    }
  }

  async send(action: string, args: any = {}): Promise<any> {
    return new Promise((resolve, reject) => {
      if (!this.isConnected) return reject(new Error("Not connected"));

      const msg_id = Math.random().toString(36).substring(7);
      const payload = JSON.stringify({ action, msg_id, args });
      const payloadBuffer = Buffer.from(payload, "utf8");

      const header = Buffer.alloc(8);
      header.writeInt32LE(0x2172746a, 0); // Magic header
      header.writeInt32LE(payloadBuffer.length, 4); // Size

      this.pendingRequests.set(msg_id, { resolve, reject });
      this.client.write(Buffer.concat([header, payloadBuffer]));
    });
  }

  close() {
    this.client.destroy();
  }
}

async function run() {
  const client = new TestClient(42424);
  try {
    console.log("Connecting to IPC Server...");
    await client.connect();
    console.log("Connected!");

    console.log("Sending ping...");
    const pingResponse = await client.send("ping");
    console.log("Ping response:", pingResponse);

    console.log("Requesting component tree...");
    const tree = await client.send("get_tree");
    console.log("Tree received. Root node ID:", tree.id);

    console.log("Sending quit command...");
    const quitResponse = await client.send("quit");
    console.log("Quit response:", quitResponse);
  } catch (e) {
    console.error("Test failed:", e);
  } finally {
    client.close();
  }
}

run();
