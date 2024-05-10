const net = require('net');
const fs = require('fs');

const server = net.createServer(socket => {
  console.log('Client connected');

  // Receive file from client
  let receivedData = Buffer.from([]);
  socket.on('data', data => {
    receivedData = Buffer.concat([receivedData, data]);
  });

  socket.on('end', () => {
    // Write received data to a file
    fs.writeFile('received_image.jpg', receivedData, err => {
      if (err) {
        console.error('Error writing file:', err);
      } else {
        console.log('File saved successfully');
      }
    });
  });

  socket.on('close', () => {
    console.log('Client disconnected');
  });
});

const PORT = 3000;
server.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});
