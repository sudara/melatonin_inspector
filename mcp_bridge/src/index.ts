import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
  ErrorCode,
  McpError,
} from "@modelcontextprotocol/sdk/types.js";
import * as net from "net";

class IpcClient {
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
      this.buffer = Buffer.concat([this.buffer, data]);
      this.processBuffer();
    });

    this.client.on("error", (err) => {
      // console.error(`IPC Connection error: ${err.message}`);
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
      // JUCE InterprocessConnection writes Magic Header and Size as 32-bit Little Endian ints.
      const magic = this.buffer.readInt32LE(0);
      const size = this.buffer.readInt32LE(4);

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
      if (!this.isConnected)
        return reject(
          new Error(
            "Not connected to JUCE application. Ensure the app is running and Inspector is open.",
          ),
        );

      const msg_id = Math.random().toString(36).substring(7);
      const payload = JSON.stringify({ action, msg_id, args });
      const payloadBuffer = Buffer.from(payload, "utf8");

      const header = Buffer.alloc(8);
      header.writeInt32LE(0x2172746a, 0); // Magic header
      header.writeInt32LE(payloadBuffer.length, 4); // Size

      this.pendingRequests.set(msg_id, { resolve, reject });
      this.client.write(Buffer.concat([header, payloadBuffer]));

      // Timeout to prevent hanging
      setTimeout(() => {
        if (this.pendingRequests.has(msg_id)) {
          this.pendingRequests.delete(msg_id);
          reject(new Error(`Request timed out for action: ${action}`));
        }
      }, 5000);
    });
  }
}

class MelatoninInspectorServer {
  private server: Server;
  private ipcClient: IpcClient;

  constructor() {
    this.server = new Server(
      {
        name: "melatonin-inspector",
        version: "1.0.0",
      },
      {
        capabilities: {
          tools: {},
        },
      },
    );

    this.ipcClient = new IpcClient(8484);

    this.setupToolHandlers();

    // Error handling
    this.server.onerror = (error) => console.error("[MCP Error]", error);
    process.on("SIGINT", async () => {
      await this.server.close();
      process.exit(0);
    });
  }

  private setupToolHandlers() {
    this.server.setRequestHandler(ListToolsRequestSchema, async () => ({
      tools: [
        {
          name: "juce_get_ui_tree",
          description:
            "Retrieve the full hierarchical tree of JUCE Components from the currently running app.",
          inputSchema: { type: "object", properties: {} },
        },
        {
          name: "juce_get_component_details",
          description:
            "Get detailed properties of a specific JUCE Component (bounds, colors, fonts, etc.).",
          inputSchema: {
            type: "object",
            properties: {
              id: {
                type: "string",
                description:
                  "The component ID (pointer string) returned from get_tree",
              },
            },
            required: ["id"],
          },
        },
        {
          name: "juce_highlight_component",
          description:
            "Visually highlight a component on the screen for debugging.",
          inputSchema: {
            type: "object",
            properties: {
              id: { type: "string" },
            },
            required: ["id"],
          },
        },
        {
          name: "juce_click_component",
          description: "Trigger a mouse click on a component (like a Button).",
          inputSchema: {
            type: "object",
            properties: {
              id: { type: "string" },
            },
            required: ["id"],
          },
        },
        {
          name: "juce_set_component_bounds",
          description: "Resize or move a component.",
          inputSchema: {
            type: "object",
            properties: {
              id: { type: "string" },
              x: { type: "number" },
              y: { type: "number" },
              w: { type: "number" },
              h: { type: "number" },
            },
            required: ["id", "x", "y", "w", "h"],
          },
        },
        {
          name: "juce_capture_screenshot",
          description: "Capture a visual snapshot of a component.",
          inputSchema: {
            type: "object",
            properties: {
              id: { type: "string" },
            },
            required: ["id"],
          },
        },
        {
          name: "juce_quit_application",
          description:
            "Sends a request to quit the running JUCE application gracefully.",
          inputSchema: { type: "object", properties: {} },
        },
      ],
    }));

    this.server.setRequestHandler(CallToolRequestSchema, async (request) => {
      try {
        switch (request.params.name) {
          case "juce_get_ui_tree": {
            const tree = await this.ipcClient.send("get_tree");
            return {
              content: [{ type: "text", text: JSON.stringify(tree, null, 2) }],
            };
          }
          case "juce_get_component_details": {
            const id = String(request.params.arguments?.id);
            const details = await this.ipcClient.send("get_details", { id });
            return {
              content: [
                { type: "text", text: JSON.stringify(details, null, 2) },
              ],
            };
          }
          case "juce_highlight_component": {
            const id = String(request.params.arguments?.id);
            await this.ipcClient.send("highlight", { id });
            return {
              content: [{ type: "text", text: `Component ${id} highlighted.` }],
            };
          }
          case "juce_click_component": {
            const id = String(request.params.arguments?.id);
            await this.ipcClient.send("click", { id });
            return {
              content: [
                { type: "text", text: `Click dispatched to component ${id}.` },
              ],
            };
          }
          case "juce_set_component_bounds": {
            const { id, x, y, w, h } = request.params.arguments as any;
            await this.ipcClient.send("set_bounds", { id, x, y, w, h });
            return {
              content: [
                { type: "text", text: `Bounds updated for component ${id}.` },
              ],
            };
          }
          case "juce_capture_screenshot": {
            const id = String(request.params.arguments?.id);
            const base64Png = await this.ipcClient.send("screenshot", { id });
            return {
              content: [
                {
                  type: "image",
                  data: base64Png,
                  mimeType: "image/png",
                },
              ],
            };
          }
          case "juce_quit_application": {
            const result = await this.ipcClient.send("quit");
            return { content: [{ type: "text", text: result }] };
          }
          default:
            throw new McpError(
              ErrorCode.MethodNotFound,
              `Unknown tool: ${request.params.name}`,
            );
        }
      } catch (error: any) {
        return {
          content: [
            { type: "text", text: `Error executing tool: ${error.message}` },
          ],
          isError: true,
        };
      }
    });
  }

  async run() {
    this.ipcClient.connect().catch(() => {});
    setInterval(() => {
      if (!this.ipcClient.isConnected) {
        this.ipcClient.connect().catch(() => {});
      }
    }, 3000);

    const transport = new StdioServerTransport();
    await this.server.connect(transport);
    console.error("Melatonin Inspector MCP Server running on stdio");
  }
}

const server = new MelatoninInspectorServer();
server.run().catch(console.error);
