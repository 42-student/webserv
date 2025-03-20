# C++ Webserver

A high-performance HTTP server implementation written in C++98 with support for multiple virtual hosts, CGI script execution, and non-blocking I/O.

## Features

- **HTTP Protocol Support**: Full implementation of HTTP methods (GET, POST, DELETE)
- **Virtual Hosting**: Support for multiple server configurations with different hostnames, ports and IPs
- **Non-blocking I/O**: Uses `poll()` for efficient connection handling without blocking threads
- **CGI Support**: Execute dynamic content through CGI scripts
- **Connection Management**: Keep-alive support and automatic timeout handling
- **Custom Error Pages**: Configure custom error responses
- **Configuration File**: Server setup through a simple configuration file

## Documentation

For more detailed documentation, please check the `documentation` folder and open `index.html` in your browser.

## Requirements

- C++ compiler with C++98 support
- Unix-like operating system (Linux, macOS)

## Building the Project

```bash
# Clone the repository
git clone git@github.com:42-student/webserv.git
cd webserv

# Compile the project
make
```

## Usage

```bash
./webserv <config_file>.conf
```

### Example Configuration File

```
server {
    listen      8080;
    server_name example.com;
    host        127.0.0.1;
    root        /var/www/html;
    index       index.html;
    client_max_body_size 1000000;
    error_page 404 404.html;

    location / {
        allow_methods GET POST;
        autoindex off;
    }

    location /cgi-bin {
        root ./;
        index index.py;
        allow_methods GET POST;
        cgi_path /usr/bin/python3;
        cgi_ext .py;
    }

}
```

## Architecture

### Main Components

1. **Parser**: Reads and validates the configuration file
2. **Webserver**: Main server class that manages connections and request handling
3. **Server**: Represents a virtual host configuration
4. **Client**: Manages client connection state
5. **Request**: Parses and represents HTTP requests
6. **Response**: Builds HTTP responses
7. **CGI**: Handles CGI script execution

### Flow

1. The server initializes based on the configuration file
2. `poll()` is used to monitor file descriptors for events
3. When a connection is accepted, a Client object is created
4. Requests are parsed as data is received
5. Once a complete request is available, a response is built
6. For CGI requests, the server handles communication with the CGI script
7. Responses are sent to clients
8. Keep-alive connections are maintained until timeout or explicit close

## CGI Support

The server can execute CGI scripts based on file extensions. This allows for dynamic content generation through languages like Python.

Key features:
- Timeout handling for CGI processes
- Environment variable setup for CGI compatibility
- Bidirectional communication with CGI scripts

## Performance Considerations

- Non-blocking I/O with `poll()` for efficient connection handling
- Connection timeouts to free resources (default: 30 seconds)
- Buffer size limits to prevent resource exhaustion

## Error Handling

The server implements robust error handling:
- HTTP error codes (4xx, 5xx)
- Custom error pages
- CGI execution errors
- Connection errors

## License

This project was developed as an academic assignment at 42Berlin.
It is provided for educational purposes only.
