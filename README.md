# tiny_file_server  
A tiny file server  
visit {endpoint}/ to see file list  

## API 

### Public API
- 1. {endpoint}/fs/open-api/v1/upload  
     POST  
     body=FormData  
- 2. {endpoint}/fs/open-api/v1/view?filename=xxx  
     GET  
- 3. {endpoint}/fs/open-api/v1/download?filename=xxx  
     GET  
- 4. {endpoint}/fs/open-api/v1/json?key=xxx  
     GET  

     POST  
     body={"xxx":"xxx"}  
- 5. {endpoint}/fs/open-api/v1/list?pageNum=1  
     GET  


### Private API
- 1. {endpoint}/fs/api/v1/upload?filename=xxx  
     POST  
     body=FormData  
- 2. {endpoint}/fs/api/v1/view?filename=xxx  
     GET  
- 3. {endpoint}/fs/api/v1/download?filename=xxx  
     GET  
- 4. {endpoint}/fs/api/v1/json?key=xxx  
     GET  

     POST  
     body={"xxx":"xxx"}  