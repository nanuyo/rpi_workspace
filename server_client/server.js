const net = require('net');
const fs = require('fs');
const PORT = 8080;
const BUFFER_SIZE = 1024;
const OUTPUT_FILE = 'received_image.jpg';

const server = net.createServer((socket) => {
    console.log('Client connected');

    const fileStream = fs.createWriteStream(OUTPUT_FILE);
    
    socket.on('data', (data) => {
        fileStream.write(data);
    });

    socket.on('end', () => {
        console.log('File received successfully');
        fileStream.end();
        socket.end();
    });

    socket.on('error', (err) => {
        console.error('Socket error:', err);
        fileStream.end();
    });

}).on('error', (err) => {
    console.error('Server error:', err);
});

server.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}`);
});
