const http = require('http');  
const fs = require('fs');  
  
const server = http.createServer((req, res) => {  
  // 设置 CORS（可选，方便前端调用）  
  res.setHeader('Access-Control-Allow-Origin', '*');  
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');  
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');  
  
  if (req.method === 'OPTIONS') {  
    res.writeHead(200);  
    res.end();  
    return;  
  }  
  console.log(req.url);
  
  if (req.url === '' || req.url === '/') {  
    let content = fs.readFileSync('./index.html', 'utf-8');  
    res.setHeader('Content-Type', 'text/html');
    res.writeHead(200);
    res.write(content)  
    res.end();     
    return;
  }
    // 解析 URL 获取 query 参数  
  let parsedUrl = new URL("http://localhost"+req.url, "http://localhost");  
  let query = parsedUrl.searchParams;

  if (req.url.startsWith('/fs/open-api/v1/list')) {  
    let pageNum = query.get('pageNum')
    let data1 = {success:true, data:{total: 11, pageNum: 1, files: [
        {"filename":"徐熠.md","time":"2026-04-03 15:15:14", "size": 1234}, 
        {"filename":"readme.md","time":"2026-04-03 14:15:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 14:12:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 13:12:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 13:10:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 13:09:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 12:55:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 12:15:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-03 12:05:14", "size": 1234}, 
        {"filename":"你好.md","time":"2026-04-01 15:15:14", "size": 1234}
    ]}};
    
    let data2 = {success:true, data:{total: 11, pageNum: 2, files: [ 
        {"filename":"你好.md","time":"2026-04-01 12:15:14", "size": 1234}
    ]}};
    res.setHeader('Content-Type', 'application/json');
    res.writeHead(200);
    res.write(JSON.stringify(pageNum==='1'?data1:data2))  
    res.end();     
    return;
  }
  
  let filename = query.get('filename')
  console.log("文件名", filename);

  if (req.url.startsWith('/fs/open-api/v1/download')) {  
    let content = fs.readFileSync('./index.html', 'utf-8');  
    res.setHeader('Content-Type', "application/octet-stream");
    res.setHeader('Content-Disposition', "attachment; filename=" + encodeURI(filename));
    res.writeHead(200);
    res.write(content)  
    res.end();     
    return;
  }
  // 处理文件上传请求  
  if (req.url.startsWith('/fs/open-api/v1/upload')) {  
    let body = "";
    req.on("data", chunk => body += chunk);
    req.on("end", () => {
        // console.log("原始数据:", body);
        res.setHeader('Content-Type', 'application/json');
        res.writeHead(200);
        res.write(JSON.stringify({success:true}))  
        res.end();   
    });
  }
});  
  
const PORT = 3000;  
server.listen(PORT, () => {  
  console.log(`HTTP 服务器运行在 http://localhost:${PORT}`);   
});  
