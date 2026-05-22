const http = require('http');

const port = Number(process.env.ORBYT_PORT || 80);
const boardModel = process.env.ORBYT_BOARD_MODEL || 'Node Orbyt Adapter';
const supportedPins = csv(process.env.ORBYT_SUPPORTED_PINS || '0,1,2,3,4,5,6,7');
const pwmPins = csv(process.env.ORBYT_PWM_PINS || '');
const startedAt = Date.now();
const pinState = new Map();

function csv(value) {
  return value
    .split(',')
    .map((part) => Number(part.trim()))
    .filter((value) => Number.isInteger(value) && value >= 0);
}

function send(res, status, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(status, {
    'content-type': 'application/json',
    'content-length': Buffer.byteLength(body),
  });
  res.end(body);
}

function readJson(req) {
  return new Promise((resolve) => {
    let body = '';
    req.on('data', (chunk) => {
      body += chunk;
    });
    req.on('end', () => {
      try {
        resolve(body ? JSON.parse(body) : {});
      } catch {
        resolve({});
      }
    });
  });
}

async function writePin(pin, value, mode) {
  pinState.set(pin, { value, mode });
  return { success: true, message: 'Pin write completed' };
}

async function readPin(pin) {
  const state = pinState.get(pin) || { value: 0, mode: 'INPUT' };
  return { pin, value: state.value, mode: state.mode };
}

const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);

  if (req.method === 'GET' && url.pathname === '/api/v1/device/status') {
    send(res, 200, {
      connected: true,
      apiVersion: '1.1',
      deviceFamily: 'linux-adapter',
      boardModel,
      board: boardModel,
      firmware: 'orbyt-node-1.0.0',
      uptimeMs: Date.now() - startedAt,
    });
    return;
  }

  if (req.method === 'GET' && url.pathname === '/api/v1/device/capabilities') {
    send(res, 200, {
      apiVersion: '1.1',
      deviceFamily: 'linux-adapter',
      boardModel,
      board: boardModel,
      transport: 'http',
      features: ['digital-read', 'digital-write', 'pwm-write'],
      supportedPins,
      digitalPins: supportedPins,
      analogPins: [],
      pwmPins,
      registeredPins: [],
      notes: 'Replace the simulated read/write functions with GPIO bindings.',
    });
    return;
  }

  if (req.method === 'POST' && url.pathname === '/api/v1/pin/write') {
    const payload = await readJson(req);
    const pin = Number(payload.pin);
    if (!supportedPins.includes(pin)) {
      send(res, 400, { success: false, message: 'Unsupported pin' });
      return;
    }
    const result = await writePin(pin, Number(payload.value || 0), payload.mode || 'OUTPUT');
    send(res, result.success ? 200 : 400, result);
    return;
  }

  if (req.method === 'GET' && url.pathname === '/api/v1/pin/read') {
    const pin = Number(url.searchParams.get('pin'));
    if (!supportedPins.includes(pin)) {
      send(res, 400, { message: 'Unsupported pin' });
      return;
    }
    send(res, 200, await readPin(pin));
    return;
  }

  send(res, 404, { message: 'Not found' });
});

server.listen(port, '0.0.0.0', () => {
  console.log(`Orbyt Node adapter ready on port ${port}`);
});
