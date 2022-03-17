var fs = require('fs')

const { exec } = require('child_process');

// Web Server
var express = require('express')
var app = express()
var port = process.env.PORT || 3000;

// Static Website
app.use(express.static('resources'))

app.get('/video', function (req, res) {
  const path = 'resources/output.mp4'
  var stat = fs.statSync(path)
  var fileSize = stat.size
  var range = req.headers.range

  if (range) {
    const parts = range.replace(/bytes=/, "").split("-")
    const start = parseInt(parts[0], 10)
    const end = parts[1]
      ? parseInt(parts[1], 10)
      : fileSize - 1

    if (start >= fileSize) {
      res.status(416).send('Requested range not satisfiable\n' + start + ' >= ' + fileSize);
      return
    }

    const chunksize = (end - start) + 1
    var file = fs.createReadStream(path, { start, end })
    const head = {
      'Content-Range': `bytes ${start}-${end}/${fileSize}`,
      'Accept-Ranges': 'bytes',
      'Content-Length': chunksize,
      'Content-Type': 'video/mp4',
    }

    res.writeHead(206, head)
    file.pipe(res)
  } else {
    const head = {
      'Content-Length': fileSize,
      'Content-Type': 'video/mp4',
    }
    res.writeHead(200, head)
    fs.createReadStream(path).pipe(res)
  }
})

var http = require('http').createServer(app)
var io = require('socket.io')(http)

function getFile(path, timeout = 100) {
  const intervalObj = setInterval(function () {

    const fileExists = fs.existsSync(path);

    if (fileExists) {
      clearInterval(intervalObj);
      let img = "data:image/png;base64," + fs.readFileSync(path, 'base64')
      io.emit('capture', img)
      io.emit('info', 'Captured Image')
    }
  }, timeout);
};


io.on('connection', async (socket) => {
  // New client connected
  const sessionID = socket.id
  console.log('[client][connection]', sessionID)

  // Notify client of the new IP address and port
  // io.emit('ip', ip, ui_port, udp_port)

  // If user requests a new image
  socket.on('capture', async () => {
    console.log('Capturing...')
    fs.stat('../uvc-gadget/input.jpg', function (err, stats) {

      if (err) {
        return console.error(err);
      }

      fs.unlink('../uvc-gadget/input.jpg', function (err) {
        if (err) return console.log(err);
      });
    });
    fs.closeSync(fs.openSync('../uvc-gadget/capture', 'w'));
    getFile('../uvc-gadget/input.jpg', 500);
  })

  // If user requests a new image
  socket.on('start', async () => {
    console.log('Recording...')
    fs.closeSync(fs.openSync('../uvc-gadget/start', 'w'));
    io.emit('info', 'Started recording')
  })

  // If user requests a new image
  socket.on('stop', async () => {
    console.log('Recorded.')
    io.emit('info', 'Video recorded. Encoding...')
    fs.closeSync(fs.openSync('../uvc-gadget/stop', 'w'));
    const intervalObj = setInterval(function () {

      const fileExists = fs.existsSync('../uvc-gadget/live.mp4');
      console.log(`Encoding Video`);

      if (fileExists) {
        fs.unlink('resources/output.mp4', function (err) {
          if (err) return console.log(err);
        });
        exec('ffmpeg -i ../uvc-gadget/live.mp4 -vcodec libx264 -acodec aac resources/output.mp4', (err, stdout, stderr) => {
          if (err) {
            console.error(`exec error: ${err}`);
            return;
          }
          console.log(`Video Encoded`);
          io.emit('encoded')
          io.emit('info', 'Video encoded')
        });
        clearInterval(intervalObj);
      }
    }, 1000);
  })

  // Client disconnected
  socket.on('disconnect', () => {

  })
})

// Create a Server
var server = http.listen(port, () => {

  var host = server.address().address
  var port = server.address().port

  console.log("App listening at http://%s:%s", host, port)
})
