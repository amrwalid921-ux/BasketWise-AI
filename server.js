// Tiny Node bridge: browser UI -> C++ engine (no npm packages needed)
const http=require('http'),fs=require('fs'),path=require('path'),{execFile}=require('child_process');
const ENGINE=path.join(__dirname,process.platform==='win32'?'engine.exe':'engine'), PORT=process.env.PORT||3100;
const MSG='Engine not found or failed. Compile it first: g++ -O2 -std=c++17 engine.cpp -o engine';
const engine=(args,res)=>execFile(ENGINE,args,{cwd:__dirname,maxBuffer:1<<24},(e,o)=>{
  res.writeHead(e?500:200,{'Content-Type':'application/json'});res.end(e?JSON.stringify({error:MSG}):o);});
const page=(res,f)=>fs.readFile(path.join(__dirname,f),(e,d)=>{res.writeHead(e?404:200,{'Content-Type':'text/html; charset=utf-8'});res.end(d||'Not found');});
http.createServer((req,res)=>{
  const u=new URL(req.url,'http://x'), q=u.searchParams;
  if(u.pathname==='/api/plan'){
    let b=parseInt(q.get('budget')); if(isNaN(b)) b=150; b=Math.min(2000,Math.max(0,b));
    return engine(['plan',String(b),(q.get('cats')||'-').replace(/[^a-z,-]/g,'')],res);
  }
  if(u.pathname==='/api/bench') return engine(['bench'],res);
  if(u.pathname==='/api/search') return engine(['search',(q.get('q')||'').replace(/[^\w ]/g,'')],res);
  if(u.pathname==='/lab') return page(res,'lab.html');
  page(res,'public/index.html');
}).listen(PORT,()=>console.log('Planner: http://localhost:'+PORT+'   Algorithm Lab: http://localhost:'+PORT+'/lab'));
