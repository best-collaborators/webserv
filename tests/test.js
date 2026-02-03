function read()
{
	console.log("JS work!");
	process.stdin.on("data", data => {
		data = data.toString();
		// str = JSON.parse(data);
		// console.log(str.name);
		process.stdout.write(data + "\n")
	})
}

read();