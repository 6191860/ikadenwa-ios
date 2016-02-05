var socket = require('engine.io-client')('ws://192.168.1.5:8080');
socket.on('open', function(){
  console.log("open");
  socket.on('message', function(data){
  	console.log("message", data);
  });
  socket.on('close', function(){
  	console.log("close");
  });
});
