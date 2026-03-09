function read() {
	let input = "";

	process.stdin.on("data", chunk => {
		input += chunk.toString();
	});

	process.stdin.on("end", () => {
		try {
			const data = JSON.parse(input);

			const html = `
			<div style="max-width: 400px; padding: 16px; border: 1px solid #ddd; border-radius: 8px; font-family: Arial, sans-serif; background-color: #fafafa;">
			  <div style="margin-bottom: 8px;">
			    <strong style="color: #555;">Name:</strong>
			    <span style="margin-left: 6px; color: #000;">${data.name}</span>
			  </div>

			  <div style="margin-bottom: 8px;">
			    <strong style="color: #555;">Email:</strong>
			    <span style="margin-left: 6px; color: #000;">${data.email}</span>
			  </div>

			  <div>
			    <strong style="color: #555;">Message:</strong>
			    <p style="margin: 6px 0 0; padding: 8px; background-color: #fff; border: 1px solid #eee; border-radius: 4px; color: #333;">
			      ${data.message}
			    </p>
			  </div>
			</div>
			`.trim();

			process.stdout.write(html);
		} catch (err) {
			process.stderr.write("Invalid JSON input\n");
			process.exit(1);
		}
	});
}

read();
