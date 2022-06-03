import { WebSocketServer } from 'ws';

const port = process.env.PORT || 7001;

const wss = new WebSocketServer({ port: port });

wss.on('connection', function connection(ws) {
  ws.on('message', function message(data) {
    console.log('received: %s', data);
  });

  ws.send('something');
});

console.log('Server listening on port ' + port);