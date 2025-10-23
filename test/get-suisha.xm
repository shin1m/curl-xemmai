system = Module("system"
io = Module("io"
suisha = Module("suisha"
curl = Module("curl"

suisha.main(@ curl.main(@
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
	loop = suisha.loop(
	wait = loop.wait
	loop.wait = @
		timeout = curl.timeout(
		timeout < 0 || loop.timer(@ null, timeout, true
		fds = curl.waitfds(
		fds.each(@(fd) loop.poll(fd[0]
			((fd[1] & curl.POLLIN) == 0 ? 0 : suisha.POLLIN) |
			((fd[1] & curl.POLLPRI) == 0 ? 0 : suisha.POLLPRI) |
			((fd[1] & curl.POLLOUT) == 0 ? 0 : suisha.POLLOUT)
			@
		try
			wait(
		finally
			fds.each(@(fd) loop.unpoll(fd[0
		curl.perform() > 0 || loop.exit(
	loop.run(
