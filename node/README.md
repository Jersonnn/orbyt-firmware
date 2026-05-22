# Orbyt Node Adapter

Local HTTP adapter for Raspberry Pi, Orange Pi, desktop gateways, and other Node.js-capable devices.

## Run

```bash
node server.js
```

Then add the host IP in Orbyt as `Linux / Node adapter` or `Custom HTTP`.

Environment variables:

- `ORBYT_PORT`, default `80`
- `ORBYT_BOARD_MODEL`, default `Node Orbyt Adapter`
- `ORBYT_SUPPORTED_PINS`, comma-separated numeric pin IDs
- `ORBYT_PWM_PINS`, comma-separated numeric pin IDs

The default implementation is safe simulation. Replace `readPin` and `writePin` with board-specific GPIO calls such as `onoff`, `pigpio`, or a vendor SDK.

