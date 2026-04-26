const http = require('http');
const fs = require('fs');
const path = require('path');

/**
 * 上传文件到指定 HTTP 服务器
 * @param {string} url 目标 URL (例如 http://localhost:3000/upload)
 * @param {string} filePath 本地文件路径
 * @param {Object} additionalFields 额外的普通字段 { fieldName: value }
 */
function uploadFile(url, filePath, additionalFields = {}) {
  const urlObj = new URL(url);
  const boundary = `----NodeJSFormBoundary${Math.random().toString(36).substring(2)}`;
  const fileFieldName = 'file'; // 文件字段名，根据服务端需求调整

  // 读取文件内容
  const fileData = fs.readFileSync(filePath);
  const fileName = path.basename(filePath);
  const fileMimeType = 'application/octet-stream'; // 可根据扩展名动态获取，这里简化

  // 用于存储所有表单部分的 Buffer 数组
  const parts = [];

  // 添加普通字段
  for (const [fieldName, value] of Object.entries(additionalFields)) {
    parts.push(Buffer.from(`--${boundary}\r\n`));
    parts.push(Buffer.from(`Content-Disposition: form-data; name="${fieldName}"\r\n\r\n`));
    parts.push(Buffer.from(`${value}\r\n`));
  }

  // 添加文件字段
  parts.push(Buffer.from(`--${boundary}\r\n`));
  parts.push(Buffer.from(
    `Content-Disposition: form-data; name="${fileFieldName}"; filename="${fileName}"\r\n`
  ));
  parts.push(Buffer.from(`Content-Type: ${fileMimeType}\r\n\r\n`));
  parts.push(fileData);
  parts.push(Buffer.from(`\r\n`));

  // 结束边界
  parts.push(Buffer.from(`--${boundary}--\r\n`));

  // 合并所有部分得到完整请求体
  const requestBody = Buffer.concat(parts);
  const contentLength = requestBody.length;

  // 请求配置
  const options = {
    hostname: urlObj.hostname,
    port: urlObj.port || 80,
    path: urlObj.pathname + urlObj.search,
    method: 'POST',
    headers: {
      'Content-Type': `multipart/form-data; boundary=${boundary}`,
      'Content-Length': contentLength,
    },
  };

  const req = http.request(options, (res) => {
    let responseData = '';
    res.on('data', (chunk) => { responseData += chunk; });
    res.on('end', () => {
      console.log(`响应状态: ${res.statusCode}`);
      console.log(`响应内容: ${responseData}`);
    });
  });

  req.on('error', (err) => {
    console.error('请求失败:', err.message);
  });

  // 发送请求体
  req.write(requestBody);
  req.end();
}

// 使用示例：上传文件 example.txt，并附带额外字段 userId=123
uploadFile('http://127.0.0.1:9607/upload', './test.png', { userId: '123' });