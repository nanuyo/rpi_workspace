const net = require('net');
const fs = require('fs');
const express = require('express');
const path = require('path');

const TCP_PORT = 8080;
const WEB_PORT = 3000;
const BUFFER_SIZE = 1024;
const OUTPUT_FILE = 'received_image.jpg';

const app = express();

// Serve static files from the public directory
app.use(express.static(path.join(__dirname, 'public')));

// Route to serve the image
app.get('/image', (req, res) => {
    res.sendFile(path.join(__dirname, OUTPUT_FILE));
});

// Start the web server
app.listen(WEB_PORT, () => {
    console.log(`Web server listening on port ${WEB_PORT}`);
});

// Create a TCP server
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

server.listen(TCP_PORT, () => {
    console.log(`TCP server listening on port ${TCP_PORT}`);
});
