const net = require('net');
const fs = require('fs');
const path = require('path');
const express = require('express');

const TCP_PORT = 8080;
const WEB_PORT = 3000;
const BUFFER_SIZE = 1024;
let imgCnt = 0;

const app = express();

// Serve static files from the public directory
app.use(express.static(path.join(__dirname, 'public')));

// Route to serve the latest image
app.get('/image', (req, res) => {
    res.sendFile(path.join(__dirname, `received_image_${imgCnt - 1}.jpg`));
});

// Start the web server
app.listen(WEB_PORT, () => {
    console.log(`Web server listening on port ${WEB_PORT}`);
});

// Create a TCP server
const server = net.createServer((socket) => {
    console.log('Client connected');

    const filename = `received_image_${imgCnt}.jpg`;
    const fileStream = fs.createWriteStream(filename);
    imgCnt++;

    socket.on('data', (data) => {
        fileStream.write(data);
    });

    socket.on('end', () => {
        console.log(`File ${filename} received successfully`);
        fileStream.end();
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
