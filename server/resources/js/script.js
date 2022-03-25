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
    window.id = socket.id;
});

function doAjax() {
    // Get form
    var form = $('#fileUploadForm')[0];

    var data = new FormData(form);
    var file = data.get('uploadfile');

    var renameFile =new File([file], window.id + '.npy' ,{type:file.type});
    var formdata = new FormData();

    formdata.append('uploadfile', renameFile);

    if(window.id) {
        $.ajax({
            type: "POST",
            enctype: 'multipart/form-data',
            url: "/api/files/upload",
            data: formdata,
            processData: false, //prevent jQuery from automatically transforming the data into a query string
            contentType: false,
            cache: false,
            success: (data) => {
                $("#uploadfile").fileinput('clear');
                socket.emit('info', 'Uploaded file');
            },
            error: (e) => {
                socket.emit('info', 'Failed to upload file');
            }
        });
    }
    else
    {
        alert('Failed to connect to server');
    }
}

$( document ).ready( () => {

    $('body').scrollspy({ target: '#main-nav', offset: -100 })

    // Navbar click scroll
    $(".navbar a").on('click', function(event) {
        // Make sure this.hash has a value before overriding default behavior
        if (this.hash !== "") {
            // Prevent default anchor click behavior
            event.preventDefault();

            // Store hash
            var hash = this.hash;

            // Using jQuery's animate() method to add smooth page scroll
            // The optional number (800) specifies the number of milliseconds it takes to scroll to the specified area
            var offset = 0;
            offset = -100;
            $('html, body').animate({
                scrollTop: ($(hash).offset().top + offset)
            }, 1000, function(){
                // Add hash (#) to URL when done scrolling (default click behavior)
                // window.location.hash = hash;
            });
        }
    });

    // Initialize file uploader
    // initialize with defaults
    // $("#uploadfile").fileinput();
    
    // with plugin options
    $("#uploadfile").fileinput({
        // theme: "fa",
        'theme': 'fas',
        showUpload:false, 
        previewFileType:'image',
        maxFileCount: 1,
        allowedFileExtensions: [".npy"],
	allowedPreviewTypes: ["image"]
    });
    $('#uploadfile').on('filecleared', function(event) {
        $('#btnSubmit').prop('disabled', true);
    });

    // Upload File
    $('#uploadfile').on('fileloaded', function(e) 
    {
        if(e.target.files[0])
        {
            var fileName = e.target.files[0].name;
            // if(fileName !== "noise.npy")
            // {
            //     Snackbar.show({
            //         text: 'Please only upload noise.npy.',
            //         pos: 'bottom-right',
            //         duration: 10000
            //     });
            //     $('#btnSubmit').prop('disabled', true);
            // }
            // else
            // {
                $('#btnSubmit').prop('disabled', false);
            // }
        }
    });

    // Submit results
    $("#btnSubmit").click(function(event) {
        // stop submit the form, we will post it manually.
        event.preventDefault();
        doAjax();
    });
});
