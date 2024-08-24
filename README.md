# SocketSyncFS - File Management System with Server-Client Architecture

This project implements a robust File Management System using a Server-Client architecture. It allows users to upload, download, delete, and list files on different servers, each dedicated to handling specific file types. The servers and the client communicate over TCP/IP, ensuring reliable data transmission and accurate file handling.

The system is built with a focus on modularity, scalability, and efficiency, enabling seamless management of files across multiple servers. The project demonstrates a practical application of networking, file handling, and inter-process communication in a distributed environment.

## Features

- **File Upload**: Upload files to the appropriate server based on their type (e.g., text files, PDF files).
- **File Download**: Download specific files from the server to the client.
- **File Deletion**: Remove files from the server.
- **File Listing**: Display a list of all files available on the server.
- **Tar File Handling**: Download a tarball of specific file types from the server.

## Project Structure

The project is divided into three main components:

1. **Main Server**: Handles file operations for general files and delegates tasks to specialized servers based on file type.
2. **Specialized Servers**: Dedicated servers for handling specific file types, such as `.txt` and `.pdf`.
3. **Client**: The client application allows users to interact with the servers, sending commands and receiving responses.

## Development Overview
- **Designed and implemented** a client-server architecture using socket programming in C, enabling seamless communication between the client and multiple servers.
- **Developed modular server applications** dedicated to handling specific file types, ensuring efficient file management and scalability.
- **Implemented file handling features** including upload, download, and deletion, with careful consideration for error handling and resource management.
- **Created a system for tarball creation and download**, allowing for batch processing and transfer of files.
- **Enhanced user interaction** by developing a command-line interface (CLI) for the client, enabling users to easily interact with the system.
- **Managed directory paths and file operations**, ensuring that the correct files are accessed and modified on the server-side.
- **Documented the entire process**, making the project easy to understand, use, and extend.

### Technologies Used

- **C Programming Language**: Implemented the server-client architecture.
- **Socket Programming**: Enabled communication between the client and servers.
- **File I/O**: Managed file operations on the server.
- **Linux System Calls**: Created, read, wrote, and deleted files and directories.

