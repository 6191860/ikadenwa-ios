var server = require('http').createServer();
var io = require('socket.io')(server);
io.on('connection', function(socket){
	console.log("on connection");
	socket.on('event', function(data){
		console.log("on event", data);

		setTimeout(function(){
			socket.emit("echo", data);
		}, 3000);
	});
	socket.on("hello", function(data) {
		console.log("hello", data);

		setTimeout(function(){
			socket.emit("echo", data);
		}, 3000);
	});
	socket.on('disconnect', function(){
		console.log("on disconnect");
	});
});
server.listen(3000);