# Limitations Testing Results
date: oct 2 2025
model: mistral-nemo-instruct-2407-Q4_K_M

## categories

### 1. newer tools and frameworks (knowledge cut-off)

**Test: "bun package manager setup"**
- Response: correct installation command `curl -fsSL https://bun.sh/install | bash`
- Result: correct

**Test: "astro framework syntax"**
- Response: specific syntax (`<script type="module">`) without explanation of Astro itself or how the syntax works with the framework
- Result: partial

**Test: "omarchy setup"**
- Response: fabricated npm-based installation workflow for a real 2024 Linux distribution
- Result: incorrect (complete hallucination)

**Test: "zed editor binds"**
- Response: generic text editor shortcuts, none were zed-specific
- Result: incorrect

**Test: "windsurf features"**
- Response: described the water sport, not the IDE
- Chat follow-up: switched to describing VSCode
- Result: incorrect (context failure even after clarification)

### 2. framework specific (focused on crow)

**Test: "crow websocket handler"**
- Response: gave a node.js websock library example
- Result: incorrect

**Test: "crow async request handling"**
- Response: gave a generic node.js example
- Result: incorrect

### 3. many-language prompts

**Test: "py asyncio vs js promises"**
- Response: accurate comparison
- Result: correct

**Test: "rust ownership vs cpp unique_ptr"**
- Response: accurate comparison, distinction between compile-time and runtime safety
- Result: correct

**Test: "go channels vs java completablefuture"**
- Response: accurate descriptions and good comparison
- Result: correct

### 4. misc/ambiguous prompts

**Test: "what is recall"**
- Response: memory retrieval definition
- Result: correct (no self-awareness, but acceptable)

**Test: "lambda syntax"**
- Response: defaulted to python lambda syntax
- Result: partial

**Test: "vector initialisation"**
- Response: cpp std::vector syntax
- Result: correct

### 5. niche technical prompts

**Test: "sqlite prepared statements with blob binding"**
- Response: python example initially
- Chat follow-up (C++): correct `sqlite3_bind_blob` usage
- Result: correct after clarification

**Test: Chat follow-up on above: "what if the length is unknown"**
- Response: suggested SQLITE_STATIC to copy the data into SQLite's internal storage
- Result: incorrect (SQLITE_TRANSIENT copies, SQLITE_STATIC doesn't)

## observations/patterns

1. Knowledge cuts off around 2023 - Mixed results on newer tools and frameworks
2. Defaults to popular alternatives - Node.js/Python are go-to options when confused/missing knowledge
3. Confident hallucination - Generates believable answers instead of admitting uncertainty
4. No self-awareness - Expected as system prompt doesn't provide context
5. Chat-mode valuable for clarification - Follow-ups can correct mistakes/ambiguity in prompts
6. Fragmented knowledge - Knows specific details but struggles with cohesive explanations
7. Technical details unreliable - SQLite flag recommendation incorrect

## stats

- total queries: 16 (with some follow-ups)
- correct: 6
- partially correct: 2
- incorrect/hallucination: 8
    - success rate: 38% (50% including partials)

system works well with:
- programming comparisons of popular languages
- syntax questions with clear context
- established/popular tools and concepts

system struggles with:
- newer tools (post-2023)
- niche frameworks (defaults to node.js when asked about crow)
- ambiguous queries
- providing cohesive explanations with fragmented knowledge
- context recovery after initial misunderstanding

---
## raw results

### Test 1: Bun Package Manager
**Prompt:** `recall bun package manager setup`
**Response:**
```
To set up bun package manager, run curl -fsSL https://bun.sh/install | bash in your
terminal. Then, verify with bun -v.
```

### Test 2: Astro Framework Syntax
**Prompt:** `recall astro framework syntax`
**Response:**
```
astro framework uses <script type="module"> for modules and <script nomodule> for fallback.
```

### Test 3: Omarchy Linux Distribution
**Prompt:** `recall omarchy setup`
**Response:**
```
To set up Omarchi, first clone the repo: git clone
https://github.com/omarchi/omarchi.git. Then, install dependencies: npm install.
Finally, start the server: npm run dev.
```

### Test 4: Zed Editor
**Prompt:** `recall zed editor binds`
**Response:**
```
• Zed editor uses Ctrl+S for save.
• Ctrl+Shift+S for save as.
```

### Test 5: Windsurf IDE
**Prompt:** `recall windsurf features`
**Response:**
```
• Windsurfing combines surfing and sailing.
• Uses a board and a sail, controlled by a harness and boom.
```
**Follow-up:** `chat i meant the ide`
**Response:**
```
• Visual Studio Code (VSCode) is a popular IDE.
• Developed by Microsoft, it's open-source and highly customizable.
```

### Test 6: Crow Websockets
**Prompt:** `recall crow websocket handler`
**Response:**
```
In Node.js, use ws library. Example: const WebSocketServer = require('ws').Server;
const wss = new WebSocketServer({ port: 8080 }); wss.on('connection', (ws) => {
ws.on('message', (message) => { console.log(message); }); });.
```

### Test 7: Crow Async Handling
**Prompt:** `recall crow async request handling`
**Response:**
```
In Node.js, use async/await with Crow for async requests:

app.get('/async', async (req, res) => {
  const result = await someAsyncFunction();
  res.send(result);
});
```

### Test 8: Python AsyncIO vs JavaScript Promises
**Prompt:** `recall py asyncio vs js promises`
**Response:**
```
• AsyncIO is Python's asynchronous I/O framework, using async/await syntax.
• Promises in JavaScript handle asynchronous operations with .then(), .catch().
```

### Test 9: Rust Ownership vs C++ unique_ptr
**Prompt:** `recall rust ownership vs cpp unique_ptr`
**Response:**
```
• Rust's ownership ensures memory safety at compile time, while C++'s unique_ptr
  manages unique ownership at runtime.
• Rust enforces strict rules, preventing data races, while unique_ptr allows manual
  management.
```

### Test 10: Go Channels vs Java CompletableFuture
**Prompt:** `recall go channels vs java completablefuture`
**Response:**
```
• Go's channels are lightweight, multiplexed, and used for goroutine communication.
• Java's CompletableFuture is a asynchronous computation with a return value.
```

### Test 11: What is Recall
**Prompt:** `recall what is recall`
**Response:**
```
Recall is a memory retrieval process. It's like fishing for info from your brain.
```

### Test 12: Lambda Syntax
**Prompt:** `recall lambda syntax`
**Response:**
```
In Python, lambda is a keyword for anonymous functions. Syntax: lambda args:
expression.
```

### Test 13: Vector Initialization
**Prompt:** `recall vector initialisation`
**Response:**
```
std::vector<int> v = {1, 2, 3};
```

### Test 14: SQLite Blob Binding
**Prompt:** `recall sqlite prepared statements with blob binding`
**Response:**
```
To bind a BLOB in SQLite prepared statements, use ? as a placeholder and pass the
BLOB data as a tuple with the rest of your parameters. Here's an example:

cursor.execute("INSERT INTO table (column) VALUES (?)", (blob_data,))
```
**Follow-up:** `chat in cpp`
**Response:**
```
In C++, use sqlite3_bind_blob with a pointer to the blob data and its length:

sqlite3_bind_blob(stmt, 1, blob_data, length, SQLITE_TRANSIENT);
```

### Test 15: SQLite Unknown Length
**Follow-up:** `chat what if the length is unknown`
**Response:**
```
If the length is unknown, use SQLITE_STATIC instead of SQLITE_TRANSIENT to copy the
data into SQLite's internal storage.
```
