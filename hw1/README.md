# WHAT IS THIS:
    It's a secure interactive shell written in language C.  

# HOW TO COMPILE EXECUTABLE SHELL CODE:
   + 1. use 'make'  
   + 2. [optional] ./hw1 1000 1000 (for local test)  

# HOW TO BUILD ENVIRONMENT AND START SHELL SERVICE:
   + 1. place hw1 under your xinetd directory  
   + 2. change the server_args to where your xinetd directory is,   
       and append with '/hw1.o 1000 1000'  
   + 3. restart xinetd service, use'sudo service xinetd restart'  
   + 4. after successfully completing make the executable shell code  
       use 'nc localhost 33445' to start service  

# COMMANDS THAT THIS SHELL SUPPORT:
+    cat {file}:              Display content of {file}.  
+    cd {dir}:                Switch current working directory to {dir}.  
+    chmod {mode} {file/dir}: Change the mode (permission) of a file or directory.  
                             {mode} is an octal number.  
                             Please do not follow symbolc links.  
+    echo {str} [filename]:   Display {str}. If [filename] is given,  
                             open [filename] and append {str} to the file.  
+    exit:                    Leave the shell.  
+    find [dir]:              List files/dirs in the current working directory  
                             or [dir] if it is given.  
                             Minimum outputs contatin file type, size, and name.  
+    help:                    Display help message.  
+    id:                      Show current euid and egid.  
+    mkdir {dir}:             Create a new directory {dir}.  
+    pwd:                     Print the current working directory.  
+    rm {file}:               Remove a file.  
+    rmdir {dir}:             Remove an empty directory.  
+    stat {file/dir}:         Display detailed information of the given file/dir  
+    touch {file}:            Create {file} if it does not exist  
                             or update its access and modification timestamp.  
+    umask {mode}:            Change the umask of the current session.  
