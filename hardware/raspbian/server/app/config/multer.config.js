const multer = require('multer');
const fs = require('fs');

var storage = multer.diskStorage({
    destination: (req, file, cb) => {
        var dir = __basedir + '/uploads/';
        if (!fs.existsSync(dir)){
            fs.mkdirSync(dir);
        }
        cb(null, __basedir + '/uploads/')
    },
    filename: async (req, file, cb) => {
        console.log('noise.npy saved.')
	
        // Copy noise.npy to uvc-gadget
        fs.copyFile('uploads/noise.npy', '../uvc-gadget/noise.npy', (err) => {
        	if (err) throw err;
        	// Notify the uvc-gadget
        	fs.closeSync(fs.openSync('../uvc-gadget/noise', 'w'));
        	console.log('noise.npy copied.')
	});


        cb(null, 'noise.npy');
    }
});

var upload = multer({storage: storage});

module.exports = upload;
