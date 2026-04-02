const http = require("http");
const fs = require('fs');

function uploadFile() {
    let data = fs.readFileSync("./README.md", 'utf-8');
    const options = {
        hostname: 'localhost',
        port: 9617,
        path: '/fs/open-api/v1/upload?filename=readme.md',
        method: 'POST',
        headers: {
            'Content-Type': 'text/markdown',
            'Content-Length': data.length,
        },
    };

    //visite http://localhost:9617/fs/open-api/v1/view?filename=readme.md
    const req = http.request(options, (res) => {
        console.log(`STATUS: ${res.statusCode}`);
        console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
        res.setEncoding('utf8');
        res.on('data', (chunk) => {
            console.log(`BODY: ${chunk}`);
        });
        res.on('end', () => {
            console.log('No more data in response.');
        });
    });

    req.on('error', (e) => {
        console.error(`problem with request: ${e.message}`);
    });

    // Write data to request body
    req.write(data);
    req.end();
}

function saveJson() {

    const json = '{"xy":"haoshuai"}';
    const options = {
        hostname: 'localhost',
        port: 9617,
        path: '/fs/open-api/v1/json?key=xy',
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Content-Length': json.length,
        },
    };

    //http://localhost:9617/fs/open-api/v1/json?key=xy
    const req = http.request(options, (res) => {
        console.log(`STATUS: ${res.statusCode}`);
        console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
        res.setEncoding('utf8');
        res.on('data', (chunk) => {
            console.log(`BODY: ${chunk}`);
        });
        res.on('end', () => {
            console.log('No more data in response.');
        });
    });

    req.on('error', (e) => {
        console.error(`problem with request: ${e.message}`);
    });


    // Write data to request body
    req.write(json);
    req.end();
}


uploadFile();
saveJson();
