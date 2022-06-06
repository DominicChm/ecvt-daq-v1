# API Documentation

## Websocket Doc

All websocket messages follow a channel-based structure. The basic format is:

```json
{
  "type": "<CHANNEL TYPE>",
  "data": "<ANY DATA>"
}
```

This structure should be sufficient for most messages. It also is purposely left
simple to allow for future expansion.

| Type | Data | Description |
|---|---|---|
| event | runs_update | Indicates that something has changed about stored runs, and they should be updated. |
| status | ... a lot (TODO)| |
| stats | ... | Stats about the internal performance of the DAQ. Emits every so often. |

## HTTP Doc