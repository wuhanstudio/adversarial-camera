var socket = io();

// Capture image
function capture() {
    socket.emit('capture');
}

// Start recording
function start() {
    socket.emit('start');
    $('#start').prop('disabled', true);
    $('#stop').prop('disabled', false);
}

// Stop recording
function stop() {
    socket.emit('stop');
    $('#start').prop('disabled', false);
    $('#stop').prop('disabled', true);
}

// Receive captured image
socket.on('capture', (img) => {
    $('#capture').attr("src", img);
});

// Receive Encoded Video
socket.on('encoded', (img) => {
    setTimeout(function () {
        location.reload();
    }, 3000);   
});

// Log message
socket.on('info', (msg) => {
    Snackbar.show({
        text: msg,
        pos: 'bottom-right',
        duration: 5000
    });
});

// User connected
socket.on('connect', () => {
    console.log('Session Id: ', socket.id);
});

