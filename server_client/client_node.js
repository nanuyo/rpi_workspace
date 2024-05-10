const net = require('net');
const fs = require('fs');

const client = new net.Socket();

const FILE_PATH = 'image.jpg'; // Path to the JPG file to send

client.connect(3000, 'localhost', () => {
  console.log('Connected to server');

  // Read file and send to server
  const fileStream = fs.createReadStream(FILE_PATH);
  fileStream.on('data', chunk => {
    client.write(chunk);
  });

  fileStream.on('end', () => {
    console.log('File sent');
    client.end();
  });
});

client.on('close', () => {
  console.log('Connection closed');
});
