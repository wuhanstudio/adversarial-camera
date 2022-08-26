module.exports = (app, router, upload) => {
    const fileWorker = require('../controllers/file.controller.js')

	var path = __basedir + '/views/'

	router.use((req,res,next) => {
		console.log("/" + req.method)
		next()
	});

	app.get('/', (req,res) => {
		res.sendFile(path + "index.html")
	});

	app.post('/api/files/upload', upload.single("uploadfile"), fileWorker.uploadFile)

	app.get('/api/files/getall', fileWorker.listAllFiles)

	app.get('/api/files/result', fileWorker.downloadFile)

	app.use('/',router)
 
	app.use('*', (req,res) => {
		res.sendFile(path + "404.html")
	})
}