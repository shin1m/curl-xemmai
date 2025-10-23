system = Module("system"
io = Module("io"
os = Module("os"
curl = Module("curl"

curl.main(@
	delta = 10
	wait = 0
	i = 0
	get = @(url, action)
		fiber = Fiber(action
		current = Fiber.current(
		fiber(@(buffer, offset, size) current(null)(buffer, offset, size
		request = curl.Request(url
		request.on_done = @(error)
			done = error ? @(b, o, s) throw Throwable(error) : @(b, o, s) 0
			:current = @(x) done
			fiber(done
		request.on_write = @(bytes, size)
			::delta = 10
			::wait = 0
			::i = 0
			for offset = 0; size > 0;; fiber(@(buffer, offset, size)
				n = size < :size ? size : :size
				bytes.copy(:offset, n, buffer, offset
				:offset = :offset + n
				:size = :size - n
				n
		request.follow_location(1
		request.send(
	serial = 0
	test = @(url)
		id = serial
		:serial = serial + 1
		get(url, @(read) try
			reader = io.Reader(read, "utf-8"
			while
				s = reader.read_line(
				s == "" && break
				system.out.write("" + id + ": "
				system.out.write(s
				system.out.flush(
		catch Throwable t
			system.error.write_line("" + id + ": " + t
			t.dump(
	test("https://github.com"
	test("https://google.com"
	for ; i < 8; i = i + 1
		curl.perform() > 0 || break
		wait = wait + delta
		delta = delta * 2
		os.sleep(wait
