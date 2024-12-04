const http = require("http");
const fs = require('fs');

let data = fs.readFileSync("./LICENSE", 'utf-8')

const options = {
    hostname: 'localhost',
    port: 10001,
    path: '/fs/open-api/v1/upload?filename=LISENCE',
    method: 'POST',
    headers: {
        'Content-Type': 'text/markdown',
        'Content-Length': data.length,
    },
};

//http://localhost:10001/fs/open-api/v1/view?filename=LISENCE
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
